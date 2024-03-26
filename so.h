#ifndef __SO_H__
#define __SO_H__
#define DLL_PUBLIC __attribute__ ((visibility("default")))
#define DLL_PRIVATE __attribute__ ((visibility("hidden")))
//nm -D test.so 查看导出符号
//Much lower chance of symbol collision. The old woe of two libraries internally using the same symbol for different things is finally behind us with this patch. Hallelujah!
// g++ -shared -o test.so -fPIC -fvisibility=hidden so.cpp

#ifdef __cplusplus
extern "C" {
#endif

void  test();
int   test2(int _v);


class foo
{
public:
    void a();
    int  b(int _v);
    DLL_PUBLIC int test();
};


#ifdef __cplusplus
}
#endif


#endif