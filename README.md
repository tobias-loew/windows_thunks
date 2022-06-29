# windows_thunks
Thunks for Windows (x86 and x64) - or how to pass non-static member-functions as Windows-Callbacks

## features
- pass non-static member functions as WIN32 CALLBACK functions
- full-generic: works for __any__ non-variadic non-static member function
- type-safe: non-static member function MUST match signature of CALLBACK (apart from hidden object-pointer) 
- works for 32-bit (x86 / IX86) and 64-bit (x64 / X64 / AMD64) builds
- no restrictions on thunk-deletion (see below)

## why do I need a thunk (and what is it anyway?)
When writing C++ code for WIN32 thunks are usually used to solve the following problem:

"I want to call a WIN32-API function that makes a call-back a user defined function. I want that call-back to be a non-stztic member function on some object, but the API only allows static functions."

To work around this, some of those API-calls allow for an additional pointer-wide value (here `lParam`)
```
BOOL EnumThreadWindows(
  [in] DWORD       dwThreadId,
  [in] WNDENUMPROC lpfn,
  [in] LPARAM      lParam
);
```
that is passed on to the call-back
```
BOOL CALLBACK EnumThreadWndProc(
  _In_ HWND   hwnd,
  _In_ LPARAM lParam
);
```
which alows to pass through a pointer to the object, but in a raw and type-unsafe manner.

But there are other famous examples, like
```
UINT_PTR SetTimer(
  [in, optional] HWND      hWnd,
  [in]           UINT_PTR  nIDEvent,
  [in]           UINT      uElapse,
  [in, optional] TIMERPROC lpTimerFunc
);
```
that do not allow such a simple approach.

In a C++-ideal world, where the WIN32-API had a C++ interface and would use `std::function<...>` for call-backs, then we would simply pass

```std::bind_front(&MyClass::foo, object)```

and this library would not even exist.

So, someone clever came up with the following idea: when we cannot pass a non-static member function, then let's pass a pointer to a piece of code that adds the object-pointer to the call and the calls the member-function. This piece of code obviously has be generated dynamically (as it holds the address to a runtime object) and is usually called a _thunk_.

## make it work type-safe and generic
Let's look at other solutions for this problem available 
- they are either not generic (https://www.codeproject.com/Articles/1121696/Cplusplus-WinAPI-Wrapper-Object-using-thunks-x-and)
- or they are not type-safe (https://www.codeproject.com/Articles/348387/Another-new-thunk-copy-from-ATL)

The aim of this libray is to offer both together.

Type safety and genericity come together by using a bit of template meta-programming: With the help of `function_traits` from Boost.TypeTraits we can
- genericity: analyse the number and type of arguments and generate the appropriate assembler code
- type-safety: compute the "static-call"-type by removing the member-function "this" part from the function-type (and adding `stdcall` for x86)

## x86 / x64 : generate assembler code on the fly
### x86
The assembler code for a "this-injecting" thunk on x86 is well known and quite short:
```
  mov ecx, %pThis
  jmp non-static-member-function-address
```
(just to make it clear: the object-address "%pThis" and the function-address are both __hard coded__ into the thunk !)
i.e. te object-pointer is moved to register `ecx` and then the code jumps to the non-static member-function. When the non-static member-function returns, it will directly return to the __caller of the thunk__ (it won't return to the thunk, since the thunk did not "call" the member-function but only "jump" there).

That's all for x86. 

### x64
For x64 it's whole different story - and a lot more complicated. So, I'll only sketch the problems briefly. The x64 calling-convention on windows roughly looks like this:

- every argument uses exactely 8 bytes on the stack or a register (larger types are passed by address)
- the first 4 args are passed in registers (`rcx`/`xmm0`, `rdx`/`xmm1`, `r8`/`xmm2`, `r9`/`xmm3`;  with floating point types passed in the `xmm...` registers)
- further arguments are passed on the stack
- the __caller__ is responsible to allocate additional 32-bytes on the stack as _shadow space_ (just below the additional arguments)
- there is no special handling for the `this`-pointer; it's handled as (hidden) first argument (so always transfered in `rcx`)

So, up to three-argument calls it's quite easy again: just shifting some registers (taking care of floating point arguments!), pushing the object's address to `rcx` and jumping to the member-function (just like for x86).

But starting with 4 arguments it get's realy nasty: simply moving up the arguments along the stack __is not possible__, since the place right above the original stack __is already used by the caller__ and we would mess up its local variables. That's bad bad bad!

So, we have to create a new frame (move the stack-pointer) with enough place and move all the pieces (stack, registers, `this`-pointer) into place and then jump to the member-function. Done.

... but wait ...

The member-function gets called but when it ends, strange things happen, e.g. my bunny Luna starts coding

![luna-bunny-coding](/pictures/luna_coding.png)


So, what happened? We moved 



If you want to learn more about x64 calling conventions, here are some resources to start with:
- https://en.wikipedia.org/wiki/X86_calling_conventions
- https://docs.microsoft.com/en-us/cpp/build/x64-software-conventions
- https://devblogs.microsoft.com/oldnewthing/20160623-00/?p=93735

## thunk-deletion
