#include "include/dumplings.h"
#include <iostream>

// print all elements
void print(auto& container)
{
    while(!container.empty())
    {
        std::cout << container.front() << std::endl;
        container.pop();
    }
}

// print all elements with front_pop
void print2(auto& container)
{
    while(!container.empty())
    {
        std::cout << container.front_pop() << std::endl;
    }
}
int main(void)
{
    dumplings::thread_safe::queue<int> queue;

    queue.push(12);
    queue.push(1313);
    queue.push(12);
    queue.push(12);
    queue.push(12);
    queue.push(12);
    queue.push(12);
    queue.push(12);
    queue.push(12);
    queue.push(12);
    queue.push(12);
    queue.push(12);
    queue.push(12);

    // print(queue);
    std::cout << "size: " << queue.size() << std::endl;

    print2(queue);

    return 0;
}
