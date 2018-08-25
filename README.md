# Chainiterator #
[![Build Status](https://travis-ci.org/erikzenker/chainiterator.svg?branch=master)](https://travis-ci.org/erikzenker/chainiterator) [![codecov](https://codecov.io/gh/erikzenker/chainiterator/branch/master/graph/badge.svg)](https://codecov.io/gh/erikzenker/chainiterator)
=

**Chainiterator** is a C++ class which concats multiple C++ containers to a single iterable object.

## Usage ##
```c++
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

    return 0;
}

``` 

Output:
```text
1
2
3
4
5
6
7
8
```

## Build Example ##
```bash
mkdir build; cd build
cmake ..
cmake --build chainiterator_example
./example/chainiterator_example
```

## Build Tests ##
```bash
mkdir build; cd build
cmake ..
cmake --build chainiterator_unit_test
./test/unit/chainiterator_unit_test

```

## Dependencies ##
* boost 1.67
* c++14

## Licence ##
MIT

## Author ##
Written by Erik Zenker (erikzenker (at) hotmail.com)
