#include <stdio.h>
#include "so.h"

#define DLL_PUBLIC __attribute__ ((visibility("default")))
//上面的操作，显示了默认情况下，Linux动态库是导出了所有符号的，另外，也展示了如何导出和使用动态库中的C++类成员。
int main(int argc, char** argv)
{
    test();
    printf("test2: %d\n", test2(3));

    foo f;
    f.a();
    printf("foo::b: %d\n", f.b(2));

    return 0;
}