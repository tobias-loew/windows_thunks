#pragma once


#include <thunks.hpp>


// ensures that structs/clases are packed dense
#pragma pack(push, 1)


namespace lunaticpp {

        // for hooking into the system
    template <auto _call, int _id_hook>
    struct thunk_hook : public thunk< _call >
    {
    private:
        HHOOK          m_hook;

    public:
        using base_type = thunk<  _call >;
        using call_type = typename base_type::call_type;

        thunk_hook(typename base_type::member_ptr_type pThis) :
            base_type{ pThis },
            m_hook(NULL)
        { }

        virtual ~thunk_hook()
        {
            unhook();
        }

        HHOOK get_hook() const
        {
            return m_hook;
        }

        void set_hook(HHOOK hook)
        {
            m_hook = hook;
        }

        BOOL hook()
        {
            if (!m_hook)
            {
                m_hook = ::SetWindowsHookEx(
                    _id_hook,
                    this->func(),
                    NULL,
                    ::GetCurrentThreadId()
                );
            }

            return m_hook != NULL;
        }

        BOOL unhook()
        {
            if (m_hook)
            {
                HHOOK hook = m_hook;
                m_hook = NULL;
                return ::UnhookWindowsHookEx(hook);
            } else
            {
                return TRUE;
            }
        }
    };


#if 0

    // unused and not working !!!

    // for hooking into the system, automatically call CallNextHookEx
    template < class T, class _call_type, _call_type _call, int _id_hook >
    struct thunk_hook_auto : public	thunk_hook<
        typename boost::function_types::member_function_pointer<
        typename boost::mpl::vector<
        LRESULT,                                                // result
        thunk_hook_auto< T, _call_type, _call, _id_hook >,      // class
        int, WPARAM, LPARAM                                     // args
        >::type
        >::type,
        _call,
        _id_hook >
    {
    public:
        using base_type =
            thunk_hook<
            typename boost::function_types::member_function_pointer<
            typename boost::mpl::vector<
            LRESULT,                                                // result
            thunk_hook_auto< T, _call_type, _call, _id_hook >,      // class
            int, WPARAM, LPARAM                                     // args
            >::type
            >::type,
            _call,
            _id_hook >;

        using call_type = typename base_type::call_type;

        T* m_pThis;

        static_assert(boost::is_same< void, typename boost::function_types::result_type< call_type >::type >::value);

    public:
        LRESULT hook_proc(
            int nCode,
            WPARAM wParam,
            LPARAM lParam
        )
        {
            if (nCode < 0)
            {
                return ::CallNextHookEx(this->get_hook(), nCode, wParam, lParam);
            } else
            {
                HHOOK hook = this->get_hook(); // store it, as "this" may be destroyed on the following call

                (m_pThis->m_call)(nCode, wParam, lParam);

                return ::CallNextHookEx(hook, nCode, wParam, lParam);
            }
        }


        typedef typename boost::function_types::member_function_pointer<
            typename boost::mpl::vector<
            LRESULT,                                                // result
            thunk_hook_auto,                                        // class
            int, WPARAM, LPARAM                                     // args
            >::type
        >::type hook_proc_type;



        thunk_hook_auto(T* pThis) :
            thunk_hook< hook_proc_type, _call, _id_hook >(&thunk_hook_auto::hook_proc, this),
            m_pThis(pThis)
        { }

        };
#endif

}	//	namespace lunaticpp

#pragma pack(pop) 

