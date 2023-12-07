#include <gtest/gtest.h>

#include <iostream>
#include "../../include/dumplings.h"
#include <thread>
#include <chrono>


namespace {

int foo1(int i)
{
    std::cout << "execute foo1...\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return --i;
}

int foo2(int& i)
{
    std::cout << "execute foo2...\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return --i;
}
int foo3(int&& i)
{
    std::cout << "execute foo3...\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return --i;
}

class Thread_Pool_Test : public testing::Test {
public:
    // Thread_Pool_Test() : pool(10) {}

protected:

    void SetUp() override
    {
        a = 12;
        b = 20;
    }
    void TearDown() override
    {
        pool.stop();
    }

    dumplings::thread_pool<> pool{10};
    int a;
    int b;
};

// Tests
TEST_F(Thread_Pool_Test, Post_ValuePara)
{
    auto future = pool.post(foo1, a);
    EXPECT_EQ(11, future.get());
    
    future = pool.post(std::move(foo1), a);
    EXPECT_EQ(11, future.get());

    future = pool.post(foo1, std::move(a));
    EXPECT_EQ(11, future.get());

    future = pool.post(std::move(foo1), std::move(a));
    EXPECT_EQ(11, future.get());
}

}


int main(void)
{
    return RUN_ALL_TESTS();
}
