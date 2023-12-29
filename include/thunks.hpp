#pragma once


#if defined(_M_IX86)
#elif defined(_M_X64)

#else
#error thunks require IX86 or X64 to be defined
#endif 

#ifdef BOOST_FT_CONFIG_HPP_INCLUDED
#ifndef BOOST_FT_AUTODETECT_CALLING_CONVENTIONS
#error boost/function_types/config/config.hpp already included without BOOST_FT_AUTODETECT_CALLING_CONVENTIONS being defined
#endif 
#endif 

#ifndef BOOST_FT_AUTODETECT_CALLING_CONVENTIONS
#define BOOST_FT_AUTODETECT_CALLING_CONVENTIONS
#endif

#include <boost/function_types/config/config.hpp>
#include <boost/type_traits/is_float.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/property_tags.hpp>
#include <boost/function_types/components.hpp>
#include <boost/function_types/function_pointer.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/is_member_function_pointer.hpp>
#include <boost/function_types/member_function_pointer.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/property_tags.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/erase.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/max_element.hpp>
#include <boost/mpl/less.hpp>
#include <boost/mpl/sizeof.hpp>
#include <boost/array.hpp>

#include <windows.h>


// ensures that structs/clases are packed dense
#pragma pack(push, 1)


namespace lunaticpp {

        namespace aux {

#if defined(_M_X64)


#pragma code_seg(push, rtext, ".text")
            __declspec(selectany)
                __declspec(allocate(".text"))
                extern const BYTE _trampoline_for_static_thunk[25]{

                // sub         rsp, 18h 
                //
                // does two jobs:
                // - make room on the stack
                // - ensure that the stack is 16-byte aligned BEFORE executing CALL

                0x48, 0x83, 0xEC, 0x18,          // sub         rsp, 18h
                0x41, 0xFF, 0xD2,                // call        r10
                0x48, 0x83, 0xC4, 0x18,          // add         rsp, 18h
                0x4D, 0x8B, 0x54, 0x24, 0xF8,    // mov         r10, qword ptr[r12 - 8]
                0x4C, 0x89, 0x14, 0x24,          // mov         qword ptr[rsp], r10
                0x4D, 0x8B, 0x24, 0x24,          // mov         r12, qword ptr[r12]
                0xC3,                            // ret
            };

#pragma code_seg(pop, rtext)

#endif




            struct memfunc_with_adjustor {
                void* func;
                DWORD adjustor;	// even in x64 it's only a DWORD !!!
#if defined(_M_X64)
                DWORD padding;	// to make the structure 16 bytes long
#endif
            };

            enum {
                sizeof_DWORD_PTR = sizeof(DWORD_PTR),
                sizeof_memfunc_with_adjustor = sizeof(memfunc_with_adjustor)
            };


            template <int size>
            struct memfunc_size;

            template <>
            struct memfunc_size< sizeof_DWORD_PTR > {
                enum { size = sizeof_DWORD_PTR };
            };

#if defined(_M_IX86)
            static_assert(sizeof(memfunc_with_adjustor) == (sizeof(void*) + sizeof(DWORD)));
#elif defined(_M_X64)
            static_assert(sizeof(memfunc_with_adjustor) == (sizeof(void*) + sizeof(DWORD) + sizeof(DWORD)));
#else
#error not supported
#endif

            template <>
            struct memfunc_size< sizeof_memfunc_with_adjustor > {
                enum { size = sizeof_memfunc_with_adjustor };
            };

        };

        // this is the new version, using template parametrized constructors
        template <auto _call>
        struct thunk
        {
        public:
            using call_type = decltype(_call);

        private:

#if defined(_M_IX86)
            struct code_thunk_t {
                BYTE	m_mov;			// mov ecx, %pThis
                DWORD	m_this; 		//
                BYTE	m_jmp;			// jmp func
                DWORD	m_relproc;		// relative jmp
            };
#elif defined(_M_X64)

            // used in case of up to 3 args
            struct code_thunk_t {

                // example
                // 4D 8B C8         mov         r9,r8 
                // 4C 8B C2         mov         r8,rdx 
                // 48 8B D1         mov         rdx,rcx
                // 48 B9 08 07 06 05 04 03 02 01 mov         rcx,102030405060708h 
                boost::array<BYTE, 11>	m_mov_registers_up_mov_this_to_rcx;
                DWORD_PTR	m_this;

                // example
                // 48 B8 02 03 04 05 06 00 00 00 mov         rax,605040302h 
                BYTE	m_mov_absproc_to_rax[2];
                DWORD_PTR	m_absproc;  // absolute address of orig func

                // example
                // FF E0            jmp         rax  
                BYTE	m_jmp[2];
            };


            // example
            // a call with seven args may look like that
            // 48 83 EC 48          sub         rsp,48h  
            // 48 8B 44 24 78       mov         rax,qword ptr [i6]  
            // 48 89 44 24 30       mov         qword ptr [rsp+30h],rax  
            // 48 8B 44 24 70       mov         rax,qword ptr [i5]  
            // 48 89 44 24 28       mov         qword ptr [rsp+28h],rax  
            // 4C 89 4C 24 20       mov         qword ptr [rsp+20h],r9  
            // 4D 8B C8             mov         r9,r8  
            // 4C 8B C2             mov         r8,rdx  
            // 48 8B D1             mov         rdx,rcx  
            // 48 B9 08 07 06 05 04 03 02 01 mov         rcx,102030405060708h 
            // 48 B8 02 03 04 05 06 00 00 00 mov         rax,605040302h 
            // FF D0                call        rax  
            //
            // 48 83 C4 48          add         rsp,48h  
            // C3                   ret  

            struct code_thunk_complex_move_stack_up {
                // example
                // 48 8B 84 24 80 00 00 00 mov         rax,qword ptr [i6]  
                // 48 89 44 24 30       mov         qword ptr [rsp+30h],rax  
                BYTE	m_mov_to_rax[4];
                BYTE    m_mov_offset;
                BYTE	m_mov_to_stack[4];
                BYTE    m_stack_offset;
            };


            // moves 4th arg to stack
            // integer/pointer version
            template< bool _is_float >
            struct code_thunk_complex_rest_mov_arg4 {
                // example
                // 4C 89 4C 24 20       mov         qword ptr [rsp+20h],r9          // integer/pointer
                boost::array<BYTE, 5>	m_mov_arg4_to_stack;

                void init()
                {
                    static constexpr boost::array<BYTE, 5> mov_arg4_to_stack = {
                        0x4C, 0x89, 0x4C, 0x24, 0x08   // mov         qword ptr [rsp+08h],r9  
                    };
                    m_mov_arg4_to_stack = mov_arg4_to_stack;
                }
            };

            // moves 4th arg to stack
            // float version
            template<>
            struct code_thunk_complex_rest_mov_arg4<true> {
                // example
                // F2 0F 11 5C 24 20    movsd       mmword ptr [rsp+20h],xmm3       // float
                boost::array<BYTE, 6>	m_mov_arg4_to_stack;

                void init()
                {
                    static constexpr boost::array<BYTE, 6> mov_arg4_to_stack = {
                        0xF2, 0x0F, 0x11, 0x5C, 0x24, 0x08   // movsd       mmword ptr [rsp+08h],xmm3
                    };
                    m_mov_arg4_to_stack = mov_arg4_to_stack;
                }
            };

            template< class func_arguments_type >
            struct code_thunk_complex_rest {
                // example
                // 4C 89 4C 24 20       mov         qword ptr [rsp+20h],r9  
                code_thunk_complex_rest_mov_arg4<boost::is_float< typename boost::mpl::at_c<func_arguments_type, 3>::type >::type::value > m_mov_arg4_to_stack;

                // example
                // 4D 8B C8             mov         r9,r8  
                // 4C 8B C2             mov         r8,rdx  
                // 48 8B D1             mov         rdx,rcx  
                // 48 B9 08 07 06 05 04 03 02 01 mov         rcx,102030405060708h 
                boost::array<BYTE, 11>	m_mov_reg_to_reg_up_mov_this_to_rcx;
                DWORD_PTR	m_this;


                // example
                //        49 BA 90 78 56 34 12 00 00 00 mov         r10, 1234567890h
                BYTE	m_mov_absproc_to_r10[2];
                DWORD_PTR	m_absproc;  // absolute address of orig func



                boost::array<BYTE, 21>	m_mov_r12_to_rsp_x_store_rsp;


                // example
                // 48 B8 02 03 04 05 06 00 00 00 mov         rax,605040302h 

                BYTE	m_mov_static_thunk_to_rax[2];
                DWORD_PTR	m_static_thunk;  // absolute address of orig func


                // example
                // FF E0                jmp        rax  
                BYTE	m_jmp_rax[2];

            };




#endif


            BYTE* m_code_thunk_bytes;
        public:
            inline static constexpr call_type m_call{ _call };


            // template-helpers for function type (de-)composition

            static_assert(boost::function_types::is_member_function_pointer<call_type>::value);


            typedef typename boost::function_types::components< call_type >::type func_components_type;

            typedef typename boost::function_types::result_type< call_type >::type func_result_type;

            using member_type =
                typename boost::mpl::at_c<
                typename boost::function_types::parameter_types< call_type >::type,
                0
                >::type;

            using member_ptr_type = std::remove_reference_t<member_type>*;

            // remove "this" arg from parameters
            typedef typename boost::mpl::pop_front<
                typename boost::function_types::parameter_types< call_type >::type
            >::type func_arguments_type;


            typedef typename boost::function_types::function_pointer<
                typename boost::mpl::push_front< func_arguments_type, func_result_type >::type
#if defined(_M_IX86)
                , boost::function_types::stdcall_cc      // calling convention for Win-API callback functions
#endif
            >::type callback_function_type;

#if defined(_M_IX86)
            // ensure it's not a variadic function
            static_assert(boost::function_types::is_member_function_pointer<call_type, boost::function_types::thiscall_cc>::value);
#elif defined(_M_X64)
            // ensure it's not a variadic function
            static_assert(boost::function_types::is_member_function_pointer<call_type, boost::function_types::non_variadic>::value);
#endif


            thunk(member_ptr_type pThis)
            {
#ifdef _M_IX86

                static_assert(sizeof(call_type) == aux::memfunc_size< sizeof(call_type) >::size);

                m_code_thunk_bytes = reinterpret_cast<BYTE*>(VirtualAlloc(NULL, sizeof(code_thunk_t), MEM_COMMIT, PAGE_READWRITE));
                {
                    auto code_thunk = reinterpret_cast<code_thunk_t*>(m_code_thunk_bytes);
                    union { DWORD func; call_type call; } addr;
                    addr.call = m_call;
                    code_thunk->m_mov = 0xB9;
                    code_thunk->m_this = ((DWORD)pThis) + get_adjustor<sizeof(call_type)>(m_call);
                    code_thunk->m_jmp = 0xE9;
                    code_thunk->m_relproc = addr.func - (DWORD)(code_thunk + 1);
                }
                DWORD dwOldProtect;
                VirtualProtect(m_code_thunk_bytes, sizeof(code_thunk_t), PAGE_EXECUTE, &dwOldProtect);

                ::FlushInstructionCache(GetCurrentProcess(), m_code_thunk_bytes, sizeof(code_thunk_t));

#elif defined(_M_X64)

                create_thunk<boost::mpl::size<func_arguments_type>::value>((DWORD_PTR)pThis);

#else
#error not supported
#endif
        }



#if defined(_M_X64)

            // information about x64-function calls
            // cf. http://www.codemachine.com/article_x64deepdive.html


        void adjust_arg0_to_arg2_for_floats(boost::array<BYTE, 11>& instr)
        {
            // replace int-register movements by xmm-register-movements for float-types
            // the next if-statements could be templated, but if unnecessary they will get optimized away anyway

            if constexpr (boost::mpl::size<func_arguments_type>::value >= 3) {
                if constexpr (boost::is_float< typename boost::mpl::at_c<func_arguments_type, 2>::type >::type::value)
                {
                    static constexpr BYTE mov_reg_to_reg[3] = {
                        0x0F, 0x28, 0xDA                // mov         xmm3,xmm2
                    };
                    memcpy(instr.data(), mov_reg_to_reg, sizeof(mov_reg_to_reg));
                }
            }
            if constexpr (boost::mpl::size<func_arguments_type>::value >= 2) {
                if constexpr (boost::is_float< typename boost::mpl::at_c<func_arguments_type, 1>::type >::type::value)
                {
                    static constexpr BYTE mov_reg_to_reg[3] = {
                        0x0F, 0x28, 0xD1                // mov         xmm2,xmm1
                    };
                    memcpy(instr.data() + sizeof(mov_reg_to_reg), mov_reg_to_reg, sizeof(mov_reg_to_reg));
                }
            }
            if constexpr (boost::mpl::size<func_arguments_type>::value >= 1) {
                if constexpr (boost::is_float< typename boost::mpl::at_c<func_arguments_type, 0>::type >::type::value)
                {
                    static constexpr BYTE mov_reg_to_reg[3] = {
                        0x0F, 0x28, 0xC8                // mov         xmm1,xmm0
                    };
                    memcpy(instr.data() + 2 * sizeof(mov_reg_to_reg), mov_reg_to_reg, sizeof(mov_reg_to_reg));
                }
            }
        }


            static constexpr auto get_static_thunk() { return aux::_trampoline_for_static_thunk; }


            template< size_t _args >
            void create_thunk(DWORD_PTR pThis)
            {
                static_assert(_args >= 4);
                static_assert(_args <= 15); // if args > 15  code_thunk_stack->m_stack_offset will get negative (>= 0x80)
                static_assert(sizeof(call_type) == aux::memfunc_size< sizeof(call_type) >::size);



                static constexpr size_t alloc_size_stack_moves = (_args - 4) * sizeof(code_thunk_complex_move_stack_up);
                static constexpr size_t alloc_size_rest = sizeof(code_thunk_complex_rest<func_arguments_type>);

                static constexpr size_t alloc_size =
                    alloc_size_stack_moves +
                    alloc_size_rest;

                //
                //
                //
                // Online Assembler
                // http://shell-storm.org/online/Online-Assembler-and-Disassembler/
                //
                //
                //
                //

                m_code_thunk_bytes = reinterpret_cast<BYTE*>(VirtualAlloc(NULL, alloc_size, MEM_COMMIT, PAGE_READWRITE));
                {
                    union { DWORD_PTR func; call_type call; } addr;
                    addr.call = m_call;

                    if constexpr (_args > 4) {

                        // move stack args down by 10h - yes, we're moving down into the shadow space!
                        for (size_t i = 0; i < _args - 4; ++i)
                        {

                            //    struct code_thunk_complex_move_stack_up{
                            //// 48 8B 84 24 80 00 00 00 mov         rax,qword ptr [i6]  
                            //// 48 89 44 24 30       mov         qword ptr [rsp+30h],rax  
                                    //BYTE	m_mov_to_rax[4];
                              //      BYTE   m_mov_offset;
                                    //BYTE	m_mov_to_stack[4];	
                              //      BYTE    m_stack_offset;
                            //    };

                            const code_thunk_complex_move_stack_up instr = {
                                0x48, 0x8B, 0x44, 0x24, (BYTE)(0x28 + 0x8 * static_cast<int>(i)),
                                0x48, 0x89, 0x44, 0x24, (BYTE)(0x10 + 0x8 * static_cast<int>(i))
                            };

                            *reinterpret_cast<code_thunk_complex_move_stack_up*>(m_code_thunk_bytes
                                + i * sizeof(code_thunk_complex_move_stack_up))
                                = instr;
                        }
                    }


                    static constexpr boost::array<BYTE, 11> mov_reg_to_reg_up_mov_this_to_rcx = {
                        0x4D, 0x8B, 0xC8,               // mov         r9,r8  
                        0x4C, 0x8B, 0xC2,               // mov         r8,rdx  
                        0x48, 0x8B, 0xD1,               // mov         rdx,rcx  
                        0x48, 0xB9                      // mov         rcx,[$adr]
                    };

                    code_thunk_complex_rest<func_arguments_type>* code_thunk_rest =
                        reinterpret_cast<code_thunk_complex_rest<func_arguments_type>*>(m_code_thunk_bytes + alloc_size - alloc_size_rest);

                    code_thunk_rest->m_mov_arg4_to_stack.init();

                    code_thunk_rest->m_mov_reg_to_reg_up_mov_this_to_rcx = mov_reg_to_reg_up_mov_this_to_rcx;
                    code_thunk_rest->m_this = ((DWORD_PTR)pThis) + get_adjustor<sizeof(call_type)>(m_call);

                    adjust_arg0_to_arg2_for_floats(code_thunk_rest->m_mov_reg_to_reg_up_mov_this_to_rcx);


                    code_thunk_rest->m_mov_absproc_to_r10[0] = 0x49;
                    code_thunk_rest->m_mov_absproc_to_r10[1] = 0xBA;
                    code_thunk_rest->m_absproc = addr.func;


                    static constexpr boost::array<BYTE, 21> mov_r12_to_rsp_x_store_rsp = {
                        // save r12 (non-volatile) to use for stack address on return
            //            4C 89 64 24 30       mov         qword ptr[rsp + 30h], r12
                            0x4C, 0x89, 0x64, 0x24, (0x8 * _args),

                            // mov r12, rsp
                            0x49, 0x89, 0xe4,

                            // add          r12, 30h
                            0x49, 0x83, 0xc4, (0x8 * _args),

                            // mov rax, qword ptr[rsp]
                            0x48, 0x8b, 0x04, 0x24,

                            // mov qword ptr[rsp + 28h], rax
                            0x48, 0x89, 0x44, 0x24, (0x8 * (_args - 1))
                    };
                    code_thunk_rest->m_mov_r12_to_rsp_x_store_rsp = mov_r12_to_rsp_x_store_rsp;


                    code_thunk_rest->m_mov_static_thunk_to_rax[0] = 0x48;
                    code_thunk_rest->m_mov_static_thunk_to_rax[1] = 0xB8;
                    code_thunk_rest->m_static_thunk = reinterpret_cast<DWORD_PTR>(get_static_thunk()); // addr.func;

                    code_thunk_rest->m_jmp_rax[0] = 0xFF;
                    code_thunk_rest->m_jmp_rax[1] = 0xE0;
                }

                DWORD dwOldProtect;
                VirtualProtect(m_code_thunk_bytes, alloc_size, PAGE_EXECUTE, &dwOldProtect);

                ::FlushInstructionCache(GetCurrentProcess(), m_code_thunk_bytes, alloc_size);
            }

            template<>
            void create_thunk<0>(DWORD_PTR pThis)
            {
                create_thunk_lt_4_args(pThis);
            }

            template<>
            void create_thunk<1>(DWORD_PTR pThis)
            {
                create_thunk_lt_4_args(pThis);
            }

            template<>
            void create_thunk<2>(DWORD_PTR pThis)
            {
                create_thunk_lt_4_args(pThis);
            }

            template<>
            void create_thunk<3>(DWORD_PTR pThis)
            {
                create_thunk_lt_4_args(pThis);
            }

            void create_thunk_lt_4_args(DWORD_PTR pThis)
            {
                static_assert(sizeof(call_type) == aux::memfunc_size< sizeof(call_type) >::size);

                m_code_thunk_bytes = reinterpret_cast<BYTE*>(VirtualAlloc(NULL, sizeof(code_thunk_t), MEM_COMMIT, PAGE_READWRITE));
                {
                    auto code_thunk = reinterpret_cast<code_thunk_t*>(m_code_thunk_bytes);
                    union { DWORD_PTR func; call_type call; } addr;
                    addr.call = m_call;

                    static constexpr boost::array<BYTE, 11> mov_registers_up_mov_this_to_rcx = {
                        0x4D, 0x8B, 0xC8,
                        0x4C, 0x8B, 0xC2,
                        0x48, 0x8B, 0xD1,
                        0x48, 0xB9
                    };

                    code_thunk->m_mov_registers_up_mov_this_to_rcx = mov_registers_up_mov_this_to_rcx;
                    code_thunk->m_this = ((DWORD_PTR)pThis) + get_adjustor<sizeof(call_type)>(m_call);

                    adjust_arg0_to_arg2_for_floats(code_thunk->m_mov_registers_up_mov_this_to_rcx);

                    code_thunk->m_mov_absproc_to_rax[0] = 0x48;
                    code_thunk->m_mov_absproc_to_rax[1] = 0xB8;
                    code_thunk->m_absproc = addr.func;

                    code_thunk->m_jmp[0] = 0xFF;
                    code_thunk->m_jmp[1] = 0xE0;
                }
                DWORD dwOldProtect;
                VirtualProtect(m_code_thunk_bytes, sizeof(code_thunk_t), PAGE_EXECUTE, &dwOldProtect);

                ::FlushInstructionCache(GetCurrentProcess(), m_code_thunk_bytes, sizeof(code_thunk_t));
            }


#endif





            template <int size>
            size_t get_adjustor(call_type call) const;

            template <>
            size_t get_adjustor< aux::sizeof_DWORD_PTR >(call_type /*call*/) const { return 0; }

            template <>
            size_t get_adjustor< aux::sizeof_memfunc_with_adjustor >(call_type call) const
            {
                return ((aux::memfunc_with_adjustor*)((void*)&call))->adjustor;
            }

            // WIN32 multiple virtual inheritance not supported
                //template <>
                //	size_t get_adjustor< 12 >( call_type& call ) const 
                //{ 
                //	return ((aux::memfunc_with_adjustor*)((void*)&call) )->adjustor;
                //}


            virtual ~thunk()
            {
                VirtualFree(m_code_thunk_bytes, 0, MEM_RELEASE);
            }

            callback_function_type func() const
            {
                return reinterpret_cast<callback_function_type>(m_code_thunk_bytes);
            }
    };

}	//	namespace lunaticpp

#pragma pack(pop) 

