// STL
#include <iostream>
#include <vector>
#include <array>
#include <list>

#include <ChainIterator.h>

int main()
{
    std::vector<int> a{ 1, 2 };
    std::vector<int> b{ 3, 4 };
    std::array<int, 2> c{ 5, 6 };
    std::list<int> d{ 7, 8 };

    auto chainIterator = makeChainIterator(a,b,c,d);

    for(auto& element : chainIterator){
        std::cout << element << std::endl;
    }

    /* Output:
     *
     * 1
     * 2
     * 3
     * 4
     * 5
     * 6
     * 7
     * 8
     */

    return 0;
}
