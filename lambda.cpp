#include <iostream>
using namespace std;
class A {
public:
    A() {
        std::cout << "AAAA" << std::endl;
    }

    A(const A& other) {
        std::cout << "Copy constructor called" << std::endl;
        // 在这里进行拷贝构造的逻辑，通常是深拷贝
    }

    A(A&& other) noexcept {
        std::cout << "Move constructor called" << std::endl;
        // 在这里进行移动构造的逻辑，通常是资源的转移
    }

    ~A() {
        std::cout << "BBBBBBBBBBBBBBB" << std::endl;
    }
};
int main() {
    int x = 10;
      A  a;
    std::cout << "************************************************************" << std::endl;
    // 通过值捕获
    auto lambda_by_value = [a]() {
        //std::cout << "Value captured: " << x << std::endl;
    };
    std::cout << "************************************************************" << std::endl;
    auto lambda_by_reference = [&a]() {
        //std::cout << "Value captured: " << x << std::endl;
    };
    
 
    return 0;
}
