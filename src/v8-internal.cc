#include "v8-internal.h"

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

/// v8/src/allocation.cc

// Copyright 2012 the V8 project authors. All rights reserved.
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

//TODO	#include "allocation.h"
//TODO	
//TODO	#include <stdlib.h>  // For free, malloc.
//TODO	#include <string.h>  // For memcpy.
//TODO	#include "checks.h"
//TODO	#include "utils.h"
//TODO	
//TODO	namespace v8 {
//TODO	namespace internal {
//TODO	
//TODO	void* Malloced::New(size_t size) {
//TODO	  void* result = malloc(size);
//TODO	  if (result == NULL) {
//TODO	    v8::internal::FatalProcessOutOfMemory("Malloced operator new");
//TODO	  }
//TODO	  return result;
//TODO	}
//TODO	
//TODO	
//TODO	void Malloced::Delete(void* p) {
//TODO	  free(p);
//TODO	}
//TODO	
//TODO	
//TODO	void Malloced::FatalProcessOutOfMemory() {
//TODO	  v8::internal::FatalProcessOutOfMemory("Out of memory");
//TODO	}
//TODO	
//TODO	
//TODO	#ifdef DEBUG
//TODO	
//TODO	static void* invalid = static_cast<void*>(NULL);
//TODO	
//TODO	void* Embedded::operator new(size_t size) {
//TODO	  UNREACHABLE();
//TODO	  return invalid;
//TODO	}
//TODO	
//TODO	
//TODO	void Embedded::operator delete(void* p) {
//TODO	  UNREACHABLE();
//TODO	}
//TODO	
//TODO	
//TODO	void* AllStatic::operator new(size_t size) {
//TODO	  UNREACHABLE();
//TODO	  return invalid;
//TODO	}
//TODO	
//TODO	
//TODO	void AllStatic::operator delete(void* p) {
//TODO	  UNREACHABLE();
//TODO	}
//TODO	
//TODO	#endif
//TODO	
//TODO	
//TODO	char* StrDup(const char* str) {
//TODO	  int length = StrLength(str);
//TODO	  char* result = NewArray<char>(length + 1);
//TODO	  memcpy(result, str, length);
//TODO	  result[length] = '\0';
//TODO	  return result;
//TODO	}
//TODO	
//TODO	
//TODO	char* StrNDup(const char* str, int n) {
//TODO	  int length = StrLength(str);
//TODO	  if (n < length) length = n;
//TODO	  char* result = NewArray<char>(length + 1);
//TODO	  memcpy(result, str, length);
//TODO	  result[length] = '\0';
//TODO	  return result;
//TODO	}
//TODO	
//TODO	
//TODO	void PreallocatedStorage::LinkTo(PreallocatedStorage* other) {
//TODO	  next_ = other->next_;
//TODO	  other->next_->previous_ = this;
//TODO	  previous_ = other;
//TODO	  other->next_ = this;
//TODO	}
//TODO	
//TODO	
//TODO	void PreallocatedStorage::Unlink() {
//TODO	  next_->previous_ = previous_;
//TODO	  previous_->next_ = next_;
//TODO	}
//TODO	
//TODO	
//TODO	PreallocatedStorage::PreallocatedStorage(size_t size)
//TODO	  : size_(size) {
//TODO	  previous_ = next_ = this;
//TODO	}
//TODO	
//TODO	} }  // namespace v8::internal

/// v8/src/utils.cc

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

//TODO	#include <stdarg.h>
//TODO	#include "../include/v8stdint.h"
//TODO	#include "checks.h"
//TODO	#include "utils.h"
//TODO	
//TODO	namespace v8 {
//TODO	namespace internal {
//TODO	
//TODO	
//TODO	SimpleStringBuilder::SimpleStringBuilder(int size) {
//TODO	  buffer_ = Vector<char>::New(size);
//TODO	  position_ = 0;
//TODO	}
//TODO	
//TODO	
//TODO	void SimpleStringBuilder::AddString(const char* s) {
//TODO	  AddSubstring(s, StrLength(s));
//TODO	}
//TODO	
//TODO	
//TODO	void SimpleStringBuilder::AddSubstring(const char* s, int n) {
//TODO	  ASSERT(!is_finalized() && position_ + n < buffer_.length());
//TODO	  ASSERT(static_cast<size_t>(n) <= strlen(s));
//TODO	  memcpy(&buffer_[position_], s, n * kCharSize);
//TODO	  position_ += n;
//TODO	}
//TODO	
//TODO	
//TODO	void SimpleStringBuilder::AddPadding(char c, int count) {
//TODO	  for (int i = 0; i < count; i++) {
//TODO	    AddCharacter(c);
//TODO	  }
//TODO	}
//TODO	
//TODO	
//TODO	void SimpleStringBuilder::AddDecimalInteger(int32_t value) {
//TODO	  uint32_t number = static_cast<uint32_t>(value);
//TODO	  if (value < 0) {
//TODO	    AddCharacter('-');
//TODO	    number = static_cast<uint32_t>(-value);
//TODO	  }
//TODO	  int digits = 1;
//TODO	  for (uint32_t factor = 10; digits < 10; digits++, factor *= 10) {
//TODO	    if (factor > number) break;
//TODO	  }
//TODO	  position_ += digits;
//TODO	  for (int i = 1; i <= digits; i++) {
//TODO	    buffer_[position_ - i] = '0' + static_cast<char>(number % 10);
//TODO	    number /= 10;
//TODO	  }
//TODO	}
//TODO	
//TODO	
//TODO	char* SimpleStringBuilder::Finalize() {
//TODO	  ASSERT(!is_finalized() && position_ < buffer_.length());
//TODO	  buffer_[position_] = '\0';
//TODO	  // Make sure nobody managed to add a 0-character to the
//TODO	  // buffer while building the string.
//TODO	  ASSERT(strlen(buffer_.start()) == static_cast<size_t>(position_));
//TODO	  position_ = -1;
//TODO	  ASSERT(is_finalized());
//TODO	  return buffer_.start();
//TODO	}
//TODO	
//TODO	
//TODO	const DivMagicNumbers DivMagicNumberFor(int32_t divisor) {
//TODO	  switch (divisor) {
//TODO	    case 3:    return DivMagicNumberFor3;
//TODO	    case 5:    return DivMagicNumberFor5;
//TODO	    case 7:    return DivMagicNumberFor7;
//TODO	    case 9:    return DivMagicNumberFor9;
//TODO	    case 11:   return DivMagicNumberFor11;
//TODO	    case 25:   return DivMagicNumberFor25;
//TODO	    case 125:  return DivMagicNumberFor125;
//TODO	    case 625:  return DivMagicNumberFor625;
//TODO	    default:   return InvalidDivMagicNumber;
//TODO	  }
//TODO	}
//TODO	
//TODO	} }  // namespace v8::internal

/// v8/src/diy-fp.cc

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

#include "../include/v8stdint.h"
//TODO	#include "globals.h"
//TODO	#include "checks.h"
//TODO	#include "diy-fp.h"

namespace v8 {
namespace internal {

void DiyFp::Multiply(const DiyFp& other) {
  // Simply "emulates" a 128 bit multiplication.
  // However: the resulting number only contains 64 bits. The least
  // significant 64 bits are only used for rounding the most significant 64
  // bits.
  const uint64_t kM32 = 0xFFFFFFFFu;
  uint64_t a = f_ >> 32;
  uint64_t b = f_ & kM32;
  uint64_t c = other.f_ >> 32;
  uint64_t d = other.f_ & kM32;
  uint64_t ac = a * c;
  uint64_t bc = b * c;
  uint64_t ad = a * d;
  uint64_t bd = b * d;
  uint64_t tmp = (bd >> 32) + (ad & kM32) + (bc & kM32);
  // By adding 1U << 31 to tmp we round the final result.
  // Halfway cases will be round up.
  tmp += 1U << 31;
  uint64_t result_f = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32);
  e_ += other.e_ + 64;
  f_ = result_f;
}

} }  // namespace v8::internal

/// v8/src/conversions.cc

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

#include <stdarg.h>
#include <math.h>
#include <limits.h>

//TODO	#include "conversions-inl.h"
//TODO	#include "dtoa.h"
//TODO	#include "strtod.h"
//TODO	#include "utils.h"

namespace v8 {
namespace internal {


//TODO	double StringToDouble(UnicodeCache* unicode_cache,
//TODO	                      const char* str, int flags, double empty_string_val) {
//TODO	  const char* end = str + StrLength(str);
//TODO	  return InternalStringToDouble(unicode_cache, str, end, flags,
//TODO	                                empty_string_val);
//TODO	}
//TODO	
//TODO	
//TODO	double StringToDouble(UnicodeCache* unicode_cache,
//TODO	                      Vector<const char> str,
//TODO	                      int flags,
//TODO	                      double empty_string_val) {
//TODO	  const char* end = str.start() + str.length();
//TODO	  return InternalStringToDouble(unicode_cache, str.start(), end, flags,
//TODO	                                empty_string_val);
//TODO	}
//TODO	
//TODO	double StringToDouble(UnicodeCache* unicode_cache,
//TODO	                      Vector<const uc16> str,
//TODO	                      int flags,
//TODO	                      double empty_string_val) {
//TODO	  const uc16* end = str.start() + str.length();
//TODO	  return InternalStringToDouble(unicode_cache, str.start(), end, flags,
//TODO	                                empty_string_val);
//TODO	}
//TODO	
//TODO	
//TODO	const char* DoubleToCString(double v, Vector<char> buffer) {
//TODO	  switch (fpclassify(v)) {
//TODO	    case FP_NAN: return "NaN";
//TODO	    case FP_INFINITE: return (v < 0.0 ? "-Infinity" : "Infinity");
//TODO	    case FP_ZERO: return "0";
//TODO	    default: {
//TODO	      SimpleStringBuilder builder(buffer.start(), buffer.length());
//TODO	      int decimal_point;
//TODO	      int sign;
//TODO	      const int kV8DtoaBufferCapacity = kBase10MaximalLength + 1;
//TODO	      char decimal_rep[kV8DtoaBufferCapacity];
//TODO	      int length;
//TODO	
//TODO	      DoubleToAscii(v, DTOA_SHORTEST, 0,
//TODO	                    Vector<char>(decimal_rep, kV8DtoaBufferCapacity),
//TODO	                    &sign, &length, &decimal_point);
//TODO	
//TODO	      if (sign) builder.AddCharacter('-');
//TODO	
//TODO	      if (length <= decimal_point && decimal_point <= 21) {
//TODO	        // ECMA-262 section 9.8.1 step 6.
//TODO	        builder.AddString(decimal_rep);
//TODO	        builder.AddPadding('0', decimal_point - length);
//TODO	
//TODO	      } else if (0 < decimal_point && decimal_point <= 21) {
//TODO	        // ECMA-262 section 9.8.1 step 7.
//TODO	        builder.AddSubstring(decimal_rep, decimal_point);
//TODO	        builder.AddCharacter('.');
//TODO	        builder.AddString(decimal_rep + decimal_point);
//TODO	
//TODO	      } else if (decimal_point <= 0 && decimal_point > -6) {
//TODO	        // ECMA-262 section 9.8.1 step 8.
//TODO	        builder.AddString("0.");
//TODO	        builder.AddPadding('0', -decimal_point);
//TODO	        builder.AddString(decimal_rep);
//TODO	
//TODO	      } else {
//TODO	        // ECMA-262 section 9.8.1 step 9 and 10 combined.
//TODO	        builder.AddCharacter(decimal_rep[0]);
//TODO	        if (length != 1) {
//TODO	          builder.AddCharacter('.');
//TODO	          builder.AddString(decimal_rep + 1);
//TODO	        }
//TODO	        builder.AddCharacter('e');
//TODO	        builder.AddCharacter((decimal_point >= 0) ? '+' : '-');
//TODO	        int exponent = decimal_point - 1;
//TODO	        if (exponent < 0) exponent = -exponent;
//TODO	        builder.AddDecimalInteger(exponent);
//TODO	      }
//TODO	    return builder.Finalize();
//TODO	    }
//TODO	  }
//TODO	}
//TODO	
//TODO	
//TODO	const char* IntToCString(int n, Vector<char> buffer) {
//TODO	  bool negative = false;
//TODO	  if (n < 0) {
//TODO	    // We must not negate the most negative int.
//TODO	    if (n == kMinInt) return DoubleToCString(n, buffer);
//TODO	    negative = true;
//TODO	    n = -n;
//TODO	  }
//TODO	  // Build the string backwards from the least significant digit.
//TODO	  int i = buffer.length();
//TODO	  buffer[--i] = '\0';
//TODO	  do {
//TODO	    buffer[--i] = '0' + (n % 10);
//TODO	    n /= 10;
//TODO	  } while (n);
//TODO	  if (negative) buffer[--i] = '-';
//TODO	  return buffer.start() + i;
//TODO	}
//TODO	
//TODO	
//TODO	char* DoubleToFixedCString(double value, int f) {
//TODO	  const int kMaxDigitsBeforePoint = 21;
//TODO	  const double kFirstNonFixed = 1e21;
//TODO	  const int kMaxDigitsAfterPoint = 20;
//TODO	  ASSERT(f >= 0);
//TODO	  ASSERT(f <= kMaxDigitsAfterPoint);
//TODO	
//TODO	  bool negative = false;
//TODO	  double abs_value = value;
//TODO	  if (value < 0) {
//TODO	    abs_value = -value;
//TODO	    negative = true;
//TODO	  }
//TODO	
//TODO	  // If abs_value has more than kMaxDigitsBeforePoint digits before the point
//TODO	  // use the non-fixed conversion routine.
//TODO	  if (abs_value >= kFirstNonFixed) {
//TODO	    char arr[100];
//TODO	    Vector<char> buffer(arr, ARRAY_SIZE(arr));
//TODO	    return StrDup(DoubleToCString(value, buffer));
//TODO	  }
//TODO	
//TODO	  // Find a sufficiently precise decimal representation of n.
//TODO	  int decimal_point;
//TODO	  int sign;
//TODO	  // Add space for the '\0' byte.
//TODO	  const int kDecimalRepCapacity =
//TODO	      kMaxDigitsBeforePoint + kMaxDigitsAfterPoint + 1;
//TODO	  char decimal_rep[kDecimalRepCapacity];
//TODO	  int decimal_rep_length;
//TODO	  DoubleToAscii(value, DTOA_FIXED, f,
//TODO	                Vector<char>(decimal_rep, kDecimalRepCapacity),
//TODO	                &sign, &decimal_rep_length, &decimal_point);
//TODO	
//TODO	  // Create a representation that is padded with zeros if needed.
//TODO	  int zero_prefix_length = 0;
//TODO	  int zero_postfix_length = 0;
//TODO	
//TODO	  if (decimal_point <= 0) {
//TODO	    zero_prefix_length = -decimal_point + 1;
//TODO	    decimal_point = 1;
//TODO	  }
//TODO	
//TODO	  if (zero_prefix_length + decimal_rep_length < decimal_point + f) {
//TODO	    zero_postfix_length = decimal_point + f - decimal_rep_length -
//TODO	                          zero_prefix_length;
//TODO	  }
//TODO	
//TODO	  unsigned rep_length =
//TODO	      zero_prefix_length + decimal_rep_length + zero_postfix_length;
//TODO	  SimpleStringBuilder rep_builder(rep_length + 1);
//TODO	  rep_builder.AddPadding('0', zero_prefix_length);
//TODO	  rep_builder.AddString(decimal_rep);
//TODO	  rep_builder.AddPadding('0', zero_postfix_length);
//TODO	  char* rep = rep_builder.Finalize();
//TODO	
//TODO	  // Create the result string by appending a minus and putting in a
//TODO	  // decimal point if needed.
//TODO	  unsigned result_size = decimal_point + f + 2;
//TODO	  SimpleStringBuilder builder(result_size + 1);
//TODO	  if (negative) builder.AddCharacter('-');
//TODO	  builder.AddSubstring(rep, decimal_point);
//TODO	  if (f > 0) {
//TODO	    builder.AddCharacter('.');
//TODO	    builder.AddSubstring(rep + decimal_point, f);
//TODO	  }
//TODO	  DeleteArray(rep);
//TODO	  return builder.Finalize();
//TODO	}
//TODO	
//TODO	
//TODO	static char* CreateExponentialRepresentation(char* decimal_rep,
//TODO	                                             int exponent,
//TODO	                                             bool negative,
//TODO	                                             int significant_digits) {
//TODO	  bool negative_exponent = false;
//TODO	  if (exponent < 0) {
//TODO	    negative_exponent = true;
//TODO	    exponent = -exponent;
//TODO	  }
//TODO	
//TODO	  // Leave room in the result for appending a minus, for a period, the
//TODO	  // letter 'e', a minus or a plus depending on the exponent, and a
//TODO	  // three digit exponent.
//TODO	  unsigned result_size = significant_digits + 7;
//TODO	  SimpleStringBuilder builder(result_size + 1);
//TODO	
//TODO	  if (negative) builder.AddCharacter('-');
//TODO	  builder.AddCharacter(decimal_rep[0]);
//TODO	  if (significant_digits != 1) {
//TODO	    builder.AddCharacter('.');
//TODO	    builder.AddString(decimal_rep + 1);
//TODO	    int rep_length = StrLength(decimal_rep);
//TODO	    builder.AddPadding('0', significant_digits - rep_length);
//TODO	  }
//TODO	
//TODO	  builder.AddCharacter('e');
//TODO	  builder.AddCharacter(negative_exponent ? '-' : '+');
//TODO	  builder.AddDecimalInteger(exponent);
//TODO	  return builder.Finalize();
//TODO	}
//TODO	
//TODO	
//TODO	
//TODO	char* DoubleToExponentialCString(double value, int f) {
//TODO	  const int kMaxDigitsAfterPoint = 20;
//TODO	  // f might be -1 to signal that f was undefined in JavaScript.
//TODO	  ASSERT(f >= -1 && f <= kMaxDigitsAfterPoint);
//TODO	
//TODO	  bool negative = false;
//TODO	  if (value < 0) {
//TODO	    value = -value;
//TODO	    negative = true;
//TODO	  }
//TODO	
//TODO	  // Find a sufficiently precise decimal representation of n.
//TODO	  int decimal_point;
//TODO	  int sign;
//TODO	  // f corresponds to the digits after the point. There is always one digit
//TODO	  // before the point. The number of requested_digits equals hence f + 1.
//TODO	  // And we have to add one character for the null-terminator.
//TODO	  const int kV8DtoaBufferCapacity = kMaxDigitsAfterPoint + 1 + 1;
//TODO	  // Make sure that the buffer is big enough, even if we fall back to the
//TODO	  // shortest representation (which happens when f equals -1).
//TODO	  ASSERT(kBase10MaximalLength <= kMaxDigitsAfterPoint + 1);
//TODO	  char decimal_rep[kV8DtoaBufferCapacity];
//TODO	  int decimal_rep_length;
//TODO	
//TODO	  if (f == -1) {
//TODO	    DoubleToAscii(value, DTOA_SHORTEST, 0,
//TODO	                  Vector<char>(decimal_rep, kV8DtoaBufferCapacity),
//TODO	                  &sign, &decimal_rep_length, &decimal_point);
//TODO	    f = decimal_rep_length - 1;
//TODO	  } else {
//TODO	    DoubleToAscii(value, DTOA_PRECISION, f + 1,
//TODO	                  Vector<char>(decimal_rep, kV8DtoaBufferCapacity),
//TODO	                  &sign, &decimal_rep_length, &decimal_point);
//TODO	  }
//TODO	  ASSERT(decimal_rep_length > 0);
//TODO	  ASSERT(decimal_rep_length <= f + 1);
//TODO	
//TODO	  int exponent = decimal_point - 1;
//TODO	  char* result =
//TODO	      CreateExponentialRepresentation(decimal_rep, exponent, negative, f+1);
//TODO	
//TODO	  return result;
//TODO	}
//TODO	
//TODO	
//TODO	char* DoubleToPrecisionCString(double value, int p) {
//TODO	  const int kMinimalDigits = 1;
//TODO	  const int kMaximalDigits = 21;
//TODO	  ASSERT(p >= kMinimalDigits && p <= kMaximalDigits);
//TODO	  USE(kMinimalDigits);
//TODO	
//TODO	  bool negative = false;
//TODO	  if (value < 0) {
//TODO	    value = -value;
//TODO	    negative = true;
//TODO	  }
//TODO	
//TODO	  // Find a sufficiently precise decimal representation of n.
//TODO	  int decimal_point;
//TODO	  int sign;
//TODO	  // Add one for the terminating null character.
//TODO	  const int kV8DtoaBufferCapacity = kMaximalDigits + 1;
//TODO	  char decimal_rep[kV8DtoaBufferCapacity];
//TODO	  int decimal_rep_length;
//TODO	
//TODO	  DoubleToAscii(value, DTOA_PRECISION, p,
//TODO	                Vector<char>(decimal_rep, kV8DtoaBufferCapacity),
//TODO	                &sign, &decimal_rep_length, &decimal_point);
//TODO	  ASSERT(decimal_rep_length <= p);
//TODO	
//TODO	  int exponent = decimal_point - 1;
//TODO	
//TODO	  char* result = NULL;
//TODO	
//TODO	  if (exponent < -6 || exponent >= p) {
//TODO	    result =
//TODO	        CreateExponentialRepresentation(decimal_rep, exponent, negative, p);
//TODO	  } else {
//TODO	    // Use fixed notation.
//TODO	    //
//TODO	    // Leave room in the result for appending a minus, a period and in
//TODO	    // the case where decimal_point is not positive for a zero in
//TODO	    // front of the period.
//TODO	    unsigned result_size = (decimal_point <= 0)
//TODO	        ? -decimal_point + p + 3
//TODO	        : p + 2;
//TODO	    SimpleStringBuilder builder(result_size + 1);
//TODO	    if (negative) builder.AddCharacter('-');
//TODO	    if (decimal_point <= 0) {
//TODO	      builder.AddString("0.");
//TODO	      builder.AddPadding('0', -decimal_point);
//TODO	      builder.AddString(decimal_rep);
//TODO	      builder.AddPadding('0', p - decimal_rep_length);
//TODO	    } else {
//TODO	      const int m = Min(decimal_rep_length, decimal_point);
//TODO	      builder.AddSubstring(decimal_rep, m);
//TODO	      builder.AddPadding('0', decimal_point - decimal_rep_length);
//TODO	      if (decimal_point < p) {
//TODO	        builder.AddCharacter('.');
//TODO	        const int extra = negative ? 2 : 1;
//TODO	        if (decimal_rep_length > decimal_point) {
//TODO	          const int len = StrLength(decimal_rep + decimal_point);
//TODO	          const int n = Min(len, p - (builder.position() - extra));
//TODO	          builder.AddSubstring(decimal_rep + decimal_point, n);
//TODO	        }
//TODO	        builder.AddPadding('0', extra + (p - builder.position()));
//TODO	      }
//TODO	    }
//TODO	    result = builder.Finalize();
//TODO	  }
//TODO	
//TODO	  return result;
//TODO	}
//TODO	
//TODO	
//TODO	char* DoubleToRadixCString(double value, int radix) {
//TODO	  ASSERT(radix >= 2 && radix <= 36);
//TODO	
//TODO	  // Character array used for conversion.
//TODO	  static const char chars[] = "0123456789abcdefghijklmnopqrstuvwxyz";
//TODO	
//TODO	  // Buffer for the integer part of the result. 1024 chars is enough
//TODO	  // for max integer value in radix 2.  We need room for a sign too.
//TODO	  static const int kBufferSize = 1100;
//TODO	  char integer_buffer[kBufferSize];
//TODO	  integer_buffer[kBufferSize - 1] = '\0';
//TODO	
//TODO	  // Buffer for the decimal part of the result.  We only generate up
//TODO	  // to kBufferSize - 1 chars for the decimal part.
//TODO	  char decimal_buffer[kBufferSize];
//TODO	  decimal_buffer[kBufferSize - 1] = '\0';
//TODO	
//TODO	  // Make sure the value is positive.
//TODO	  bool is_negative = value < 0.0;
//TODO	  if (is_negative) value = -value;
//TODO	
//TODO	  // Get the integer part and the decimal part.
//TODO	  double integer_part = floor(value);
//TODO	  double decimal_part = value - integer_part;
//TODO	
//TODO	  // Convert the integer part starting from the back.  Always generate
//TODO	  // at least one digit.
//TODO	  int integer_pos = kBufferSize - 2;
//TODO	  do {
//TODO	    integer_buffer[integer_pos--] =
//TODO	        chars[static_cast<int>(fmod(integer_part, radix))];
//TODO	    integer_part /= radix;
//TODO	  } while (integer_part >= 1.0);
//TODO	  // Sanity check.
//TODO	  ASSERT(integer_pos > 0);
//TODO	  // Add sign if needed.
//TODO	  if (is_negative) integer_buffer[integer_pos--] = '-';
//TODO	
//TODO	  // Convert the decimal part.  Repeatedly multiply by the radix to
//TODO	  // generate the next char.  Never generate more than kBufferSize - 1
//TODO	  // chars.
//TODO	  //
//TODO	  // TODO(1093998): We will often generate a full decimal_buffer of
//TODO	  // chars because hitting zero will often not happen.  The right
//TODO	  // solution would be to continue until the string representation can
//TODO	  // be read back and yield the original value.  To implement this
//TODO	  // efficiently, we probably have to modify dtoa.
//TODO	  int decimal_pos = 0;
//TODO	  while ((decimal_part > 0.0) && (decimal_pos < kBufferSize - 1)) {
//TODO	    decimal_part *= radix;
//TODO	    decimal_buffer[decimal_pos++] =
//TODO	        chars[static_cast<int>(floor(decimal_part))];
//TODO	    decimal_part -= floor(decimal_part);
//TODO	  }
//TODO	  decimal_buffer[decimal_pos] = '\0';
//TODO	
//TODO	  // Compute the result size.
//TODO	  int integer_part_size = kBufferSize - 2 - integer_pos;
//TODO	  // Make room for zero termination.
//TODO	  unsigned result_size = integer_part_size + decimal_pos;
//TODO	  // If the number has a decimal part, leave room for the period.
//TODO	  if (decimal_pos > 0) result_size++;
//TODO	  // Allocate result and fill in the parts.
//TODO	  SimpleStringBuilder builder(result_size + 1);
//TODO	  builder.AddSubstring(integer_buffer + integer_pos + 1, integer_part_size);
//TODO	  if (decimal_pos > 0) builder.AddCharacter('.');
//TODO	  builder.AddSubstring(decimal_buffer, decimal_pos);
//TODO	  return builder.Finalize();
//TODO	}

} }  // namespace v8::internal

