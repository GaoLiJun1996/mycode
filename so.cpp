#include <stdio.h>
#include "so.h"


void  test()
{
    printf("test\n");
}


int  test2(int _v)
{
    return _v*_v;
}


void  foo::a()
{
    printf("foo::a()\n");
}


int  foo::b(int _v)
{
    return _v*_v;
}

int  foo::test()
{
    return 0;
}