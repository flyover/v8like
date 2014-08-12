/// v8/src/globals.h

// Copyright 2012 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//  	 notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//  	 copyright notice, this list of conditions and the following
//  	 disclaimer in the documentation and/or other materials provided
//  	 with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//  	 contributors may be used to endorse or promote products derived
//  	 from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef V8_GLOBALS_H_
#define V8_GLOBALS_H_

// Define V8_INFINITY
#define V8_INFINITY INFINITY

// GCC specific stuff
#ifdef __GNUC__

#define __GNUC_VERSION_FOR_INFTY__ (__GNUC__ * 10000 + __GNUC_MINOR__ * 100)

// Unfortunately, the INFINITY macro cannot be used with the '-pedantic'
// warning flag and certain versions of GCC due to a bug:
// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=11931
// For now, we use the more involved template-based version from <limits>, but
// only when compiling with GCC versions affected by the bug (2.96.x - 4.0.x)
// __GNUC_PREREQ is not defined in GCC for Mac OS X, so we define our own macro
#if __GNUC_VERSION_FOR_INFTY__ >= 29600 && __GNUC_VERSION_FOR_INFTY__ < 40100
#include <limits>
#undef V8_INFINITY
#define V8_INFINITY std::numeric_limits<double>::infinity()
#endif
#undef __GNUC_VERSION_FOR_INFTY__

#endif  // __GNUC__

#ifdef _MSC_VER
#undef V8_INFINITY
#define V8_INFINITY HUGE_VAL
#endif


#include "../include/v8stdint.h"

namespace v8 {
namespace internal {

//TODO	// Processor architecture detection.  For more info on what's defined, see:
//TODO	//   http://msdn.microsoft.com/en-us/library/b0084kay.aspx
//TODO	//   http://www.agner.org/optimize/calling_conventions.pdf
//TODO	//   or with gcc, run: "echo | gcc -E -dM -"
//TODO	#if defined(_M_X64) || defined(__x86_64__)
//TODO	#define V8_HOST_ARCH_X64 1
//TODO	#define V8_HOST_ARCH_64_BIT 1
//TODO	#define V8_HOST_CAN_READ_UNALIGNED 1
//TODO	#elif defined(_M_IX86) || defined(__i386__)
//TODO	#define V8_HOST_ARCH_IA32 1
//TODO	#define V8_HOST_ARCH_32_BIT 1
//TODO	#define V8_HOST_CAN_READ_UNALIGNED 1
//TODO	#elif defined(__ARMEL__)
//TODO	#define V8_HOST_ARCH_ARM 1
//TODO	#define V8_HOST_ARCH_32_BIT 1
//TODO	// Some CPU-OS combinations allow unaligned access on ARM. We assume
//TODO	// that unaligned accesses are not allowed unless the build system
//TODO	// defines the CAN_USE_UNALIGNED_ACCESSES macro to be non-zero.
//TODO	#if CAN_USE_UNALIGNED_ACCESSES
//TODO	#define V8_HOST_CAN_READ_UNALIGNED 1
//TODO	#endif
//TODO	#elif defined(__MIPSEL__)
//TODO	#define V8_HOST_ARCH_MIPS 1
//TODO	#define V8_HOST_ARCH_32_BIT 1
//TODO	#else
//TODO	#error Host architecture was not detected as supported by v8
//TODO	#endif

// Target architecture detection. This may be set externally. If not, detect
// in the same way as the host architecture, that is, target the native
// environment as presented by the compiler.
#if !defined(V8_TARGET_ARCH_X64) && !defined(V8_TARGET_ARCH_IA32) && \
	!defined(V8_TARGET_ARCH_ARM) && !defined(V8_TARGET_ARCH_MIPS)
#if defined(_M_X64) || defined(__x86_64__)
#define V8_TARGET_ARCH_X64 1
#elif defined(_M_IX86) || defined(__i386__)
#define V8_TARGET_ARCH_IA32 1
#elif defined(__ARMEL__)
#define V8_TARGET_ARCH_ARM 1
#elif defined(__MIPSEL__)
#define V8_TARGET_ARCH_MIPS 1
#else
#error Target architecture was not detected as supported by v8
#endif
#endif

//TODO	// Check for supported combinations of host and target architectures.
//TODO	#if defined(V8_TARGET_ARCH_IA32) && !defined(V8_HOST_ARCH_IA32)
//TODO	#error Target architecture ia32 is only supported on ia32 host
//TODO	#endif
//TODO	#if defined(V8_TARGET_ARCH_X64) && !defined(V8_HOST_ARCH_X64)
//TODO	#error Target architecture x64 is only supported on x64 host
//TODO	#endif
//TODO	#if (defined(V8_TARGET_ARCH_ARM) && \
//TODO  	!(defined(V8_HOST_ARCH_IA32) || defined(V8_HOST_ARCH_ARM)))
//TODO	#error Target architecture arm is only supported on arm and ia32 host
//TODO	#endif
//TODO	#if (defined(V8_TARGET_ARCH_MIPS) && \
//TODO  	!(defined(V8_HOST_ARCH_IA32) || defined(V8_HOST_ARCH_MIPS)))
//TODO	#error Target architecture mips is only supported on mips and ia32 host
//TODO	#endif
//TODO	
//TODO	// Determine whether we are running in a simulated environment.
//TODO	// Setting USE_SIMULATOR explicitly from the build script will force
//TODO	// the use of a simulated environment.
//TODO	#if !defined(USE_SIMULATOR)
//TODO	#if (defined(V8_TARGET_ARCH_ARM) && !defined(V8_HOST_ARCH_ARM))
//TODO	#define USE_SIMULATOR 1
//TODO	#endif
//TODO	#if (defined(V8_TARGET_ARCH_MIPS) && !defined(V8_HOST_ARCH_MIPS))
//TODO	#define USE_SIMULATOR 1
//TODO	#endif
//TODO	#endif

// Support for alternative bool type. This is only enabled if the code is
// compiled with USE_MYBOOL defined. This catches some nasty type bugs.
// For instance, 'bool b = "false";' results in b == true! This is a hidden
// source of bugs.
// However, redefining the bool type does have some negative impact on some
// platforms. It gives rise to compiler warnings (i.e. with
// MSVC) in the API header files when mixing code that uses the standard
// bool with code that uses the redefined version.
// This does not actually belong in the platform code, but needs to be
// defined here because the platform code uses bool, and platform.h is
// include very early in the main include file.

#ifdef USE_MYBOOL
typedef unsigned int __my_bool__;
#define bool __my_bool__  // use 'indirection' to avoid name clashes
#endif

typedef uint8_t byte;
typedef byte* Address;

// Define our own macros for writing 64-bit constants.  This is less fragile
// than defining __STDC_CONSTANT_MACROS before including <stdint.h>, and it
// works on compilers that don't have it (like MSVC).
#if V8_HOST_ARCH_64_BIT
#if defined(_MSC_VER)
#define V8_UINT64_C(x)  (x ## UI64)
#define V8_INT64_C(x)   (x ## I64)
#define V8_INTPTR_C(x)  (x ## I64)
#define V8_PTR_PREFIX "ll"
#elif defined(__MINGW64__)
#define V8_UINT64_C(x)  (x ## ULL)
#define V8_INT64_C(x)   (x ## LL)
#define V8_INTPTR_C(x)  (x ## LL)
#define V8_PTR_PREFIX "I64"
#else
#define V8_UINT64_C(x)  (x ## UL)
#define V8_INT64_C(x)   (x ## L)
#define V8_INTPTR_C(x)  (x ## L)
#define V8_PTR_PREFIX "l"
#endif
#else  // V8_HOST_ARCH_64_BIT
#define V8_INTPTR_C(x)  (x)
#define V8_PTR_PREFIX ""
#endif  // V8_HOST_ARCH_64_BIT

// The following macro works on both 32 and 64-bit platforms.
// Usage: instead of writing 0x1234567890123456
//  	write V8_2PART_UINT64_C(0x12345678,90123456);
#define V8_2PART_UINT64_C(a, b) (((static_cast<uint64_t>(a) << 32) + 0x##b##u))

#define V8PRIxPTR V8_PTR_PREFIX "x"
#define V8PRIdPTR V8_PTR_PREFIX "d"
#define V8PRIuPTR V8_PTR_PREFIX "u"

// Fix for Mac OS X defining uintptr_t as "unsigned long":
#if defined(__APPLE__) && defined(__MACH__)
#undef V8PRIxPTR
#define V8PRIxPTR "lx"
#endif

#if (defined(__APPLE__) && defined(__MACH__)) || \
	defined(__FreeBSD__) || defined(__OpenBSD__)
#define USING_BSD_ABI
#endif

// -----------------------------------------------------------------------------
// Constants

const int KB = 1024;
const int MB = KB * KB;
const int GB = KB * KB * KB;
const int kMaxInt = 0x7FFFFFFF;
const int kMinInt = -kMaxInt - 1;

const uint32_t kMaxUInt32 = 0xFFFFFFFFu;

const int kCharSize 	= sizeof(char); 	 // NOLINT
const int kShortSize	= sizeof(short);	 // NOLINT
const int kIntSize  	= sizeof(int);  	 // NOLINT
const int kDoubleSize   = sizeof(double);    // NOLINT
const int kIntptrSize   = sizeof(intptr_t);  // NOLINT
const int kPointerSize  = sizeof(void*);	 // NOLINT

const int kDoubleSizeLog2 = 3;

// Size of the state of a the random number generator.
const int kRandomStateSize = 2 * kIntSize;

#if V8_HOST_ARCH_64_BIT
const int kPointerSizeLog2 = 3;
const intptr_t kIntptrSignBit = V8_INT64_C(0x8000000000000000);
const uintptr_t kUintptrAllBitsSet = V8_UINT64_C(0xFFFFFFFFFFFFFFFF);
#else
const int kPointerSizeLog2 = 2;
const intptr_t kIntptrSignBit = 0x80000000;
const uintptr_t kUintptrAllBitsSet = 0xFFFFFFFFu;
#endif

const int kBitsPerByte = 8;
const int kBitsPerByteLog2 = 3;
const int kBitsPerPointer = kPointerSize * kBitsPerByte;
const int kBitsPerInt = kIntSize * kBitsPerByte;

// IEEE 754 single precision floating point number bit layout.
const uint32_t kBinary32SignMask = 0x80000000u;
const uint32_t kBinary32ExponentMask = 0x7f800000u;
const uint32_t kBinary32MantissaMask = 0x007fffffu;
const int kBinary32ExponentBias = 127;
const int kBinary32MaxExponent  = 0xFE;
const int kBinary32MinExponent  = 0x01;
const int kBinary32MantissaBits = 23;
const int kBinary32ExponentShift = 23;

// Quiet NaNs have bits 51 to 62 set, possibly the sign bit, and no
// other bits set.
const uint64_t kQuietNaNMask = static_cast<uint64_t>(0xfff) << 51;

// ASCII/UTF-16 constants
// Code-point values in Unicode 4.0 are 21 bits wide.
// Code units in UTF-16 are 16 bits wide.
typedef uint16_t uc16;
typedef int32_t uc32;
const int kASCIISize	= kCharSize;
const int kUC16Size 	= sizeof(uc16); 	 // NOLINT
const uc32 kMaxAsciiCharCode = 0x7f;
const uint32_t kMaxAsciiCharCodeU = 0x7fu;


// The expression OFFSET_OF(type, field) computes the byte-offset
// of the specified field relative to the containing type. This
// corresponds to 'offsetof' (in stddef.h), except that it doesn't
// use 0 or NULL, which causes a problem with the compiler warnings
// we have enabled (which is also why 'offsetof' doesn't seem to work).
// Here we simply use the non-zero value 4, which seems to work.
#define OFFSET_OF(type, field)  										\
  (reinterpret_cast<intptr_t>(&(reinterpret_cast<type*>(4)->field)) - 4)


// The expression ARRAY_SIZE(a) is a compile-time constant of type
// size_t which represents the number of elements of the given
// array. You should only use ARRAY_SIZE on statically allocated
// arrays.
#define ARRAY_SIZE(a)   								\
  ((sizeof(a) / sizeof(*(a))) / 						\
  static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))


// The USE(x) template is used to silence C++ compiler warnings
// issued for (yet) unused variables (typically parameters).
template <typename T>
inline void USE(T) { }


// FUNCTION_ADDR(f) gets the address of a C function f.
#define FUNCTION_ADDR(f)										\
  (reinterpret_cast<v8::internal::Address>(reinterpret_cast<intptr_t>(f)))


// FUNCTION_CAST<F>(addr) casts an address into a function
// of type F. Used to invoke generated code from within C.
template <typename F>
F FUNCTION_CAST(Address addr) {
  return reinterpret_cast<F>(reinterpret_cast<intptr_t>(addr));
}


// A macro to disallow the evil copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName)  	\
  TypeName(const TypeName&);					\
  void operator=(const TypeName&)


// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class
// that wants to prevent anyone from instantiating it. This is
// especially useful for classes containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName();   								 \
  DISALLOW_COPY_AND_ASSIGN(TypeName)


// Define used for helping GCC to make better inlining. Don't bother for debug
// builds. On GCC 3.4.5 using __attribute__((always_inline)) causes compilation
// errors in debug build.
#if defined(__GNUC__) && !defined(DEBUG)
#if (__GNUC__ >= 4)
#define INLINE(header) inline header  __attribute__((always_inline))
#define NO_INLINE(header) header __attribute__((noinline))
#else
#define INLINE(header) inline __attribute__((always_inline)) header
#define NO_INLINE(header) __attribute__((noinline)) header
#endif
#elif defined(_MSC_VER) && !defined(DEBUG)
#define INLINE(header) __forceinline header
#define NO_INLINE(header) header
#else
#define INLINE(header) inline header
#define NO_INLINE(header) header
#endif


#if defined(__GNUC__) && __GNUC__ >= 4
#define MUST_USE_RESULT __attribute__ ((warn_unused_result))
#else
#define MUST_USE_RESULT
#endif


// Define DISABLE_ASAN macros.
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define DISABLE_ASAN __attribute__((no_address_safety_analysis))
#endif
#endif


#ifndef DISABLE_ASAN
#define DISABLE_ASAN
#endif


// -----------------------------------------------------------------------------
// Forward declarations for frequently used classes
// (sorted alphabetically)

class FreeStoreAllocationPolicy;
template <typename T, class P = FreeStoreAllocationPolicy> class List;

// -----------------------------------------------------------------------------
// Declarations for use in both the preparser and the rest of V8.

// The different language modes that V8 implements. ES5 defines two language
// modes: an unrestricted mode respectively a strict mode which are indicated by
// CLASSIC_MODE respectively STRICT_MODE in the enum. The harmony spec drafts
// for the next ES standard specify a new third mode which is called 'extended
// mode'. The extended mode is only available if the harmony flag is set. It is
// based on the 'strict mode' and adds new functionality to it. This means that
// most of the semantics of these two modes coincide.
//
// In the current draft the term 'base code' is used to refer to code that is
// neither in strict nor extended mode. However, the more distinguishing term
// 'classic mode' is used in V8 instead to avoid mix-ups.

enum LanguageMode {
  CLASSIC_MODE,
  STRICT_MODE,
  EXTENDED_MODE
};


// The Strict Mode (ECMA-262 5th edition, 4.2.2).
//
// This flag is used in the backend to represent the language mode. So far
// there is no semantic difference between the strict and the extended mode in
// the backend, so both modes are represented by the kStrictMode value.
enum StrictModeFlag {
  kNonStrictMode,
  kStrictMode
};


} }  // namespace v8::internal

#endif  // V8_GLOBALS_H_

/// v8/src/checks.h

// Copyright 2012 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//  	 notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//  	 copyright notice, this list of conditions and the following
//  	 disclaimer in the documentation and/or other materials provided
//  	 with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//  	 contributors may be used to endorse or promote products derived
//  	 from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef V8_CHECKS_H_
#define V8_CHECKS_H_

#include <string.h>

#include "../include/v8stdint.h"
extern "C" void V8_Fatal(const char* file, int line, const char* format, ...);

// The FATAL, UNREACHABLE and UNIMPLEMENTED macros are useful during
// development, but they should not be relied on in the final product.
#ifdef DEBUG
#define FATAL(msg)  							\
  V8_Fatal(__FILE__, __LINE__, "%s", (msg))
#define UNIMPLEMENTED() 						\
  V8_Fatal(__FILE__, __LINE__, "unimplemented code")
#define UNREACHABLE()   						\
  V8_Fatal(__FILE__, __LINE__, "unreachable code")
#else
#define FATAL(msg)  							\
  V8_Fatal("", 0, "%s", (msg))
#define UNIMPLEMENTED() 						\
  V8_Fatal("", 0, "unimplemented code")
#define UNREACHABLE() ((void) 0)
#endif


// The CHECK macro checks that the given condition is true; if not, it
// prints a message to stderr and aborts.
#define CHECK(condition) do {   									\
	if (!(condition)) { 											\
	  V8_Fatal(__FILE__, __LINE__, "CHECK(%s) failed", #condition); \
	}   															\
  } while (0)


// Helper function used by the CHECK_EQ function when given int
// arguments.  Should not be called directly.
inline void CheckEqualsHelper(const char* file, int line,
							  const char* expected_source, int expected,
							  const char* value_source, int value) {
  if (expected != value) {
	V8_Fatal(file, line,
			 "CHECK_EQ(%s, %s) failed\n#   Expected: %i\n#   Found: %i",
			 expected_source, value_source, expected, value);
  }
}


// Helper function used by the CHECK_EQ function when given int64_t
// arguments.  Should not be called directly.
inline void CheckEqualsHelper(const char* file, int line,
							  const char* expected_source,
							  int64_t expected,
							  const char* value_source,
							  int64_t value) {
  if (expected != value) {
	// Print int64_t values in hex, as two int32s,
	// to avoid platform-dependencies.
	V8_Fatal(file, line,
			 "CHECK_EQ(%s, %s) failed\n#"
			 "   Expected: 0x%08x%08x\n#   Found: 0x%08x%08x",
			 expected_source, value_source,
			 static_cast<uint32_t>(expected >> 32),
			 static_cast<uint32_t>(expected),
			 static_cast<uint32_t>(value >> 32),
			 static_cast<uint32_t>(value));
  }
}


// Helper function used by the CHECK_NE function when given int
// arguments.  Should not be called directly.
inline void CheckNonEqualsHelper(const char* file,
								 int line,
								 const char* unexpected_source,
								 int unexpected,
								 const char* value_source,
								 int value) {
  if (unexpected == value) {
	V8_Fatal(file, line, "CHECK_NE(%s, %s) failed\n#   Value: %i",
			 unexpected_source, value_source, value);
  }
}


// Helper function used by the CHECK function when given string
// arguments.  Should not be called directly.
inline void CheckEqualsHelper(const char* file,
							  int line,
							  const char* expected_source,
							  const char* expected,
							  const char* value_source,
							  const char* value) {
  if ((expected == NULL && value != NULL) ||
	  (expected != NULL && value == NULL) ||
	  (expected != NULL && value != NULL && strcmp(expected, value) != 0)) {
	V8_Fatal(file, line,
			 "CHECK_EQ(%s, %s) failed\n#   Expected: %s\n#   Found: %s",
			 expected_source, value_source, expected, value);
  }
}


inline void CheckNonEqualsHelper(const char* file,
								 int line,
								 const char* expected_source,
								 const char* expected,
								 const char* value_source,
								 const char* value) {
  if (expected == value ||
	  (expected != NULL && value != NULL && strcmp(expected, value) == 0)) {
	V8_Fatal(file, line, "CHECK_NE(%s, %s) failed\n#   Value: %s",
			 expected_source, value_source, value);
  }
}


// Helper function used by the CHECK function when given pointer
// arguments.  Should not be called directly.
inline void CheckEqualsHelper(const char* file,
							  int line,
							  const char* expected_source,
							  const void* expected,
							  const char* value_source,
							  const void* value) {
  if (expected != value) {
	V8_Fatal(file, line,
			 "CHECK_EQ(%s, %s) failed\n#   Expected: %p\n#   Found: %p",
			 expected_source, value_source,
			 expected, value);
  }
}


inline void CheckNonEqualsHelper(const char* file,
								 int line,
								 const char* expected_source,
								 const void* expected,
								 const char* value_source,
								 const void* value) {
  if (expected == value) {
	V8_Fatal(file, line, "CHECK_NE(%s, %s) failed\n#   Value: %p",
			 expected_source, value_source, value);
  }
}


// Helper function used by the CHECK function when given floating
// point arguments.  Should not be called directly.
inline void CheckEqualsHelper(const char* file,
							  int line,
							  const char* expected_source,
							  double expected,
							  const char* value_source,
							  double value) {
  // Force values to 64 bit memory to truncate 80 bit precision on IA32.
  volatile double* exp = new double[1];
  *exp = expected;
  volatile double* val = new double[1];
  *val = value;
  if (*exp != *val) {
	V8_Fatal(file, line,
			 "CHECK_EQ(%s, %s) failed\n#   Expected: %f\n#   Found: %f",
			 expected_source, value_source, *exp, *val);
  }
  delete[] exp;
  delete[] val;
}


inline void CheckNonEqualsHelper(const char* file,
								 int line,
								 const char* expected_source,
								 double expected,
								 const char* value_source,
								 double value) {
  // Force values to 64 bit memory to truncate 80 bit precision on IA32.
  volatile double* exp = new double[1];
  *exp = expected;
  volatile double* val = new double[1];
  *val = value;
  if (*exp == *val) {
	V8_Fatal(file, line,
			 "CHECK_NE(%s, %s) failed\n#   Value: %f",
			 expected_source, value_source, *val);
  }
  delete[] exp;
  delete[] val;
}


#define CHECK_EQ(expected, value) CheckEqualsHelper(__FILE__, __LINE__, \
  #expected, expected, #value, value)


#define CHECK_NE(unexpected, value) CheckNonEqualsHelper(__FILE__, __LINE__, \
  #unexpected, unexpected, #value, value)


#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))


// This is inspired by the static assertion facility in boost.  This
// is pretty magical.  If it causes you trouble on a platform you may
// find a fix in the boost code.
template <bool> class StaticAssertion;
template <> class StaticAssertion<true> { };
// This macro joins two tokens.  If one of the tokens is a macro the
// helper call causes it to be resolved before joining.
#define SEMI_STATIC_JOIN(a, b) SEMI_STATIC_JOIN_HELPER(a, b)
#define SEMI_STATIC_JOIN_HELPER(a, b) a##b
// Causes an error during compilation of the condition is not
// statically known to be true.  It is formulated as a typedef so that
// it can be used wherever a typedef can be used.  Beware that this
// actually causes each use to introduce a new defined type with a
// name depending on the source line.
template <int> class StaticAssertionHelper { };
#define STATIC_CHECK(test)  												  \
  typedef   																  \
	StaticAssertionHelper<sizeof(StaticAssertion<static_cast<bool>((test))>)> \
	SEMI_STATIC_JOIN(__StaticAssertTypedef__, __LINE__)


extern bool FLAG_enable_slow_asserts;


// The ASSERT macro is equivalent to CHECK except that it only
// generates code in debug builds.
#ifdef DEBUG
#define ASSERT_RESULT(expr)    CHECK(expr)
#define ASSERT(condition)      CHECK(condition)
#define ASSERT_EQ(v1, v2)      CHECK_EQ(v1, v2)
#define ASSERT_NE(v1, v2)      CHECK_NE(v1, v2)
#define ASSERT_GE(v1, v2)      CHECK_GE(v1, v2)
#define ASSERT_LT(v1, v2)      CHECK_LT(v1, v2)
#define ASSERT_LE(v1, v2)      CHECK_LE(v1, v2)
#define SLOW_ASSERT(condition) CHECK(!FLAG_enable_slow_asserts || (condition))
#else
#define ASSERT_RESULT(expr)    (expr)
#define ASSERT(condition)      ((void) 0)
#define ASSERT_EQ(v1, v2)      ((void) 0)
#define ASSERT_NE(v1, v2)      ((void) 0)
#define ASSERT_GE(v1, v2)      ((void) 0)
#define ASSERT_LT(v1, v2)      ((void) 0)
#define ASSERT_LE(v1, v2)      ((void) 0)
#define SLOW_ASSERT(condition) ((void) 0)
#endif
// Static asserts has no impact on runtime performance, so they can be
// safely enabled in release mode. Moreover, the ((void) 0) expression
// obeys different syntax rules than typedef's, e.g. it can't appear
// inside class declaration, this leads to inconsistency between debug
// and release compilation modes behavior.
#define STATIC_ASSERT(test)  STATIC_CHECK(test)

#define ASSERT_NOT_NULL(p)  ASSERT_NE(NULL, p)

// "Extra checks" are lightweight checks that are enabled in some release
// builds.
#ifdef ENABLE_EXTRA_CHECKS
#define EXTRA_CHECK(condition) CHECK(condition)
#else
#define EXTRA_CHECK(condition) ((void) 0)
#endif

#endif  // V8_CHECKS_H_

/// v8/src/checks.cc

// Copyright 2006-2008 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//TODO	#include <stdarg.h>
//TODO	
//TODO	#include "v8.h"
//TODO	
//TODO	#include "platform.h"
//TODO	
//TODO	// TODO(isolates): is it necessary to lift this?
//TODO	static int fatal_error_handler_nesting_depth = 0;
//TODO	
//TODO	// Contains protection against recursive calls (faults while handling faults).
//TODO	extern "C" void V8_Fatal(const char* file, int line, const char* format, ...) {
//TODO	  fflush(stdout);
//TODO	  fflush(stderr);
//TODO	  fatal_error_handler_nesting_depth++;
//TODO	  // First time we try to print an error message
//TODO	  if (fatal_error_handler_nesting_depth < 2) {
//TODO	    i::OS::PrintError("\n\n#\n# Fatal error in %s, line %d\n# ", file, line);
//TODO	    va_list arguments;
//TODO	    va_start(arguments, format);
//TODO	    i::OS::VPrintError(format, arguments);
//TODO	    va_end(arguments);
//TODO	    i::OS::PrintError("\n#\n\n");
//TODO	  }
//TODO	  // First two times we may try to print a stack dump.
//TODO	  if (fatal_error_handler_nesting_depth < 3) {
//TODO	    if (i::FLAG_stack_trace_on_abort) {
//TODO	      // Call this one twice on double fault
//TODO	      i::Isolate::Current()->PrintStack();
//TODO	    }
//TODO	  }
//TODO	  i::OS::Abort();
//TODO	}
//TODO	
//TODO	
//TODO	void CheckEqualsHelper(const char* file,
//TODO	                       int line,
//TODO	                       const char* expected_source,
//TODO	                       v8::Handle<v8::Value> expected,
//TODO	                       const char* value_source,
//TODO	                       v8::Handle<v8::Value> value) {
//TODO	  if (!expected->Equals(value)) {
//TODO	    v8::String::Utf8Value value_str(value);
//TODO	    v8::String::Utf8Value expected_str(expected);
//TODO	    V8_Fatal(file, line,
//TODO	             "CHECK_EQ(%s, %s) failed\n#   Expected: %s\n#   Found: %s",
//TODO	             expected_source, value_source, *expected_str, *value_str);
//TODO	  }
//TODO	}
//TODO	
//TODO	
//TODO	void CheckNonEqualsHelper(const char* file,
//TODO	                          int line,
//TODO	                          const char* unexpected_source,
//TODO	                          v8::Handle<v8::Value> unexpected,
//TODO	                          const char* value_source,
//TODO	                          v8::Handle<v8::Value> value) {
//TODO	  if (unexpected->Equals(value)) {
//TODO	    v8::String::Utf8Value value_str(value);
//TODO	    V8_Fatal(file, line, "CHECK_NE(%s, %s) failed\n#   Value: %s",
//TODO	             unexpected_source, value_source, *value_str);
//TODO	  }
//TODO	}
//TODO	
//TODO	
//TODO	void API_Fatal(const char* location, const char* format, ...) {
//TODO	  i::OS::PrintError("\n#\n# Fatal error in %s\n# ", location);
//TODO	  va_list arguments;
//TODO	  va_start(arguments, format);
//TODO	  i::OS::VPrintError(format, arguments);
//TODO	  va_end(arguments);
//TODO	  i::OS::PrintError("\n#\n\n");
//TODO	  i::OS::Abort();
//TODO	}
//TODO	
//TODO	
//TODO	namespace v8 { namespace internal {
//TODO	
//TODO	  bool EnableSlowAsserts() { return FLAG_enable_slow_asserts; }
//TODO	
//TODO	  intptr_t HeapObjectTagMask() { return kHeapObjectTagMask; }
//TODO	
//TODO	} }  // namespace v8::internal

/// v8/src/allocation.h

// Copyright 2012 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are

// met:
//
//     * Redistributions of source code must retain the above copyright
//  	 notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//  	 copyright notice, this list of conditions and the following
//  	 disclaimer in the documentation and/or other materials provided
//  	 with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//  	 contributors may be used to endorse or promote products derived
//  	 from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef V8_ALLOCATION_H_
#define V8_ALLOCATION_H_

//TODO	#include "globals.h"

namespace v8 {
namespace internal {

// Called when allocation routines fail to allocate.
// This function should not return, but should terminate the current
// processing.
void FatalProcessOutOfMemory(const char* message);

// Superclass for classes managed with new & delete.
class Malloced {
 public:
  void* operator new(size_t size) { return New(size); }
  void  operator delete(void* p) { Delete(p); }

  static void FatalProcessOutOfMemory();
  static void* New(size_t size);
  static void Delete(void* p);
};


// A macro is used for defining the base class used for embedded instances.
// The reason is some compilers allocate a minimum of one word for the
// superclass. The macro prevents the use of new & delete in debug mode.
// In release mode we are not willing to pay this overhead.

#ifdef DEBUG
// Superclass for classes with instances allocated inside stack
// activations or inside other objects.
class Embedded {
 public:
  void* operator new(size_t size);
  void  operator delete(void* p);
};
#define BASE_EMBEDDED : public Embedded
#else
#define BASE_EMBEDDED
#endif


// Superclass for classes only using statics.
class AllStatic {
#ifdef DEBUG
 public:
  void* operator new(size_t size);
  void operator delete(void* p);
#endif
};


template <typename T>
T* NewArray(size_t size) {
  T* result = new T[size];
  if (result == NULL) Malloced::FatalProcessOutOfMemory();
  return result;
}


template <typename T>
void DeleteArray(T* array) {
  delete[] array;
}


// The normal strdup functions use malloc.  These versions of StrDup
// and StrNDup uses new and calls the FatalProcessOutOfMemory handler
// if allocation fails.
char* StrDup(const char* str);
char* StrNDup(const char* str, int n);


// Allocation policy for allocating in the C free store using malloc
// and free. Used as the default policy for lists.
class FreeStoreAllocationPolicy {
 public:
  INLINE(void* New(size_t size)) { return Malloced::New(size); }
  INLINE(static void Delete(void* p)) { Malloced::Delete(p); }
};


// Allocation policy for allocating in preallocated space.
// Used as an allocation policy for ScopeInfo when generating
// stack traces.
class PreallocatedStorage {
 public:
  explicit PreallocatedStorage(size_t size);
  size_t size() { return size_; }

 private:
  size_t size_;
  PreallocatedStorage* previous_;
  PreallocatedStorage* next_;

  void LinkTo(PreallocatedStorage* other);
  void Unlink();

  friend class Isolate;

  DISALLOW_IMPLICIT_CONSTRUCTORS(PreallocatedStorage);
};


struct PreallocatedStorageAllocationPolicy {
  INLINE(void* New(size_t size));
  INLINE(static void Delete(void* ptr));
};


} }  // namespace v8::internal

#endif  // V8_ALLOCATION_H_

/// v8/src/allocation-inl.h

// Copyright 2010 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef V8_ALLOCATION_INL_H_
#define V8_ALLOCATION_INL_H_

//TODO	#include "allocation.h"
//TODO	
//TODO	namespace v8 {
//TODO	namespace internal {
//TODO	
//TODO	
//TODO	void* PreallocatedStorageAllocationPolicy::New(size_t size) {
//TODO	  return Isolate::Current()->PreallocatedStorageNew(size);
//TODO	}
//TODO	
//TODO	
//TODO	void PreallocatedStorageAllocationPolicy::Delete(void* p) {
//TODO	  return Isolate::Current()->PreallocatedStorageDelete(p);
//TODO	}
//TODO	
//TODO	
//TODO	} }  // namespace v8::internal

#endif  // V8_ALLOCATION_INL_H_

/// v8/src/utils.h

// Copyright 2012 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//  	 notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//  	 copyright notice, this list of conditions and the following
//  	 disclaimer in the documentation and/or other materials provided
//  	 with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//  	 contributors may be used to endorse or promote products derived
//  	 from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef V8_UTILS_H_
#define V8_UTILS_H_

#include <stdlib.h>
#include <string.h>
#include <climits>

//TODO	#include "globals.h"
//TODO	#include "checks.h"
//TODO	#include "allocation.h"

namespace v8 {
namespace internal {

// ----------------------------------------------------------------------------
// General helper functions

#define IS_POWER_OF_TWO(x) (((x) & ((x) - 1)) == 0)

// Returns true iff x is a power of 2 (or zero). Cannot be used with the
// maximally negative value of the type T (the -1 overflows).
template <typename T>
inline bool IsPowerOf2(T x) {
  return IS_POWER_OF_TWO(x);
}


// X must be a power of 2.  Returns the number of trailing zeros.
inline int WhichPowerOf2(uint32_t x) {
  ASSERT(IsPowerOf2(x));
  ASSERT(x != 0);
  int bits = 0;
#ifdef DEBUG
  int original_x = x;
#endif
  if (x >= 0x10000) {
	bits += 16;
	x >>= 16;
  }
  if (x >= 0x100) {
	bits += 8;
	x >>= 8;
  }
  if (x >= 0x10) {
	bits += 4;
	x >>= 4;
  }
  switch (x) {
	default: UNREACHABLE();
	case 8: bits++;  // Fall through.
	case 4: bits++;  // Fall through.
	case 2: bits++;  // Fall through.
	case 1: break;
  }
  ASSERT_EQ(1 << bits, original_x);
  return bits;
  return 0;
}


// Magic numbers for integer division.
// These are kind of 2's complement reciprocal of the divisors.
// Details and proofs can be found in:
// - Hacker's Delight, Henry S. Warren, Jr.
// - The PowerPC Compiler Writer’s Guide
// and probably many others.
// See details in the implementation of the algorithm in
// lithium-codegen-arm.cc : LCodeGen::TryEmitSignedIntegerDivisionByConstant().
struct DivMagicNumbers {
  unsigned M;
  unsigned s;
};

const DivMagicNumbers InvalidDivMagicNumber= {0, 0};
const DivMagicNumbers DivMagicNumberFor3   = {0x55555556, 0};
const DivMagicNumbers DivMagicNumberFor5   = {0x66666667, 1};
const DivMagicNumbers DivMagicNumberFor7   = {0x92492493, 2};
const DivMagicNumbers DivMagicNumberFor9   = {0x38e38e39, 1};
const DivMagicNumbers DivMagicNumberFor11  = {0x2e8ba2e9, 1};
const DivMagicNumbers DivMagicNumberFor25  = {0x51eb851f, 3};
const DivMagicNumbers DivMagicNumberFor125 = {0x10624dd3, 3};
const DivMagicNumbers DivMagicNumberFor625 = {0x68db8bad, 8};

const DivMagicNumbers DivMagicNumberFor(int32_t divisor);


// The C++ standard leaves the semantics of '>>' undefined for
// negative signed operands. Most implementations do the right thing,
// though.
inline int ArithmeticShiftRight(int x, int s) {
  return x >> s;
}


// Compute the 0-relative offset of some absolute value x of type T.
// This allows conversion of Addresses and integral types into
// 0-relative int offsets.
template <typename T>
inline intptr_t OffsetFrom(T x) {
  return x - static_cast<T>(0);
}


// Compute the absolute value of type T for some 0-relative offset x.
// This allows conversion of 0-relative int offsets into Addresses and
// integral types.
template <typename T>
inline T AddressFrom(intptr_t x) {
  return static_cast<T>(static_cast<T>(0) + x);
}


// Return the largest multiple of m which is <= x.
template <typename T>
inline T RoundDown(T x, intptr_t m) {
  ASSERT(IsPowerOf2(m));
  return AddressFrom<T>(OffsetFrom(x) & -m);
}


// Return the smallest multiple of m which is >= x.
template <typename T>
inline T RoundUp(T x, intptr_t m) {
  return RoundDown<T>(static_cast<T>(x + m - 1), m);
}


template <typename T>
int Compare(const T& a, const T& b) {
  if (a == b)
	return 0;
  else if (a < b)
	return -1;
  else
	return 1;
}


template <typename T>
int PointerValueCompare(const T* a, const T* b) {
  return Compare<T>(*a, *b);
}


//TODO	// Compare function to compare the object pointer value of two
//TODO	// handlified objects. The handles are passed as pointers to the
//TODO	// handles.
//TODO	template<typename T> class Handle;  // Forward declaration.
//TODO	template <typename T>
//TODO	int HandleObjectPointerCompare(const Handle<T>* a, const Handle<T>* b) {
//TODO    return Compare<T*>(*(*a), *(*b));
//TODO	}


// Returns the smallest power of two which is >= x. If you pass in a
// number that is already a power of two, it is returned as is.
// Implementation is from "Hacker's Delight" by Henry S. Warren, Jr.,
// figure 3-3, page 48, where the function is called clp2.
inline uint32_t RoundUpToPowerOf2(uint32_t x) {
  ASSERT(x <= 0x80000000u);
  x = x - 1;
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x + 1;
}


inline uint32_t RoundDownToPowerOf2(uint32_t x) {
  uint32_t rounded_up = RoundUpToPowerOf2(x);
  if (rounded_up > x) return rounded_up >> 1;
  return rounded_up;
}


template <typename T, typename U>
inline bool IsAligned(T value, U alignment) {
  return (value & (alignment - 1)) == 0;
}


// Returns true if (addr + offset) is aligned.
inline bool IsAddressAligned(Address addr,
							 intptr_t alignment,
							 int offset = 0) {
  intptr_t offs = OffsetFrom(addr + offset);
  return IsAligned(offs, alignment);
}


// Returns the maximum of the two parameters.
template <typename T>
T Max(T a, T b) {
  return a < b ? b : a;
}


// Returns the minimum of the two parameters.
template <typename T>
T Min(T a, T b) {
  return a < b ? a : b;
}


inline int StrLength(const char* string) {
  size_t length = strlen(string);
  ASSERT(length == static_cast<size_t>(static_cast<int>(length)));
  return static_cast<int>(length);
}


// ----------------------------------------------------------------------------
// BitField is a help template for encoding and decode bitfield with
// unsigned content.
template<class T, int shift, int size>
class BitField {
 public:
  // A uint32_t mask of bit field.  To use all bits of a uint32 in a
  // bitfield without compiler warnings we have to compute 2^32 without
  // using a shift count of 32.
  static const uint32_t kMask = ((1U << shift) << size) - (1U << shift);
  static const uint32_t kShift = shift;

  // Value for the field with all bits set.
  static const T kMax = static_cast<T>((1U << size) - 1);

  // Tells whether the provided value fits into the bit field.
  static bool is_valid(T value) {
	return (static_cast<uint32_t>(value) & ~static_cast<uint32_t>(kMax)) == 0;
  }

  // Returns a uint32_t with the bit field value encoded.
  static uint32_t encode(T value) {
	ASSERT(is_valid(value));
	return static_cast<uint32_t>(value) << shift;
  }

  // Returns a uint32_t with the bit field value updated.
  static uint32_t update(uint32_t previous, T value) {
	return (previous & ~kMask) | encode(value);
  }

  // Extracts the bit field from the value.
  static T decode(uint32_t value) {
	return static_cast<T>((value & kMask) >> shift);
  }
};


// ----------------------------------------------------------------------------
// Hash function.

static const uint32_t kZeroHashSeed = 0;

// Thomas Wang, Integer Hash Functions.
// http://www.concentric.net/~Ttwang/tech/inthash.htm
inline uint32_t ComputeIntegerHash(uint32_t key, uint32_t seed) {
  uint32_t hash = key;
  hash = hash ^ seed;
  hash = ~hash + (hash << 15);  // hash = (hash << 15) - hash - 1;
  hash = hash ^ (hash >> 12);
  hash = hash + (hash << 2);
  hash = hash ^ (hash >> 4);
  hash = hash * 2057;  // hash = (hash + (hash << 3)) + (hash << 11);
  hash = hash ^ (hash >> 16);
  return hash;
}


inline uint32_t ComputeLongHash(uint64_t key) {
  uint64_t hash = key;
  hash = ~hash + (hash << 18);  // hash = (hash << 18) - hash - 1;
  hash = hash ^ (hash >> 31);
  hash = hash * 21;  // hash = (hash + (hash << 2)) + (hash << 4);
  hash = hash ^ (hash >> 11);
  hash = hash + (hash << 6);
  hash = hash ^ (hash >> 22);
  return (uint32_t) hash;
}


inline uint32_t ComputePointerHash(void* ptr) {
  return ComputeIntegerHash(
	  static_cast<uint32_t>(reinterpret_cast<intptr_t>(ptr)),
	  v8::internal::kZeroHashSeed);
}


// ----------------------------------------------------------------------------
// Miscellaneous

// A static resource holds a static instance that can be reserved in
// a local scope using an instance of Access.  Attempts to re-reserve
// the instance will cause an error.
template <typename T>
class StaticResource {
 public:
  StaticResource() : is_reserved_(false)  {}

 private:
  template <typename S> friend class Access;
  T instance_;
  bool is_reserved_;
};


// Locally scoped access to a static resource.
template <typename T>
class Access {
 public:
  explicit Access(StaticResource<T>* resource)
	: resource_(resource)
	, instance_(&resource->instance_) {
	ASSERT(!resource->is_reserved_);
	resource->is_reserved_ = true;
  }

  ~Access() {
	resource_->is_reserved_ = false;
	resource_ = NULL;
	instance_ = NULL;
  }

  T* value()  { return instance_; }
  T* operator -> ()  { return instance_; }

 private:
  StaticResource<T>* resource_;
  T* instance_;
};


template <typename T>
class Vector {
 public:
  Vector() : start_(NULL), length_(0) {}
  Vector(T* data, int length) : start_(data), length_(length) {
	ASSERT(length == 0 || (length > 0 && data != NULL));
  }

  static Vector<T> New(int length) {
	return Vector<T>(NewArray<T>(length), length);
  }

  // Returns a vector using the same backing storage as this one,
  // spanning from and including 'from', to but not including 'to'.
  Vector<T> SubVector(int from, int to) {
	ASSERT(to <= length_);
	ASSERT(from < to);
	ASSERT(0 <= from);
	return Vector<T>(start() + from, to - from);
  }

  // Returns the length of the vector.
  int length() const { return length_; }

  // Returns whether or not the vector is empty.
  bool is_empty() const { return length_ == 0; }

  // Returns the pointer to the start of the data in the vector.
  T* start() const { return start_; }

  // Access individual vector elements - checks bounds in debug mode.
  T& operator[](int index) const {
	ASSERT(0 <= index && index < length_);
	return start_[index];
  }

  const T& at(int index) const { return operator[](index); }

  T& first() { return start_[0]; }

  T& last() { return start_[length_ - 1]; }

  // Returns a clone of this vector with a new backing store.
  Vector<T> Clone() const {
	T* result = NewArray<T>(length_);
	for (int i = 0; i < length_; i++) result[i] = start_[i];
	return Vector<T>(result, length_);
  }

  void Sort(int (*cmp)(const T*, const T*)) {
	typedef int (*RawComparer)(const void*, const void*);
	qsort(start(),
		  length(),
		  sizeof(T),
		  reinterpret_cast<RawComparer>(cmp));
  }

  void Sort() {
	Sort(PointerValueCompare<T>);
  }

  void Truncate(int length) {
	ASSERT(length <= length_);
	length_ = length;
  }

  // Releases the array underlying this vector. Once disposed the
  // vector is empty.
  void Dispose() {
	DeleteArray(start_);
	start_ = NULL;
	length_ = 0;
  }

  inline Vector<T> operator+(int offset) {
	ASSERT(offset < length_);
	return Vector<T>(start_ + offset, length_ - offset);
  }

  // Factory method for creating empty vectors.
  static Vector<T> empty() { return Vector<T>(NULL, 0); }

  template<typename S>
  static Vector<T> cast(Vector<S> input) {
	return Vector<T>(reinterpret_cast<T*>(input.start()),
					 input.length() * sizeof(S) / sizeof(T));
  }

 protected:
  void set_start(T* start) { start_ = start; }

 private:
  T* start_;
  int length_;
};


// A pointer that can only be set once and doesn't allow NULL values.
template<typename T>
class SetOncePointer {
 public:
  SetOncePointer() : pointer_(NULL) { }

  bool is_set() const { return pointer_ != NULL; }

  T* get() const {
	ASSERT(pointer_ != NULL);
	return pointer_;
  }

  void set(T* value) {
	ASSERT(pointer_ == NULL && value != NULL);
	pointer_ = value;
  }

 private:
  T* pointer_;
};


template <typename T, int kSize>
class EmbeddedVector : public Vector<T> {
 public:
  EmbeddedVector() : Vector<T>(buffer_, kSize) { }

  explicit EmbeddedVector(T initial_value) : Vector<T>(buffer_, kSize) {
	for (int i = 0; i < kSize; ++i) {
	  buffer_[i] = initial_value;
	}
  }

  // When copying, make underlying Vector to reference our buffer.
  EmbeddedVector(const EmbeddedVector& rhs)
	  : Vector<T>(rhs) {
	memcpy(buffer_, rhs.buffer_, sizeof(T) * kSize);
	set_start(buffer_);
  }

  EmbeddedVector& operator=(const EmbeddedVector& rhs) {
	if (this == &rhs) return *this;
	Vector<T>::operator=(rhs);
	memcpy(buffer_, rhs.buffer_, sizeof(T) * kSize);
	this->set_start(buffer_);
	return *this;
  }

 private:
  T buffer_[kSize];
};


template <typename T>
class ScopedVector : public Vector<T> {
 public:
  explicit ScopedVector(int length) : Vector<T>(NewArray<T>(length), length) { }
  ~ScopedVector() {
	DeleteArray(this->start());
  }

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ScopedVector);
};


inline Vector<const char> CStrVector(const char* data) {
  return Vector<const char>(data, StrLength(data));
}

inline Vector<char> MutableCStrVector(char* data) {
  return Vector<char>(data, StrLength(data));
}

inline Vector<char> MutableCStrVector(char* data, int max) {
  int length = StrLength(data);
  return Vector<char>(data, (length < max) ? length : max);
}


/*
 * A class that collects values into a backing store.
 * Specialized versions of the class can allow access to the backing store
 * in different ways.
 * There is no guarantee that the backing store is contiguous (and, as a
 * consequence, no guarantees that consecutively added elements are adjacent
 * in memory). The collector may move elements unless it has guaranteed not
 * to.
 */
template <typename T, int growth_factor = 2, int max_growth = 1 * MB>
class Collector {
 public:
  explicit Collector(int initial_capacity = kMinCapacity)
	  : index_(0), size_(0) {
	current_chunk_ = Vector<T>::New(initial_capacity);
  }

  virtual ~Collector() {
	// Free backing store (in reverse allocation order).
	current_chunk_.Dispose();
	for (int i = chunks_.length() - 1; i >= 0; i--) {
	  chunks_.at(i).Dispose();
	}
  }

  // Add a single element.
  inline void Add(T value) {
	if (index_ >= current_chunk_.length()) {
	  Grow(1);
	}
	current_chunk_[index_] = value;
	index_++;
	size_++;
  }

  // Add a block of contiguous elements and return a Vector backed by the
  // memory area.
  // A basic Collector will keep this vector valid as long as the Collector
  // is alive.
  inline Vector<T> AddBlock(int size, T initial_value) {
	ASSERT(size > 0);
	if (size > current_chunk_.length() - index_) {
	  Grow(size);
	}
	T* position = current_chunk_.start() + index_;
	index_ += size;
	size_ += size;
	for (int i = 0; i < size; i++) {
	  position[i] = initial_value;
	}
	return Vector<T>(position, size);
  }


  // Add a contiguous block of elements and return a vector backed
  // by the added block.
  // A basic Collector will keep this vector valid as long as the Collector
  // is alive.
  inline Vector<T> AddBlock(Vector<const T> source) {
	if (source.length() > current_chunk_.length() - index_) {
	  Grow(source.length());
	}
	T* position = current_chunk_.start() + index_;
	index_ += source.length();
	size_ += source.length();
	for (int i = 0; i < source.length(); i++) {
	  position[i] = source[i];
	}
	return Vector<T>(position, source.length());
  }


  // Write the contents of the collector into the provided vector.
  void WriteTo(Vector<T> destination) {
	ASSERT(size_ <= destination.length());
	int position = 0;
	for (int i = 0; i < chunks_.length(); i++) {
	  Vector<T> chunk = chunks_.at(i);
	  for (int j = 0; j < chunk.length(); j++) {
		destination[position] = chunk[j];
		position++;
	  }
	}
	for (int i = 0; i < index_; i++) {
	  destination[position] = current_chunk_[i];
	  position++;
	}
  }

  // Allocate a single contiguous vector, copy all the collected
  // elements to the vector, and return it.
  // The caller is responsible for freeing the memory of the returned
  // vector (e.g., using Vector::Dispose).
  Vector<T> ToVector() {
	Vector<T> new_store = Vector<T>::New(size_);
	WriteTo(new_store);
	return new_store;
  }

  // Resets the collector to be empty.
  virtual void Reset();

  // Total number of elements added to collector so far.
  inline int size() { return size_; }

 protected:
  static const int kMinCapacity = 16;
  List<Vector<T> > chunks_;
  Vector<T> current_chunk_;  // Block of memory currently being written into.
  int index_;  // Current index in current chunk.
  int size_;  // Total number of elements in collector.

  // Creates a new current chunk, and stores the old chunk in the chunks_ list.
  void Grow(int min_capacity) {
	ASSERT(growth_factor > 1);
	int new_capacity;
	int current_length = current_chunk_.length();
	if (current_length < kMinCapacity) {
	  // The collector started out as empty.
	  new_capacity = min_capacity * growth_factor;
	  if (new_capacity < kMinCapacity) new_capacity = kMinCapacity;
	} else {
	  int growth = current_length * (growth_factor - 1);
	  if (growth > max_growth) {
		growth = max_growth;
	  }
	  new_capacity = current_length + growth;
	  if (new_capacity < min_capacity) {
		new_capacity = min_capacity + growth;
	  }
	}
	NewChunk(new_capacity);
	ASSERT(index_ + min_capacity <= current_chunk_.length());
  }

  // Before replacing the current chunk, give a subclass the option to move
  // some of the current data into the new chunk. The function may update
  // the current index_ value to represent data no longer in the current chunk.
  // Returns the initial index of the new chunk (after copied data).
  virtual void NewChunk(int new_capacity)  {
	Vector<T> new_chunk = Vector<T>::New(new_capacity);
	if (index_ > 0) {
	  chunks_.Add(current_chunk_.SubVector(0, index_));
	} else {
	  current_chunk_.Dispose();
	}
	current_chunk_ = new_chunk;
	index_ = 0;
  }
};


/*
 * A collector that allows sequences of values to be guaranteed to
 * stay consecutive.
 * If the backing store grows while a sequence is active, the current
 * sequence might be moved, but after the sequence is ended, it will
 * not move again.
 * NOTICE: Blocks allocated using Collector::AddBlock(int) can move
 * as well, if inside an active sequence where another element is added.
 */
template <typename T, int growth_factor = 2, int max_growth = 1 * MB>
class SequenceCollector : public Collector<T, growth_factor, max_growth> {
 public:
  explicit SequenceCollector(int initial_capacity)
	  : Collector<T, growth_factor, max_growth>(initial_capacity),
		sequence_start_(kNoSequence) { }

  virtual ~SequenceCollector() {}

  void StartSequence() {
	ASSERT(sequence_start_ == kNoSequence);
	sequence_start_ = this->index_;
  }

  Vector<T> EndSequence() {
	ASSERT(sequence_start_ != kNoSequence);
	int sequence_start = sequence_start_;
	sequence_start_ = kNoSequence;
	if (sequence_start == this->index_) return Vector<T>();
	return this->current_chunk_.SubVector(sequence_start, this->index_);
  }

  // Drops the currently added sequence, and all collected elements in it.
  void DropSequence() {
	ASSERT(sequence_start_ != kNoSequence);
	int sequence_length = this->index_ - sequence_start_;
	this->index_ = sequence_start_;
	this->size_ -= sequence_length;
	sequence_start_ = kNoSequence;
  }

  virtual void Reset() {
	sequence_start_ = kNoSequence;
	this->Collector<T, growth_factor, max_growth>::Reset();
  }

 private:
  static const int kNoSequence = -1;
  int sequence_start_;

  // Move the currently active sequence to the new chunk.
  virtual void NewChunk(int new_capacity) {
	if (sequence_start_ == kNoSequence) {
	  // Fall back on default behavior if no sequence has been started.
	  this->Collector<T, growth_factor, max_growth>::NewChunk(new_capacity);
	  return;
	}
	int sequence_length = this->index_ - sequence_start_;
	Vector<T> new_chunk = Vector<T>::New(sequence_length + new_capacity);
	ASSERT(sequence_length < new_chunk.length());
	for (int i = 0; i < sequence_length; i++) {
	  new_chunk[i] = this->current_chunk_[sequence_start_ + i];
	}
	if (sequence_start_ > 0) {
	  this->chunks_.Add(this->current_chunk_.SubVector(0, sequence_start_));
	} else {
	  this->current_chunk_.Dispose();
	}
	this->current_chunk_ = new_chunk;
	this->index_ = sequence_length;
	sequence_start_ = 0;
  }
};


// Compare ASCII/16bit chars to ASCII/16bit chars.
template <typename lchar, typename rchar>
inline int CompareChars(const lchar* lhs, const rchar* rhs, int chars) {
  const lchar* limit = lhs + chars;
#ifdef V8_HOST_CAN_READ_UNALIGNED
  if (sizeof(*lhs) == sizeof(*rhs)) {
	// Number of characters in a uintptr_t.
	static const int kStepSize = sizeof(uintptr_t) / sizeof(*lhs);  // NOLINT
	while (lhs <= limit - kStepSize) {
	  if (*reinterpret_cast<const uintptr_t*>(lhs) !=
		  *reinterpret_cast<const uintptr_t*>(rhs)) {
		break;
	  }
	  lhs += kStepSize;
	  rhs += kStepSize;
	}
  }
#endif
  while (lhs < limit) {
	int r = static_cast<int>(*lhs) - static_cast<int>(*rhs);
	if (r != 0) return r;
	++lhs;
	++rhs;
  }
  return 0;
}


// Calculate 10^exponent.
inline int TenToThe(int exponent) {
  ASSERT(exponent <= 9);
  ASSERT(exponent >= 1);
  int answer = 10;
  for (int i = 1; i < exponent; i++) answer *= 10;
  return answer;
}


// The type-based aliasing rule allows the compiler to assume that pointers of
// different types (for some definition of different) never alias each other.
// Thus the following code does not work:
//
// float f = foo();
// int fbits = *(int*)(&f);
//
// The compiler 'knows' that the int pointer can't refer to f since the types
// don't match, so the compiler may cache f in a register, leaving random data
// in fbits.  Using C++ style casts makes no difference, however a pointer to
// char data is assumed to alias any other pointer.  This is the 'memcpy
// exception'.
//
// Bit_cast uses the memcpy exception to move the bits from a variable of one
// type of a variable of another type.  Of course the end result is likely to
// be implementation dependent.  Most compilers (gcc-4.2 and MSVC 2005)
// will completely optimize BitCast away.
//
// There is an additional use for BitCast.
// Recent gccs will warn when they see casts that may result in breakage due to
// the type-based aliasing rule.  If you have checked that there is no breakage
// you can use BitCast to cast one pointer type to another.  This confuses gcc
// enough that it can no longer see that you have cast one pointer type to
// another thus avoiding the warning.

// We need different implementations of BitCast for pointer and non-pointer
// values. We use partial specialization of auxiliary struct to work around
// issues with template functions overloading.
template <class Dest, class Source>
struct BitCastHelper {
  STATIC_ASSERT(sizeof(Dest) == sizeof(Source));

  INLINE(static Dest cast(const Source& source)) {
	Dest dest;
	memcpy(&dest, &source, sizeof(dest));
	return dest;
  }
};

template <class Dest, class Source>
struct BitCastHelper<Dest, Source*> {
  INLINE(static Dest cast(Source* source)) {
	return BitCastHelper<Dest, uintptr_t>::
		cast(reinterpret_cast<uintptr_t>(source));
  }
};

template <class Dest, class Source>
INLINE(Dest BitCast(const Source& source));

template <class Dest, class Source>
inline Dest BitCast(const Source& source) {
  return BitCastHelper<Dest, Source>::cast(source);
}


template<typename ElementType, int NumElements>
class EmbeddedContainer {
 public:
  EmbeddedContainer() : elems_() { }

  int length() const { return NumElements; }
  const ElementType& operator[](int i) const {
	ASSERT(i < length());
	return elems_[i];
  }
  ElementType& operator[](int i) {
	ASSERT(i < length());
	return elems_[i];
  }

 private:
  ElementType elems_[NumElements];
};


template<typename ElementType>
class EmbeddedContainer<ElementType, 0> {
 public:
  int length() const { return 0; }
  const ElementType& operator[](int i) const {
	UNREACHABLE();
	static ElementType t = 0;
	return t;
  }
  ElementType& operator[](int i) {
	UNREACHABLE();
	static ElementType t = 0;
	return t;
  }
};


// Helper class for building result strings in a character buffer. The
// purpose of the class is to use safe operations that checks the
// buffer bounds on all operations in debug mode.
// This simple base class does not allow formatted output.
class SimpleStringBuilder {
 public:
  // Create a string builder with a buffer of the given size. The
  // buffer is allocated through NewArray<char> and must be
  // deallocated by the caller of Finalize().
  explicit SimpleStringBuilder(int size);

  SimpleStringBuilder(char* buffer, int size)
	  : buffer_(buffer, size), position_(0) { }

  ~SimpleStringBuilder() { if (!is_finalized()) Finalize(); }

  int size() const { return buffer_.length(); }

  // Get the current position in the builder.
  int position() const {
	ASSERT(!is_finalized());
	return position_;
  }

  // Reset the position.
  void Reset() { position_ = 0; }

  // Add a single character to the builder. It is not allowed to add
  // 0-characters; use the Finalize() method to terminate the string
  // instead.
  void AddCharacter(char c) {
	ASSERT(c != '\0');
	ASSERT(!is_finalized() && position_ < buffer_.length());
	buffer_[position_++] = c;
  }

  // Add an entire string to the builder. Uses strlen() internally to
  // compute the length of the input string.
  void AddString(const char* s);

  // Add the first 'n' characters of the given string 's' to the
  // builder. The input string must have enough characters.
  void AddSubstring(const char* s, int n);

  // Add character padding to the builder. If count is non-positive,
  // nothing is added to the builder.
  void AddPadding(char c, int count);

  // Add the decimal representation of the value.
  void AddDecimalInteger(int value);

  // Finalize the string by 0-terminating it and returning the buffer.
  char* Finalize();

 protected:
  Vector<char> buffer_;
  int position_;

  bool is_finalized() const { return position_ < 0; }

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(SimpleStringBuilder);
};


// A poor man's version of STL's bitset: A bit set of enums E (without explicit
// values), fitting into an integral type T.
template <class E, class T = int>
class EnumSet {
 public:
  explicit EnumSet(T bits = 0) : bits_(bits) {}
  bool IsEmpty() const { return bits_ == 0; }
  bool Contains(E element) const { return (bits_ & Mask(element)) != 0; }
  bool ContainsAnyOf(const EnumSet& set) const {
	return (bits_ & set.bits_) != 0;
  }
  void Add(E element) { bits_ |= Mask(element); }
  void Add(const EnumSet& set) { bits_ |= set.bits_; }
  void Remove(E element) { bits_ &= ~Mask(element); }
  void Remove(const EnumSet& set) { bits_ &= ~set.bits_; }
  void RemoveAll() { bits_ = 0; }
  void Intersect(const EnumSet& set) { bits_ &= set.bits_; }
  T ToIntegral() const { return bits_; }
  bool operator==(const EnumSet& set) { return bits_ == set.bits_; }

 private:
  T Mask(E element) const {
	// The strange typing in ASSERT is necessary to avoid stupid warnings, see:
	// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=43680
	ASSERT(static_cast<int>(element) < static_cast<int>(sizeof(T) * CHAR_BIT));
	return 1 << element;
  }

  T bits_;
};


class TypeFeedbackId {
 public:
  explicit TypeFeedbackId(int id) : id_(id) { }
  int ToInt() const { return id_; }

  static TypeFeedbackId None() { return TypeFeedbackId(kNoneId); }
  bool IsNone() const { return id_ == kNoneId; }

 private:
  static const int kNoneId = -1;

  int id_;
};


class BailoutId {
 public:
  explicit BailoutId(int id) : id_(id) { }
  int ToInt() const { return id_; }

  static BailoutId None() { return BailoutId(kNoneId); }
  static BailoutId FunctionEntry() { return BailoutId(kFunctionEntryId); }
  static BailoutId Declarations() { return BailoutId(kDeclarationsId); }
  static BailoutId FirstUsable() { return BailoutId(kFirstUsableId); }

  bool IsNone() const { return id_ == kNoneId; }
  bool operator==(const BailoutId& other) const { return id_ == other.id_; }

 private:
  static const int kNoneId = -1;

  // Using 0 could disguise errors.
  static const int kFunctionEntryId = 2;

  // This AST id identifies the point after the declarations have been visited.
  // We need it to capture the environment effects of declarations that emit
  // code (function declarations).
  static const int kDeclarationsId = 3;

  // Ever FunctionState starts with this id.
  static const int kFirstUsableId = 4;

  int id_;
};

} }  // namespace v8::internal

#endif  // V8_UTILS_H_

/// v8/src/utils-inl.h

// Copyright 2011 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef V8_UTILS_INL_H_
#define V8_UTILS_INL_H_

//TODO	#include "list-inl.h"
//TODO	
//TODO	namespace v8 {
//TODO	namespace internal {
//TODO	
//TODO	template<typename T, int growth_factor, int max_growth>
//TODO	void Collector<T, growth_factor, max_growth>::Reset() {
//TODO	  for (int i = chunks_.length() - 1; i >= 0; i--) {
//TODO	    chunks_.at(i).Dispose();
//TODO	  }
//TODO	  chunks_.Rewind(0);
//TODO	  index_ = 0;
//TODO	  size_ = 0;
//TODO	}
//TODO	
//TODO	} }  // namespace v8::internal

#endif  // V8_UTILS_INL_H_

/// v8/src/diy-fp.h

// Copyright 2011 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//  	 notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//  	 copyright notice, this list of conditions and the following
//  	 disclaimer in the documentation and/or other materials provided
//  	 with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//  	 contributors may be used to endorse or promote products derived
//  	 from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef V8_DIY_FP_H_
#define V8_DIY_FP_H_

namespace v8 {
namespace internal {

// This "Do It Yourself Floating Point" class implements a floating-point number
// with a uint64 significand and an int exponent. Normalized DiyFp numbers will
// have the most significant bit of the significand set.
// Multiplication and Subtraction do not normalize their results.
// DiyFp are not designed to contain special doubles (NaN and Infinity).
class DiyFp {
 public:
  static const int kSignificandSize = 64;

  DiyFp() : f_(0), e_(0) {}
  DiyFp(uint64_t f, int e) : f_(f), e_(e) {}

  // this = this - other.
  // The exponents of both numbers must be the same and the significand of this
  // must be bigger than the significand of other.
  // The result will not be normalized.
  void Subtract(const DiyFp& other) {
	ASSERT(e_ == other.e_);
	ASSERT(f_ >= other.f_);
	f_ -= other.f_;
  }

  // Returns a - b.
  // The exponents of both numbers must be the same and this must be bigger
  // than other. The result will not be normalized.
  static DiyFp Minus(const DiyFp& a, const DiyFp& b) {
	DiyFp result = a;
	result.Subtract(b);
	return result;
  }


  // this = this * other.
  void Multiply(const DiyFp& other);

  // returns a * b;
  static DiyFp Times(const DiyFp& a, const DiyFp& b) {
	DiyFp result = a;
	result.Multiply(b);
	return result;
  }

  void Normalize() {
	ASSERT(f_ != 0);
	uint64_t f = f_;
	int e = e_;

	// This method is mainly called for normalizing boundaries. In general
	// boundaries need to be shifted by 10 bits. We thus optimize for this case.
	const uint64_t k10MSBits = static_cast<uint64_t>(0x3FF) << 54;
	while ((f & k10MSBits) == 0) {
	  f <<= 10;
	  e -= 10;
	}
	while ((f & kUint64MSB) == 0) {
	  f <<= 1;
	  e--;
	}
	f_ = f;
	e_ = e;
  }

  static DiyFp Normalize(const DiyFp& a) {
	DiyFp result = a;
	result.Normalize();
	return result;
  }

  uint64_t f() const { return f_; }
  int e() const { return e_; }

  void set_f(uint64_t new_value) { f_ = new_value; }
  void set_e(int new_value) { e_ = new_value; }

 private:
  static const uint64_t kUint64MSB = static_cast<uint64_t>(1) << 63;

  uint64_t f_;
  int e_;
};

} }  // namespace v8::internal

#endif  // V8_DIY_FP_H_

/// v8/src/double.h

// Copyright 2011 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//  	 notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//  	 copyright notice, this list of conditions and the following
//  	 disclaimer in the documentation and/or other materials provided
//  	 with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//  	 contributors may be used to endorse or promote products derived
//  	 from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef V8_DOUBLE_H_
#define V8_DOUBLE_H_

//TODO	#include "diy-fp.h"

namespace v8 {
namespace internal {

// We assume that doubles and uint64_t have the same endianness.
inline uint64_t double_to_uint64(double d) { return BitCast<uint64_t>(d); }
inline double uint64_to_double(uint64_t d64) { return BitCast<double>(d64); }

// Helper functions for doubles.
class Double {
 public:
  static const uint64_t kSignMask = V8_2PART_UINT64_C(0x80000000, 00000000);
  static const uint64_t kExponentMask = V8_2PART_UINT64_C(0x7FF00000, 00000000);
  static const uint64_t kSignificandMask =
	  V8_2PART_UINT64_C(0x000FFFFF, FFFFFFFF);
  static const uint64_t kHiddenBit = V8_2PART_UINT64_C(0x00100000, 00000000);
  static const int kPhysicalSignificandSize = 52;  // Excludes the hidden bit.
  static const int kSignificandSize = 53;

  Double() : d64_(0) {}
  explicit Double(double d) : d64_(double_to_uint64(d)) {}
  explicit Double(uint64_t d64) : d64_(d64) {}
  explicit Double(DiyFp diy_fp)
	: d64_(DiyFpToUint64(diy_fp)) {}

  // The value encoded by this Double must be greater or equal to +0.0.
  // It must not be special (infinity, or NaN).
  DiyFp AsDiyFp() const {
	ASSERT(Sign() > 0);
	ASSERT(!IsSpecial());
	return DiyFp(Significand(), Exponent());
  }

  // The value encoded by this Double must be strictly greater than 0.
  DiyFp AsNormalizedDiyFp() const {
	ASSERT(value() > 0.0);
	uint64_t f = Significand();
	int e = Exponent();

	// The current double could be a denormal.
	while ((f & kHiddenBit) == 0) {
	  f <<= 1;
	  e--;
	}
	// Do the final shifts in one go.
	f <<= DiyFp::kSignificandSize - kSignificandSize;
	e -= DiyFp::kSignificandSize - kSignificandSize;
	return DiyFp(f, e);
  }

  // Returns the double's bit as uint64.
  uint64_t AsUint64() const {
	return d64_;
  }

  // Returns the next greater double. Returns +infinity on input +infinity.
  double NextDouble() const {
	if (d64_ == kInfinity) return Double(kInfinity).value();
	if (Sign() < 0 && Significand() == 0) {
	  // -0.0
	  return 0.0;
	}
	if (Sign() < 0) {
	  return Double(d64_ - 1).value();
	} else {
	  return Double(d64_ + 1).value();
	}
  }

  int Exponent() const {
	if (IsDenormal()) return kDenormalExponent;

	uint64_t d64 = AsUint64();
	int biased_e =
		static_cast<int>((d64 & kExponentMask) >> kPhysicalSignificandSize);
	return biased_e - kExponentBias;
  }

  uint64_t Significand() const {
	uint64_t d64 = AsUint64();
	uint64_t significand = d64 & kSignificandMask;
	if (!IsDenormal()) {
	  return significand + kHiddenBit;
	} else {
	  return significand;
	}
  }

  // Returns true if the double is a denormal.
  bool IsDenormal() const {
	uint64_t d64 = AsUint64();
	return (d64 & kExponentMask) == 0;
  }

  // We consider denormals not to be special.
  // Hence only Infinity and NaN are special.
  bool IsSpecial() const {
	uint64_t d64 = AsUint64();
	return (d64 & kExponentMask) == kExponentMask;
  }

  bool IsInfinite() const {
	uint64_t d64 = AsUint64();
	return ((d64 & kExponentMask) == kExponentMask) &&
		((d64 & kSignificandMask) == 0);
  }

  int Sign() const {
	uint64_t d64 = AsUint64();
	return (d64 & kSignMask) == 0? 1: -1;
  }

  // Precondition: the value encoded by this Double must be greater or equal
  // than +0.0.
  DiyFp UpperBoundary() const {
	ASSERT(Sign() > 0);
	return DiyFp(Significand() * 2 + 1, Exponent() - 1);
  }

  // Returns the two boundaries of this.
  // The bigger boundary (m_plus) is normalized. The lower boundary has the same
  // exponent as m_plus.
  // Precondition: the value encoded by this Double must be greater than 0.
  void NormalizedBoundaries(DiyFp* out_m_minus, DiyFp* out_m_plus) const {
	ASSERT(value() > 0.0);
	DiyFp v = this->AsDiyFp();
	bool significand_is_zero = (v.f() == kHiddenBit);
	DiyFp m_plus = DiyFp::Normalize(DiyFp((v.f() << 1) + 1, v.e() - 1));
	DiyFp m_minus;
	if (significand_is_zero && v.e() != kDenormalExponent) {
	  // The boundary is closer. Think of v = 1000e10 and v- = 9999e9.
	  // Then the boundary (== (v - v-)/2) is not just at a distance of 1e9 but
	  // at a distance of 1e8.
	  // The only exception is for the smallest normal: the largest denormal is
	  // at the same distance as its successor.
	  // Note: denormals have the same exponent as the smallest normals.
	  m_minus = DiyFp((v.f() << 2) - 1, v.e() - 2);
	} else {
	  m_minus = DiyFp((v.f() << 1) - 1, v.e() - 1);
	}
	m_minus.set_f(m_minus.f() << (m_minus.e() - m_plus.e()));
	m_minus.set_e(m_plus.e());
	*out_m_plus = m_plus;
	*out_m_minus = m_minus;
  }

  double value() const { return uint64_to_double(d64_); }

  // Returns the significand size for a given order of magnitude.
  // If v = f*2^e with 2^p-1 <= f <= 2^p then p+e is v's order of magnitude.
  // This function returns the number of significant binary digits v will have
  // once its encoded into a double. In almost all cases this is equal to
  // kSignificandSize. The only exception are denormals. They start with leading
  // zeroes and their effective significand-size is hence smaller.
  static int SignificandSizeForOrderOfMagnitude(int order) {
	if (order >= (kDenormalExponent + kSignificandSize)) {
	  return kSignificandSize;
	}
	if (order <= kDenormalExponent) return 0;
	return order - kDenormalExponent;
  }

 private:
  static const int kExponentBias = 0x3FF + kPhysicalSignificandSize;
  static const int kDenormalExponent = -kExponentBias + 1;
  static const int kMaxExponent = 0x7FF - kExponentBias;
  static const uint64_t kInfinity = V8_2PART_UINT64_C(0x7FF00000, 00000000);

  const uint64_t d64_;

  static uint64_t DiyFpToUint64(DiyFp diy_fp) {
	uint64_t significand = diy_fp.f();
	int exponent = diy_fp.e();
	while (significand > kHiddenBit + kSignificandMask) {
	  significand >>= 1;
	  exponent++;
	}
	if (exponent >= kMaxExponent) {
	  return kInfinity;
	}
	if (exponent < kDenormalExponent) {
	  return 0;
	}
	while (exponent > kDenormalExponent && (significand & kHiddenBit) == 0) {
	  significand <<= 1;
	  exponent--;
	}
	uint64_t biased_exponent;
	if (exponent == kDenormalExponent && (significand & kHiddenBit) == 0) {
	  biased_exponent = 0;
	} else {
	  biased_exponent = static_cast<uint64_t>(exponent + kExponentBias);
	}
	return (significand & kSignificandMask) |
		(biased_exponent << kPhysicalSignificandSize);
  }
};

} }  // namespace v8::internal

#endif  // V8_DOUBLE_H_

/// v8/src/conversions.h

// Copyright 2011 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//  	 notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//  	 copyright notice, this list of conditions and the following
//  	 disclaimer in the documentation and/or other materials provided
//  	 with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//  	 contributors may be used to endorse or promote products derived
//  	 from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef V8_CONVERSIONS_H_
#define V8_CONVERSIONS_H_

//TODO	#include "utils.h"

namespace v8 {
namespace internal {

class UnicodeCache;

// Maximum number of significant digits in decimal representation.
// The longest possible double in decimal representation is
// (2^53 - 1) * 2 ^ -1074 that is (2 ^ 53 - 1) * 5 ^ 1074 / 10 ^ 1074
// (768 digits). If we parse a number whose first digits are equal to a
// mean of 2 adjacent doubles (that could have up to 769 digits) the result
// must be rounded to the bigger one unless the tail consists of zeros, so
// we don't need to preserve all the digits.
const int kMaxSignificantDigits = 772;


inline bool isDigit(int x, int radix) {
  return (x >= '0' && x <= '9' && x < '0' + radix)
	  || (radix > 10 && x >= 'a' && x < 'a' + radix - 10)
	  || (radix > 10 && x >= 'A' && x < 'A' + radix - 10);
}


// The fast double-to-(unsigned-)int conversion routine does not guarantee
// rounding towards zero.
// For NaN and values outside the int range, return INT_MIN or INT_MAX.
inline int FastD2IChecked(double x) {
  if (!(x >= INT_MIN)) return INT_MIN;  // Negation to catch NaNs.
  if (x > INT_MAX) return INT_MAX;
  return static_cast<int>(x);
}


// The fast double-to-(unsigned-)int conversion routine does not guarantee
// rounding towards zero.
// The result is unspecified if x is infinite or NaN, or if the rounded
// integer value is outside the range of type int.
inline int FastD2I(double x) {
  return static_cast<int>(x);
}

inline unsigned int FastD2UI(double x);


inline double FastI2D(int x) {
  // There is no rounding involved in converting an integer to a
  // double, so this code should compile to a few instructions without
  // any FPU pipeline stalls.
  return static_cast<double>(x);
}


inline double FastUI2D(unsigned x) {
  // There is no rounding involved in converting an unsigned integer to a
  // double, so this code should compile to a few instructions without
  // any FPU pipeline stalls.
  return static_cast<double>(x);
}


// This function should match the exact semantics of ECMA-262 9.4.
inline double DoubleToInteger(double x);


// This function should match the exact semantics of ECMA-262 9.5.
inline int32_t DoubleToInt32(double x);


// This function should match the exact semantics of ECMA-262 9.6.
inline uint32_t DoubleToUint32(double x) {
  return static_cast<uint32_t>(DoubleToInt32(x));
}


//TODO	// Enumeration for allowing octals and ignoring junk when converting
//TODO	// strings to numbers.
//TODO	enum ConversionFlags {
//TODO	  NO_FLAGS = 0,
//TODO	  ALLOW_HEX = 1,
//TODO	  ALLOW_OCTALS = 2,
//TODO	  ALLOW_TRAILING_JUNK = 4
//TODO	};
//TODO	
//TODO	
//TODO	// Converts a string into a double value according to ECMA-262 9.3.1
//TODO	double StringToDouble(UnicodeCache* unicode_cache,
//TODO						  Vector<const char> str,
//TODO						  int flags,
//TODO						  double empty_string_val = 0);
//TODO	double StringToDouble(UnicodeCache* unicode_cache,
//TODO						  Vector<const uc16> str,
//TODO						  int flags,
//TODO						  double empty_string_val = 0);
//TODO	// This version expects a zero-terminated character array.
//TODO	double StringToDouble(UnicodeCache* unicode_cache,
//TODO						  const char* str,
//TODO						  int flags,
//TODO						  double empty_string_val = 0);
//TODO	
//TODO	const int kDoubleToCStringMinBufferSize = 100;
//TODO	
//TODO	// Converts a double to a string value according to ECMA-262 9.8.1.
//TODO	// The buffer should be large enough for any floating point number.
//TODO	// 100 characters is enough.
//TODO	const char* DoubleToCString(double value, Vector<char> buffer);
//TODO	
//TODO	// Convert an int to a null-terminated string. The returned string is
//TODO	// located inside the buffer, but not necessarily at the start.
//TODO	const char* IntToCString(int n, Vector<char> buffer);
//TODO	
//TODO	// Additional number to string conversions for the number type.
//TODO	// The caller is responsible for calling free on the returned pointer.
//TODO	char* DoubleToFixedCString(double value, int f);
//TODO	char* DoubleToExponentialCString(double value, int f);
//TODO	char* DoubleToPrecisionCString(double value, int f);
//TODO	char* DoubleToRadixCString(double value, int radix);

} }  // namespace v8::internal

#endif  // V8_CONVERSIONS_H_

/// v8/src/conversions-inl.h

// Copyright 2011 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//  	 notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//  	 copyright notice, this list of conditions and the following
//  	 disclaimer in the documentation and/or other materials provided
//  	 with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//  	 contributors may be used to endorse or promote products derived
//  	 from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef V8_CONVERSIONS_INL_H_
#define V8_CONVERSIONS_INL_H_

#include <limits.h> 	   // Required for INT_MAX etc.
#include <math.h>
#include <float.h>  	   // Required for DBL_MAX and on Win32 for finite()
#include <stdarg.h>
//TODO	#include "globals.h"  	 // Required for V8_INFINITY

// ----------------------------------------------------------------------------
// Extra POSIX/ANSI functions for Win32/MSVC.

//TODO	#include "conversions.h"
//TODO	#include "double.h"
//TODO	#include "platform.h"
//TODO	#include "scanner.h"
//TODO	#include "strtod.h"

namespace v8 {
namespace internal {

inline double JunkStringValue() {
  return BitCast<double, uint64_t>(kQuietNaNMask);
}


inline double SignedZero(bool negative) {
  return negative ? uint64_to_double(Double::kSignMask) : 0.0;
}


// The fast double-to-unsigned-int conversion routine does not guarantee
// rounding towards zero, or any reasonable value if the argument is larger
// than what fits in an unsigned 32-bit integer.
inline unsigned int FastD2UI(double x) {
  // There is no unsigned version of lrint, so there is no fast path
  // in this function as there is in FastD2I. Using lrint doesn't work
  // for values of 2^31 and above.

  // Convert "small enough" doubles to uint32_t by fixing the 32
  // least significant non-fractional bits in the low 32 bits of the
  // double, and reading them from there.
  const double k2Pow52 = 4503599627370496.0;
  bool negative = x < 0;
  if (negative) {
	x = -x;
  }
  if (x < k2Pow52) {
	x += k2Pow52;
	uint32_t result;
	Address mantissa_ptr = reinterpret_cast<Address>(&x);
	// Copy least significant 32 bits of mantissa.
	memcpy(&result, mantissa_ptr, sizeof(result));
	return negative ? ~result + 1 : result;
  }
  // Large number (outside uint32 range), Infinity or NaN.
  return 0x80000000u;  // Return integer indefinite.
}


inline double DoubleToInteger(double x) {
  if (isnan(x)) return 0;
  if (!isfinite(x) || x == 0) return x;
  return (x >= 0) ? floor(x) : ceil(x);
}


int32_t DoubleToInt32(double x) {
  int32_t i = FastD2I(x);
  if (FastI2D(i) == x) return i;
  Double d(x);
  int exponent = d.Exponent();
  if (exponent < 0) {
	if (exponent <= -Double::kSignificandSize) return 0;
	return d.Sign() * static_cast<int32_t>(d.Significand() >> -exponent);
  } else {
	if (exponent > 31) return 0;
	return d.Sign() * static_cast<int32_t>(d.Significand() << exponent);
  }
}


//TODO	template <class Iterator, class EndMark>
//TODO	bool SubStringEquals(Iterator* current,
//TODO  					 EndMark end,
//TODO  					 const char* substring) {
//TODO    ASSERT(**current == *substring);
//TODO    for (substring++; *substring != '\0'; substring++) {
//TODO  	++*current;
//TODO  	if (*current == end || **current != *substring) return false;
//TODO    }
//TODO    ++*current;
//TODO    return true;
//TODO	}
//TODO	
//TODO	
//TODO	// Returns true if a nonspace character has been found and false if the
//TODO	// end was been reached before finding a nonspace character.
//TODO	template <class Iterator, class EndMark>
//TODO	inline bool AdvanceToNonspace(UnicodeCache* unicode_cache,
//TODO  							  Iterator* current,
//TODO  							  EndMark end) {
//TODO    while (*current != end) {
//TODO  	if (!unicode_cache->IsWhiteSpace(**current)) return true;
//TODO  	++*current;
//TODO    }
//TODO    return false;
//TODO	}
//TODO	
//TODO	
//TODO	// Parsing integers with radix 2, 4, 8, 16, 32. Assumes current != end.
//TODO	template <int radix_log_2, class Iterator, class EndMark>
//TODO	double InternalStringToIntDouble(UnicodeCache* unicode_cache,
//TODO  								 Iterator current,
//TODO  								 EndMark end,
//TODO  								 bool negative,
//TODO  								 bool allow_trailing_junk) {
//TODO    ASSERT(current != end);
//TODO	
//TODO    // Skip leading 0s.
//TODO    while (*current == '0') {
//TODO  	++current;
//TODO  	if (current == end) return SignedZero(negative);
//TODO    }
//TODO	
//TODO    int64_t number = 0;
//TODO    int exponent = 0;
//TODO    const int radix = (1 << radix_log_2);
//TODO	
//TODO    do {
//TODO  	int digit;
//TODO  	if (*current >= '0' && *current <= '9' && *current < '0' + radix) {
//TODO  	  digit = static_cast<char>(*current) - '0';
//TODO  	} else if (radix > 10 && *current >= 'a' && *current < 'a' + radix - 10) {
//TODO  	  digit = static_cast<char>(*current) - 'a' + 10;
//TODO  	} else if (radix > 10 && *current >= 'A' && *current < 'A' + radix - 10) {
//TODO  	  digit = static_cast<char>(*current) - 'A' + 10;
//TODO  	} else {
//TODO  	  if (allow_trailing_junk ||
//TODO  		  !AdvanceToNonspace(unicode_cache, &current, end)) {
//TODO  		break;
//TODO  	  } else {
//TODO  		return JunkStringValue();
//TODO  	  }
//TODO  	}
//TODO	
//TODO  	number = number * radix + digit;
//TODO  	int overflow = static_cast<int>(number >> 53);
//TODO  	if (overflow != 0) {
//TODO  	  // Overflow occurred. Need to determine which direction to round the
//TODO  	  // result.
//TODO  	  int overflow_bits_count = 1;
//TODO  	  while (overflow > 1) {
//TODO  		overflow_bits_count++;
//TODO  		overflow >>= 1;
//TODO  	  }
//TODO	
//TODO  	  int dropped_bits_mask = ((1 << overflow_bits_count) - 1);
//TODO  	  int dropped_bits = static_cast<int>(number) & dropped_bits_mask;
//TODO  	  number >>= overflow_bits_count;
//TODO  	  exponent = overflow_bits_count;
//TODO	
//TODO  	  bool zero_tail = true;
//TODO  	  while (true) {
//TODO  		++current;
//TODO  		if (current == end || !isDigit(*current, radix)) break;
//TODO  		zero_tail = zero_tail && *current == '0';
//TODO  		exponent += radix_log_2;
//TODO  	  }
//TODO	
//TODO  	  if (!allow_trailing_junk &&
//TODO  		  AdvanceToNonspace(unicode_cache, &current, end)) {
//TODO  		return JunkStringValue();
//TODO  	  }
//TODO	
//TODO  	  int middle_value = (1 << (overflow_bits_count - 1));
//TODO  	  if (dropped_bits > middle_value) {
//TODO  		number++;  // Rounding up.
//TODO  	  } else if (dropped_bits == middle_value) {
//TODO  		// Rounding to even to consistency with decimals: half-way case rounds
//TODO  		// up if significant part is odd and down otherwise.
//TODO  		if ((number & 1) != 0 || !zero_tail) {
//TODO  		  number++;  // Rounding up.
//TODO  		}
//TODO  	  }
//TODO	
//TODO  	  // Rounding up may cause overflow.
//TODO  	  if ((number & ((int64_t)1 << 53)) != 0) {
//TODO  		exponent++;
//TODO  		number >>= 1;
//TODO  	  }
//TODO  	  break;
//TODO  	}
//TODO  	++current;
//TODO    } while (current != end);
//TODO	
//TODO    ASSERT(number < ((int64_t)1 << 53));
//TODO    ASSERT(static_cast<int64_t>(static_cast<double>(number)) == number);
//TODO	
//TODO    if (exponent == 0) {
//TODO  	if (negative) {
//TODO  	  if (number == 0) return -0.0;
//TODO  	  number = -number;
//TODO  	}
//TODO  	return static_cast<double>(number);
//TODO    }
//TODO	
//TODO    ASSERT(number != 0);
//TODO    return ldexp(static_cast<double>(negative ? -number : number), exponent);
//TODO	}
//TODO	
//TODO	
//TODO	template <class Iterator, class EndMark>
//TODO	double InternalStringToInt(UnicodeCache* unicode_cache,
//TODO  						   Iterator current,
//TODO  						   EndMark end,
//TODO  						   int radix) {
//TODO    const bool allow_trailing_junk = true;
//TODO    const double empty_string_val = JunkStringValue();
//TODO	
//TODO    if (!AdvanceToNonspace(unicode_cache, &current, end)) {
//TODO  	return empty_string_val;
//TODO    }
//TODO	
//TODO    bool negative = false;
//TODO    bool leading_zero = false;
//TODO	
//TODO    if (*current == '+') {
//TODO  	// Ignore leading sign; skip following spaces.
//TODO  	++current;
//TODO  	if (current == end) {
//TODO  	  return JunkStringValue();
//TODO  	}
//TODO    } else if (*current == '-') {
//TODO  	++current;
//TODO  	if (current == end) {
//TODO  	  return JunkStringValue();
//TODO  	}
//TODO  	negative = true;
//TODO    }
//TODO	
//TODO    if (radix == 0) {
//TODO  	// Radix detection.
//TODO  	radix = 10;
//TODO  	if (*current == '0') {
//TODO  	  ++current;
//TODO  	  if (current == end) return SignedZero(negative);
//TODO  	  if (*current == 'x' || *current == 'X') {
//TODO  		radix = 16;
//TODO  		++current;
//TODO  		if (current == end) return JunkStringValue();
//TODO  	  } else {
//TODO  		leading_zero = true;
//TODO  	  }
//TODO  	}
//TODO    } else if (radix == 16) {
//TODO  	if (*current == '0') {
//TODO  	  // Allow "0x" prefix.
//TODO  	  ++current;
//TODO  	  if (current == end) return SignedZero(negative);
//TODO  	  if (*current == 'x' || *current == 'X') {
//TODO  		++current;
//TODO  		if (current == end) return JunkStringValue();
//TODO  	  } else {
//TODO  		leading_zero = true;
//TODO  	  }
//TODO  	}
//TODO    }
//TODO	
//TODO    if (radix < 2 || radix > 36) return JunkStringValue();
//TODO	
//TODO    // Skip leading zeros.
//TODO    while (*current == '0') {
//TODO  	leading_zero = true;
//TODO  	++current;
//TODO  	if (current == end) return SignedZero(negative);
//TODO    }
//TODO	
//TODO    if (!leading_zero && !isDigit(*current, radix)) {
//TODO  	return JunkStringValue();
//TODO    }
//TODO	
//TODO    if (IsPowerOf2(radix)) {
//TODO  	switch (radix) {
//TODO  	  case 2:
//TODO  		return InternalStringToIntDouble<1>(
//TODO  			unicode_cache, current, end, negative, allow_trailing_junk);
//TODO  	  case 4:
//TODO  		return InternalStringToIntDouble<2>(
//TODO  			unicode_cache, current, end, negative, allow_trailing_junk);
//TODO  	  case 8:
//TODO  		return InternalStringToIntDouble<3>(
//TODO  			unicode_cache, current, end, negative, allow_trailing_junk);
//TODO	
//TODO  	  case 16:
//TODO  		return InternalStringToIntDouble<4>(
//TODO  			unicode_cache, current, end, negative, allow_trailing_junk);
//TODO	
//TODO  	  case 32:
//TODO  		return InternalStringToIntDouble<5>(
//TODO  			unicode_cache, current, end, negative, allow_trailing_junk);
//TODO  	  default:
//TODO  		UNREACHABLE();
//TODO  	}
//TODO    }
//TODO	
//TODO    if (radix == 10) {
//TODO  	// Parsing with strtod.
//TODO  	const int kMaxSignificantDigits = 309;  // Doubles are less than 1.8e308.
//TODO  	// The buffer may contain up to kMaxSignificantDigits + 1 digits and a zero
//TODO  	// end.
//TODO  	const int kBufferSize = kMaxSignificantDigits + 2;
//TODO  	char buffer[kBufferSize];
//TODO  	int buffer_pos = 0;
//TODO  	while (*current >= '0' && *current <= '9') {
//TODO  	  if (buffer_pos <= kMaxSignificantDigits) {
//TODO  		// If the number has more than kMaxSignificantDigits it will be parsed
//TODO  		// as infinity.
//TODO  		ASSERT(buffer_pos < kBufferSize);
//TODO  		buffer[buffer_pos++] = static_cast<char>(*current);
//TODO  	  }
//TODO  	  ++current;
//TODO  	  if (current == end) break;
//TODO  	}
//TODO	
//TODO  	if (!allow_trailing_junk &&
//TODO  		AdvanceToNonspace(unicode_cache, &current, end)) {
//TODO  	  return JunkStringValue();
//TODO  	}
//TODO	
//TODO  	ASSERT(buffer_pos < kBufferSize);
//TODO  	buffer[buffer_pos] = '\0';
//TODO  	Vector<const char> buffer_vector(buffer, buffer_pos);
//TODO  	return negative ? -Strtod(buffer_vector, 0) : Strtod(buffer_vector, 0);
//TODO    }
//TODO	
//TODO    // The following code causes accumulating rounding error for numbers greater
//TODO    // than ~2^56. It's explicitly allowed in the spec: "if R is not 2, 4, 8, 10,
//TODO    // 16, or 32, then mathInt may be an implementation-dependent approximation to
//TODO    // the mathematical integer value" (15.1.2.2).
//TODO	
//TODO    int lim_0 = '0' + (radix < 10 ? radix : 10);
//TODO    int lim_a = 'a' + (radix - 10);
//TODO    int lim_A = 'A' + (radix - 10);
//TODO	
//TODO    // NOTE: The code for computing the value may seem a bit complex at
//TODO    // first glance. It is structured to use 32-bit multiply-and-add
//TODO    // loops as long as possible to avoid loosing precision.
//TODO	
//TODO    double v = 0.0;
//TODO    bool done = false;
//TODO    do {
//TODO  	// Parse the longest part of the string starting at index j
//TODO  	// possible while keeping the multiplier, and thus the part
//TODO  	// itself, within 32 bits.
//TODO  	unsigned int part = 0, multiplier = 1;
//TODO  	while (true) {
//TODO  	  int d;
//TODO  	  if (*current >= '0' && *current < lim_0) {
//TODO  		d = *current - '0';
//TODO  	  } else if (*current >= 'a' && *current < lim_a) {
//TODO  		d = *current - 'a' + 10;
//TODO  	  } else if (*current >= 'A' && *current < lim_A) {
//TODO  		d = *current - 'A' + 10;
//TODO  	  } else {
//TODO  		done = true;
//TODO  		break;
//TODO  	  }
//TODO	
//TODO  	  // Update the value of the part as long as the multiplier fits
//TODO  	  // in 32 bits. When we can't guarantee that the next iteration
//TODO  	  // will not overflow the multiplier, we stop parsing the part
//TODO  	  // by leaving the loop.
//TODO  	  const unsigned int kMaximumMultiplier = 0xffffffffU / 36;
//TODO  	  uint32_t m = multiplier * radix;
//TODO  	  if (m > kMaximumMultiplier) break;
//TODO  	  part = part * radix + d;
//TODO  	  multiplier = m;
//TODO  	  ASSERT(multiplier > part);
//TODO	
//TODO  	  ++current;
//TODO  	  if (current == end) {
//TODO  		done = true;
//TODO  		break;
//TODO  	  }
//TODO  	}
//TODO	
//TODO  	// Update the value and skip the part in the string.
//TODO  	v = v * multiplier + part;
//TODO    } while (!done);
//TODO	
//TODO    if (!allow_trailing_junk &&
//TODO  	  AdvanceToNonspace(unicode_cache, &current, end)) {
//TODO  	return JunkStringValue();
//TODO    }
//TODO	
//TODO    return negative ? -v : v;
//TODO	}
//TODO	
//TODO	
//TODO	// Converts a string to a double value. Assumes the Iterator supports
//TODO	// the following operations:
//TODO	// 1. current == end (other ops are not allowed), current != end.
//TODO	// 2. *current - gets the current character in the sequence.
//TODO	// 3. ++current (advances the position).
//TODO	template <class Iterator, class EndMark>
//TODO	double InternalStringToDouble(UnicodeCache* unicode_cache,
//TODO  							  Iterator current,
//TODO  							  EndMark end,
//TODO  							  int flags,
//TODO  							  double empty_string_val) {
//TODO    // To make sure that iterator dereferencing is valid the following
//TODO    // convention is used:
//TODO    // 1. Each '++current' statement is followed by check for equality to 'end'.
//TODO    // 2. If AdvanceToNonspace returned false then current == end.
//TODO    // 3. If 'current' becomes be equal to 'end' the function returns or goes to
//TODO    // 'parsing_done'.
//TODO    // 4. 'current' is not dereferenced after the 'parsing_done' label.
//TODO    // 5. Code before 'parsing_done' may rely on 'current != end'.
//TODO    if (!AdvanceToNonspace(unicode_cache, &current, end)) {
//TODO  	return empty_string_val;
//TODO    }
//TODO	
//TODO    const bool allow_trailing_junk = (flags & ALLOW_TRAILING_JUNK) != 0;
//TODO	
//TODO    // The longest form of simplified number is: "-<significant digits>'.1eXXX\0".
//TODO    const int kBufferSize = kMaxSignificantDigits + 10;
//TODO    char buffer[kBufferSize];  // NOLINT: size is known at compile time.
//TODO    int buffer_pos = 0;
//TODO	
//TODO    // Exponent will be adjusted if insignificant digits of the integer part
//TODO    // or insignificant leading zeros of the fractional part are dropped.
//TODO    int exponent = 0;
//TODO    int significant_digits = 0;
//TODO    int insignificant_digits = 0;
//TODO    bool nonzero_digit_dropped = false;
//TODO	
//TODO    enum Sign {
//TODO  	NONE,
//TODO  	NEGATIVE,
//TODO  	POSITIVE
//TODO    };
//TODO	
//TODO    Sign sign = NONE;
//TODO	
//TODO    if (*current == '+') {
//TODO  	// Ignore leading sign.
//TODO  	++current;
//TODO  	if (current == end) return JunkStringValue();
//TODO  	sign = POSITIVE;
//TODO    } else if (*current == '-') {
//TODO  	++current;
//TODO  	if (current == end) return JunkStringValue();
//TODO  	sign = NEGATIVE;
//TODO    }
//TODO	
//TODO    static const char kInfinitySymbol[] = "Infinity";
//TODO    if (*current == kInfinitySymbol[0]) {
//TODO  	if (!SubStringEquals(&current, end, kInfinitySymbol)) {
//TODO  	  return JunkStringValue();
//TODO  	}
//TODO	
//TODO  	if (!allow_trailing_junk &&
//TODO  		AdvanceToNonspace(unicode_cache, &current, end)) {
//TODO  	  return JunkStringValue();
//TODO  	}
//TODO	
//TODO  	ASSERT(buffer_pos == 0);
//TODO  	return (sign == NEGATIVE) ? -V8_INFINITY : V8_INFINITY;
//TODO    }
//TODO	
//TODO    bool leading_zero = false;
//TODO    if (*current == '0') {
//TODO  	++current;
//TODO  	if (current == end) return SignedZero(sign == NEGATIVE);
//TODO	
//TODO  	leading_zero = true;
//TODO	
//TODO  	// It could be hexadecimal value.
//TODO  	if ((flags & ALLOW_HEX) && (*current == 'x' || *current == 'X')) {
//TODO  	  ++current;
//TODO  	  if (current == end || !isDigit(*current, 16) || sign != NONE) {
//TODO  		return JunkStringValue();  // "0x".
//TODO  	  }
//TODO	
//TODO  	  return InternalStringToIntDouble<4>(unicode_cache,
//TODO  										  current,
//TODO  										  end,
//TODO  										  false,
//TODO  										  allow_trailing_junk);
//TODO  	}
//TODO	
//TODO  	// Ignore leading zeros in the integer part.
//TODO  	while (*current == '0') {
//TODO  	  ++current;
//TODO  	  if (current == end) return SignedZero(sign == NEGATIVE);
//TODO  	}
//TODO    }
//TODO	
//TODO    bool octal = leading_zero && (flags & ALLOW_OCTALS) != 0;
//TODO	
//TODO    // Copy significant digits of the integer part (if any) to the buffer.
//TODO    while (*current >= '0' && *current <= '9') {
//TODO  	if (significant_digits < kMaxSignificantDigits) {
//TODO  	  ASSERT(buffer_pos < kBufferSize);
//TODO  	  buffer[buffer_pos++] = static_cast<char>(*current);
//TODO  	  significant_digits++;
//TODO  	  // Will later check if it's an octal in the buffer.
//TODO  	} else {
//TODO  	  insignificant_digits++;  // Move the digit into the exponential part.
//TODO  	  nonzero_digit_dropped = nonzero_digit_dropped || *current != '0';
//TODO  	}
//TODO  	octal = octal && *current < '8';
//TODO  	++current;
//TODO  	if (current == end) goto parsing_done;
//TODO    }
//TODO	
//TODO    if (significant_digits == 0) {
//TODO  	octal = false;
//TODO    }
//TODO	
//TODO    if (*current == '.') {
//TODO  	if (octal && !allow_trailing_junk) return JunkStringValue();
//TODO  	if (octal) goto parsing_done;
//TODO	
//TODO  	++current;
//TODO  	if (current == end) {
//TODO  	  if (significant_digits == 0 && !leading_zero) {
//TODO  		return JunkStringValue();
//TODO  	  } else {
//TODO  		goto parsing_done;
//TODO  	  }
//TODO  	}
//TODO	
//TODO  	if (significant_digits == 0) {
//TODO  	  // octal = false;
//TODO  	  // Integer part consists of 0 or is absent. Significant digits start after
//TODO  	  // leading zeros (if any).
//TODO  	  while (*current == '0') {
//TODO  		++current;
//TODO  		if (current == end) return SignedZero(sign == NEGATIVE);
//TODO  		exponent--;  // Move this 0 into the exponent.
//TODO  	  }
//TODO  	}
//TODO	
//TODO  	// There is a fractional part.  We don't emit a '.', but adjust the exponent
//TODO  	// instead.
//TODO  	while (*current >= '0' && *current <= '9') {
//TODO  	  if (significant_digits < kMaxSignificantDigits) {
//TODO  		ASSERT(buffer_pos < kBufferSize);
//TODO  		buffer[buffer_pos++] = static_cast<char>(*current);
//TODO  		significant_digits++;
//TODO  		exponent--;
//TODO  	  } else {
//TODO  		// Ignore insignificant digits in the fractional part.
//TODO  		nonzero_digit_dropped = nonzero_digit_dropped || *current != '0';
//TODO  	  }
//TODO  	  ++current;
//TODO  	  if (current == end) goto parsing_done;
//TODO  	}
//TODO    }
//TODO	
//TODO    if (!leading_zero && exponent == 0 && significant_digits == 0) {
//TODO  	// If leading_zeros is true then the string contains zeros.
//TODO  	// If exponent < 0 then string was [+-]\.0*...
//TODO  	// If significant_digits != 0 the string is not equal to 0.
//TODO  	// Otherwise there are no digits in the string.
//TODO  	return JunkStringValue();
//TODO    }
//TODO	
//TODO    // Parse exponential part.
//TODO    if (*current == 'e' || *current == 'E') {
//TODO  	if (octal) return JunkStringValue();
//TODO  	++current;
//TODO  	if (current == end) {
//TODO  	  if (allow_trailing_junk) {
//TODO  		goto parsing_done;
//TODO  	  } else {
//TODO  		return JunkStringValue();
//TODO  	  }
//TODO  	}
//TODO  	char sign = '+';
//TODO  	if (*current == '+' || *current == '-') {
//TODO  	  sign = static_cast<char>(*current);
//TODO  	  ++current;
//TODO  	  if (current == end) {
//TODO  		if (allow_trailing_junk) {
//TODO  		  goto parsing_done;
//TODO  		} else {
//TODO  		  return JunkStringValue();
//TODO  		}
//TODO  	  }
//TODO  	}
//TODO	
//TODO  	if (current == end || *current < '0' || *current > '9') {
//TODO  	  if (allow_trailing_junk) {
//TODO  		goto parsing_done;
//TODO  	  } else {
//TODO  		return JunkStringValue();
//TODO  	  }
//TODO  	}
//TODO	
//TODO  	const int max_exponent = INT_MAX / 2;
//TODO  	ASSERT(-max_exponent / 2 <= exponent && exponent <= max_exponent / 2);
//TODO  	int num = 0;
//TODO  	do {
//TODO  	  // Check overflow.
//TODO  	  int digit = *current - '0';
//TODO  	  if (num >= max_exponent / 10
//TODO  		  && !(num == max_exponent / 10 && digit <= max_exponent % 10)) {
//TODO  		num = max_exponent;
//TODO  	  } else {
//TODO  		num = num * 10 + digit;
//TODO  	  }
//TODO  	  ++current;
//TODO  	} while (current != end && *current >= '0' && *current <= '9');
//TODO	
//TODO  	exponent += (sign == '-' ? -num : num);
//TODO    }
//TODO	
//TODO    if (!allow_trailing_junk &&
//TODO  	  AdvanceToNonspace(unicode_cache, &current, end)) {
//TODO  	return JunkStringValue();
//TODO    }
//TODO	
//TODO    parsing_done:
//TODO    exponent += insignificant_digits;
//TODO	
//TODO    if (octal) {
//TODO  	return InternalStringToIntDouble<3>(unicode_cache,
//TODO  										buffer,
//TODO  										buffer + buffer_pos,
//TODO  										sign == NEGATIVE,
//TODO  										allow_trailing_junk);
//TODO    }
//TODO	
//TODO    if (nonzero_digit_dropped) {
//TODO  	buffer[buffer_pos++] = '1';
//TODO  	exponent--;
//TODO    }
//TODO	
//TODO    ASSERT(buffer_pos < kBufferSize);
//TODO    buffer[buffer_pos] = '\0';
//TODO	
//TODO    double converted = Strtod(Vector<const char>(buffer, buffer_pos), exponent);
//TODO    return (sign == NEGATIVE) ? -converted : converted;
//TODO	}

} }  // namespace v8::internal

#endif  // V8_CONVERSIONS_INL_H_

