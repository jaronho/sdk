#include <iostream>
#include <vector>
#include <chrono>
#include "ThreadPool.h"

void fun1(int slp) {
    printf("  hello, fun1 !  %d\n", std::this_thread::get_id());
    if (slp > 0) {
        printf(" ======= fun1 sleep %d  =========  %d\n", slp, std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::milliseconds(slp));
    }
}

struct gfun {
    int operator() (int n) {
        printf("%d  hello, gfun !  %d\n", n, std::this_thread::get_id());
        return 42;
    }
};

class A {
public:
    static std::string Afun(int n, std::string str, char c) {
        std::cout << n << "  hello, Afun !  " << str.c_str() <<"  " << (int)c << "  " << std::this_thread::get_id() << std::endl;
        return str;
    }
};

int main() {
    try {
        ThreadPool pool1(50);
        std::future<void> ff = pool1.add(fun1, 0);
        std::future<int> fg = pool1.add(gfun{}, 0);
        std::future<std::string> gh = pool1.add(A::Afun, 9998, "mult args", 123);
        std::future<std::string> fh = pool1.add([]()->std::string {
            std::cout << "hello, fh !  " << std::this_thread::get_id() << std::endl;
            return "hello,fh ret !";
        });
        std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(900));
        for (int i = 0; i < 50; ++i) {
            pool1.add(fun1, i * 100);
        }
        std::cout << " =======  add all ========= " << std::this_thread::get_id() << " idlsize=" << pool1.idleCount() << std::endl;
        std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        ff.get(); /* 调用.get()获取返回值会等待线程执行完,获取返回值 */
        std::cout << fg.get() << "  " << fh.get().c_str() << "  " << std::this_thread::get_id() << std::endl;
        std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << " =======  fun1,55 ========= " << std::this_thread::get_id() << std::endl;
        pool1.add(fun1,55).get();    /* 调用.get()获取返回值会等待线程执行完 */
        std::cout << "end... " << std::this_thread::get_id() << std::endl;
        //----------------------------------------------------------------------
        ThreadPool pool2(4);
        std::vector<std::future<int>> results;
        for (int i = 0; i < 8; ++i) {
            results.emplace_back(
                pool2.add([i]{
                    std::cout << "hello " << i << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    std::cout << "world " << i << std::endl;
                    return i*i;
                })
            );
        }
        std::cout << " =======  add all2 ========= " << std::this_thread::get_id() << std::endl;
        for (auto && result : results) {
            std::cout << result.get() << ' ';
        }
        std::cout << std::endl;
    } catch (std::exception& e) {
        std::cout << "some unhappy happened...  " << std::this_thread::get_id() << e.what() << std::endl;
    }
    return 0;
}