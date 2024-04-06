
#include <boost/core/lightweight_test_trait.hpp>

#include "../include/thunks.hpp"

class SimpleObject
{
public:
    int add(int a, int b)
    {
        return a + b;
    }
};

void member_func_test()
{
    SimpleObject dt;
    lunaticpp::thunk<LUNATIC_THUNKS_MAKE_CALL(SimpleObject::add)> thunk(&dt);
    int (*add)(int,int)=thunk.func();

    auto result=add(1,2);
    BOOST_TEST_EQ(3,result);
}

void std_function_test()
{
    using func_type = int(int, int);
    std::function<func_type> f=[](int a, int b){return a+b;};
    lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(std::function<func_type>::operator ())> thunk(&f);
    int (*add)(int,int)=thunk.func();

    auto result=add(1,2);
    BOOST_TEST_EQ(result, 3);
}

int main() {
    member_func_test();
    std_function_test();

    return boost::report_errors();
}
