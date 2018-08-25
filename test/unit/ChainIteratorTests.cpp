#include <ChainIterator.h>

#include <boost/test/unit_test.hpp>

#include <iostream>

struct ChainIteratorTests {

    using Value = int;

    std::vector<Value> a{0,1,2,3};
    std::vector<Value> b{4,5,6,7};
    std::array<Value, 4> c{8,9,10,11};
};

BOOST_FIXTURE_TEST_CASE(should_construct_chain_iterator, ChainIteratorTests)
{
    makeChainIterator(a, b);
}

BOOST_FIXTURE_TEST_CASE(should_construct_chain_iterator_with_different_types, ChainIteratorTests)
{
    makeChainIterator(a, b, c);
}

BOOST_FIXTURE_TEST_CASE(should_dereference_chain_iterator, ChainIteratorTests)
{
    auto chainIterator = makeChainIterator(a, b);
    BOOST_REQUIRE_EQUAL(0, *chainIterator);
}

BOOST_FIXTURE_TEST_CASE(end_and_begin_should_not_be_equal, ChainIteratorTests)
{
    auto chainIterator = makeChainIterator(a, b);

    BOOST_REQUIRE(chainIterator.begin() != chainIterator.end());
}

BOOST_FIXTURE_TEST_CASE(should_increment_chain_iterator, ChainIteratorTests)
{
    auto chainIterator = makeChainIterator(a, b);

    for(int i = 0; i < 8; ++i){
        BOOST_REQUIRE_EQUAL(i, *chainIterator);
        chainIterator++;
    }
}

BOOST_FIXTURE_TEST_CASE(should_increment_chain_iterator_in_for_each_loop, ChainIteratorTests)
{
    auto chainIterator = makeChainIterator(a, b);

    int i = 0;
    for(auto elem : chainIterator){
        BOOST_REQUIRE_EQUAL(i, elem);
        i++;
    }
}

BOOST_FIXTURE_TEST_CASE(should_increment_chain_iterator_in_for_each_loop_with_diferent_types, ChainIteratorTests)
{
    auto chainIterator = makeChainIterator(a, b, c);

    int i = 0;
    for(auto elem : chainIterator){
        BOOST_REQUIRE_EQUAL(i, elem);
        i++;
    }
}
