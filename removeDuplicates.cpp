#include <vector>
#include <iostream>
using namespace std;
class Solution {
public:
    int removeDuplicates(vector<int>& nums) {
        int n = nums.size();
        if (n <= 2) return n; // 如果数组长度小于等于2，则无需删除，直接返回数组长度

        int slow = 2; // 慢指针，表示当前不重复元素的位置
        for (int fast = 2; fast < n; ++fast) {
            // 判断当前元素是否与慢指针指向的前两个元素相同
            if (nums[fast] != nums[slow - 2]) 
            {
                // 如果不同，则将当前元素放到慢指针指向的位置
                nums[slow] = nums[fast];
                ++slow; // 慢指针向后移动一位
            }
        }
        return slow; // 返回最终数组长度
    }
};


int main()
{
    vector<int> nums = {1,1,1,2,2,2,2,3};
      Solution  s;
     int i = s.removeDuplicates(nums);
     cout<<i<<endl;
    for(auto i: nums)
    {
        cout<<i<<",";   
    }
    cout<<endl;
}