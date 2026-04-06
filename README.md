tta_assert
====================================================================================================

This [single-header file](https://github.com/nothings/stb/blob/master/docs/other_libs.md) provides
a set of assert macros supporting additional functionality comparing to standard C library
[`assert`](https://en.wikipedia.org/wiki/Assertion_(software_development)).

Key Features
----------------------------------------------------------------------------------------------------
- **Passthrough** vs **Instrumented** mode choice at compile time.
- **Instrumented** mode supporting multiple levels of reporting:
  - Triggering a callback to report an assertion information
  - Triggering a callback to report an assertion information and executing a debug break when debugger is attached
  - Triggering a callback to report an assertion information and throwing an exception (in C++ mode) or
    calling `longjmp` (in C mode) to simplify re-use of existing code already instrumented with `TTA_ASSERT*`
    asserts in any unit-testing framework via `tta_AssertCallAndCatch` accepting a callback implementing
    a unit-test and returning `1` or `0` depending on whether any assert has been triggered
- **Extra** assertion variants to support checking condition directly in `if` statement :
  ```c
  TTA_ASSERT_IF_MSG(0 != x /* assert triggers if (0 == x) */, "Expected 0 != x")
  {
      /* do something if 0 != x */
  }
  TTA_ASSERT_IF(0 != x /* assert triggers if (0 == x) */)
  {
      /* do something if 0 != x */
  }
  ```
- **Extra** assertion variants for use as a replacement for `if (!condition) { Report(...); return retval; }`
  ```c
  TTA_ASSERT_RET_MSG(/* conditition */0 != x, /* retval */E_INVALIDARG, "Expected 0 != x");
  TTA_ASSERT_RET(/* conditition */0 != x, /* retval */E_INVALIDARG);
  ```
- **Unit Testing** set up support by reusing existing code
  ```c
    int UnitTest_MyFunctionA(void *userdata)
    {
        MyFunctionA(); /* call the MyFunctionA with TTA_ASSERT or calling other function with */
    }

    void RunUnitTests(void)
    {
        int failCount = 0;

        failCount += tta_AssertCallAndCatch(UnitTest_MyFunction, 0);
    }
  ```
- **No (mandatory) external dependencies**. Only standard C library `<stdarg.h>` include in the implementation
- **Supports Clang / MSVC compilers** and compiling as *C89/C99/C11/C17/C23* and *C++98/C++11/C++14/C++17/C++20/C++23*

Build
----------------------------------------------------------------------------------------------------

The example file `tta_assert_test.cpp` showing how to setup `tta_assert.h` to create a small
unit testing sandbox can be built with a single command line:
  - `cl.exe tta_assert_test.cpp /link /out:test.exe`
  - `clang.exe tta_assert_test.cpp -o test.exe`


It also possible to build multiple configurations at once via `build.cmd` batch file. The prerequisite
is to have `cl.exe` and `clang.exe` in `PATH`. The batch supports the following arguments:
  - `debug` -- for all chosen compilers selects **debug** variants of all configurations
  - `release` -- for all chosen compilers selects **optimised** variants of configurations
  - `clang` -- chooses **Clang** compiler to build all chose configurations
  - `msvc` -- chooses **MSVC** compiler to build all chosen configurations
  - `run` -- runs built configurations
  - `clean` -- cleans built configurations
  - `cleanall` -- cleans all configurations

All specified arguments can be passed into `build.cmd` in any order. If no compiler is specified
**MSVC** compiler is chosen as a default. If no configuration is specified **Release** variants are chosen.


Acknowledgements
----------------------------------------------------------------------------------------------------
- [Sean Barrett](http://nothings.org/): for his remarkable single-header C libraries.
