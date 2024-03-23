#define BOOST_TEST_MODULE Thunk Test
#include <boost/test/included/unit_test.hpp>
#include "../../include/thunks.hpp"

class SimpleObject
{
public:
    int add(int a, int b)
    {
        return a + b;
    }
};

BOOST_AUTO_TEST_CASE(dummy_test)
{
    BOOST_TEST(1==1);
}

BOOST_AUTO_TEST_CASE(member_func_test)
{
    SimpleObject dt;
    lunaticpp::thunk<&SimpleObject::add> thunk(&dt);
    int (*add)(int,int)=thunk.func();

    auto result=add(1,2);
    BOOST_CHECK_EQUAL(3,result);
}

BOOST_AUTO_TEST_CASE(std_function_test)
{
    std::function<int(int, int)> f=[](int a, int b){return a+b;};
    lunaticpp::thunk<&std::function<int(int, int)>::operator ()> thunk(&f);
    int (*add)(int,int)=thunk.func();

    auto result=add(1,2);
    BOOST_CHECK_EQUAL(result, 3);
}
