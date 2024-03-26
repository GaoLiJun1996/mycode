#include <string>
#include <iostream>
using namespace std;

void get_next(const string& T, int next[]) {
    int i = 0, j = -1;
    next[0] = -1; // 注意数组下标从 0 开始，初始值设为 -1
    while (i < T.length())
    {
        if (j == -1 || T[i] == T[j])
        {
            ++i;
            ++j;
            next[i] = j; // 若T[i] == T[j]，则 next[i+1] = j+1
        }
        else
        {
            j = next[j]; // 否则令 j = next[j]，循环继续
        }
    }
}

int main(int argc, char** argv)
{

    if(argc!=2)
    {
        cout<<"usage error"<<endl;
        return -1;
    }
    string temp = string(argv[1]);
     cout<<"str:"<<temp<<endl;
    int a[temp.length()]; // next 数组的大小为 T.length()
    get_next(temp, a);
    cout<<"next[]:";
    for (int i = 0; i <= temp.length()-1; ++i) {
        cout << a[i] << ",";
    }
    cout << endl;
    return 0;
}
