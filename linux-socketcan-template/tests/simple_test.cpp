#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test of tests"

#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(TestTest)
{
    BOOST_CHECK(true);
};
