/* tta_assert test suite - Google Test style output */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
#ifdef __cplusplus
extern "C"
{
#endif
    __declspec(dllimport) void *__stdcall GetStdHandle(unsigned long nStdHandle);
    __declspec(dllimport) int __stdcall GetConsoleMode(void *hConsoleHandle, unsigned long *lpMode);
    __declspec(dllimport) int __stdcall SetConsoleMode(void *hConsoleHandle, unsigned long dwMode);
#ifdef __cplusplus
}
#endif

static void enable_ansi_colors(void)
{
    void         *h;
    unsigned long mode;
    h = GetStdHandle((unsigned long)-11 /**STD_OUTPUT_HANDLE */);
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | 0x0004u /* Enable Virtual Terminal Processing */);
    h = GetStdHandle((unsigned long)-12 /**STD_ERROR_HANDLE */);
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | 0x0004u /* Enable Virtual Terminal Processing */);
}
#else
static void enable_ansi_colors(void)
{
}
#endif

#define TTA_ASSERT_IMPL 1
#define TTA_ASSERT_MODE TTA_ASSERT_MODE_PASSTHROUGH
#include "tta_assert.h"

/** ANSI color codes */
#define CLR_GREEN "\033[32m"
#define CLR_RED   "\033[31m"
#define CLR_RESET "\033[0m"

/** GTest-like test result annotation */
#define T_BAR  CLR_GREEN "[----------]" CLR_RESET
#define T_HEAD CLR_GREEN "[==========]" CLR_RESET
#define T_RUN  CLR_GREEN "[ RUN      ]" CLR_RESET
#define T_OK   CLR_GREEN "[       OK ]" CLR_RESET
#define T_FAIL CLR_RED   "[  FAILED  ]" CLR_RESET
#define T_PASS CLR_GREEN "[  PASSED  ]" CLR_RESET


static int tta_Test_ReportCback_CallCount = 0;

#ifdef __cplusplus
extern "C" {
#endif

static void tta_Test_NoReportCback(const char *expr, const char *file, int line, const char *func, const char *msg, void *args)
{
    (void)expr;
    (void)file;
    (void)line;
    (void)func;
    (void)msg;
    (void)args;
    tta_Test_ReportCback_CallCount += 1;
    printf("Called\n");
}

static void tta_Test_ReportCback(const char *expr, const char *file, int line, const char *func, const char *msg, void *args)
{
    fprintf(stderr, CLR_RED "%s(%d): ASSERT FAILED in %s" CLR_RESET "\n", file, line, func);
    fprintf(stderr, "  Expression: %s\n", expr);
    if (msg && args)
    {
        va_list *pargs = (va_list *)args;
        fprintf(stderr, "  Message:    ");
        vfprintf(stderr, msg, *pargs);
        fprintf(stderr, "\n");
    }
    else if (msg)
    {
        fprintf(stderr, "  Message:    %s\n", msg);
    }
    tta_Test_ReportCback_CallCount += 1;
}

#ifdef __cplusplus
}
#endif

typedef int callback(void *);

#define TEST_EXPECT_GENERIC(name, cond, pass_postfix, fail_postfix)                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        tests_run++;                                                                                                   \
        printf(T_RUN " " name "\n");                                                                                   \
        if (cond)                                                                                                      \
        {                                                                                                              \
            printf(T_OK " " name "" pass_postfix "\n");                                                                \
            tests_passed++;                                                                                            \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            printf(T_FAIL " " name "" fail_postfix "\n");                                                              \
        }                                                                                                              \
    } while (0)

#define TEST_EXPECT(name, cond) TEST_EXPECT_GENERIC(name, cond, "", "")

#define TEST_RUN(name, call, data) TEST_EXPECT_GENERIC(name, tta_AssertCallAndCatch((callback *)call, (void *)data) == 0, "", "")

#define TEST_RUN_EXPECT_FAIL(name, call, data) TEST_EXPECT_GENERIC(name, tta_AssertCallAndCatch((callback *)call, (void *)data) != 0, " (expected fail)", " (should have failed)")

struct tta_Test_Context
{
    int expect;
    int actual;
};

#ifndef __cplusplus
typedef struct tta_Test_Context tta_Test_Context;
#endif

static int tta_Test_Passthrough_Assert(tta_Test_Context *ctx)
{
    TTA_ASSERT(ctx->expect == ctx->actual);
    return ctx->expect - ctx->actual;
}

static int tta_Test_Passthrough_Assert_Msg(tta_Test_Context *ctx)
{
    TTA_ASSERT_MSG(ctx->expect == ctx->actual, "Must be identical");
    return ctx->expect - ctx->actual;
}

static int tta_Test_Passthrough_Assert_Msg_Arg(tta_Test_Context *ctx)
{
    TTA_ASSERT_MSG(ctx->expect == ctx->actual, "Expect '%d' vs Actual '%d', but must be identical", ctx->expect, ctx->actual);
    return ctx->expect - ctx->actual;
}

static int tta_Test_Passthrough_Assert_If(tta_Test_Context *ctx)
{
    TTA_ASSERT_IF(ctx->expect == ctx->actual)
        return 0;
    return ctx->expect - ctx->actual;
}

static int tta_Test_Passthrough_Assert_If_Msg(tta_Test_Context *ctx)
{
    TTA_ASSERT_IF_MSG(ctx->expect == ctx->actual, "Must be identical")
        return 0;
    return ctx->expect - ctx->actual;
}

static int tta_Test_Passthrough_Assert_If_Msg_Arg(tta_Test_Context *ctx)
{
    TTA_ASSERT_IF_MSG(ctx->expect == ctx->actual, "Expect '%d' vs Actual '%d', but must be identical", ctx->expect, ctx->actual)
        return 0;
    return ctx->expect - ctx->actual;
}

static int tta_Test_Passthrough_Assert_Ret(tta_Test_Context *ctx)
{
    TTA_ASSERT_RET(ctx->expect == ctx->actual, ctx->expect - ctx->actual);
    return 0;
}

static int tta_Test_Passthrough_Assert_Ret_Msg(tta_Test_Context *ctx)
{
    TTA_ASSERT_RET_MSG(ctx->expect == ctx->actual, ctx->expect - ctx->actual, "Must be identical");
    return 0;
}

static int tta_Test_Passthrough_Assert_Ret_Msg_Arg(tta_Test_Context *ctx)
{
    TTA_ASSERT_RET_MSG(ctx->expect == ctx->actual, ctx->expect - ctx->actual, "Expect '%d' vs Actual '%d', but must be identical", ctx->expect, ctx->actual);
    return 0;
}

/** Instrumented mode tests  */
#undef TTA_ASSERT_MODE
#define TTA_ASSERT_MODE TTA_ASSERT_MODE_INSTRUMENTED
#include "tta_assert.h"

static int tta_Test_Instrumented_Assert(tta_Test_Context *ctx)
{
    TTA_ASSERT(ctx->expect == ctx->actual);
    return ctx->expect - ctx->actual;
}

static int tta_Test_Instrumented_Assert_Msg(tta_Test_Context *ctx)
{
    TTA_ASSERT_MSG(ctx->expect == ctx->actual, "Must be identical");
    return ctx->expect - ctx->actual;
}

static int tta_Test_Instrumented_Assert_Msg_Arg(tta_Test_Context *ctx)
{
    TTA_ASSERT_MSG(ctx->expect == ctx->actual, "Expect '%d' vs Actual '%d', but must be identical", ctx->expect, ctx->actual);
    return ctx->expect - ctx->actual;
}

static int tta_Test_Instrumented_Assert_If(tta_Test_Context *ctx)
{
    TTA_ASSERT_IF(ctx->expect == ctx->actual)
        return 0;
    return ctx->expect - ctx->actual;
}

static int tta_Test_Instrumented_Assert_If_Msg(tta_Test_Context *ctx)
{
    TTA_ASSERT_IF_MSG(ctx->expect == ctx->actual, "Must be identical")
        return 0;
    return ctx->expect - ctx->actual;
}

static int tta_Test_Instrumented_Assert_If_Msg_Arg(tta_Test_Context *ctx)
{
    TTA_ASSERT_IF_MSG(ctx->expect == ctx->actual, "Expect '%d' vs Actual '%d', but must be identical", ctx->expect, ctx->actual)
        return 0;
    return ctx->expect - ctx->actual;
}

static int tta_Test_Instrumented_Assert_Ret(tta_Test_Context *ctx)
{
    TTA_ASSERT_RET(ctx->expect == ctx->actual, ctx->expect - ctx->actual);
    return 0;
}

static int tta_Test_Instrumented_Assert_Ret_Msg(tta_Test_Context *ctx)
{
    TTA_ASSERT_RET_MSG(ctx->expect == ctx->actual, ctx->expect - ctx->actual, "Must be identical");
    return 0;
}

static int tta_Test_Instrumented_Assert_Ret_Msg_Arg(tta_Test_Context *ctx)
{
    TTA_ASSERT_RET_MSG(ctx->expect == ctx->actual, ctx->expect - ctx->actual, "Expect '%d' vs Actual '%d', but must be identical", ctx->expect, ctx->actual);
    return 0;
}

/** Handler tests */
static int         handler_called    = 0;
static const char *handler_last_expr = 0;
static const char *handler_last_msg  = 0;

#ifdef __cplusplus
extern "C" {
#endif

static void tracking_handler(const char *expr, const char *file, int line, const char *func, const char *msg, void *args)
{
    (void)file;
    (void)line;
    (void)func;
    (void)args;
    handler_last_expr = expr;
    handler_last_msg  = msg;
    ++handler_called;
}

#ifdef __cplusplus
}
#endif

int test_handler_fn(void *ud)
{
    (void)ud;
    tta_AssertSetReportCback(tracking_handler);
    TTA_ASSERT_MSG(1 == 2, "handler test");
    return 0;
}

int main(void)
{
    int tests_run    = 0;
    int tests_passed = 0;
    tta_Test_Context ctx = { 666, 666 };

    enable_ansi_colors();

    printf(T_HEAD " Running tta_assert tests\n");
    printf(T_BAR "\n");

    /** Passthrough */
    printf(T_BAR " Passthrough mode\n");
    tta_AssertSetReportCback(tta_Test_NoReportCback);
    /** We test assert in passthrough mode by checking no callbacks is called, the return value of the test isn't modified */
    TEST_EXPECT("Passthrough.Assert.Success",           0 == tta_Test_Passthrough_Assert(&ctx)              && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertMsg.Success",        0 == tta_Test_Passthrough_Assert_Msg(&ctx)          && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertMsgArg.Success",     0 == tta_Test_Passthrough_Assert_Msg_Arg(&ctx)      && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertIf.Success",         0 == tta_Test_Passthrough_Assert_If(&ctx)           && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertIfMsg.Success",      0 == tta_Test_Passthrough_Assert_If_Msg(&ctx)       && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertIfMsgArg.Success",   0 == tta_Test_Passthrough_Assert_If_Msg_Arg(&ctx)   && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertRet.Success",        0 == tta_Test_Passthrough_Assert_Ret(&ctx)          && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertRetMsg.Success",     0 == tta_Test_Passthrough_Assert_Ret_Msg(&ctx)      && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertRetMsgArg.Success",  0 == tta_Test_Passthrough_Assert_Ret_Msg_Arg(&ctx)  && 0 == tta_Test_ReportCback_CallCount);

    ctx.actual = 13;
    TEST_EXPECT("Passthrough.Assert.Failure",           653 == tta_Test_Passthrough_Assert(&ctx)            && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertMsg.Failure",        653 == tta_Test_Passthrough_Assert_Msg(&ctx)        && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertMsgArg.Failure",     653 == tta_Test_Passthrough_Assert_Msg_Arg(&ctx)    && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertIf.Failure",         653 == tta_Test_Passthrough_Assert_If(&ctx)         && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertIfMsg.Failure",      653 == tta_Test_Passthrough_Assert_If_Msg(&ctx)     && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertIfMsgArg.Failure",   653 == tta_Test_Passthrough_Assert_If_Msg_Arg(&ctx) && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertRet.Failure",        653 == tta_Test_Passthrough_Assert_Ret(&ctx)        && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertRetMsg.Failure",     653 == tta_Test_Passthrough_Assert_Ret_Msg(&ctx)    && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Passthrough.AssertRetMsgArg.Failure",  653 == tta_Test_Passthrough_Assert_Ret_Msg_Arg(&ctx)&& 0 == tta_Test_ReportCback_CallCount);

    /** Instrumented Print mode */
    printf(T_BAR " Instrumented (Print mode)\n");
    tta_AssertSetReportCback(tta_Test_ReportCback);
    tta_AssertSetReportLevel(ktta_AssertReportLevel_Print);

    /**
     *  We test assert in intrumented mode by checking no callbacks is called it should succeed
     *  the return value of the test is zero
     */
    ctx.expect = 13;
    TEST_EXPECT("Instrumented.Assert.Success",           0 == tta_Test_Instrumented_Assert(&ctx)              && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertMsg.Success",        0 == tta_Test_Instrumented_Assert_Msg(&ctx)          && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertMsgArg.Success",     0 == tta_Test_Instrumented_Assert_Msg_Arg(&ctx)      && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertIf.Success",         0 == tta_Test_Instrumented_Assert_If(&ctx)           && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertIfMsg.Success",      0 == tta_Test_Instrumented_Assert_If_Msg(&ctx)       && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertIfMsgArg.Success",   0 == tta_Test_Instrumented_Assert_If_Msg_Arg(&ctx)   && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertRet.Success",        0 == tta_Test_Instrumented_Assert_Ret(&ctx)          && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertRetMsg.Success",     0 == tta_Test_Instrumented_Assert_Ret_Msg(&ctx)      && 0 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertRetMsgArg.Success",  0 == tta_Test_Instrumented_Assert_Ret_Msg_Arg(&ctx)  && 0 == tta_Test_ReportCback_CallCount);

    /**
     *  We also test assert in intrumented mode by checking callbacks are called when assertions trigger,
     *  the return value in this case is the delta between between 'actual' and 'expect'
     */
    ctx.actual = 3;
    TEST_EXPECT("Instrumented.Assert.Failure",           10 == tta_Test_Instrumented_Assert(&ctx)            && 1 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertMsg.Failure",        10 == tta_Test_Instrumented_Assert_Msg(&ctx)        && 2 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertMsgArg.Failure",     10 == tta_Test_Instrumented_Assert_Msg_Arg(&ctx)    && 3 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertIf.Failure",         10 == tta_Test_Instrumented_Assert_If(&ctx)         && 4 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertIfMsg.Failure",      10 == tta_Test_Instrumented_Assert_If_Msg(&ctx)     && 5 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertIfMsgArg.Failure",   10 == tta_Test_Instrumented_Assert_If_Msg_Arg(&ctx) && 6 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertRet.Failure",        10 == tta_Test_Instrumented_Assert_Ret(&ctx)        && 7 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertRetMsg.Failure",     10 == tta_Test_Instrumented_Assert_Ret_Msg(&ctx)    && 8 == tta_Test_ReportCback_CallCount);
    TEST_EXPECT("Instrumented.AssertRetMsgArg.Failure",  10 == tta_Test_Instrumented_Assert_Ret_Msg_Arg(&ctx)&& 9 == tta_Test_ReportCback_CallCount);

    /** Get/Set API */
    printf(T_BAR " API\n");
    TEST_EXPECT("API.GetLevel", tta_AssertGetReportLevel() == ktta_AssertReportLevel_Print);

    tta_AssertSetReportLevel(ktta_AssertReportLevel_PrintAndBreak);
    TEST_EXPECT("API.SetLevel", tta_AssertGetReportLevel() == ktta_AssertReportLevel_PrintAndBreak);
    tta_AssertSetReportLevel(ktta_AssertReportLevel_Print);

    /** CallAndCatch (PrintAndThrow) */
    printf(T_BAR " CallAndCatch\n");
    ctx.expect = 3;

    /**
     *  We also test assert in intrumented mode by calling test function from `tta_AssertCallAndCatch`
     *  with a given context to check the assert doesn't trigger when the test condition holds
     *  and triggers in this case if test condition fails
     */
    TEST_RUN("CallAndCatch.Assert.Success",          tta_Test_Instrumented_Assert,              &ctx);
    TEST_RUN("CallAndCatch.AssertMsg.Success",       tta_Test_Instrumented_Assert_Msg,          &ctx);
    TEST_RUN("CallAndCatch.AssertMsgArg.Success",    tta_Test_Instrumented_Assert_Msg_Arg,      &ctx);
    TEST_RUN("CallAndCatch.AssertIf.Success",        tta_Test_Instrumented_Assert_If,           &ctx);
    TEST_RUN("CallAndCatch.AssertIfMsg.Success",     tta_Test_Instrumented_Assert_If_Msg,       &ctx);
    TEST_RUN("CallAndCatch.AssertIfMsgArg.Success",  tta_Test_Instrumented_Assert_If_Msg_Arg,   &ctx);
    TEST_RUN("CallAndCatch.AssertRet.Success",       tta_Test_Instrumented_Assert_Ret,          &ctx);
    TEST_RUN("CallAndCatch.AssertRetMsg.Success",    tta_Test_Instrumented_Assert_Ret_Msg,      &ctx);
    TEST_RUN("CallAndCatch.AssertRetMsgArg.Success", tta_Test_Instrumented_Assert_Ret_Msg_Arg,  &ctx);

    ctx.expect = 2;
    TEST_RUN_EXPECT_FAIL("CallAndCatch.Assert.Failure",          tta_Test_Instrumented_Assert,              &ctx);
    TEST_RUN_EXPECT_FAIL("CallAndCatch.AssertMsg.Failure",       tta_Test_Instrumented_Assert_Msg,          &ctx);
    TEST_RUN_EXPECT_FAIL("CallAndCatch.AssertMsgArg.Failure",    tta_Test_Instrumented_Assert_Msg_Arg,      &ctx);
    TEST_RUN_EXPECT_FAIL("CallAndCatch.AssertIf.Failure",        tta_Test_Instrumented_Assert_If,           &ctx);
    TEST_RUN_EXPECT_FAIL("CallAndCatch.AssertIfMsg.Failure",     tta_Test_Instrumented_Assert_If_Msg,       &ctx);
    TEST_RUN_EXPECT_FAIL("CallAndCatch.AssertIfMsgArg.Failure",  tta_Test_Instrumented_Assert_If_Msg_Arg,   &ctx);
    TEST_RUN_EXPECT_FAIL("CallAndCatch.AssertRet.Failure",       tta_Test_Instrumented_Assert_Ret,          &ctx);
    TEST_RUN_EXPECT_FAIL("CallAndCatch.AssertRetMsg.Failure",    tta_Test_Instrumented_Assert_Ret_Msg,      &ctx);
    TEST_RUN_EXPECT_FAIL("CallAndCatch.AssertRetMsgArg.Failure", tta_Test_Instrumented_Assert_Ret_Msg_Arg,  &ctx);

    TEST_EXPECT("CallAndCatch.ReportCback.CallCount", 18 == tta_Test_ReportCback_CallCount);

    TEST_RUN_EXPECT_FAIL("CallAndCatch.HandlerFail", test_handler_fn, NULL);

    /** Handler arg verification */
    printf(T_BAR " Handler\n");
    TEST_EXPECT("Handler.Called", 1 == handler_called);
    TEST_EXPECT("Handler.Expr", handler_last_expr != 0 && handler_last_expr[0] == '1');
    TEST_EXPECT("Handler.Msg", handler_last_msg != 0 && strcmp(handler_last_msg, "handler test") == 0);

    /** CallAndCatch restores previous callback */
    TEST_EXPECT("Handler.Restored", tta_AssertGetReportCback() == tta_Test_ReportCback);

    /** Set/Get/Reset */
    tta_AssertSetReportCback(tracking_handler);
    TEST_EXPECT("Handler.SetCustom", tta_AssertGetReportCback() == tracking_handler);
    tta_AssertSetReportCback(0);
    TEST_EXPECT("Handler.ResetDefault", tta_AssertGetReportCback() != 0);

    /** Summary */
    {
        int tests_failed = tests_run - tests_passed;
        printf(T_HEAD " %d tests ran\n", tests_run);
        if (tests_passed > 0)
        {
            printf(T_PASS " %d tests\n", tests_passed);
        }
        if (tests_failed > 0)
        {
            printf(T_FAIL " %d tests\n", tests_failed);
        }
        return tests_failed;
    }
}
