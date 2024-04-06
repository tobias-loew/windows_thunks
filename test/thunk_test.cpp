
#include <boost/core/lightweight_test_trait.hpp>
#include <boost/mp11.hpp>

#include "../include/thunks.hpp"

class SimpleObject
{
public:
    int add(int a, int b)
    {
        return a + b;
    }

    void foo() {
        auto add = m_thunk.func();
        auto result = add(1, 2);
        BOOST_TEST_EQ(3, result);
    }

    lunaticpp::thunk<LUNATIC_THUNKS_MAKE_CALL(SimpleObject::add)> m_thunk{ this };
};

void member_func_test()
{
    SimpleObject dt;
    lunaticpp::thunk<LUNATIC_THUNKS_MAKE_CALL(SimpleObject::add)> thunk(&dt);
    auto add = thunk.func();

    auto result=add(1,2);
    BOOST_TEST_EQ(3,result);

    dt.foo();
}

void std_function_test()
{
    using func_type = int(int, int);
    std::function<func_type> f=[](int a, int b){return a+b;};
    lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(std::function<func_type>::operator ())> thunk(&f);
    auto add = thunk.func();

    auto result=add(1,2);
    BOOST_TEST_EQ(result, 3);
}

namespace struct_test {

    struct hook {
        static constexpr int n_value = 42;
        static hook* p_hook;

        hook() {
            // store this pointer for later check
            p_hook = this;
        }

        LONG PvectoredExceptionHandler(_EXCEPTION_POINTERS* /*ExceptionInfo*/)
        {
            return 0;
        }

        lunaticpp::thunk<LUNATIC_THUNKS_MAKE_CALL(PvectoredExceptionHandler)> m_thunk_PvectoredExceptionHandler{ this };

        void foo() {
            auto   VEH_Handle = AddVectoredExceptionHandler(true, (PVECTORED_EXCEPTION_HANDLER)m_thunk_PvectoredExceptionHandler.func());
            (void)(VEH_Handle);
        }

        void  HookMe() {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
        }

        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe)> m_thunk_HookMe{ this };

        void  HookMe1(int n1) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe1)> m_thunk_HookMe1{ this };

        void  HookMe2(int n1, int n2) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
            BOOST_TEST_EQ(n2, 2);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe2)> m_thunk_HookMe2{ this };

        void  HookMe3(int n1, int n2, int n3) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
            BOOST_TEST_EQ(n2, 2);
            BOOST_TEST_EQ(n3, 3);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe3)> m_thunk_HookMe3{ this };

        void  HookMe4(int n1, int n2, int n3, int n4) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
            BOOST_TEST_EQ(n2, 2);
            BOOST_TEST_EQ(n3, 3);
            BOOST_TEST_EQ(n4, 4);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe4)> m_thunk_HookMe4{ this };

        void  HookMe5(int n1, int n2, int n3, int n4, int n5) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
            BOOST_TEST_EQ(n2, 2);
            BOOST_TEST_EQ(n3, 3);
            BOOST_TEST_EQ(n4, 4);
            BOOST_TEST_EQ(n5, 5);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe5)> m_thunk_HookMe5{ this };

        void  HookMe6(int n1, int n2, int n3, int n4, int n5, int n6) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
            BOOST_TEST_EQ(n2, 2);
            BOOST_TEST_EQ(n3, 3);
            BOOST_TEST_EQ(n4, 4);
            BOOST_TEST_EQ(n5, 5);
            BOOST_TEST_EQ(n6, 6);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe6)> m_thunk_HookMe6{ this };

        int n = n_value;
    };
    hook* hook::p_hook{};


    void arity_test()
    {
        hook c{};
        c.foo();

        c.m_thunk_HookMe.func()();
        c.m_thunk_HookMe1.func()(1);
        c.m_thunk_HookMe2.func()(1, 2);
        c.m_thunk_HookMe3.func()(1, 2, 3);
        c.m_thunk_HookMe4.func()(1, 2, 3, 4);
        c.m_thunk_HookMe5.func()(1, 2, 3, 4, 5);
        c.m_thunk_HookMe6.func()(1, 2, 3, 4, 5, 6);
    }
}


namespace struct_template_test {

    template<
        typename T1
        , typename T2
        , typename T3
        , typename T4
        , typename T5
        , typename T6
    >
    struct hook {
        static constexpr int n_value = 42;
        static hook* p_hook;

        hook() {
            // store this pointer for later check
            p_hook = this;
        }

        LONG PvectoredExceptionHandler(_EXCEPTION_POINTERS* /*ExceptionInfo*/)
        {
            return 0;
        }

        lunaticpp::thunk<LUNATIC_THUNKS_MAKE_CALL(PvectoredExceptionHandler)> m_thunk_PvectoredExceptionHandler{ this };

        void foo() {
            auto   VEH_Handle = AddVectoredExceptionHandler(true, (PVECTORED_EXCEPTION_HANDLER)m_thunk_PvectoredExceptionHandler.func());
            (void)(VEH_Handle);
        }

        void  HookMe() {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
        }

        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe)> m_thunk_HookMe{ this };

        void  HookMe1(T1 n1) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe1)> m_thunk_HookMe1{ this };

        void  HookMe2(T1 n1, T2 n2) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
            BOOST_TEST_EQ(n2, 2);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe2)> m_thunk_HookMe2{ this };

        void  HookMe3(T1 n1, T2 n2, T3 n3) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
            BOOST_TEST_EQ(n2, 2);
            BOOST_TEST_EQ(n3, 3);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe3)> m_thunk_HookMe3{ this };

        void  HookMe4(T1 n1, T2 n2, T3 n3, T4 n4) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
            BOOST_TEST_EQ(n2, 2);
            BOOST_TEST_EQ(n3, 3);
            BOOST_TEST_EQ(n4, 4);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe4)> m_thunk_HookMe4{ this };

        void  HookMe5(T1 n1, T2 n2, T3 n3, T4 n4, T5 n5) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
            BOOST_TEST_EQ(n2, 2);
            BOOST_TEST_EQ(n3, 3);
            BOOST_TEST_EQ(n4, 4);
            BOOST_TEST_EQ(n5, 5);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe5)> m_thunk_HookMe5{ this };

        void  HookMe6(T1 n1, T2 n2, T3 n3, T4 n4, T5 n5, T6 n6) {
            BOOST_TEST_EQ(this, p_hook);
            BOOST_TEST_EQ(n, n_value);
            BOOST_TEST_EQ(n1, 1);
            BOOST_TEST_EQ(n2, 2);
            BOOST_TEST_EQ(n3, 3);
            BOOST_TEST_EQ(n4, 4);
            BOOST_TEST_EQ(n5, 5);
            BOOST_TEST_EQ(n6, 6);
        }
        lunaticpp::thunk< LUNATIC_THUNKS_MAKE_CALL(HookMe6)> m_thunk_HookMe6{ this };

        int n = n_value;
    };

    template<
        typename T1
        , typename T2
        , typename T3
        , typename T4
        , typename T5
        , typename T6
    >
    hook<T1,T2,T3,T4,T5,T6>* hook<T1, T2, T3, T4, T5, T6>::p_hook;


    using base_types = boost::mp11::mp_list<int, double>;
    using types_product = boost::mp11::mp_product<boost::mp11::mp_list, base_types, base_types, base_types, base_types, base_types, base_types>;

    template<typename types>
    void single_arity_test(types const&)
    {
        hook<
            boost::mp11::mp_at_c<types, 0>
            , boost::mp11::mp_at_c<types, 1>
            , boost::mp11::mp_at_c<types, 2>
            , boost::mp11::mp_at_c<types, 3>
            , boost::mp11::mp_at_c<types, 4>
            , boost::mp11::mp_at_c<types, 5>            
            >c{};
        c.foo();

        c.m_thunk_HookMe.func()();
        c.m_thunk_HookMe1.func()(1);
        c.m_thunk_HookMe2.func()(1, 2);
        c.m_thunk_HookMe3.func()(1, 2, 3);
        c.m_thunk_HookMe4.func()(1, 2, 3, 4);
        c.m_thunk_HookMe5.func()(1, 2, 3, 4, 5);
        c.m_thunk_HookMe6.func()(1, 2, 3, 4, 5, 6);
    }

    void arity_test()
    {
        boost::mp11::mp_for_each<types_product>([](auto&& p) { single_arity_test(p); });
    }
}


int main() {
    member_func_test();
    std_function_test();
    struct_test::arity_test();
    struct_template_test::arity_test();

    return boost::report_errors();
}
