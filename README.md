# windows_thunks
Thunks for Windows (x86 and x64) - or how to pass non-static member functions as Windows-Callbacks

## features
- pass non-static member functions as WIN32 CALLBACK functions
- full-generic: works for __any__ non-variadic non-static member function
- type-safe: non-static member function MUST match signature of CALLBACK (apart from hidden object-pointer) 
- works for 32-bit (x86 / IX86) and 64-bit (x64 / X64 / AMD64) builds
- no restrictions on thunk-deletion (see below)

## requirements
- C++17 conforming compiler
- Boost.TypeTraits (or a complete Boost installation)

## what is a thunk and why do I need it?
When writing C++ code for WIN32 thunks are usually used to solve the following problem:

"I want to call a WIN32-API function that makes a callback to a user defined function. I want that callback to be a non-static member function on some object. But the API only allows static functions."

To work around this problem, some of those API-calls allow for an additional pointer-wide value (here `lParam`)
```
BOOL EnumThreadWindows(
  [in] DWORD       dwThreadId,
  [in] WNDENUMPROC lpfn,
  [in] LPARAM      lParam
);
```
that is passed on to the callback
```
BOOL CALLBACK EnumThreadWndProc(
  _In_ HWND   hwnd,
  _In_ LPARAM lParam
);
```
which allows to pass through a pointer to the object, but in a raw and type-unsafe manner.

But here is another well known examples
```
UINT_PTR SetTimer(
  [in, optional] HWND      hWnd,
  [in]           UINT_PTR  nIDEvent,
  [in]           UINT      uElapse,
  [in, optional] TIMERPROC lpTimerFunc
);
```
that does not allow such a simple approach.

In a C++-ideal world, where the WIN32-API had a C++ interface and would use `std::function<...>` for callbacks, then we would simply pass

```std::bind_front(&MyClass::foo, object)```

and this library would not even exist.

So, someone clever came up with the following idea: when we cannot pass a non-static member function, then let's pass a pointer to a piece of code that adds the object-pointer to the call and then proceeds to the member function. This piece of code obviously has to be generated dynamically (as it holds the address of a runtime object) and is usually called a _thunk_.

## make it work type-safe and generic
Let's look at other available solutions for this problem 
- they are either not generic (https://www.codeproject.com/Articles/1121696/Cplusplus-WinAPI-Wrapper-Object-using-thunks-x-and, only works for `WndProc`)
- or they are not type-safe (https://www.codeproject.com/Articles/348387/Another-new-thunk-copy-from-ATL, thunk address is returned as `void*`)

The aim of this libray is to offer both __type-safety and genericity__ together.

Type safety and genericity achieved by using template meta-programming: with the help of `function_traits` from Boost.TypeTraits we get
- type-safety: compute the "static-call"-type by removing the member function "this" part from the function-type (and adding `stdcall` for x86)
- genericity: analyze the number and types of arguments and generate the appropriate assembler code

## x86 / x64 : generate assembler code on the fly
### x86
The assembler code for a "this-injecting" thunk on x86 is well known and quite short:
```
  mov ecx, %pThis
  jmp non-static-member function-address
```
i.e. the object-pointer is moved to register `ecx` and then the code jumps to the non-static member function. When the non-static member function returns, it will directly return to the __caller of the thunk__ (it won't return to the thunk, since the thunk did not "CALL" the member function but only "JuMP" there).

Just to make it clear: the object-address "%pThis" and the function-address are both __hard coded__ into the thunk! Each thunk instance generates its own piece of assembler code.

That's all for x86. 

### x64
For x64 it's whole different story - and a lot more complicated. So, I'll only sketch the problems briefly. The x64 calling-convention on windows roughly looks like this:

- every argument uses exactly 8 bytes on the stack or a register (larger types are passed by address)
- the first 4 args are passed in registers (`rcx`/`xmm0`, `rdx`/`xmm1`, `r8`/`xmm2`, `r9`/`xmm3`;  with floating point types passed in the `xmm...` registers)
- further arguments are passed on the stack
- the __caller__ is responsible to allocate additional 32-bytes on the stack, known as _shadow space_ (just below the additional arguments), which can be used freely by the __callee__ 
- there is no special handling for the `this`-pointer; it's handled as (hidden) first argument (thus always transfered in `rcx`)

So, for up to three-argument calls it's quite easy: just shifting some registers (taking care of floating point arguments!), pushing the object's address to `rcx` and jumping to the member function (just like for x86).

But starting with 4 arguments it gets really nasty: simply moving up the arguments along the stack __is not possible__, since the place right above the original stack __belongs to the caller__ and we would mess up its stack-frame. That's bad bad bad!

So, we have to create a new frame (move the stack-pointer) with enough place, move all the pieces into place (stack, registers, `this`-pointer) and then jump to the member function. Done.

... but wait ...

The member function gets called. But when it ends, strange things happen, e.g. my bunny Luna starts coding

![luna-coding](/pictures/luna_coding.png)


Something definitely went wrong, since bunnies don't code - they are consulting experts for secure tunneling.

![luna-checking-tunnel](/pictures/luna_checking_tunnel.png)

So, what went wrong? In our thunk we moved the stack-pointer but never moved it back. There is no magic that resets the stack-pointer to it's original value when returning from a call: `RET` simply returns to the address at the current stack-pointer and pops that - nothing more. Back at the caller we ended up with an incorrect stack-pointer ... and strange things happened.

A solution to this would be `CALL`ing (not jumping to) the member function and readjusting the stack after the call returned.

Seems to work ...

... but then there is this object with a thunk as member that `delete`s itself while in a thunked callback: CRASH! BOOM! BANG!

What happened now?

While in the callback, the thunk's destructor got called and it freed the memory with the assembler-code. And when the program `RET`urned from the callback to it, we got a page-fault.

So, if we want to `delete` the thunk, while running in the thunked callback, then we are not allowed to return to the __dynamically__ allocated memory.

But of course, we are allowed to return to __static__ memory. 

So, the final solution is roughly as follows:
- in the dynamic thunk
  - write the `this`-pointer (and the some additional data) to the shadow space (we can freely use the shadow space)
  - then `JuMP` to (not `CALL`!) the static code
- in the static code
  - store the current stack-pointer (and some additional data)
  - create a new frame, i.e. move the stack-pointer, with enough place for all arguments (and some additional place on top)
  - move all the pieces (stack, registers, `this`-pointer) into place
  - `CALL` the member function
  - after member function `RET`urned: restore the stack-pointer (and do additional cleanups)
 
Writing this static piece of assembler code such that it works for any number of arguments, holds some extra hoops to jump through, which are indicated in the parenthesis. For more information search in thunks.hpp for `r12`.

That's all for x64. 

If you want to learn more about x64 calling conventions, here are some resources to start with:
- https://en.wikipedia.org/wiki/X86_calling_conventions
- https://docs.microsoft.com/en-us/cpp/build/x64-software-conventions
- https://devblogs.microsoft.com/oldnewthing/20160623-00/?p=93735

## thunk-deletion
It is save to delete a thunks from this library while being in the thunked callback: the code never returns through the dynamically allocated code. For details, please read the previous section.


### And now have fun with windows-thunks!
