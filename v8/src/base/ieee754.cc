// The following is adapted from fdlibm (http://www.netlib.org/fdlibm).
//
// ====================================================
// Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
//
// Developed at SunSoft, a Sun Microsystems, Inc. business.
// Permission to use, copy, modify, and distribute this
// software is freely granted, provided that this notice
// is preserved.
// ====================================================
//
// The original source code covered by the above license above has been
// modified significantly by Google Inc.
// Copyright 2016 the V8 project authors. All rights reserved.

#define LIBC_MATH                             \
  (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT | \
   LIBC_MATH_ASSUME_ROUND_NEAREST_ONLY)

#include "src/base/ieee754.h"

#include <cmath>
#include <limits>

#include "src/base/build_config.h"
#include "src/base/macros.h"
#include "src/base/overflowing-math.h"
#include "third_party/llvm-libc/src/shared/math.h"

namespace v8 {
namespace base {
namespace ieee754 {

/*
 * The original fdlibm code used statements like:
 *  n0 = ((*(int*)&one)>>29)^1;   * index of high word *
 *  ix0 = *(n0+(int*)&x);     * high word of x *
 *  ix1 = *((1-n0)+(int*)&x);   * low word of x *
 * to dig two 32 bit words out of the 64 bit IEEE floating point
 * value.  That is non-ANSI, and, moreover, the gcc instruction
 * scheduler gets it wrong.  We instead use the following macros.
 * Unlike the original code, we determine the endianness at compile
 * time, not at run time; I don't see much benefit to selecting
 * endianness at run time.
 */

/* Get two 32 bit ints from a double.  */

#define EXTRACT_WORDS(ix0, ix1, d)               \
  do {                                           \
    uint64_t bits = base::bit_cast<uint64_t>(d); \
    (ix0) = bits >> 32;                          \
    (ix1) = bits & 0xFFFFFFFFu;                  \
  } while (false)

/* Get the more significant 32 bit int from a double.  */

#define GET_HIGH_WORD(i, d)                      \
  do {                                           \
    uint64_t bits = base::bit_cast<uint64_t>(d); \
    (i) = bits >> 32;                            \
  } while (false)

/* Set the more significant 32 bits of a double from an int.  */

#define SET_HIGH_WORD(d, v)                      \
  do {                                           \
    uint64_t bits = base::bit_cast<uint64_t>(d); \
    bits &= 0x0000'0000'FFFF'FFFF;               \
    bits |= static_cast<uint64_t>(v) << 32;      \
    (d) = base::bit_cast<double>(bits);          \
  } while (false)

double acos(double x) { return LIBC_NAMESPACE::shared::acos(x); }

/* acosh(x)
 * Method :
 *      Based on
 *              acosh(x) = log [ x + sqrt(x*x-1) ]
 *      we have
 *              acosh(x) := log(x)+ln2, if x is large; else
 *              acosh(x) := log(2x-1/(sqrt(x*x-1)+x)) if x>2; else
 *              acosh(x) := log1p(t+sqrt(2.0*t+t*t)); where t=x-1.
 *
 * Special cases:
 *      acosh(x) is NaN with signal if x<1.
 *      acosh(NaN) is NaN without signal.
 */
double acosh(double x) {
  static const double
      one = 1.0,
      ln2 = 6.93147180559945286227e-01; /* 0x3FE62E42, 0xFEFA39EF */
  double t;
  int32_t hx;
  uint32_t lx;
  EXTRACT_WORDS(hx, lx, x);
  if (hx < 0x3FF00000) { /* x < 1 */
    return std::numeric_limits<double>::signaling_NaN();
  } else if (hx >= 0x41B00000) { /* x > 2**28 */
    if (hx >= 0x7FF00000) {      /* x is inf of NaN */
      return x + x;
    } else {
      return log(x) + ln2; /* acosh(huge)=log(2x) */
    }
  } else if (((hx - 0x3FF00000) | lx) == 0) {
    return 0.0;                 /* acosh(1) = 0 */
  } else if (hx > 0x40000000) { /* 2**28 > x > 2 */
    t = x * x;
    return log(2.0 * x - one / (x + sqrt(t - one)));
  } else { /* 1<x<2 */
    t = x - one;
    return log1p(t + sqrt(2.0 * t + t * t));
  }
}

double asin(double x) { return LIBC_NAMESPACE::shared::asin(x); }

/* asinh(x)
 * Method :
 *      Based on
 *              asinh(x) = sign(x) * log [ |x| + sqrt(x*x+1) ]
 *      we have
 *      asinh(x) := x  if  1+x*x=1,
 *               := sign(x)*(log(x)+ln2)) for large |x|, else
 *               := sign(x)*log(2|x|+1/(|x|+sqrt(x*x+1))) if|x|>2, else
 *               := sign(x)*log1p(|x| + x^2/(1 + sqrt(1+x^2)))
 */
double asinh(double x) {
  static const double
      one = 1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
      ln2 = 6.93147180559945286227e-01, /* 0x3FE62E42, 0xFEFA39EF */
      huge = 1.00000000000000000000e+300;

  double t, w;
  int32_t hx, ix;
  GET_HIGH_WORD(hx, x);
  ix = hx & 0x7FFFFFFF;
  if (ix >= 0x7FF00000) return x + x; /* x is inf or NaN */
  if (ix < 0x3E300000) {              /* |x|<2**-28 */
    if (huge + x > one) return x;     /* return x inexact except 0 */
  }
  if (ix > 0x41B00000) { /* |x| > 2**28 */
    w = log(fabs(x)) + ln2;
  } else if (ix > 0x40000000) { /* 2**28 > |x| > 2.0 */
    t = fabs(x);
    w = log(2.0 * t + one / (sqrt(x * x + one) + t));
  } else { /* 2.0 > |x| > 2**-28 */
    t = x * x;
    w = log1p(fabs(x) + t / (one + sqrt(one + t)));
  }
  if (hx > 0) {
    return w;
  } else {
    return -w;
  }
}

double atan(double x) { return LIBC_NAMESPACE::shared::atan(x); }
double atan2(double y, double x) { return LIBC_NAMESPACE::shared::atan2(y, x); }

double cos(double x) { return LIBC_NAMESPACE::shared::cos(x); }

double exp(double x) { return LIBC_NAMESPACE::shared::exp(x); }

/*
 * Method :
 *    1.Reduced x to positive by atanh(-x) = -atanh(x)
 *    2.For x>=0.5
 *              1              2x                          x
 *  atanh(x) = --- * log(1 + -------) = 0.5 * log1p(2 * --------)
 *              2             1 - x                      1 - x
 *
 *   For x<0.5
 *  atanh(x) = 0.5*log1p(2x+2x*x/(1-x))
 *
 * Special cases:
 *  atanh(x) is NaN if |x| > 1 with signal;
 *  atanh(NaN) is that NaN with no signal;
 *  atanh(+-1) is +-INF with signal.
 *
 */
double atanh(double x) {
  static const double one = 1.0, huge = 1e300;
  static const double zero = 0.0;

  double t;
  int32_t hx, ix;
  uint32_t lx;
  EXTRACT_WORDS(hx, lx, x);
  ix = hx & 0x7FFFFFFF;
  if ((ix | ((lx | NegateWithWraparound<int32_t>(lx)) >> 31)) > 0x3FF00000) {
    /* |x|>1 */
    return std::numeric_limits<double>::signaling_NaN();
  }
  if (ix == 0x3FF00000) {
    return x > 0 ? std::numeric_limits<double>::infinity()
                 : -std::numeric_limits<double>::infinity();
  }
  if (ix < 0x3E300000 && (huge + x) > zero) return x; /* x<2**-28 */
  SET_HIGH_WORD(x, ix);
  if (ix < 0x3FE00000) { /* x < 0.5 */
    t = x + x;
    t = 0.5 * log1p(t + t * x / (one - x));
  } else {
    t = 0.5 * log1p((x + x) / (one - x));
  }
  if (hx >= 0) {
    return t;
  } else {
    return -t;
  }
}

double log(double x) { return LIBC_NAMESPACE::shared::log(x); }
double log1p(double x) { return LIBC_NAMESPACE::shared::log1p(x); }
double log2(double x) { return LIBC_NAMESPACE::shared::log2(x); }
double log10(double x) { return LIBC_NAMESPACE::shared::log10(x); }

double expm1(double x) { return LIBC_NAMESPACE::shared::expm1(x); }

double cbrt(double x) { return LIBC_NAMESPACE::shared::cbrt(x); }

double sin(double x) { return LIBC_NAMESPACE::shared::sin(x); }

double tan(double x) { return LIBC_NAMESPACE::shared::tan(x); }

double cosh(double x) { return LIBC_NAMESPACE::shared::cosh(x); }

namespace legacy {

double pow(double x, double y) { return LIBC_NAMESPACE::shared::pow(x, y); }

}  // namespace legacy

double sinh(double x) { return LIBC_NAMESPACE::shared::sinh(x); }

// ★ Math.tanh 的公开入口（pyv8web 定制）：转调平台分派。
//
// 所有调用路径都经由这里：
//   - 解释器 / Sparkplug：CSA macro Float64Tanh
//   - TurboFan / Maglev：Ieee754Float64Tanh 指令 → ExternalReference
//   - ExternalReference::ieee754_tanh_function() 指向本符号
// 因此改这一处即可覆盖全部运行期场景。

// ===================================================================
// pyv8web: Math.tanh OS fingerprint dispatch, reworked for V8 16.0.
// 16.0 routes tanh to LLVM libc's shared implementation; default keeps
// that bit pattern, Windows uses the host UCRT std::tanh, macOS applies
// the calibrated ULP offsets (libsystem_m is closed source).
// ===================================================================
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace {
enum TanhPlatform : int32_t {
  kTanhPlatformDefault = 0,
  kTanhPlatformLinux = 1,
  kTanhPlatformWindows = 2,
  kTanhPlatformMacOS = 3,
};
std::atomic<int32_t> g_tanh_platform{kTanhPlatformDefault};

struct TanhUlpEntry { uint64_t key; int32_t delta; };
const TanhUlpEntry kMacOsTanhUlp[] = {
    {0x3FE356FB17AF2E91ull, 0},
    {0x3FE53FCA0A748A40ull, +1},
    {0x3FE6EBE982D6605Cull, +1},
};
inline uint64_t DoubleToBits(double d) {
  uint64_t u; std::memcpy(&u, &d, sizeof(u)); return u;
}
inline double BitsToDouble(uint64_t u) {
  double d; std::memcpy(&d, &u, sizeof(d)); return d;
}
inline double ShiftUlp(double x, int32_t delta) {
  const uint64_t u = DoubleToBits(x);
  const uint64_t sign = u & 0x8000000000000000ull;
  uint64_t mag = u & 0x7FFFFFFFFFFFFFFFull;
  mag = (mag + static_cast<uint64_t>(static_cast<int64_t>(delta))) &
        0x7FFFFFFFFFFFFFFFull;
  return BitsToDouble(sign | mag);
}
}  // namespace

void SetTanhPlatform(int32_t platform) {
  g_tanh_platform.store(platform, std::memory_order_relaxed);
}
int32_t GetTanhPlatform() {
  return g_tanh_platform.load(std::memory_order_relaxed);
}

namespace {
double TanhDispatch(double x) {
  const int32_t plat = g_tanh_platform.load(std::memory_order_relaxed);
  switch (plat) {
    case kTanhPlatformWindows:
      return std::tanh(x);
    case kTanhPlatformMacOS: {
      const double v = std::tanh(x);
      const uint64_t key = DoubleToBits(std::fabs(v));
      for (size_t i = 0; i < sizeof(kMacOsTanhUlp) / sizeof(kMacOsTanhUlp[0]);
           ++i) {
        if (kMacOsTanhUlp[i].key == key)
          return ShiftUlp(v, kMacOsTanhUlp[i].delta);
      }
      return v;
    }
    case kTanhPlatformLinux:
    default:
      return LIBC_NAMESPACE::shared::tanh(x);
  }
}
}  // namespace

double tanh(double x) { return TanhDispatch(x); }

#undef EXTRACT_WORDS
#undef GET_HIGH_WORD
#undef SET_HIGH_WORD


}  // namespace ieee754
}  // namespace base
}  // namespace v8
