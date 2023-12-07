// Test Dumplings library
#include "include/dumplings.h"
#include <gsl/gsl>

#include <algorithm>
#include <iostream>

const uintmax_t gN = 10000000;
const int64_t MIN = -1000000; 
const int64_t MAX = 10000000; 


void printArr(auto arr) noexcept 
{ 
    std::ranges::for_each(arr, [](auto& i){ 
            std::cout << i << " "; }); 
} 

int main(int argv, char* args[]) 
{
    auto res = Dumplings::genNRandom(gN, MIN, MAX);

    gsl::Expects(3 > 1);

    printArr(res);
}
