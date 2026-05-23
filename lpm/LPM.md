## LPM

lpm 即 最长前缀匹配， 从最长掩码开始匹配，找到最精确的路由

### 前缀树 (Trie) 法（最经典）

把 IP 地址的每一位当作树的分支，从根节点开始逐位匹配：

- 0 走左子树，1 走右子树

- 每个节点存储该位置对应的下一跳

- 走到叶子节点或没有子节点时，返回路径上最深的有效下一跳

优点

- 查询复杂度**O(32)**（IPv4 固定 32 位），与路由表大小无关
- 插入 / 删除操作简单，只需修改对应路径上的节点

致命问题

- **内存爆炸**：最坏情况下需要 232 个节点，完全不可行
- **Cache 极度不友好**：节点分散在内存中，每次查询要 32 次随机访存，Cache 命中率几乎为 0
- 实际性能比线性查找还差，几乎没有生产环境直接用原生 Trie



### 路径压缩前缀树 Radix/Patricia Trie

#### 标准定义

**Radix Trie（基数树）** 是一种对普通前缀树进行**垂直路径压缩**的数据结构，它合并所有只有单个子节点的连续节点，只在存在分支的位置创建节点。

**Patricia Trie（Practical Algorithm To Retrieve Information Coded In Alphanumeric）** 是 Radix Trie 的**二进制（基数为 2）特例**，也是工业界最常用的实现，专门针对最长前缀匹配（LPM）场景优化。

#### 一句话核心本质

**Patricia Trie = 只在分叉点创建节点，跳过所有没有分支的直线路径**

普通 Trie 是 "一步一位"，Patricia 是 "一步跳到下一个分叉点"。

#### 与普通二叉 Trie 的本质区别

| 特性         | 普通二叉 Trie                       | Patricia Trie                        |
| ------------ | ----------------------------------- | ------------------------------------ |
| 节点创建时机 | 每一位都创建一个节点                | 只有存在分支时才创建节点             |
| 树高         | 固定等于最长键长度（IPv4 为 32 层） | 等于分叉点数量（平均仅为 10 层左右） |
| 节点数量     | O (总位数)                          | O (键的数量)                         |
| 内存效率     | 极低（大量空节点）                  | 极高（几乎无空节点）                 |

#### 核心思想：为什么路径压缩能生效？

普通前缀树存在一个致命问题：**绝大多数路径都是没有分支的直线**。

例如在路由表中，`10.0.0.0/8` 这个前缀下可能只有 `10.1.0.0/16` 和 `10.2.0.0/16` 两个子前缀，中间的第 9-16 位完全没有分支。

Patricia Trie 做的事情非常简单：

1. 把这 8 个连续的、没有分支的节点**全部合并成一个节点**
2. 在这个节点里记录 "跳过了 8 位"
3. 查找时直接跳过这 8 位，直接判断第 9 位的分支

#### 数据结构

```c
typedef struct patricia_node {
    uint8_t   bit_pos;     // 【核心】下一个要比较的位的位置（0-31）
    uint32_t  prefix;      // 本节点对应的前缀值
    uint8_t   prefix_len;  // 本节点前缀的长度
    uint32_t  nexthop;     // 有效路由的下一跳（0表示无效）
    struct patricia_node *left;  // 0分支
    struct patricia_node *right; // 1分支
} pat_node_t;
```



#### 代码实现

```c
pat_node_t* pat_create_node(uint8_t skip, uint32_t key) {
    pat_node_t* n = (pat_node_t*)malloc(sizeof(*n));
    n->skip = skip;
    n->key = key;
    n->prefix_len = 0;
    n->nexthop = 0;
    n->child0 = n->child1 = NULL;
    return n;
}

void pat_insert(pat_node_t** root, uint32_t ip, uint8_t len, uint32_t hop) {
    if (!*root) {
        *root = pat_create_node(len, ip);
        (*root)->nexthop = hop;
        (*root)->prefix_len = len;
        return;
    }
    pat_node_t* cur = *root;
    uint32_t mask = 0xFFFFFFFF << (32 - len);
    uint32_t key = ip & mask;
    while (1) {
        if (cur->prefix_len > 0) break;
        uint8_t bit = (key >> (31 - cur->skip)) & 1;
        if (bit == 0) {
            if (!cur->child0) cur->child0 = pat_create_node(len, key);
            cur = cur->child0;
        } else {
            if (!cur->child1) cur->child1 = pat_create_node(len, key);
            cur = cur->child1;
        }
    }
    cur->nexthop = hop;
}

uint32_t pat_lookup(pat_node_t* root, uint32_t ip) {
    pat_node_t* cur = root;
    uint32_t best = 0;
    while (cur) {
        if (cur->nexthop != 0) best = cur->nexthop;
        uint8_t bit = (ip >> (31 - cur->skip)) & 1;
        cur = (bit == 0) ? cur->child0 : cur->child1;
    }
    return best;
}

```



### Level Compressed Trie，层级压缩前缀树

核心是**将前缀树中连续、密集的多层节点水平压缩为一个 “超级节点”**，一次处理 k 个二进制位，降低树的高度，提升查询性能。

#### 与路径压缩的本质区别

| 压缩类型                      | 压缩方向 | 核心操作                           | 解决的问题                     |
| ----------------------------- | -------- | ---------------------------------- | ------------------------------ |
| **路径压缩（Patricia Trie）** | 垂直方向 | 合并**只有一个子节点的单分支节点** | 减少无效空节点，缩短路径长度   |
| **层级压缩（LC-Trie）**       | 水平方向 | 合并**连续多层的满 / 密集节点**    | 一次处理多位，直接降低树的高度 |

#### 核心原理

##### 关键参数 k

LC-Trie 的核心是**一次处理 k 个二进制位**，k 是可配置的常数（最常用 k=4，对应 IPv4 地址的一个十六进制位）。

- 每个节点拥有 **2k 个子节点**（k=4 时为 16 个）
- 子节点通过**数组索引直接访问**，无需指针遍历
- 32 位 IPv4 地址的树高 = 32 /k（k=4 时树高仅为 8 层）

##### 数据结构

```c
struct lc_trie_node {
    uint8_t  k;              // 一次处理的位数（固定为4）
    uint8_t  prefix_len;     // 该节点对应的路由前缀长度
    uint32_t nexthop;        // 有效路由的下一跳（0表示无效）
    struct lc_trie_node *children[16];  // 16个子节点数组
};
```

##### 代码实现

```c
struct lc_trie_node {
    uint8_t  k;
    uint8_t  prefix_len;
    uint32_t nexthop;
    struct lc_trie_node *children[16];
};

struct lc_trie_node* create_node(uint8_t k) {
    struct lc_trie_node* node = (struct lc_trie_node*)malloc(sizeof(*node));
    node->k = k;
    node->prefix_len = 0;
    node->nexthop = 0;
    for (int i = 0; i < (1 << k); i++)
        node->children[i] = NULL;
    return node;
}

void lc_insert(struct lc_trie_node* root, uint32_t ip, uint8_t prefix_len, uint32_t nexthop) {
    struct lc_trie_node* cur = root;
    int steps = (prefix_len + 3) / 4;
    for (int step = 0; step < steps; step++) {
        uint8_t idx = (ip >> (28 - step * 4)) & 0xF;
        if (!cur->children[idx])
            cur->children[idx] = create_node(4);
        cur = cur->children[idx];
    }
    cur->prefix_len = prefix_len;
    cur->nexthop = nexthop;
}

uint32_t lc_lookup(struct lc_trie_node* root, uint32_t ip) {
    struct lc_trie_node* cur = root;
    uint32_t best_hop = 0;
    for (int step = 0; step < 8 && cur; step++) {
        if (cur->nexthop != 0)
            best_hop = cur->nexthop;
        uint8_t idx = (ip >> (28 - step * 4)) & 0xF;
        cur = cur->children[idx];
    }
    return best_hop;
}
```

### 一个demo

```c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

// ==================== 工具函数 ====================
static inline uint64_t get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// ============================================================================
// 1. 基础 LC-Trie（层级压缩 4bit）
// ============================================================================
struct lc_trie_node {
    uint8_t  k;
    uint8_t  prefix_len;
    uint32_t nexthop;
    struct lc_trie_node *children[16];
};

struct lc_trie_node* create_node(uint8_t k) {
    struct lc_trie_node* node = (struct lc_trie_node*)malloc(sizeof(*node));
    node->k = k;
    node->prefix_len = 0;
    node->nexthop = 0;
    for (int i = 0; i < (1 << k); i++)
        node->children[i] = NULL;
    return node;
}

void lc_insert(struct lc_trie_node* root, uint32_t ip, uint8_t prefix_len, uint32_t nexthop) {
    struct lc_trie_node* cur = root;
    int steps = (prefix_len + 3) / 4;
    for (int step = 0; step < steps; step++) {
        uint8_t idx = (ip >> (28 - step * 4)) & 0xF;
        if (!cur->children[idx])
            cur->children[idx] = create_node(4);
        cur = cur->children[idx];
    }
    cur->prefix_len = prefix_len;
    cur->nexthop = nexthop;
}

uint32_t lc_lookup(struct lc_trie_node* root, uint32_t ip) {
    struct lc_trie_node* cur = root;
    uint32_t best_hop = 0;
    for (int step = 0; step < 8 && cur; step++) {
        if (cur->nexthop != 0)
            best_hop = cur->nexthop;
        uint8_t idx = (ip >> (28 - step * 4)) & 0xF;
        cur = cur->children[idx];
    }
    return best_hop;
}

// ============================================================================
// 2. 基础二叉 Trie
// ============================================================================
struct bt_trie_node {
    uint32_t nexthop;
    struct bt_trie_node *children[2];
};

struct bt_trie_node* create_bt_node() {
    struct bt_trie_node* node = (struct bt_trie_node*)malloc(sizeof(*node));
    node->nexthop = 0;
    node->children[0] = node->children[1] = NULL;
    return node;
}

void bt_insert(struct bt_trie_node* root, uint32_t ip, uint8_t prefix_len, uint32_t nexthop) {
    struct bt_trie_node* cur = root;
    for (int pos = 0; pos < prefix_len; pos++) {
        uint8_t bit = (ip >> (31 - pos)) & 1;
        if (!cur->children[bit])
            cur->children[bit] = create_bt_node();
        cur = cur->children[bit];
    }
    cur->nexthop = nexthop;
}

uint32_t bt_lookup(struct bt_trie_node* root, uint32_t ip) {
    struct bt_trie_node* cur = root;
    uint32_t best_hop = 0;
    for (int pos = 0; pos < 32 && cur; pos++) {
        if (cur->nexthop != 0)
            best_hop = cur->nexthop;
        uint8_t bit = (ip >> (31 - pos)) & 1;
        cur = cur->children[bit];
    }
    return best_hop;
}

// ============================================================================
// 3. Patricia Trie（路径压缩）
// ============================================================================
typedef struct pat_node {
    uint8_t   skip;        // 跳过的位数
    uint8_t   prefix_len;
    uint32_t  nexthop;
    uint32_t  key;         // 存储前缀
    struct pat_node *child0, *child1;
} pat_node_t;

pat_node_t* pat_create_node(uint8_t skip, uint32_t key) {
    pat_node_t* n = (pat_node_t*)malloc(sizeof(*n));
    n->skip = skip;
    n->key = key;
    n->prefix_len = 0;
    n->nexthop = 0;
    n->child0 = n->child1 = NULL;
    return n;
}

void pat_insert(pat_node_t** root, uint32_t ip, uint8_t len, uint32_t hop) {
    if (!*root) {
        *root = pat_create_node(len, ip);
        (*root)->nexthop = hop;
        (*root)->prefix_len = len;
        return;
    }
    pat_node_t* cur = *root;
    uint32_t mask = 0xFFFFFFFF << (32 - len);
    uint32_t key = ip & mask;
    while (1) {
        if (cur->prefix_len > 0) break;
        uint8_t bit = (key >> (31 - cur->skip)) & 1;
        if (bit == 0) {
            if (!cur->child0) cur->child0 = pat_create_node(len, key);
            cur = cur->child0;
        } else {
            if (!cur->child1) cur->child1 = pat_create_node(len, key);
            cur = cur->child1;
        }
    }
    cur->nexthop = hop;
}

uint32_t pat_lookup(pat_node_t* root, uint32_t ip) {
    pat_node_t* cur = root;
    uint32_t best = 0;
    while (cur) {
        if (cur->nexthop != 0) best = cur->nexthop;
        uint8_t bit = (ip >> (31 - cur->skip)) & 1;
        cur = (bit == 0) ? cur->child0 : cur->child1;
    }
    return best;
}

// ============================================================================
// 4. LC + Patricia 混合树（层级压缩 + 路径压缩）
// ============================================================================
typedef struct lc_pat_node {
    uint8_t   k;           // 一次处理 4 位
    uint8_t   skip;        // 路径压缩：跳过多少组
    uint8_t   prefix_len;
    uint32_t  nexthop;
    struct lc_pat_node *children[16];
} lc_pat_node_t;

lc_pat_node_t* lc_pat_create(uint8_t k) {
    lc_pat_node_t* n = (lc_pat_node_t*)malloc(sizeof(*n));
    n->k = k;
    n->skip = 0;
    n->prefix_len = 0;
    n->nexthop = 0;
    for (int i = 0; i < 16; i++) n->children[i] = NULL;
    return n;
}

void lc_pat_insert(lc_pat_node_t* root, uint32_t ip, uint8_t len, uint32_t hop) {
    lc_pat_node_t* cur = root;
    int rem = len;
    int pos = 0;
    while (rem > 0) {
        uint8_t idx = (ip >> (28 - pos)) & 0xF;
        if (!cur->children[idx])
            cur->children[idx] = lc_pat_create(4);
        cur = cur->children[idx];
        pos += 4;
        rem -= 4;
    }
    cur->prefix_len = len;
    cur->nexthop = hop;
}

uint32_t lc_pat_lookup(lc_pat_node_t* root, uint32_t ip) {
    lc_pat_node_t* cur = root;
    uint32_t best = 0;
    int pos = 0;
    while (cur && pos < 32) {
        if (cur->nexthop != 0)
            best = cur->nexthop;
        uint8_t idx = (ip >> (28 - pos)) & 0xF;
        cur = cur->children[idx];
        pos += 4;
    }
    return best;
}

// ============================================================================
// 性能测试
// ============================================================================
void performance_test() {
    const int INSERT_NUM = 100000;
    const int LOOKUP_NUM = 2000000;

    printf("========== LC-Trie vs Binary Trie vs Patricia vs LC+Patricia ==========\n");
    printf("路由条目：%d\n查询次数：%d\n", INSERT_NUM, LOOKUP_NUM);

    uint32_t *ips = (uint32_t*)malloc(INSERT_NUM * 4);
    uint8_t  *lens = (uint8_t*)malloc(INSERT_NUM);
    uint32_t *hops = (uint32_t*)malloc(INSERT_NUM * 4);
    uint32_t *look_ips = (uint32_t*)malloc(LOOKUP_NUM * 4);

    srand(time(NULL));
    for (int i = 0; i < INSERT_NUM; i++) {
        ips[i] = ((uint32_t)rand() << 16) | rand();
        lens[i] = 8 + 4 * (rand() % 7);
        hops[i] = i + 1;
    }
    for (int i = 0; i < LOOKUP_NUM; i++)
        look_ips[i] = ((uint32_t)rand() << 16) | rand();

    // -------- LC-Trie --------
    uint64_t t1 = get_time_ms();
    struct lc_trie_node *rlc = create_node(4);
    for (int i = 0; i < INSERT_NUM; i++)
        lc_insert(rlc, ips[i], lens[i], hops[i]);
    uint64_t t2 = get_time_ms();

    // -------- Binary Trie --------
    uint64_t t3 = get_time_ms();
    struct bt_trie_node *rbt = create_bt_node();
    for (int i = 0; i < INSERT_NUM; i++)
        bt_insert(rbt, ips[i], lens[i], hops[i]);
    uint64_t t4 = get_time_ms();

    // -------- Patricia --------
    uint64_t ta = get_time_ms();
    pat_node_t *rpat = NULL;
    for (int i = 0; i < INSERT_NUM; i++)
        pat_insert(&rpat, ips[i], lens[i], hops[i]);
    uint64_t tb = get_time_ms();

    // -------- LC+Patricia --------
    uint64_t t5 = get_time_ms();
    lc_pat_node_t *rlcp = lc_pat_create(4);
    for (int i = 0; i < INSERT_NUM; i++)
        lc_pat_insert(rlcp, ips[i], lens[i], hops[i]);
    uint64_t t6 = get_time_ms();

    uint32_t dummy = 0;

    // -------- 查询 LC --------
    uint64_t t7 = get_time_ms();
    for (int i = 0; i < LOOKUP_NUM; i++)
        dummy ^= lc_lookup(rlc, look_ips[i]);
    uint64_t t8 = get_time_ms();

    // -------- 查询 BT --------
    uint64_t t9 = get_time_ms();
    for (int i = 0; i < LOOKUP_NUM; i++)
        dummy ^= bt_lookup(rbt, look_ips[i]);
    uint64_t t10 = get_time_ms();

    // -------- 查询 PAT --------
    uint64_t t11 = get_time_ms();
    for (int i = 0; i < LOOKUP_NUM; i++)
        dummy ^= pat_lookup(rpat, look_ips[i]);
    uint64_t t12 = get_time_ms();

    // -------- 查询 LC+PAT --------
    uint64_t t13 = get_time_ms();
    for (int i = 0; i < LOOKUP_NUM; i++)
        dummy ^= lc_pat_lookup(rlcp, look_ips[i]);
    uint64_t t14 = get_time_ms();

    printf("Binary Trie:  插入%4lu ms | 查询%4lu ms\n", t4-t3, t10-t9);
    printf("LC-Trie:      插入%4lu ms | 查询%4lu ms\n", t2-t1, t8-t7);
    printf("Patricia:     插入%4lu ms | 查询%4lu ms\n", tb-ta, t12-t11);
    printf("LC+Patricia:  插入%4lu ms | 查询%4lu ms\n", t6-t5, t14-t13);

    double q1 = LOOKUP_NUM / ((t10-t9)/1000.0) / 10000;
    double q2 = LOOKUP_NUM / ((t8-t7)/1000.0) / 10000;
    double q3 = LOOKUP_NUM / ((t12-t11)/1000.0) / 10000;
    double q4 = LOOKUP_NUM / ((t14-t13)/1000.0) / 10000;

    printf("BT: %.1f 万 | LC: %.1f 万 | PAT: %.1f 万 | LC+PAT: %.1f 万 QPS\n", q1, q2, q3, q4);
    printf("=======================================================================\n");
    (void)dummy;
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    struct lc_trie_node* root = create_node(4);
    lc_insert(root, 0x0A000000, 8,  100);
    lc_insert(root, 0x0A010000, 16, 200);
    lc_insert(root, 0x0A010101, 32, 300);

    printf("ip1: %u\n", lc_lookup(root, 0x0A010101));
    printf("ip2: %u\n", lc_lookup(root, 0x0A010202));
    printf("ip3: %u\n", lc_lookup(root, 0x0A050505));
    printf("\n");

    performance_test();
    return 0;
}
```

demo运行结果

![image-20260523152607338](https://yian-1324200595.cos.ap-guangzhou.myqcloud.com/imgimage-20260523152607338.png)