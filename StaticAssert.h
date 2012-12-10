#pragma once

template <bool> struct CompileTimeChecker
{ CompileTimeChecker(...);
};
template <> struct CompileTimeChecker<false> {};
#define STATIC_ASSERT(expr, msg) \
{ struct ERROR_##msg { ERROR_##msg(int){} }; \
  (void)sizeof(CompileTimeChecker<(expr)!=0>(ERROR_##msg(1))); \
}