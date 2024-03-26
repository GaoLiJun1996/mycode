#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

int calculateScore(const string& s) {
    int count[3] = {0}; // count['A'], count['B'], count['C']

    for (char ch : s) {
        count[ch - 'A']++;
    }

    int maxScore = 0;

    // 尝试在两个相邻字符之间插入字符，计算得分
    for (int i = 0; i <= s.size(); ++i)
    {
        for (char c = 'A'; c <= 'C'; ++c) 
        {
            string t = s.substr(0, i) + c + s.substr(i);
            int score = 0;
            bool flag = true;

            while (flag) 
            {
                flag = false;
                for (int j = 0; j < t.size();) 
                {
                    int k = j + 1;
                    while (k < t.size() && t[j] == t[k])
                    {

                        ++k;
                    }
                    if (k - j > 1) {
                        score += k - j;
                        t = t.substr(0, j) + t.substr(k);
                        flag = true;
                    } else {
                        ++j;
                    }
                }
            }

            maxScore = max(maxScore, score);
        }
    }

    return maxScore;
}

int main() {
    int T;
    cin >> T;

    while (T--) {
        string s;
        cin >> s;
        cout << calculateScore(s) << endl;
    }

    return 0;
}
