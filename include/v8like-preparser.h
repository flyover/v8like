/**
 * Copyright (c) Flyover Games, LLC.  All rights reserved. 
 *  
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated 
 * documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to 
 * whom the Software is furnished to do so, subject to the 
 * following conditions: 
 *  
 * The above copyright notice and this permission notice shall 
 * be included in all copies or substantial portions of the 
 * Software. 
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY 
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 */

#ifndef V8LIKE_PREPARSER_H_
#define V8LIKE_PREPARSER_H_

#include "v8stdint.h"

#ifdef _WIN32

typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;  // NOLINT
typedef long long int64_t;  // NOLINT

#if defined(BUILDING_V8_SHARED) && defined(USING_V8_SHARED)
#error both BUILDING_V8_SHARED and USING_V8_SHARED are set
#endif

#ifdef BUILDING_V8_SHARED
#define V8EXPORT __declspec(dllexport)
#elif USING_V8_SHARED
#define V8EXPORT __declspec(dllimport)
#else
#define V8EXPORT
#endif

#else  // _WIN32

#if defined(__GNUC__) && (__GNUC__ >= 4) && defined(V8_SHARED)
#define V8EXPORT __attribute__ ((visibility("default")))
#else
#define V8EXPORT
#endif

#endif // _WIN32

namespace v8 {

class V8EXPORT PreParserData
{
public:
	PreParserData(size_t size, const uint8_t* data) : data_(data), size_(size) {}
	static PreParserData StackOverflow() { return PreParserData(0, NULL); }
	bool stack_overflow() { return size_ == 0u; }
	size_t size() const { return size_; }
	const uint8_t* data() const { return data_; }

private:
	const uint8_t* const data_;
	const size_t size_;
};

class V8EXPORT UnicodeInputStream
{
public:
	virtual ~UnicodeInputStream();
	virtual int32_t Next() = 0;
};

PreParserData V8EXPORT Preparse(UnicodeInputStream* input, size_t max_stack_size);

}  // namespace v8

#undef V8EXPORT

#endif  // V8LIKE_PREPARSER_H_

