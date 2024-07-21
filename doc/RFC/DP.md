# DP

```c
#include <limits.h>

// Function to find the minimum number of coins needed to make up a given amount
int coinChange(int* coins, int coinsSize, int amount) {
    // Create an array to store the minimum coins needed for each amount
    int dp[amount + 1];
    
    // Initialize the dp array with a large value (amount + 1)
    for (int i = 0; i <= amount; i++) {
        dp[i] = amount + 1;
    }
    
    // Base case: 0 amount needs 0 coins
    dp[0] = 0;
    
    // Iterate over each amount from 1 to the given amount
    for (int i = 1; i <= amount; i++) {
        // Iterate over each coin in the coins array
        for (int j = 0; j < coinsSize; j++) {
            // If the current coin can be used to make the amount i
            if (coins[j] <= i) {
                // Update the dp array with the minimum coins needed
                dp[i] = dp[i] < 1 + dp[i - coins[j]] ? dp[i] : 1 + dp[i - coins[j]];
            }
        }
    }
    
    // If dp[amount] is still amount + 1, it means it's not possible to make the amount
    return dp[amount] > amount ? -1 : dp[amount];
}

```

### 494 目标和

`dp[i]` 表示能够得到和为 `i` 的不同表达方式的数量。

```c
#include <stdlib.h>

int findTargetSumWays(int* nums, int numsSize, int target) {
    // Calculate the total sum of all elements in nums
    int sum = 0;
    for (int i = 0; i < numsSize; i++) {
        sum += nums[i];
    }
    
    // If the absolute value of target is greater than the total sum, there are no possible ways
    if (abs(target) > sum) {
        return 0;
    }
    
    // If (sum + target) is odd, it's not possible to split the array into two parts with equal sums
    if ((sum + target) % 2 != 0) {
        return 0;
    }
    
    // Calculate the subset sum we need to find
    int subsetSum = (sum + target) / 2;
    
    // Create a dp array to store the number of ways to reach each sum
    int dp[subsetSum + 1];
    for (int i = 0; i <= subsetSum; i++) {
        dp[i] = 0;
    }
    dp[0] = 1;  // There's one way to reach sum 0 (by choosing no elements)
    
    // Fill the dp array
    for (int i = 0; i < numsSize; i++) {
        for (int j = subsetSum; j >= nums[i]; j--) {
            dp[j] += dp[j - nums[i]];
        }
    }
    
    return dp[subsetSum];
}

```

