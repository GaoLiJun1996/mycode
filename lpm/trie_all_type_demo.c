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