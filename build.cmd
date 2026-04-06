@echo off

setlocal enabledelayedexpansion
cls

cd /D "%~dp0"

set SRC_DIR=%CD%

for %%a in (%*) do set "%%a=1"

:: cleanall-only (no compiler, no config, no run) -- just delete everything and exit
if "%cleanall%"=="1" if not "%run%"=="1" if not "%release%"=="1" if not "%debug%"=="1" if not "%msvc%"=="1" if not "%clang%"=="1" (
    for /d %%d in (.build_*) do (
        echo [removing %%d]
        rmdir /s /q "%%d"
    )
    goto :eof
)

:: bare 'clean' (no compiler, no config, no run) removes everything
if "%clean%"=="1" if not "%cleanall%"=="1" if not "%run%"=="1" if not "%release%"=="1" if not "%debug%"=="1" if not "%msvc%"=="1" if not "%clang%"=="1" (
    for /d %%d in (.build_*) do (
        echo [removing %%d]
        rmdir /s /q "%%d"
    )
    goto :eof
)

if not "%release%"=="1" if not "%debug%"=="1" echo [Configuration ("Debug" or "Release") was not set. Choosing "Release"] && set release=1
if not "%msvc%"=="1" if not "%clang%"=="1" echo [Compiler ("MSVC" or "Clang") was not set. Choosing "MSVC"] && set msvc=1

:: Detect compiler versions

:: MSVC: extract version from cl.exe, e.g. "19.29.30159" -> major "19" minor "29"
set msvc_ver_major=0
set msvc_ver_minor=0
if "%msvc%"=="1" (
    for /f "tokens=7 delims= " %%v in ('cl 2^>^&1 ^| find "Version"') do (
        for /f "tokens=1,2 delims=." %%a in ("%%v") do (
            set msvc_ver_major=%%a
            set msvc_ver_minor=%%b
        )
    )
    echo [MSVC compiler version: !msvc_ver_major!.!msvc_ver_minor!]
)

:: Clang: extract version, e.g. "15.0.2" -> major "15"
set clang_ver_major=0
if "%clang%"=="1" (
    for /f "tokens=3 delims= " %%v in ('clang --version 2^>^&1 ^| find "clang version"') do (
        for /f "tokens=1 delims=." %%a in ("%%v") do set clang_ver_major=%%a
    )
    echo [Clang compiler version: !clang_ver_major!]
)

:: Determine supported standards

:: MSVC C++ standards:
::   < 19.10 (before VS2015 U3): no /std: flag, use "default"
::   19.10+  (VS2015 U3):        c++14, c++latest
::   19.11+  (VS2017 15.3):      + c++17
::   19.28+  (VS2019 16.8):      + c++20
::   19.35+  (VS2022 17.5):      + c++23
set msvc_cpp_standards=default
if !msvc_ver_minor! GEQ 10 set msvc_cpp_standards=c++14
if !msvc_ver_minor! GEQ 11 set msvc_cpp_standards=!msvc_cpp_standards! c++17
if !msvc_ver_minor! GEQ 28 set msvc_cpp_standards=!msvc_cpp_standards! c++20
if !msvc_ver_minor! GEQ 35 set msvc_cpp_standards=!msvc_cpp_standards! c++23
if "%msvc%"=="1" echo [MSVC C++ standards: %msvc_cpp_standards%]

:: MSVC C standards:
::   < 19.28 (before VS2019 16.8): no /std:c flag, use "default"
::   19.28+  (VS2019 16.8):        c11, c17
set msvc_c_standards=default
if !msvc_ver_minor! GEQ 28 set msvc_c_standards=!msvc_c_standards! c11 c17
if "%msvc%"=="1" echo [MSVC C standards: %msvc_c_standards%]

:: Clang C++ standards: c++98 always; c++11 from 3; c++14 from 3.4; c++17 from 5; c++20 from 10; c++23 from 17
set clang_cpp_standards=c++98 c++11
if !clang_ver_major! GEQ 4  set clang_cpp_standards=!clang_cpp_standards! c++14
if !clang_ver_major! GEQ 5  set clang_cpp_standards=!clang_cpp_standards! c++17
if !clang_ver_major! GEQ 10 set clang_cpp_standards=!clang_cpp_standards! c++20
if !clang_ver_major! GEQ 17 set clang_cpp_standards=!clang_cpp_standards! c++23
if "%clang%"=="1" echo [Clang C++ standards: %clang_cpp_standards%]

:: Clang C standards: c89 always; c99 from 3; c11 from 3.1; c17 from 6; c23 from 18
set clang_c_standards=c89 c99
if !clang_ver_major! GEQ 4  set clang_c_standards=!clang_c_standards! c11
if !clang_ver_major! GEQ 6  set clang_c_standards=!clang_c_standards! c17
if !clang_ver_major! GEQ 18 set clang_c_standards=!clang_c_standards! c23
if "%clang%"=="1" echo [Clang C standards: %clang_c_standards%]

:: Compiler flags (MSVC)
::   Disabled warnings:
::      4514 - unreferenced inline function has been removed
::      4505 - unreferenced local function has been removed
::      5045 - Compiler will insert Spectre mitigation for memory load if /Qspectre specified
::      4710 - function not inlined
::      4711 - function selected for inline expansion
set msvc_compile_flags_std=/nologo /Zi /GR- /EHsc /Zl /permissive- /Wall /WX /WL /wd4514 /wd4505
set msvc_compile_flags_dbg=/Od /GS /RTCscu /D_ALLOW_RTCc_IN_STL=1 /D_DEBUG=1 /wd5045 /wd4710 %msvc_compile_flags_std%
set msvc_compile_flags_opt=/O2 /GL /GS- /DNDEBUG=1 /wd4710 /wd4711 /wd4702 /wd5045 %msvc_compile_flags_std%

:: MSVC C mode: /TC forces C, no /EHsc /GR- /permissive- (C++ flags), add /wd4820 for struct padding
set msvc_compile_flags_c_std=/nologo /Zi /TC /Zl /Wall /WX /WL /wd4514 /wd4505
set msvc_compile_flags_c_dbg=/Od /GS /D_DEBUG=1 /wd5045 /wd4710 %msvc_compile_flags_c_std%
set msvc_compile_flags_c_opt=/O2 /GL /GS- /DNDEBUG=1 /wd4710 /wd4711 /wd4702 /wd5045 %msvc_compile_flags_c_std%

set msvc_compile=cl
set msvc_compile_link=/link
set msvc_compile_link_out=/out:
set msvc_std_flag=/std:

set msvc_link_flags_std=/nologo /incremental:no /nodefaultlib /subsystem:console /machine:x64 /debug
set msvc_link_flags_opt=/ltcg %msvc_link_flags_std%
set msvc_link_flags_dbg=%msvc_link_flags_std%
set msvc_link_flags_c_opt=%msvc_link_flags_opt%
set msvc_link_flags_c_dbg=%msvc_link_flags_dbg%
set msvc_link_libs_std=kernel32.lib

set msvc_link_libs_opt=libcmt.lib libucrt.lib libvcruntime.lib libcpmt.lib %msvc_link_libs_std%
set msvc_link_libs_dbg=msvcrtd.lib ucrtd.lib vcruntimed.lib msvcprtd.lib %msvc_link_libs_std%

:: MSVC C mode link libs (no C++ runtime)
set msvc_link_libs_c_opt=libcmt.lib libucrt.lib libvcruntime.lib %msvc_link_libs_std%
set msvc_link_libs_c_dbg=msvcrtd.lib ucrtd.lib vcruntimed.lib %msvc_link_libs_std%

:: ---- Compiler flags (Clang) ----
set clang_compile_flags_std=-g -fno-rtti -Wall -Wextra -Werror
set clang_compile_flags_dbg=-O0 -D_DEBUG=1 %clang_compile_flags_std%
set clang_compile_flags_opt=-O3 -DNDEBUG=1 %clang_compile_flags_std%

:: Clang C mode: -x c forces C
set clang_compile_flags_c_std=-g -x c -Wall -Wextra -Werror
set clang_compile_flags_c_dbg=-O0 -D_DEBUG=1 %clang_compile_flags_c_std%
set clang_compile_flags_c_opt=-O3 -DNDEBUG=1 %clang_compile_flags_c_std%

set clang_compile=clang
set clang_compile_link=-fuse-ld=lld
set clang_compile_link_out=-o
set clang_std_flag=-std=

set clang_link_flags_opt=
for %%a in (%msvc_link_flags_opt%) do set "clang_link_flags_opt=!clang_link_flags_opt! -Xlinker %%a"

set clang_link_flags_dbg=
for %%a in (%msvc_link_flags_dbg%) do set "clang_link_flags_dbg=!clang_link_flags_dbg! -Xlinker %%a"

set clang_link_libs_opt=
for %%a in (%msvc_link_libs_opt%) do set "clang_link_libs_opt=!clang_link_libs_opt! -Xlinker %%a"

set clang_link_libs_dbg=
for %%a in (%msvc_link_libs_dbg%) do set "clang_link_libs_dbg=!clang_link_libs_dbg! -Xlinker %%a"

set clang_link_flags_c_opt=!clang_link_flags_opt!
set clang_link_flags_c_dbg=!clang_link_flags_dbg!

set clang_link_libs_c_opt=
for %%a in (%msvc_link_libs_c_opt%) do set "clang_link_libs_c_opt=!clang_link_libs_c_opt! -Xlinker %%a"

set clang_link_libs_c_dbg=
for %%a in (%msvc_link_libs_c_dbg%) do set "clang_link_libs_c_dbg=!clang_link_libs_c_dbg! -Xlinker %%a"

:: Build all variants
if "%release%"=="1" call :for_all opt build
if "%debug%"=="1"   call :for_all dbg build

:: Run selected variants
if "%run%"=="1" (
    set run_pass=0
    set run_fail=0
    if "%release%"=="1" call :for_all opt run
    if "%debug%"=="1"   call :for_all dbg run
    echo.
    echo [!run_pass! passed, !run_fail! failed]
)

:: Clean selected variants
:clean
if "%clean%"=="1" (
    if "%release%"=="1" call :for_all opt clean
    if "%debug%"=="1"   call :for_all dbg clean
)

:: Clean ALL build folders
if "%cleanall%"=="1" (
    for /d %%d in (.build_*) do (
        echo [removing %%d]
        rmdir /s /q "%%d"
    )
)

goto :eof

:: %1 = config (opt|dbg), %2 = action (build|run|clean)
:for_all
    if "%msvc%"=="1" (
        for %%s in (%msvc_cpp_standards%) do call :%2_config msvc %1 cpp %%s
        for %%s in (%msvc_c_standards%)   do call :%2_config msvc %1 c %%s
    )
    if "%clang%"=="1" (
        for %%s in (%clang_cpp_standards%) do call :%2_config clang %1 cpp %%s
        for %%s in (%clang_c_standards%)   do call :%2_config clang %1 c %%s
    )
exit /b 0

::  %1 = compiler, %2 = config, %3 = lang, %4 = standard
:build_config
    setlocal
    echo [building %1 %3 %4 %2]
    if not exist .build_%1_%2_%3_%4 mkdir .build_%1_%2_%3_%4
    pushd .build_%1_%2_%3_%4
        set compile=%%%1_compile%%
        set compile_link=%%%1_compile_link%%
        set compile_link_out=%%%1_compile_link_out%%
        set std_flag=%%%1_std_flag%%
        if "%3"=="c" (
            set compile_flags=%%%1_compile_flags_c_%2%%
            set link_flags=%%%1_link_flags_c_%2%%
            set link_libs=%%%1_link_libs_c_%2%%
        ) else (
            set compile_flags=%%%1_compile_flags_%2%%
            set link_flags=%%%1_link_flags_%2%%
            set link_libs=%%%1_link_libs_%2%%
        )
        :: append /std: or -std= flag (skip for MSVC C "default" which has no /std: flag)
        if not "%4"=="default" set compile_flags=%compile_flags% %std_flag%%4
        call %compile% %compile_flags% ..\tta_assert_test.cpp %compile_link% %compile_link_out%tta_assert_test_%1_%2_%3_%4.exe %link_flags% %link_libs%
    popd
exit /b 0

::  %1 = compiler, %2 = config, %3 = lang, %4 = standard
:run_config
    setlocal
    set "exe_path=.build_%1_%2_%3_%4\tta_assert_test_%1_%2_%3_%4.exe"
    if not exist "%exe_path%" (
        echo [SKIP] %1 %3 %4 %2 - not built
        exit /b 0
    )
    "%exe_path%" >nul 2>nul
    if errorlevel 1 (
        echo [FAIL] %1 %3 %4 %2
        set /a run_fail+=1
    ) else (
        echo [OK]   %1 %3 %4 %2
        set /a run_pass+=1
    )
    endlocal & set run_pass=%run_pass%& set run_fail=%run_fail%
exit /b 0

::  %1 = compiler, %2 = config, %3 = lang, %4 = standard
:clean_config
    if exist ".build_%1_%2_%3_%4" (
        echo [removing .build_%1_%2_%3_%4]
        rmdir /s /q ".build_%1_%2_%3_%4"
    )
exit /b 0

:eof
