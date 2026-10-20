// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_BASE_IEEE754_H_
#define V8_BASE_IEEE754_H_

#include "src/base/base-export.h"

#include <cstdint>

#if defined(V8_USE_LIBM_TRIG_FUNCTIONS)
#include "third_party/glibc/src/sysdeps/ieee754/dbl-64/trig.h"  // nogncheck
#endif

namespace v8 {
namespace base {
namespace ieee754 {

// Returns the arc cosine of |x|; that is the value whose cosine is |x|.
V8_BASE_EXPORT double acos(double x);

// Returns the inverse hyperbolic cosine of |x|; that is the value whose
// hyperbolic cosine is |x|.
V8_BASE_EXPORT double acosh(double x);

// Returns the arc sine of |x|; that is the value whose sine is |x|.
V8_BASE_EXPORT double asin(double x);

// Returns the inverse hyperbolic sine of |x|; that is the value whose
// hyperbolic sine is |x|.
V8_BASE_EXPORT double asinh(double x);

// Returns the principal value of the arc tangent of |x|; that is the value
// whose tangent is |x|.
V8_BASE_EXPORT double atan(double x);

// Returns the principal value of the arc tangent of |y/x|, using the signs of
// the two arguments to determine the quadrant of the result.
V8_BASE_EXPORT double atan2(double y, double x);

#if defined(V8_USE_LIBM_TRIG_FUNCTIONS)
// To ensure there aren't problems with libm's sin/cos, both implementations
// are shipped. The plan is to transition to libm once we ensure there are no
// compatibility or performance issues.
V8_BASE_EXPORT double fdlibm_sin(double x);
V8_BASE_EXPORT double fdlibm_cos(double x);

#if !defined(BUILDING_V8_BASE_SHARED) && !defined(USING_V8_BASE_SHARED)
inline double libm_sin(double x) { return glibc_sin(x); }
inline double libm_cos(double x) { return glibc_cos(x); }
#else
V8_BASE_EXPORT double libm_sin(double x);
V8_BASE_EXPORT double libm_cos(double x);
#endif
#else
V8_BASE_EXPORT double cos(double x);
V8_BASE_EXPORT double sin(double x);
#endif

// Returns the base-e exponential of |x|.
V8_BASE_EXPORT double exp(double x);

V8_BASE_EXPORT double atanh(double x);

// Returns the natural logarithm of |x|.
V8_BASE_EXPORT double log(double x);

// Returns a value equivalent to |log(1+x)|, but computed in a way that is
// accurate even if the value of |x| is near zero.
V8_BASE_EXPORT double log1p(double x);

// Returns the base 2 logarithm of |x|.
V8_BASE_EXPORT double log2(double x);

// Returns the base 10 logarithm of |x|.
V8_BASE_EXPORT double log10(double x);

// Returns the cube root of |x|.
V8_BASE_EXPORT double cbrt(double x);

// Returns exp(x)-1, the exponential of |x| minus 1.
V8_BASE_EXPORT double expm1(double x);

// Returns |x| to the power of |y|.
// The result of base ** exponent when base is 1 or -1 and exponent is
// +Infinity or -Infinity differs from IEEE 754-2008. The first edition
// of ECMAScript specified a result of NaN for this operation, whereas
// later versions of IEEE 754-2008 specified 1. The historical ECMAScript
// behaviour is preserved for compatibility reasons.
V8_BASE_EXPORT double pow(double x, double y);

// Returns the tangent of |x|, where |x| is given in radians.
V8_BASE_EXPORT double tan(double x);

// Returns the hyperbolic cosine of |x|, where |x| is given radians.
V8_BASE_EXPORT double cosh(double x);

// Returns the hyperbolic sine of |x|, where |x| is given radians.
V8_BASE_EXPORT double sinh(double x);

// Returns the hyperbolic tangent of |x|, where |x| is given radians.
V8_BASE_EXPORT double tanh(double x);

// ===================================================================
// pyv8web 定制（2026-10-18）：Math.tanh 按模拟平台返回不同位模式
// ===================================================================
// Chrome 148 起 V8 把 Math.tanh 从自带 fdlibm 换成 std::tanh，结果落到
// 宿主 libm（glibc / libsystem_m / UCRT），约 1/4 输入相差 1~2 ULP，
// 成为 OS 指纹。本项目默认仍走 fdlibm，可通过下面的接口按
// browser_profile 的 platform 切换到对应平台的位模式。
//
// 取值：
//   0 = 默认（fdlibm，跨平台 bit 一致，行为与定制前完全相同）
//   1 = Linux  （glibc，与 fdlibm 在探测点上同值）
//   2 = Windows（std::tanh ⇒ 宿主 UCRT libm，bit 精确）
//   3 = macOS  （std::tanh + 已知 ULP 校准；libsystem_m 闭源，
//                仅覆盖已标定的探测点）
V8_BASE_EXPORT void SetTanhPlatform(int32_t platform);
V8_BASE_EXPORT int32_t GetTanhPlatform();

}  // namespace ieee754
}  // namespace base
}  // namespace v8

#endif  // V8_BASE_IEEE754_H_
