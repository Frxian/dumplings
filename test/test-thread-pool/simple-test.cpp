#include "../../include/dumplings.h"
#include <iostream>

#define DELAY(x) std::this_thread::sleep_for(std::chrono::seconds(x));

const uint32_t N = 1000;

int foo1(int i)
{
    // DELAY(2);
    std::cout << "Executing foo1...\n";
    return --i;
}

int foo2(int i, int j)
{
    std::cout << "Executing foo2...\n";
    return --i + j--;
}

int main(void)
{
    dumplings::thread_pool pool(10u);
    // dumplings::thread_pool pool(100u);
    // dumplings::thread_pool pool(1u);

    for (int i = 0; i < N; ++i)
    {
        pool.post(foo1, 12);
    }
    for (int i = 0; i < N; ++i)
    {
        pool.post(foo2, 12, 20);
    }

    pool.wait();

    pool.stop();

    return 0;
}
