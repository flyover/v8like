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

#include "v8like.h"

#include "v8-internal.h" // borrowed *.h and *-inl.h from v8/src/...

#include <cmath>
#include <limits>

extern "C" void JSSynchronousGarbageCollectForDebugging(JSContextRef ctx);

static int JSStringCompare(JSStringRef str1, JSStringRef str2)
{
	const size_t n1 = JSStringGetLength(str1);
	const size_t n2 = JSStringGetLength(str2);
	if (n1 != n2)
	{
		return ((n1 < n2) ? -1 : +1);
	}
	else
	{
		const JSChar* s1 = JSStringGetCharactersPtr(str1);
		const JSChar* s2 = JSStringGetCharactersPtr(str2);
		size_t n = n1; // n1 == n2
		for ( ; n--; s1++, s2++)
			if (*s1 != *s2)
				return ((*s1 < *s2) ? -1 : +1);
		return 0;
	}
}

static bool JSValueIsBooleanObject(JSContextRef ctx, JSValueRef value)
{
	JSStringRef name = JSStringCreateWithUTF8CString("Boolean");
	JSObjectRef ctor = JSValueToObject(ctx, JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), name, NULL), NULL);
	JSStringRelease(name);

	return JSValueIsInstanceOfConstructor(ctx, value, ctor, NULL);
}

static bool JSValueIsNumberObject(JSContextRef ctx, JSValueRef value)
{
	JSStringRef name = JSStringCreateWithUTF8CString("Number");
	JSObjectRef ctor = JSValueToObject(ctx, JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), name, NULL), NULL);
	JSStringRelease(name);

	return JSValueIsInstanceOfConstructor(ctx, value, ctor, NULL);
}

static bool JSValueIsStringObject(JSContextRef ctx, JSValueRef value)
{
	JSStringRef name = JSStringCreateWithUTF8CString("String");
	JSObjectRef ctor = JSValueToObject(ctx, JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), name, NULL), NULL);
	JSStringRelease(name);

	return JSValueIsInstanceOfConstructor(ctx, value, ctor, NULL);
}

static bool JSValueIsFunction(JSContextRef ctx, JSValueRef value)
{
	//JSStringRef name = JSStringCreateWithUTF8CString("Function");
	//JSObjectRef ctor = JSValueToObject(ctx, JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), name, NULL), NULL);
	//JSStringRelease(name);

	//return JSValueIsInstanceOfConstructor(ctx, value, ctor, NULL);

	if (JSValueIsObject(ctx, value))
	{
		JSObjectRef object = JSValueToObject(ctx, value, NULL);
		return JSObjectIsFunction(ctx, object);
	}
	return false;
}

static bool JSValueIsArray(JSContextRef ctx, JSValueRef value)
{
	JSStringRef name = JSStringCreateWithUTF8CString("Array");
	JSObjectRef ctor = JSValueToObject(ctx, JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), name, NULL), NULL);
	JSStringRelease(name);

	//return JSValueIsInstanceOfConstructor(ctx, value, ctor, NULL);

	name = JSStringCreateWithUTF8CString("isArray");
	JSObjectRef func = JSValueToObject(ctx, JSObjectGetProperty(ctx, ctor, name, NULL), NULL);
	JSStringRelease(name);

	JSValueRef retval = JSObjectCallAsFunction(ctx, func, NULL, 1, &value, NULL);

	if (JSValueIsBoolean(ctx, retval))
	{
		return JSValueToBoolean(ctx, retval);
	}

	return false;
}

static bool JSValueIsDate(JSContextRef ctx, JSValueRef value)
{
	JSStringRef name = JSStringCreateWithUTF8CString("Date");
	JSObjectRef ctor = JSValueToObject(ctx, JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), name, NULL), NULL);
	JSStringRelease(name);

	return JSValueIsInstanceOfConstructor(ctx, value, ctor, NULL);
}

static bool JSValueIsRegExp(JSContextRef ctx, JSValueRef value)
{
	JSStringRef name = JSStringCreateWithUTF8CString("RegExp");
	JSObjectRef ctor = JSValueToObject(ctx, JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), name, NULL), NULL);
	JSStringRelease(name);

	return JSValueIsInstanceOfConstructor(ctx, value, ctor, NULL);
}

static bool JSValueIsError(JSContextRef ctx, JSValueRef value)
{
	JSStringRef name = JSStringCreateWithUTF8CString("Error");
	JSObjectRef ctor = JSValueToObject(ctx, JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), name, NULL), NULL);
	JSStringRelease(name);

	return JSValueIsInstanceOfConstructor(ctx, value, ctor, NULL);
}

#if 1 // V8LIKE_DEBUG
static bool sg_debug_print_script_stack_enable = false;
static void JSDebugPrintScriptStack(JSContextRef js_ctx)
{
	if (sg_debug_print_script_stack_enable)
	{
		sg_debug_print_script_stack_enable = false;
		JSValueRef js_exception = NULL;
		JSValueToObject(js_ctx, JSValueMakeUndefined(js_ctx), &js_exception);
		JSObjectRef js_error = JSValueToObject(js_ctx, js_exception, NULL);
		JSStringRef js_name = JSStringCreateWithUTF8CString("stack");
		JSValueRef js_stack = JSObjectGetProperty(js_ctx, js_error, js_name, NULL);
		JSStringRef js_string = JSValueToStringCopy(js_ctx, js_stack, NULL);
		size_t js_size = JSStringGetMaximumUTF8CStringSize(js_string);
		char* js_buffer = new char[js_size];
		JSStringGetUTF8CString(js_string, js_buffer, js_size);
		printf("%s\n", js_buffer);
		delete[] js_buffer; js_buffer = NULL;
		JSStringRelease(js_string); js_string = NULL;
		JSStringRelease(js_name); js_name = NULL;
	}
}
#endif

namespace v8 {

static Handle<Value> sg_last_exception;

/// v8::internal::SmartObject

void internal::SmartObject::IncRef()
{
	if (!_IsNearDeath())
	{
		if (m_ref++ == 0)
		{
			_OnCreate();
		}
		assert(m_ref > 0); // check for overflow
	}
}
void internal::SmartObject::DecRef()
{
	if (!_IsNearDeath())
	{
		assert(m_ref > 0); // check for underflow
		if (--m_ref == 0)
		{
			_OnDelete();
			delete this;
		}
	}
}

/// v8::internal

JSValueRef internal::ExportValue(Handle<Value> value)
{
	return value->m_js_value;
}

JSObjectRef internal::ExportObject(Handle<Object> object)
{
	return object->m_js_object;
}

Handle<Value> internal::ImportValue(JSContextRef js_ctx, JSValueRef js_value)
{
	if (JSValueIsObjectOfClass(js_ctx, js_value, External::GetJSClass()))
	{
		// import pure-V8 external
		JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
		assert(js_object == js_value);
		Handle<Value> value((External*) JSObjectGetPrivate(js_object));
		assert(!value.IsEmpty());
		return value;
	}

	if (JSValueIsObject(js_ctx, js_value))
	{
		// import object
		JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
		assert(js_object == js_value);
		return Handle<Value>(internal::ImportObject(js_ctx, js_object));
	}

	if (JSValueIsBoolean(js_ctx, js_value))
	{
		// import pure-JS boolean value
		return Handle<Value>(new Boolean(js_ctx, js_value));
	}

	if (JSValueIsNumber(js_ctx, js_value))
	{
		// import pure-JS number value
		return Handle<Value>(new Number(js_ctx, js_value));
	}

	if (JSValueIsString(js_ctx, js_value))
	{
		// import pure-JS string value
		return Handle<Value>(new String(js_ctx, js_value));
	}

	// import pure-JS value (null or undefined)
	assert(JSValueIsNull(js_ctx, js_value) || JSValueIsUndefined(js_ctx, js_value));
	return Handle<Value>(new Value(js_ctx, js_value));
}

Handle<Object> internal::ImportObject(JSContextRef js_ctx, JSObjectRef js_object)
{
	if (JSValueIsObjectOfClass(js_ctx, js_object, Object::GetJSClass()))
	{
		// import pure-V8 object
		Handle<Object> object((Object*) JSObjectGetPrivate(js_object));
		assert(!object.IsEmpty());
		return object;
	}

	#if 0

	if (JSValueIsBooleanObject(js_ctx, js_object))
	{
		// import pure-JS boolean object
		return Handle<Object>(new BooleanObject(js_ctx, js_object));
	}

	if (JSValueIsNumberObject(js_ctx, js_object))
	{
		// import pure-JS number object
		return Handle<Object>(new NumberObject(js_ctx, js_object));
	}

	if (JSValueIsStringObject(js_ctx, js_object))
	{
		// import pure-JS string object
		return Handle<Object>(new StringObject(js_ctx, js_object, NULL));
	}

	#endif

	if (JSValueIsArray(js_ctx, js_object))
	{
		// import pure-JS array object
		return Handle<Object>(new Array(js_ctx, js_object));
	}

	if (JSValueIsFunction(js_ctx, js_object))
	{
		// import pure-JS function object
		return Handle<Object>(new Function(js_ctx, js_object));
	}

	if (JSValueIsDate(js_ctx, js_object))
	{
		// import pure-JS date object
		return Handle<Object>(new Date(js_ctx, js_object));
	}

	if (JSValueIsRegExp(js_ctx, js_object))
	{
		// import pure-JS regexp object
		return Handle<Object>(new RegExp(js_ctx, js_object));
	}

	if (JSValueIsError(js_ctx, js_object))
	{
		// import pure-JS error object
		return Handle<Object>(new Object(js_ctx, js_object));
	}

	// import pure-JS object
	return Handle<Object>(new Object(js_ctx, js_object));
}

/// v8::Isolate

void Isolate::SetAbortOnUncaughtException(abort_on_uncaught_exception_t callback)
{
}

void Isolate::Enter()
{
	m_previous_isolate = sm_current_isolate;
	sm_current_isolate = this;
}

void Isolate::Exit()
{
	sm_current_isolate = m_previous_isolate;
	m_previous_isolate = NULL;
}

void Isolate::Dispose()
{
	delete this;
}

/*static*/ Isolate* Isolate::sm_current_isolate = NULL;

/*static*/ Isolate* Isolate::New()
{
	return new Isolate();
}

/*static*/ Isolate* Isolate::GetCurrent()
{
	return sm_current_isolate;
}

/// v8::Handle<T>

/// v8::Local<T>

/// v8::Persistent<T>

/// v8::HandleScope

/// v8::Data

RTTI_IMPLEMENT_ROOT(v8::Data);

/// v8::Value

RTTI_IMPLEMENT(v8::Value, v8::Data);

Value::Value() :
	m_js_ctx(NULL), m_js_value(NULL), m_js_ref(0), m_js_is_protected(false),
	m_is_weak(false), m_weak_callback(NULL), m_weak_parameter(NULL),
	m_is_independent(false), m_class_id(0), m_is_near_death(false)
{
}

Value::Value(JSContextRef js_ctx, JSValueRef js_value) :
	m_js_ctx(NULL), m_js_value(NULL), m_js_ref(0), m_js_is_protected(false),
	m_is_weak(false), m_weak_callback(NULL), m_weak_parameter(NULL),
	m_is_independent(false), m_class_id(0), m_is_near_death(false)
{
	AttachJSContextAndJSValue(js_ctx, js_value);
}

Value::~Value()
{
	if (m_js_ctx != NULL)
	{
		m_js_ctx = NULL;
	}

	if (m_js_value != NULL)
	{
		m_js_value = NULL;
	}
}

void Value::IncRef()
{
	SmartObject::IncRef();
	IncJSRef();
}

void Value::DecRef()
{
	DecJSRef();
	SmartObject::DecRef();
}

void Value::_OnCreate()
{
}

void Value::_OnDelete()
{
	m_is_near_death = true;
	if (m_is_weak)
	{
		m_weak_callback(Persistent<Value>(this), m_weak_parameter);
		_ClearWeak();
	}
}

void Value::_MakeWeak(void* parameter, void (*callback)(Persistent<Value> object, void* parameter))
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	if (!JSValueIsObjectOfClass(js_ctx, m_js_value, Object::GetJSClass()) &&
		!JSValueIsObjectOfClass(js_ctx, m_js_value, External::GetJSClass()))
	{
		printf("warning: no way to track garbage collection on this v8::Value\n");
	}

	if (!m_is_weak)
	{
		m_is_weak = true;
		m_weak_callback = callback;
		m_weak_parameter = parameter;
		SmartObject::IncRef(); // v8::Value v8::Persistent weak reference
	}
}

void Value::_ClearWeak()
{
	if (m_is_weak)
	{
		m_is_weak = false;
		m_weak_callback = NULL;
		m_weak_parameter = NULL;
		SmartObject::DecRef(); // v8::Value v8::Persistent weak reference
	}
}

void Value::_MarkIndependent()
{
	m_is_independent = true;
}

bool Value::_IsIndependent() const
{
	return m_is_independent;
}

bool Value::_IsNearDeath() const
{
	return m_is_near_death;
}

bool Value::_IsWeak() const
{
	return m_is_weak;
}

void Value::_SetWrapperClassId(uint16_t class_id)
{
	m_class_id = class_id;
}

uint16_t Value::_WrapperClassId() const
{
	return m_class_id;
}

void Value::IncJSRef()
{
	if (m_js_ref++ <= ((m_is_weak)?(1):(0)))
	{
		if (!m_js_is_protected)
		{
			m_js_is_protected = true;
			JSValueProtect(Context::GetCurrentJSGlobalContext(), m_js_value);
		}
	}
	assert(m_js_ref > 0); // check for overflow
}

void Value::DecJSRef()
{
	assert(m_js_ref > 0); // check for underflow
	if (--m_js_ref <= ((m_is_weak)?(1):(0)))
	{
		if (m_js_is_protected)
		{
			m_js_is_protected = false;
			JSValueUnprotect(Context::GetCurrentJSGlobalContext(), m_js_value);
		}
	}
}

void Value::AttachJSContextAndJSValue(JSContextRef js_ctx, JSValueRef js_value)
{
	assert(js_ctx != NULL);
	assert(m_js_ctx == NULL);
	m_js_ctx = js_ctx;

	assert(js_value != NULL);
	assert(m_js_value == NULL);
	m_js_value = js_value;
}

void Value::DetachJSContextAndJSValue()
{
	assert(m_js_ctx != NULL);
	m_js_ctx = NULL;

	assert(m_js_value != NULL);
	m_js_value = NULL;

	if (m_is_weak)
	{
		SmartObject::DecRef(); // v8::Value v8::Persistent weak reference
	}
}

bool Value::IsUndefined()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsUndefined(js_ctx, m_js_value);
}

bool Value::IsNull()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsNull(js_ctx, m_js_value);
}

bool Value::IsBoolean()
{
	if (RTTI_IsKindOf(Boolean, this))
	{
		return true;
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsBoolean(js_ctx, m_js_value);
}

bool Value::IsFalse()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return IsBoolean() && !JSValueToBoolean(js_ctx, m_js_value);
}

bool Value::IsTrue()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return IsBoolean() && JSValueToBoolean(js_ctx, m_js_value);
}

bool Value::IsNumber()
{
	if (RTTI_IsKindOf(Number, this))
	{
		return true;
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsNumber(js_ctx, m_js_value);
}

bool Value::IsInt32()
{
	if (IsNumber())
	{
		double num = NumberValue();
		double whole = 0.0;
		return ((std::modf(num, &whole) == 0.0) && (std::numeric_limits<int32_t>::min() <= whole) && (whole <= std::numeric_limits<int32_t>::max()));
	}
	return false;
}

bool Value::IsUint32()
{
	if (IsNumber())
	{
		double num = NumberValue();
		double whole = 0.0;
		return ((std::modf(num, &whole) == 0.0) && (std::numeric_limits<uint32_t>::min() <= whole) && (whole <= std::numeric_limits<uint32_t>::max()));
	}
	return false;
}

bool Value::IsString()
{
	if (RTTI_IsKindOf(String, this))
	{
		return true;
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsString(js_ctx, m_js_value);
}

bool Value::IsObject()
{
	if (RTTI_IsKindOf(Object, this))
	{
		return true;
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
//	assert(!JSValueIsObjectOfClass(js_ctx, m_js_value, Object::GetJSClass()));
//	if (JSValueIsObjectOfClass(js_ctx, m_js_value, Object::GetJSClass()))
//	{
//		return true;
//	}
	return JSValueIsObject(js_ctx, m_js_value);
}

bool Value::IsArray()
{
	if (RTTI_IsKindOf(Array, this))
	{
		return true;
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
//	assert(!JSValueIsObjectOfClass(js_ctx, m_js_value, Object::GetJSClass()));
//	if (JSValueIsObjectOfClass(js_ctx, m_js_value, Array::GetJSClass()))
//	{
//		return true;
//	}
	return JSValueIsArray(js_ctx, m_js_value);
}

bool Value::IsFunction()
{
	if (RTTI_IsKindOf(Function, this))
	{
		return true;
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	assert(!JSValueIsObjectOfClass(js_ctx, m_js_value, Function::GetJSClass()));
//	if (JSValueIsObjectOfClass(js_ctx, m_js_value, Function::GetJSClass()))
//	{
//		return true;
//	}
	return JSValueIsFunction(js_ctx, m_js_value);
}

bool Value::IsExternal()
{
	if (RTTI_IsKindOf(External, this))
	{
		return true;
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	assert(!JSValueIsObjectOfClass(js_ctx, m_js_value, External::GetJSClass()));
//	return JSValueIsObjectOfClass(js_ctx, m_js_value, External::GetJSClass());
	return false;
}

bool Value::IsRegExp()
{
	if (RTTI_IsKindOf(RegExp, this))
	{
		return true;
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	assert(!JSValueIsObjectOfClass(js_ctx, m_js_value, Object::GetJSClass()));
//	if (JSValueIsObjectOfClass(js_ctx, m_js_value, RegExp::GetJSClass()))
//	{
//		return true;
//	}
	return JSValueIsRegExp(js_ctx, m_js_value);
}

bool Value::IsNativeError()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsError(js_ctx, m_js_value);
}

bool Value::IsBooleanObject()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsBooleanObject(js_ctx, m_js_value);
}

bool Value::IsNumberObject()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsNumberObject(js_ctx, m_js_value);
}

bool Value::IsStringObject()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsStringObject(js_ctx, m_js_value);
}

Local<Boolean> Value::ToBoolean() const
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSValueRef js_value = JSValueMakeBoolean(js_ctx, JSValueToBoolean(js_ctx, m_js_value));
	return Local<Boolean>(new Boolean(js_ctx, js_value));
}

Local<Number> Value::ToNumber() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSValueRef js_value = JSValueMakeNumber(js_ctx, JSValueToNumber(js_ctx, m_js_value, &js_exception));
	return Local<Number>(new Number(js_ctx, js_value));
}

Local<Integer> Value::ToInteger() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSValueRef js_value = JSValueMakeNumber(js_ctx, JSValueToNumber(js_ctx, m_js_value, &js_exception));
	return Local<Integer>(new Integer(js_ctx, js_value));
}

Local<Int32> Value::ToInt32() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSValueRef js_value = JSValueMakeNumber(js_ctx, JSValueToNumber(js_ctx, m_js_value, &js_exception));
	return Local<Int32>(new Int32(js_ctx, js_value));
}

Local<Uint32> Value::ToUint32() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSValueRef js_value = JSValueMakeNumber(js_ctx, JSValueToNumber(js_ctx, m_js_value, &js_exception));
	return Local<Uint32>(new Uint32(js_ctx, js_value));
}

Local<Uint32> Value::ToArrayIndex() const
{
	return ToUint32();
}

Local<String> Value::ToString() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSStringRef js_string = JSValueToStringCopy(js_ctx, m_js_value, &js_exception);
	JSValueRef js_value = JSValueMakeString(js_ctx, js_string);
	JSStringRelease(js_string); js_string = NULL;
	if (js_exception == NULL)
	{
		return Local<String>(new String(js_ctx, js_value));
	}
	return Local<String>();
}

Local<String> Value::ToDetailString() const
{
	return ToString();
}

Local<Object> Value::ToObject() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();

	if (JSValueIsObjectOfClass(js_ctx, m_js_value, Object::GetJSClass()))
	{
		JSObjectRef js_object = JSValueToObject(js_ctx, m_js_value, NULL);
		assert(js_object == m_js_value);
		return Local<Object>((Object*) JSObjectGetPrivate(js_object));
	}

	JSObjectRef js_object = JSValueToObject(js_ctx, m_js_value, &js_exception);
	if (js_exception == NULL)
	{
		return Local<Object>(internal::ImportObject(js_ctx, js_object));
	}

	return Local<Object>();
}

bool Value::BooleanValue() const
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueToBoolean(js_ctx, m_js_value);
}

double Value::NumberValue() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueToNumber(js_ctx, m_js_value, &js_exception);
}

int64_t Value::IntegerValue() const
{
	return internal::DoubleToInteger(NumberValue());
}

int32_t Value::Int32Value() const
{
	return internal::DoubleToInt32(NumberValue());
}

uint32_t Value::Uint32Value() const
{
	return internal::DoubleToUint32(NumberValue());
}

bool Value::Equals(Handle<Value> that) const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsEqual(js_ctx, m_js_value, that->m_js_value, &js_exception);
}

bool Value::StrictEquals(Handle<Value> that) const
{
	if (m_js_value == that->m_js_value)
	{
		return true;
	}
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueIsStrictEqual(js_ctx, m_js_value, that->m_js_value);
}

/// v8::Primitive

RTTI_IMPLEMENT(v8::Primitive, v8::Value);

/// v8::Boolean

RTTI_IMPLEMENT(v8::Boolean, v8::Primitive);

bool Boolean::Value() const
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueToBoolean(js_ctx, m_js_value);
}

/*static*/ Handle<Boolean> Boolean::New(bool value)
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return Handle<Boolean>(new Boolean(js_ctx, JSValueMakeBoolean(js_ctx, value)));
}

/*static*/ Boolean* Boolean::Cast(v8::Value* value)
{
	if (RTTI_IsKindOf(Boolean, value))
	{
		return RTTI_StaticCast(Boolean, value);
	}

	if (value)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = value->m_js_value;
		if (JSValueIsBoolean(js_ctx, js_value))
		{
			return new Boolean(js_ctx, js_value);
		}
	}
	return NULL;
}

/// v8::Number

RTTI_IMPLEMENT(v8::Number, v8::Primitive);

double Number::Value() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return JSValueToNumber(js_ctx, m_js_value, &js_exception);
}

/*static*/ Local<Number> Number::New(double value)
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return Local<Number>(new Number(js_ctx, JSValueMakeNumber(js_ctx, value)));
}

/*static*/ Number* Number::Cast(v8::Value* value)
{
	if (RTTI_IsKindOf(Number, value))
	{
		return RTTI_StaticCast(Number, value);
	}

	if (value)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = value->m_js_value;
		if (JSValueIsNumber(js_ctx, js_value))
		{
			return new Number(js_ctx, js_value);
		}
	}
	return NULL;
}

/// v8::Integer

RTTI_IMPLEMENT(v8::Integer, v8::Number);

int64_t Integer::Value() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return internal::DoubleToInteger(JSValueToNumber(js_ctx, m_js_value, &js_exception));
}

/*static*/ Local<Integer> Integer::New(int32_t value)
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return Local<Integer>(new Integer(js_ctx, JSValueMakeNumber(js_ctx, internal::FastI2D(value))));
}

/*static*/ Local<Integer> Integer::NewFromUnsigned(uint32_t value)
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return Local<Integer>(new Integer(js_ctx, JSValueMakeNumber(js_ctx, internal::FastUI2D(value))));
}

/*static*/ Integer* Integer::Cast(v8::Value* value)
{
	if (value)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = value->m_js_value;
		if (JSValueIsNumber(js_ctx, js_value))
		{
			return new Integer(js_ctx, js_value);
		}
	}
	return NULL;
}

/// v8::Int32

RTTI_IMPLEMENT(v8::Int32, v8::Integer);

int32_t Int32::Value() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return internal::DoubleToInt32(JSValueToNumber(js_ctx, m_js_value, &js_exception));
}

/// v8::Uint32

RTTI_IMPLEMENT(v8::Uint32, v8::Integer);

uint32_t Uint32::Value() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return internal::DoubleToUint32(JSValueToNumber(js_ctx, m_js_value, &js_exception));
}

/// v8::String::Utf8Value

String::Utf8Value::Utf8Value(Handle<v8::Value> value) : m_str(NULL), m_length(0)
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSValueRef js_value = value->m_js_value;
	JSStringRef js_string = JSValueToStringCopy(js_ctx, js_value, &js_exception);
	if (!js_exception)
	{
		size_t size = JSStringGetMaximumUTF8CStringSize(js_string);
		m_str = new char[size];
		assert(m_str);
		if (m_str)
		{
			m_str[0] = 0;
			// NOTE: bytes_written includes the null-terminator byte
			size_t bytes_written = JSStringGetUTF8CString(js_string, m_str, size);
			m_length = (int)(bytes_written - 1);
		}
	}
	JSStringRelease(js_string); js_string = NULL;
}

String::Utf8Value::~Utf8Value()
{
	if (m_str)
	{
		delete[] m_str; m_str = NULL;
	}
}

/// v8::String::AsciiValue

String::AsciiValue::AsciiValue(Handle<v8::Value> value) : m_str(NULL), m_length(0)
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSValueRef js_value = value->m_js_value;
	JSStringRef js_string = JSValueToStringCopy(js_ctx, js_value, &js_exception);
	if (!js_exception)
	{
		size_t size = JSStringGetMaximumUTF8CStringSize(js_string);
		m_str = new char[size];
		assert(m_str);
		if (m_str)
		{
			m_str[0] = 0;
			// NOTE: bytes_written includes the null-terminator byte
			size_t bytes_written = JSStringGetUTF8CString(js_string, m_str, size);
			m_length = (int)(bytes_written - 1);
		}
	}
	JSStringRelease(js_string); js_string = NULL;
}

String::AsciiValue::~AsciiValue()
{
	if (m_str)
	{
		delete[] m_str; m_str = NULL;
	}
}

/// v8::String::Value

String::Value::Value(Handle<v8::Value> value) : m_str(NULL), m_length(0)
{
	// TODO
///	JSValueRef js_exception = NULL;
///	JSContextRef js_ctx = Context::GetCurrentJSContext();
///	JSValueRef js_value = value->m_js_value;
///	JSStringRef js_string = JSValueToStringCopy(js_ctx, js_value, &js_exception);
///	if (!js_exception)
///	{
///		size_t size = JSStringGetMaximumUTF8CStringSize(js_string);
///		m_str = new char[size];
///		assert(m_str);
///		if (m_str)
///		{
///			m_str[0] = 0;
///			// NOTE: bytes_written includes the null-terminator byte
///			size_t bytes_written = JSStringGetUTF8CString(js_string, m_str, size);
///			m_length = (int)(bytes_written - 1);
///		}
///	}
///	JSStringRelease(js_string); js_string = NULL;
}

String::Value::~Value()
{
	// TODO
///	if (m_str)
///	{
///		delete[] m_str; m_str = NULL;
///	}
}

/// v8::String

RTTI_IMPLEMENT(v8::String, v8::Primitive);

String::String(JSContextRef js_ctx, JSValueRef js_value) :
	Primitive(js_ctx, js_value)
{
}

String::~String()
{
}

int String::Length()
{
	int length = 0;

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSStringRef js_string = JSValueToStringCopy(js_ctx, m_js_value, NULL);

	if (js_string)
	{
		length = (int) JSStringGetLength(js_string);
	}

	JSStringRelease(js_string);

	return length;
}

int String::Utf8Length()
{
	int length = 0;

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSStringRef js_string = JSValueToStringCopy(js_ctx, m_js_value, NULL);

	if (js_string)
	{
		size_t size = JSStringGetMaximumUTF8CStringSize(js_string);
		char* str = new char[size];
		assert(str);
		// NOTE: bytes_written includes the null-terminator byte
		size_t bytes_written = JSStringGetUTF8CString(js_string, str, size);
		delete[] str; str = NULL;
		length = (int) bytes_written;
	}

	JSStringRelease(js_string);

	return length;
}

bool String::MayContainNonAscii() const
{
	/*TODO();*/ return true;
}

int String::Write(uint16_t* buffer, int start, int length, int options) const
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSStringRef js_string = JSValueToStringCopy(js_ctx, m_js_value, NULL);

	assert(buffer != NULL);
	assert(start >= 0);
	assert(length >= 0); // TODO: handle length == -1

	const JSChar* in_buffer = JSStringGetCharactersPtr(js_string);
	size_t in_length = JSStringGetLength(js_string);

	for (int i = 0; i < length; ++i)
	{
		assert((start + i) < in_length);
		buffer[i] = in_buffer[start + i];
	}

	JSStringRelease(js_string);

	return length;
}

int String::WriteAscii(char* buffer, int start, int length, int options) const
{
	assert(buffer != NULL);
	assert(start >= 0);
	assert(length >= 0); // TODO: handle length == -1

	TODO();

	return 0;
}

int String::WriteUtf8(char* buffer, int length, int* nchars_ref, int options) const
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSStringRef js_string = JSValueToStringCopy(js_ctx, m_js_value, NULL);

	assert(buffer != NULL);
	assert(length >= 0); // TODO: handle length == -1

	size_t size = JSStringGetMaximumUTF8CStringSize(js_string);
	char* str = new char[size];
	assert(str);
	memset(str, 0, size);

	// NOTE: bytes_written includes the null-terminator byte
	size_t bytes_written = JSStringGetUTF8CString(js_string, str, size);

	strncpy(buffer, str, length);
	delete[] str; str = NULL;

	if (options & NO_NULL_TERMINATION)
	{
		--bytes_written;
	}

	if (nchars_ref != NULL)
	{
		*nchars_ref = (int) bytes_written;
	}

	JSStringRelease(js_string);

	return (int) bytes_written;
}

bool String::IsExternal() const
{
	TODO(); return false;
}

bool String::IsExternalAscii() const
{
	TODO(); return false;
}

String::ExternalStringResourceBase* String::GetExternalStringResourceBase(String::Encoding* encoding_out) const
{
	TODO(); return NULL;
}

String::ExternalStringResource* String::GetExternalStringResource() const
{
	TODO(); return NULL;
}

const String::ExternalAsciiStringResource* String::GetExternalAsciiStringResource() const
{
	TODO(); return NULL;
}

bool String::MakeExternal(String::ExternalStringResource* resource)
{
	TODO(); return false;
}

bool String::MakeExternal(String::ExternalAsciiStringResource* resource)
{
	TODO(); return false;
}

bool String::CanMakeExternal()
{
	TODO(); return false;
}

/*static*/ Local<String> String::Empty()
{
	return String::New("");
}

/*static*/ String* String::Cast(v8::Value* value)
{
	if (RTTI_IsKindOf(String, value))
	{
		return RTTI_StaticCast(String, value);
	}

	if (value)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = value->m_js_value;
		if (JSValueIsString(js_ctx, js_value))
		{
			return new String(js_ctx, js_value);
		}
	}
	return NULL;
}

/*static*/ Local<String> String::New(const char* data, int length)
{
	JSStringRef js_string = NULL;

	if (data)
	{
		if (length >= 0)
		{
			JSChar* buffer = new JSChar[length];
			assert(buffer);

			for (int i = 0; i < length; ++i)
			{
				buffer[i] = data[i];
			}

			js_string = JSStringCreateWithCharacters(buffer, (size_t) length);
			assert(JSStringGetCharactersPtr(js_string) != buffer);
			assert(JSStringGetLength(js_string) == (size_t) length);

			delete[] buffer; buffer = NULL;
		}
		else
		{
			js_string = JSStringCreateWithUTF8CString(data);
		}
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	Handle<String> v8_string(new String(js_ctx, JSValueMakeString(js_ctx, js_string)));

	JSStringRelease(js_string); js_string = NULL;

	return Local<String>(v8_string);
}

/*static*/ Local<String> String::New(const uint16_t* data, int length)
{
	JSStringRef js_string = NULL;

	if (data)
	{
		assert(length >= 0); // TODO: handle length == -1

		if (length >= 0)
		{
			js_string = JSStringCreateWithCharacters((const JSChar*) data, (size_t) length);
			assert(JSStringGetCharactersPtr(js_string) != data);
			assert(JSStringGetLength(js_string) == length);
		}
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	Handle<String> v8_string(new String(js_ctx, JSValueMakeString(js_ctx, js_string)));

	JSStringRelease(js_string); js_string = NULL;

	return Local<String>(v8_string);
}

/*static*/ Local<String> String::NewSymbol(const char* data, int length)
{
	return String::New(data, length);
}

/*static*/ Local<String> String::Concat(Handle<String> left, Handle<String> right)
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();

	JSStringRef js_string_l = JSValueToStringCopy(js_ctx, left->m_js_value, &js_exception);
	const size_t js_length_l = JSStringGetLength(js_string_l);
	const JSChar* js_data_l = JSStringGetCharactersPtr(js_string_l);

	JSStringRef js_string_r = JSValueToStringCopy(js_ctx, right->m_js_value, &js_exception);
	const size_t js_length_r = JSStringGetLength(js_string_r);
	const JSChar* js_data_r = JSStringGetCharactersPtr(js_string_r);

	const size_t js_length = js_length_l + js_length_r;
	JSChar* js_data = new JSChar[js_length];
	assert(js_data);

	size_t js_index = 0;

	for (size_t js_index_l = 0; js_index_l < js_length_l; ++js_index, ++js_index_l)
	{
		js_data[js_index] = js_data_l[js_index_l];
	}

	for (size_t js_index_r = 0; js_index_r < js_length_r; ++js_index, ++js_index_r)
	{
		js_data[js_index] = js_data_r[js_index_r];
	}

	JSStringRelease(js_string_l); js_string_l = NULL; js_data_l = NULL;
	JSStringRelease(js_string_r); js_string_r = NULL; js_data_r = NULL;

	JSStringRef js_string = JSStringCreateWithCharacters(js_data, js_length);
	assert(JSStringGetCharactersPtr(js_string) != js_data);
	assert(JSStringGetLength(js_string) == js_length);

	delete[] js_data; js_data = NULL;

	Handle<String> v8_string(new String(js_ctx, JSValueMakeString(js_ctx, js_string)));

	JSStringRelease(js_string); js_string = NULL;

	return Local<String>(v8_string);
}

/*static*/ Local<String> String::NewExternal(String::ExternalStringResource* resource)
{
	JSStringRef js_string = NULL;

	const uint16_t* data = resource->data();
	size_t length = resource->length();

	if (data)
	{
		js_string = JSStringCreateWithCharacters((const JSChar*) data, (size_t) length);
		assert(JSStringGetCharactersPtr(js_string) != (const JSChar*) data);
		assert(JSStringGetLength(js_string) == (size_t) length);
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	Handle<String> v8_string(new String(js_ctx, JSValueMakeString(js_ctx, js_string)));

	JSStringRelease(js_string); js_string = NULL;

	delete resource;

	return Local<String>(v8_string);
}

/*static*/ Local<String> String::NewExternal(String::ExternalAsciiStringResource* resource)
{
	JSStringRef js_string = NULL;

	const char* data = resource->data();
	size_t length = resource->length();

	if (data)
	{
		JSChar* buffer = new JSChar[length];
		assert(buffer);

		for (int i = 0; i < length; ++i)
		{
			buffer[i] = data[i];
		}

		js_string = JSStringCreateWithCharacters(buffer, (size_t) length);
		assert(JSStringGetCharactersPtr(js_string) != buffer);
		assert(JSStringGetLength(js_string) == (size_t) length);

		delete[] buffer; buffer = NULL;
	}

	JSContextRef js_ctx = Context::GetCurrentJSContext();
	Handle<String> v8_string(new String(js_ctx, JSValueMakeString(js_ctx, js_string)));

	JSStringRelease(js_string); js_string = NULL;

	delete resource;

	return Local<String>(v8_string);
}

/*static*/ Local<String> String::NewUndetectable(const char* data, int length)
{
	return String::New(data, length);
}

/*static*/ Local<String> String::NewUndetectable(const uint16_t* data, int length)
{
	return String::New(data, length);
}

/// v8::AccessorInfo

AccessorInfo::AccessorInfo(Isolate* isolate, Handle<Object> that, Handle<Object> holder, Handle<Value> data) :
	m_isolate(isolate), m_that(that), m_holder(holder), m_data(data) {}

AccessorInfo::~AccessorInfo()
{
	m_isolate = NULL;
}

Isolate* AccessorInfo::GetIsolate() const
{
	return m_isolate;
}

Local<Object> AccessorInfo::This() const
{
	return Local<Object>(m_that);
}

Local<Object> AccessorInfo::Holder() const
{
	return Local<Object>(m_holder);
}

Local<Value> AccessorInfo::Data() const
{
	return Local<Value>(m_data);
}

/// v8::internal::StringCompare

bool internal::StringCompare::operator()(const Handle<String>& key1, const Handle<String>& key2) const
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSStringRef js_key1 = JSValueToStringCopy(js_ctx, key1->m_js_value, NULL);
	JSStringRef js_key2 = JSValueToStringCopy(js_ctx, key2->m_js_value, NULL);
	int ret = JSStringCompare(js_key1, js_key2);
	JSStringRelease(js_key1);
	JSStringRelease(js_key2);
	return ret < 0;
}

/// v8::internal::Helper

internal::Helper::Helper() :
	m_internal_field_array(NULL), m_internal_field_count(0),
	m_pixel_data(NULL), m_pixel_data_length(0),
	m_external_array_data(NULL), m_external_array_data_type((ExternalArrayType) 0), m_external_array_data_length(0)
{
}

internal::Helper::~Helper()
{
	if (m_internal_field_array != NULL)
	{
		delete[] m_internal_field_array; m_internal_field_array = NULL;
	}
	m_internal_field_count = 0;

	if (m_pixel_data != NULL)
	{
		m_pixel_data = NULL;
	}
	m_pixel_data_length = 0;

	if (m_external_array_data != NULL)
	{
		m_external_array_data = NULL;
	}
	m_external_array_data_type = (ExternalArrayType) 0;
	m_external_array_data_length = 0;
}

void internal::Helper::SetInternalFieldCount(int internal_field_count)
{
	if (m_internal_field_count != internal_field_count)
	{
		if (m_internal_field_array != NULL)
		{
			delete[] m_internal_field_array; m_internal_field_array = NULL;
		}

		m_internal_field_count = internal_field_count;

		if (m_internal_field_count > 0)
		{
			m_internal_field_array = new internal::Helper::InternalField[m_internal_field_count];
		}
	}
}

/*static*/ void internal::Helper::WeakFree(Persistent<Value> object, void* parameter)
{
	delete static_cast<internal::Helper*>(parameter);
}

/// v8::Object

RTTI_IMPLEMENT(v8::Object, v8::Primitive);

Object::Object() :
	m_js_object(NULL), m_is_native(false), m_helper(NULL)
{
}

Object::Object(JSContextRef js_ctx) :
	m_js_object(NULL), m_is_native(false), m_helper(NULL)
{
	Object::AttachJSContextAndJSObject(js_ctx, JSObjectMake(js_ctx, Object::GetJSClass(), this));
	assert(JSValueIsObjectOfClass(js_ctx, m_js_value, Object::GetJSClass()));
	assert(JSObjectGetPrivate(m_js_object) == this);

	// m_js_object.__proto__ = new Object();
	//JSObjectRef js_prototype = JSObjectMake(js_ctx, NULL, NULL);
	//JSObjectSetPrototype(js_ctx, m_js_object, js_prototype);

	// m_js_object.__proto__ = new Object();
	JSStringRef js_name = JSStringCreateWithUTF8CString("Object");
	JSObjectRef js_ctor = JSValueToObject(js_ctx, JSObjectGetProperty(js_ctx, Context::GetJSGlobalObject(js_ctx, JSContextGetGlobalObject(js_ctx)), js_name, NULL), NULL);
	JSObjectRef js_prototype = JSObjectCallAsConstructor(js_ctx, js_ctor, 0, NULL, NULL);
	JSObjectSetPrototype(js_ctx, m_js_object, js_prototype);
	JSStringRelease(js_name); js_name = NULL;

	assert(JSObjectGetPrivate(m_js_object) == this);
	assert(JSValueIsObject(js_ctx, m_js_value));
}

Object::Object(JSContextRef js_ctx, JSObjectRef js_object) :
	m_js_object(NULL), m_is_native(false), m_helper(NULL)
{
	Object::AttachJSContextAndJSObject(js_ctx, js_object);
	//assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
}

Object::~Object()
{
	if (m_is_native)
	{
		if (m_helper != NULL)
		{
			delete m_helper;
			m_helper = NULL;
		}

		if (m_js_object)
		{
			assert(JSObjectGetPrivate(m_js_object) == this);
			bool ret = JSObjectSetPrivate(m_js_object, NULL);
			assert(ret);
			m_js_object = NULL;
		}
	}
	else
	{
		if (m_helper != NULL)
		{
			// owned by v8::External
			m_helper = NULL;
		}

		if (m_js_object)
		{
			assert(JSObjectGetPrivate(m_js_object) == NULL);
			m_js_object = NULL;
		}
	}
}

void Object::AttachJSContextAndJSObject(JSContextRef js_ctx, JSObjectRef js_object)
{
	assert(m_js_object == NULL);
	AttachJSContextAndJSValue(js_ctx, m_js_object = js_object);
	assert(m_js_object != NULL);
	assert(m_js_object == m_js_value);

	m_is_native = JSValueIsObjectOfClass(js_ctx, m_js_value, Object::GetJSClass());

	if (m_is_native)
	{
		//printf("note: object %p using v8::Object class\n", m_js_object);
		if (JSObjectGetPrivate(m_js_object) == NULL)
		{
			// JSGlobalContextCreate and JSGlobalContextCreateInGroup do not
			// accept user data; so, set it here and call Object::_JS_Initialize
			bool ret = JSObjectSetPrivate(m_js_object, this);
			assert(ret);
			assert(JSObjectGetPrivate(m_js_object) == this);
			Object::_JS_Initialize(js_ctx, m_js_object);
		}
		else
		{
			assert(JSObjectGetPrivate(m_js_object) == this);
		}

		SmartObject::IncRef(); // v8::Object (native) JSObjectRef private reference
	}
	else
	{
		//printf("note: object %p using JSC Object class\n", m_js_object);
	}
}

void Object::DetachJSContextAndJSObject()
{
	assert(m_is_native);
	assert(m_js_object != NULL);
	assert(JSObjectGetPrivate(m_js_object) == this);
	bool ret = JSObjectSetPrivate(m_js_object, NULL);
	assert(ret);
	m_js_object = NULL;
	assert(m_js_object == NULL);

	DetachJSContextAndJSValue();

	SmartObject::DecRef(); // v8::Object (native) JSObjectRef private reference
}

internal::Helper* Object::GetHelper()
{
	if (m_is_native)
	{
		if (m_helper == NULL)
		{
			m_helper = new internal::Helper();
		}
		return m_helper;
	}
	else
	{
		if (m_helper == NULL)
		{
			JSValueRef js_exception = NULL;
			JSContextRef js_ctx = Context::GetCurrentJSContext();
			internal::JSStringWrap js_name("__helper__");
			if (!JSObjectHasProperty(js_ctx, m_js_object, js_name))
			{
				internal::Helper* helper = new internal::Helper();
				Persistent<Value> value(External::New(helper));
				value.MakeWeak(helper, internal::Helper::WeakFree);
				JSValueRef js_value = internal::ExportValue(value);
				JSPropertyAttributes js_attrib = kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete;
				JSObjectSetProperty(js_ctx, m_js_object, js_name, js_value, js_attrib, &js_exception);
				assert(js_exception == NULL);
			}
			JSValueRef js_value = JSObjectGetProperty(js_ctx, m_js_object, js_name, &js_exception);
			assert(js_exception == NULL);
			assert(JSValueIsObjectOfClass(js_ctx, js_value, External::GetJSClass()));
			Handle<Value> value = internal::ImportValue(js_ctx, js_value);
			m_helper = (internal::Helper*) External::Unwrap(value);
		}
		return m_helper;
	}
}

bool Object::_ObjectHasProperty(Handle<String> name)
{
	internal::Helper* helper = GetHelper();

	// check object accessor map
	internal::AccessorMap::iterator accessor_it = helper->m_accessor_map.find(name);
	if (accessor_it != helper->m_accessor_map.end())
	{
		return true;
	}

	// check object template, for Objects created by ObjectTemplate::NewInstance()
	if (!helper->m_object_template.IsEmpty() && helper->m_object_template->_ObjectTemplateHasProperty(Handle<Object>(this), name))
	{
		return true;
	}

	// check function, for Objects created by Function::NewInstance()
	if (!helper->m_function.IsEmpty() && helper->m_function->_FunctionHasProperty(name))
	{
		return true;
	}

	#if 0 // V8LIKE_PROPERTY_MAP
	// check object property map
	internal::PropertyMap::iterator property_it = helper->m_property_map.find(name);
	if (property_it != helper->m_property_map.end())
	{
		return true;
	}
	#endif

	return false;
}

Handle<Value> Object::_ObjectGetProperty(Handle<String> name)
{
	Handle<Value> value;

	internal::Helper* helper = GetHelper();

	// check object accessor map
	internal::AccessorMap::iterator accessor_it = helper->m_accessor_map.find(name);
	if (accessor_it != helper->m_accessor_map.end())
	{
		AccessorInfo accessor_info(Isolate::GetCurrent(), Handle<Object>(this), Handle<Object>(this), accessor_it->second.m_data); // TODO: holder?
		return accessor_it->second.m_getter(Local<String>(name), accessor_info);
	}

	// check object template, for Objects created by ObjectTemplate::NewInstance()
	if (!helper->m_object_template.IsEmpty())
	{
		value = helper->m_object_template->_ObjectTemplateGetProperty(Handle<Object>(this), name);
		if (!value.IsEmpty())
		{
			return value;
		}
	}

	// check function, for Objects created by Function::NewInstance()
	if (!helper->m_function.IsEmpty())
	{
		value = helper->m_function->_FunctionGetProperty(name);
		if (!value.IsEmpty())
		{
			return value;
		}
	}

	#if 0 // V8LIKE_PROPERTY_MAP
	// check object property map
	internal::PropertyMap::iterator property_it = helper->m_property_map.find(name);
	if (property_it != helper->m_property_map.end())
	{
		return property_it->second.m_value;
	}
	#endif

	return value;
}

bool Object::_ObjectSetProperty(Handle<String> name, Handle<Value> value, PropertyAttribute attrib)
{
	internal::Helper* helper = GetHelper();

	// check object accessor map
	internal::AccessorMap::iterator accessor_it = helper->m_accessor_map.find(name);
	if (accessor_it != helper->m_accessor_map.end())
	{
		AccessorInfo accessor_info(Isolate::GetCurrent(), Handle<Object>(this), Handle<Object>(this), accessor_it->second.m_data); // TODO: holder?
		accessor_it->second.m_setter(Local<String>(name), Local<Value>(value), accessor_info);
		return true;
	}

	// check object template, for Objects created by ObjectTemplate::NewInstance()
	if (!helper->m_object_template.IsEmpty() && helper->m_object_template->_ObjectTemplateSetProperty(Handle<Object>(this), name, value))
	{
		return true;
	}

	// check function, for Objects created by Function::NewInstance()
	if (!helper->m_function.IsEmpty() && helper->m_function->_FunctionSetProperty(name, value, attrib))
	{
		return true;
	}

	#if 0 // V8LIKE_PROPERTY_MAP
	// check object property map
	internal::PropertyMap::iterator property_it = helper->m_property_map.find(name);
	if (property_it != helper->m_property_map.end())
	{
		if (attrib == None) { attrib = property_it->second.m_attrib; } // preserve attributes
		property_it->second = internal::Property(value, attrib);
	}
	else
	{
		helper->m_property_map[name] = internal::Property(value, attrib);
	}
	return true;
	#endif

	return false;
}

bool Object::_ObjectDeleteProperty(Handle<String> name)
{
	internal::Helper* helper = GetHelper();

	// check object accessor map
	internal::AccessorMap::iterator accessor_it = helper->m_accessor_map.find(name);
	if (accessor_it != helper->m_accessor_map.end())
	{
		helper->m_accessor_map.erase(accessor_it);
		return true;
	}

	// check object template, for Objects created by ObjectTemplate::NewInstance()
	if (!helper->m_object_template.IsEmpty() && helper->m_object_template->_ObjectTemplateDeleteProperty(Handle<Object>(this), name))
	{
		return true;
	}

	// check function, for Objects created by Function::NewInstance()
	if (!helper->m_function.IsEmpty() && helper->m_function->_FunctionDeleteProperty(name))
	{
		return true;
	}

	#if 0 // V8LIKE_PROPERTY_MAP
	// check object property map
	internal::PropertyMap::iterator property_it = helper->m_property_map.find(name);
	if (property_it != helper->m_property_map.end())
	{
		helper->m_property_map.erase(property_it);
		return true;
	}
	#endif

	return false;
}

void Object::_ObjectGetPropertyNames(JSPropertyNameAccumulatorRef js_name_accumulator)
{
	internal::Helper* helper = GetHelper();

	JSContextRef js_ctx = Context::GetCurrentJSContext();

	// check object accessor map
	for (internal::AccessorMap::iterator accessor_it = helper->m_accessor_map.begin(); accessor_it != helper->m_accessor_map.end(); ++accessor_it)
	{
		if (!(accessor_it->second.m_attrib & DontEnum))
		{
			JSStringRef js_name = JSValueToStringCopy(js_ctx, accessor_it->first->m_js_value, NULL);
			JSPropertyNameAccumulatorAddName(js_name_accumulator, js_name);
			JSStringRelease(js_name);
		}
	}

	// check object template, for Objects created by ObjectTemplate::NewInstance()
	if (!helper->m_object_template.IsEmpty())
	{
		helper->m_object_template->_ObjectTemplateGetPropertyNames(Handle<Object>(this), js_name_accumulator);
	}

	// check function, for Objects created by Function::NewInstance()
	if (!helper->m_function.IsEmpty())
	{
		helper->m_function->_FunctionGetPropertyNames(js_name_accumulator);
	}

	#if 0 // V8LIKE_PROPERTY_MAP
	// check object property map
	for (internal::PropertyMap::iterator property_it = helper->m_property_map.begin(); property_it != helper->m_property_map.end(); ++property_it)
	{
		if (!(property_it->second.m_attrib & DontEnum))
		{
			JSStringRef js_name = JSValueToStringCopy(js_ctx, property_it->first->m_js_value, NULL);
			JSPropertyNameAccumulatorAddName(js_name_accumulator, js_name);
			JSStringRelease(js_name);
		}
	}
	#endif
}

bool Object::Has(Handle<String> key)
{
	if (m_js_object)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSStringRef js_name = JSValueToStringCopy(js_ctx, key->m_js_value, &js_exception);
		bool has = JSObjectHasProperty(js_ctx, m_js_object, js_name);
		JSStringRelease(js_name); js_name = NULL;
		return has;
	}
	else
	{
		return _ObjectHasProperty(key);
	}
}

Local<Value> Object::Get(Handle<Value> key)
{
	if (m_js_object)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSStringRef js_name = JSValueToStringCopy(js_ctx, key->m_js_value, &js_exception);
		JSValueRef js_value = JSObjectGetProperty(js_ctx, m_js_object, js_name, &js_exception);
		JSStringRelease(js_name); js_name = NULL;
		return Local<Value>(internal::ImportValue(js_ctx, js_value));
	}
	else
	{
		return Local<Value>(_ObjectGetProperty(key.As<String>()));
	}
}

bool Object::Set(Handle<Value> key, Handle<Value> value, PropertyAttribute attrib)
{
	if (m_js_object)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSStringRef js_name = JSValueToStringCopy(js_ctx, key->m_js_value, &js_exception);
		JSValueRef js_value = internal::ExportValue(value);
		JSPropertyAttributes js_attrib = kJSPropertyAttributeNone;
		if (attrib & ReadOnly)   { js_attrib |= kJSPropertyAttributeReadOnly; }
		if (attrib & DontEnum)   { js_attrib |= kJSPropertyAttributeDontEnum; }
		if (attrib & DontDelete) { js_attrib |= kJSPropertyAttributeDontDelete; }
		JSObjectSetProperty(js_ctx, m_js_object, js_name, js_value, js_attrib, &js_exception);
		JSStringRelease(js_name); js_name = NULL;
		if (js_exception)
		{
			ThrowException(internal::ImportValue(js_ctx, js_exception));
		}
		return (js_exception)?(false):(true);
	}
	else
	{
		return _ObjectSetProperty(key.As<String>(), value, attrib);
	}
}

bool Object::ForceSet(Handle<Value> key, Handle<Value> value, PropertyAttribute attrib)
{
	return Object::Set(key, value, attrib); // TODO: force
}

bool Object::Delete(Handle<String> key)
{
	if (m_js_object)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSStringRef js_name = JSValueToStringCopy(js_ctx, key->m_js_value, &js_exception);
		bool deleted = JSObjectDeleteProperty(js_ctx, m_js_object, js_name, &js_exception);
		JSStringRelease(js_name); js_name = NULL;
		return deleted;
	}
	else
	{
		return _ObjectDeleteProperty(key);
	}
}

bool Object::ForceDelete(Handle<Value> key)
{
	return Object::Delete(key.As<String>()); // TODO: force
}

PropertyAttribute Object::GetPropertyAttributes(Handle<Value> key)
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSObjectRef js_ctor = JSValueToObject(js_ctx, JSObjectGetProperty(js_ctx, Context::GetJSGlobalObject(js_ctx, JSContextGetGlobalObject(js_ctx)), internal::JSStringWrap("Object"), &js_exception), &js_exception);
	JSObjectRef js_func = JSValueToObject(js_ctx, JSObjectGetProperty(js_ctx, js_ctor, internal::JSStringWrap("getOwnPropertyDescriptor"), &js_exception), &js_exception);
	JSObjectRef js_that = NULL;
	const JSValueRef js_argv[] = { m_js_object, key->m_js_value };
	size_t js_argc = sizeof(js_argv) / sizeof(*js_argv);
	JSObjectRef js_desc = JSValueToObject(js_ctx, JSObjectCallAsFunction(js_ctx, js_func, js_that, js_argc, js_argv, &js_exception), &js_exception);
	JSValueRef js_writable     = JSObjectGetProperty(js_ctx, js_desc, internal::JSStringWrap("writable"    ), &js_exception);
	JSValueRef js_enumerable   = JSObjectGetProperty(js_ctx, js_desc, internal::JSStringWrap("enumberable" ), &js_exception);
	JSValueRef js_configurable = JSObjectGetProperty(js_ctx, js_desc, internal::JSStringWrap("configurable"), &js_exception);
	int prop = None;
	if (JSValueIsBoolean(js_ctx, js_writable) && !JSValueToBoolean(js_ctx, js_writable))
	{
		prop |= ReadOnly;
	}
	if (JSValueIsBoolean(js_ctx, js_enumerable) && !JSValueToBoolean(js_ctx, js_enumerable))
	{
		prop |= DontEnum;
	}
	if (JSValueIsBoolean(js_ctx, js_configurable) && !JSValueToBoolean(js_ctx, js_configurable))
	{
		prop |= DontDelete;
	}
	return (PropertyAttribute) prop;
}

bool Object::Has(uint32_t index)
{
	if (HasIndexedPropertiesInExternalArrayData())
	{
		int array_data_length = GetIndexedPropertiesExternalArrayDataLength();
		return (index < array_data_length);
	}

	if (m_js_object)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		unsigned js_index = (unsigned) index;
		JSValueRef js_value = JSObjectGetPropertyAtIndex(js_ctx, m_js_object, js_index, &js_exception);
		return !JSValueIsUndefined(js_ctx, js_value);
	}
	else
	{
		Handle<Value> key = Integer::NewFromUnsigned(index);
		return _ObjectHasProperty(key.As<String>());
	}
}

Local<Value> Object::Get(uint32_t index)
{
	if (HasIndexedPropertiesInExternalArrayData())
	{
		int array_data_length = GetIndexedPropertiesExternalArrayDataLength();
		if (index < array_data_length)
		{
			ExternalArrayType array_data_type = GetIndexedPropertiesExternalArrayDataType();
			void* array_data = GetIndexedPropertiesExternalArrayData();
			JSContextRef js_ctx = Context::GetCurrentJSContext();
			JSValueRef js_value = NULL;
			switch (array_data_type)
			{
			case kExternalByteArray:			js_value = JSValueMakeNumber(js_ctx, internal::FastI2D (((char*)		   array_data)[index])); break;
			case kExternalShortArray:   		js_value = JSValueMakeNumber(js_ctx, internal::FastI2D (((short*)   	   array_data)[index])); break;
			case kExternalIntArray: 			js_value = JSValueMakeNumber(js_ctx, internal::FastI2D (((int*) 		   array_data)[index])); break;
			case kExternalPixelArray:			js_value = JSValueMakeNumber(js_ctx, internal::FastUI2D(((unsigned char*)  array_data)[index])); break;
			case kExternalUnsignedByteArray:	js_value = JSValueMakeNumber(js_ctx, internal::FastUI2D(((unsigned char*)  array_data)[index])); break;
			case kExternalUnsignedShortArray:   js_value = JSValueMakeNumber(js_ctx, internal::FastUI2D(((unsigned short*) array_data)[index])); break;
			case kExternalUnsignedIntArray:		js_value = JSValueMakeNumber(js_ctx, internal::FastUI2D(((unsigned int*)   array_data)[index])); break;
			case kExternalFloatArray:			js_value = JSValueMakeNumber(js_ctx, (double)   	   (((float*)   	   array_data)[index])); break;
			case kExternalDoubleArray:			js_value = JSValueMakeNumber(js_ctx,				   (((double*)  	   array_data)[index])); break;
			default: assert(0); return Local<Value>();
			}
			return Local<Value>(new Number(js_ctx, js_value));
		}

		return Local<Value>();
	}

	if (m_js_object)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		unsigned js_index = (unsigned) index;
		JSValueRef js_value = JSObjectGetPropertyAtIndex(js_ctx, m_js_object, js_index, &js_exception);
		return Local<Value>(internal::ImportValue(js_ctx, js_value));
	}
	else
	{
		Handle<Value> key = Integer::NewFromUnsigned(index);
		return Local<Value>(_ObjectGetProperty(key.As<String>()));
	}
}

bool Object::Set(uint32_t index, Handle<Value> value)
{
	if (HasIndexedPropertiesInExternalArrayData())
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = internal::ExportValue(value);
		double number = JSValueToNumber(js_ctx, js_value, NULL);
		int array_data_length = GetIndexedPropertiesExternalArrayDataLength();
		if (index < array_data_length)
		{
			ExternalArrayType array_data_type = GetIndexedPropertiesExternalArrayDataType();
			void* array_data = GetIndexedPropertiesExternalArrayData();
			switch (array_data_type)
			{
			case kExternalByteArray:			((char*)		   array_data)[index] = internal::DoubleToInt32 (number);	return true;
			case kExternalShortArray:   		((short*)   	   array_data)[index] = internal::DoubleToInt32 (number);	return true;
			case kExternalIntArray: 			((int*) 		   array_data)[index] = internal::DoubleToInt32 (number);	return true;
			case kExternalPixelArray:			((unsigned char*)  array_data)[index] = internal::DoubleToUint32(number);	return true;
			case kExternalUnsignedByteArray:	((unsigned char*)  array_data)[index] = internal::DoubleToUint32(number);	return true;
			case kExternalUnsignedShortArray:   ((unsigned short*) array_data)[index] = internal::DoubleToUint32(number);	return true;
			case kExternalUnsignedIntArray:		((unsigned int*)   array_data)[index] = internal::DoubleToUint32(number);	return true;
			case kExternalFloatArray:			((float*)   	   array_data)[index] = (float) 				 number;	return true;
			case kExternalDoubleArray:			((double*)  	   array_data)[index] = 						 number;	return true;
			default: assert(0); return false;
			}
		}

		return false;
	}

	if (m_js_object)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		unsigned js_index = (unsigned) index;
		JSValueRef js_value = internal::ExportValue(value);
		JSObjectSetPropertyAtIndex(js_ctx, m_js_object, js_index, js_value, &js_exception);
		return (js_exception)?(false):(true);
	}
	else
	{
		Handle<Value> key = Integer::NewFromUnsigned(index);
		return _ObjectSetProperty(key.As<String>(), value, None);
	}
}

bool Object::Delete(uint32_t index)
{
	if (m_js_object)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_key = JSValueMakeNumber(js_ctx, internal::FastUI2D(index));
		JSStringRef js_name = JSValueToStringCopy(js_ctx, js_key, &js_exception);
		bool deleted = JSObjectDeleteProperty(js_ctx, m_js_object, js_name, &js_exception);
		JSStringRelease(js_name); js_name = NULL;
		return deleted;
	}
	else
	{
		Handle<Value> key = Integer::NewFromUnsigned(index);
		return _ObjectDeleteProperty(key.As<String>());
	}
}

bool Object::SetAccessor(Handle<String> name, AccessorGetter getter, AccessorSetter setter, Handle<Value> data, AccessControl settings, PropertyAttribute attrib)
{
	internal::Helper* helper = GetHelper();
	helper->m_accessor_map[name] = internal::Accessor(getter, setter, data, settings, attrib);
	return true;
}

Local<Array> Object::GetPropertyNames()
{
	Local<Array> names = Array::New();

	if (m_js_object)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSPropertyNameArrayRef js_names = JSObjectCopyPropertyNames(js_ctx, m_js_object);
		size_t js_names_count = JSPropertyNameArrayGetCount(js_names);
		for (size_t index = 0; index < js_names_count; ++index)
		{
			JSStringRef js_name = JSPropertyNameArrayGetNameAtIndex(js_names, index);
			names->Set((uint32_t) index, Handle<String>(new String(js_ctx, JSValueMakeString(js_ctx, js_name))));
		}
		JSPropertyNameArrayRelease(js_names); js_names = NULL;
	}
	else
	{
		TODO();
	}

	return names;
}

Local<Array> Object::GetOwnPropertyNames()
{
	TODO(); return GetPropertyNames();
}

Local<Value> Object::GetPrototype()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSValueRef js_prototype = JSObjectGetPrototype(js_ctx, m_js_object);
	if (js_prototype != NULL)
	{
		return Local<Value>(internal::ImportValue(js_ctx, js_prototype));
	}
	return Local<Value>(Null());
}

bool Object::SetPrototype(Handle<Value> prototype)
{
	TODO(); return false;
}

Local<Object> Object::FindInstanceInPrototypeChain(Handle<FunctionTemplate> tmpl)
{
	TODO(); return Local<Object>();
}

Local<String> Object::ObjectProtoToString()
{
	TODO(); return Local<String>();
}

Local<Value> Object::GetConstructor()
{
	Local<Value> prototype = GetPrototype();
	if (prototype->IsObject())
	{
		return prototype.As<Object>()->Get(String::NewSymbol("constructor"));
	}
	return Local<Value>(Null());
}

Local<String> Object::GetConstructorName()
{
	TODO(); return Local<String>();
}

int Object::InternalFieldCount()
{
	internal::Helper* helper = GetHelper();
	return (int) helper->m_internal_field_count;
}

Local<Value> Object::GetInternalField(int index)
{
	internal::Helper* helper = GetHelper();
	if ((helper->m_internal_field_array != NULL) && (0 <= index) && (index < InternalFieldCount()))
	{
		return Local<Value>(helper->m_internal_field_array[index].m_value);
	}
	return Local<Value>(Undefined());
}

void Object::SetInternalField(int index, Handle<Value> value)
{
	internal::Helper* helper = GetHelper();
	if ((helper->m_internal_field_array != NULL) && (0 <= index) && (index < InternalFieldCount()))
	{
		helper->m_internal_field_array[index].m_value = value;
	}
}

void* Object::GetPointerFromInternalField(int index)
{
	return External::Unwrap(GetInternalField(index));
}

void Object::SetPointerInInternalField(int index, void* value)
{
	SetInternalField(index, External::Wrap(value));
}

bool Object::HasOwnProperty(Handle<String> key)
{
	TODO(); return false;
}

bool Object::HasRealNamedProperty(Handle<String> key)
{
	TODO(); return false;
}

bool Object::HasRealIndexedProperty(uint32_t index)
{
	TODO(); return false;
}

bool Object::HasRealNamedCallbackProperty(Handle<String> key)
{
	TODO(); return false;
}

Local<Value> Object::GetRealNamedPropertyInPrototypeChain(Handle<String> key)
{
	TODO(); return Local<Value>();
}

Local<Value> Object::GetRealNamedProperty(Handle<String> key)
{
	TODO(); return Local<Value>();
}

bool Object::HasNamedLookupInterceptor()
{
	TODO(); return false;
}

bool Object::HasIndexedLookupInterceptor()
{
	TODO(); return false;
}

void Object::TurnOnAccessCheck()
{
	TODO();
}

int Object::GetIdentityHash()
{
	TODO(); return 0;
}

bool Object::SetHiddenValue(Handle<String> key, Handle<Value> value)
{
	return Set(key, value, DontEnum);
}

Local<Value> Object::GetHiddenValue(Handle<String> key)
{
	return Get(key);
}

bool Object::DeleteHiddenValue(Handle<String> key)
{
	return Delete(key);
}

bool Object::IsDirty()
{
	TODO(); return false;
}

Local<Object> Object::Clone()
{
	TODO(); return Local<Object>();
}

Local<Context> Object::CreationContext()
{
	TODO(); return Local<Context>();
}

void Object::SetIndexedPropertiesToPixelData(uint8_t* data, int length)
{
	internal::Helper* helper = GetHelper();
	helper->m_pixel_data = data;
	helper->m_pixel_data_length = length;
}

bool Object::HasIndexedPropertiesInPixelData()
{
	internal::Helper* helper = GetHelper();
	return (helper->m_pixel_data != NULL);
}

uint8_t* Object::GetIndexedPropertiesPixelData()
{
	internal::Helper* helper = GetHelper();
	return helper->m_pixel_data;
}

int Object::GetIndexedPropertiesPixelDataLength()
{
	internal::Helper* helper = GetHelper();
	return helper->m_pixel_data_length;
}

void Object::SetIndexedPropertiesToExternalArrayData(void* data, ExternalArrayType array_type, int number_of_elements)
{
	internal::Helper* helper = GetHelper();
	helper->m_external_array_data = data;
	helper->m_external_array_data_type = array_type;
	helper->m_external_array_data_length = number_of_elements;
}

bool Object::HasIndexedPropertiesInExternalArrayData()
{
	internal::Helper* helper = GetHelper();
	return (helper->m_external_array_data != NULL);
}

void* Object::GetIndexedPropertiesExternalArrayData()
{
	internal::Helper* helper = GetHelper();
	return helper->m_external_array_data;
}

ExternalArrayType Object::GetIndexedPropertiesExternalArrayDataType()
{
	internal::Helper* helper = GetHelper();
	return helper->m_external_array_data_type;
}

int Object::GetIndexedPropertiesExternalArrayDataLength()
{
	internal::Helper* helper = GetHelper();
	return helper->m_external_array_data_length;
}

bool Object::IsCallable()
{
	internal::Helper* helper = GetHelper();
	Handle<ObjectTemplate> object_template = helper->m_object_template;
	if (!object_template.IsEmpty())
	{
		if (object_template->m_call_as_function_invocation_callback)
		{
			return true;
		}
	}
	return false;
}

Local<Value> Object::CallAsFunction(Handle<Object> that, int argc, Handle<Value> argv[])
{
	internal::Helper* helper = GetHelper();
	Handle<ObjectTemplate> object_template = helper->m_object_template;
	if (!object_template.IsEmpty())
	{
		if (object_template->m_call_as_function_invocation_callback)
		{
			Arguments args(Isolate::GetCurrent(), Handle<Function>(Function::Cast(this)), that, that, object_template->m_call_as_function_data, argc, argv, false); // TODO: holder?
			Handle<Value> result = object_template->m_call_as_function_invocation_callback(args);
			return Local<Value>(result);
		}
	}
	return Local<Value>();
}

Local<Value> Object::CallAsConstructor(int argc, Handle<Value> argv[])
{
	internal::Helper* helper = GetHelper();
	Handle<ObjectTemplate> object_template = helper->m_object_template;
	if (!object_template.IsEmpty())
	{
		if (object_template->m_call_as_function_invocation_callback)
		{
			Local<Object> that = object_template->NewInstance();

			//printf("%s: %s\n", __PRETTY_FUNCTION__, m_name.IsEmpty()?"anonymous":*String::Utf8Value(m_name));
			//printf("Function::NewInstance object %p\n", that->m_js_object);

			JSContextRef js_ctx = Context::GetCurrentJSContext();

			#if 0
			// that->m_js_object.__proto__ = m_js_object.__proto__;
			JSObjectSetPrototype(js_ctx, that->m_js_object, JSObjectGetPrototype(js_ctx, m_js_object));
			#endif

			#if 0
			// that->m_js_object.prototype = m_js_object.prototype;
			JSValueRef js_prototype = JSObjectGetProperty(js_ctx, m_js_object, internal::JSStringWrap("prototype"), NULL);
			JSObjectSetProperty(js_ctx, that->m_js_object, internal::JSStringWrap("prototype"), js_prototype, kJSPropertyAttributeDontEnum, NULL);
			#endif

			#if 1
			// that->m_js_object.__proto__ = m_js_object.prototype;
			JSValueRef js_prototype = JSObjectGetProperty(js_ctx, m_js_object, internal::JSStringWrap("prototype"), NULL);
			JSObjectSetPrototype(js_ctx, that->m_js_object, js_prototype);
			#endif

			Arguments args(Isolate::GetCurrent(), Handle<Function>(Function::Cast(this)), that, that, object_template->m_call_as_function_data, argc, argv, true); // TODO: holder?
			Handle<Value> result = object_template->m_call_as_function_invocation_callback(args);
			if (!result.IsEmpty())
			{
				//return Local<Object>(Object::Cast(*result));
				return Local<Object>(result.As<Object>());
			}

			return that;
		}
	}
	return Local<Value>();
}

/*static*/ JSClassRef Object::sm_js_class = NULL;

/*static*/ Local<Object> Object::New()
{
	return Local<Object>(new Object(Context::GetCurrentJSContext()));
}

/*static*/ Object* Object::Cast(Value* value)
{
	if (RTTI_IsKindOf(Object, value))
	{
		return RTTI_StaticCast(Object, value);
	}

	if (value)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = value->m_js_value;
		if (JSValueIsObjectOfClass(js_ctx, js_value, Object::GetJSClass()))
		{
			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
			assert(js_object == js_value);
			assert(JSObjectGetPrivate(js_object) != NULL);
			return (Object*) JSObjectGetPrivate(js_object);
		}
		if (JSValueIsObject(js_ctx, js_value))
		{
			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
			assert(js_object == js_value);
			return Local<Object>(internal::ImportObject(js_ctx, js_object));
		}
	}
	return NULL;
}

/*static*/ JSClassRef Object::GetJSClass()
{
	if (!sm_js_class)
	{
		JSClassDefinition js_def = kJSClassDefinitionEmpty;
		js_def.attributes = kJSClassAttributeNoAutomaticPrototype;
		js_def.className = "Object";
		//js_def.parentClass = NULL;
		//js_def.staticValues = NULL;
		//js_def.staticFunctions = NULL;
		js_def.initialize = Object::_JS_Initialize;
		js_def.finalize = Object::_JS_Finalize;
		js_def.hasProperty = Object::_JS_HasProperty;
		js_def.getProperty = Object::_JS_GetProperty;
		js_def.setProperty = Object::_JS_SetProperty;
		js_def.deleteProperty = Object::_JS_DeleteProperty;
		js_def.getPropertyNames = Object::_JS_GetPropertyNames;
		//js_def.callAsFunction = Object::_JS_CallAsFunction;
		//js_def.callAsConstructor = Object::_JS_CallAsConstructor;
		js_def.hasInstance = Object::_JS_HasInstance;
		js_def.convertToType = Object::_JS_ConvertToType;
		sm_js_class = JSClassCreate(&js_def);
	}
	return sm_js_class;
}

/*static*/ void Object::_JS_Initialize(JSContextRef js_ctx, JSObjectRef js_object)
{
}

/*static*/ void Object::_JS_Finalize(JSObjectRef js_object)
{
//	Handle<Object> object((Object*) JSObjectGetPrivate(js_object));
//	assert(!object.IsEmpty());
//	if (!object.IsEmpty())
	Object* object = (Object*) JSObjectGetPrivate(js_object);
	assert(object != NULL);
	if (object != NULL)
	{
		object->DetachJSContextAndJSObject();
	}
}

/**
 * If this function returns false, the hasProperty request
 * forwards to object's statically declared properties, then its
 * parent class chain (which includes the default object class),
 * then its prototype chain.
 */
/*static*/ bool Object::_JS_HasProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name)
{
	#if 1 // V8LIKE_DEBUG
	JSDebugPrintScriptStack(js_ctx);
	#endif

	Handle<Object> object((Object*) JSObjectGetPrivate(js_object));
	assert(!object.IsEmpty());
	if (!object.IsEmpty())
	{
		JSValueRef js_key = JSValueMakeString(js_ctx, js_name);

		if (object->HasIndexedPropertiesInExternalArrayData())
		{
			double key = JSValueToNumber(js_ctx, js_key, NULL);
			double whole = 0.0;
			if ((std::modf(key, &whole) == 0.0) && (std::numeric_limits<uint32_t>::min() <= whole) && (whole <= std::numeric_limits<uint32_t>::max()))
			{
				uint32_t index = internal::DoubleToUint32(whole);
				int array_data_length = object->GetIndexedPropertiesExternalArrayDataLength();
				return (index < array_data_length);
			}
		}

		Handle<String> name = Handle<String>(new String(js_ctx, js_key));
		return object->_ObjectHasProperty(name);
	}
	return false;
}

/**
 * If this function returns NULL, the get request forwards to
 * object's statically declared properties, then its parent
 * class chain (which includes the default object class), then
 * its prototype chain.
 */
/*static*/ JSValueRef Object::_JS_GetProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef* js_exception)
{
	#if 1 // V8LIKE_DEBUG
	JSDebugPrintScriptStack(js_ctx);
	#endif

	Handle<Object> object((Object*) JSObjectGetPrivate(js_object));
	assert(!object.IsEmpty());
	if (!object.IsEmpty())
	{
		JSValueRef js_key = JSValueMakeString(js_ctx, js_name);

		if (object->HasIndexedPropertiesInExternalArrayData())
		{
			double key = JSValueToNumber(js_ctx, js_key, NULL);
			double whole = 0.0;
			if ((std::modf(key, &whole) == 0.0) && (std::numeric_limits<uint32_t>::min() <= whole) && (whole <= std::numeric_limits<uint32_t>::max()))
			{
				uint32_t index = internal::DoubleToUint32(whole);
				int array_data_length = object->GetIndexedPropertiesExternalArrayDataLength();
				if (index < array_data_length)
				{
					ExternalArrayType array_data_type = object->GetIndexedPropertiesExternalArrayDataType();
					void* array_data = object->GetIndexedPropertiesExternalArrayData();
					JSValueRef js_value = NULL;
					switch (array_data_type)
					{
					case kExternalByteArray:			js_value = JSValueMakeNumber(js_ctx, internal::FastI2D (((char*)		   array_data)[index])); break;
					case kExternalShortArray:   		js_value = JSValueMakeNumber(js_ctx, internal::FastI2D (((short*)   	   array_data)[index])); break;
					case kExternalIntArray: 			js_value = JSValueMakeNumber(js_ctx, internal::FastI2D (((int*) 		   array_data)[index])); break;
					case kExternalPixelArray:			js_value = JSValueMakeNumber(js_ctx, internal::FastUI2D(((unsigned char*)  array_data)[index])); break;
					case kExternalUnsignedByteArray:	js_value = JSValueMakeNumber(js_ctx, internal::FastUI2D(((unsigned char*)  array_data)[index])); break;
					case kExternalUnsignedShortArray:   js_value = JSValueMakeNumber(js_ctx, internal::FastUI2D(((unsigned short*) array_data)[index])); break;
					case kExternalUnsignedIntArray:		js_value = JSValueMakeNumber(js_ctx, internal::FastUI2D(((unsigned int*)   array_data)[index])); break;
					case kExternalFloatArray:			js_value = JSValueMakeNumber(js_ctx, (double)   	   (((float*)   	   array_data)[index])); break;
					case kExternalDoubleArray:			js_value = JSValueMakeNumber(js_ctx,				   (((double*)  	   array_data)[index])); break;
					default: assert(0); js_value = JSValueMakeUndefined(js_ctx); break;
					}
					return js_value;
				}

				return JSValueMakeUndefined(js_ctx);
			}
		}

		Handle<String> name = Handle<String>(new String(js_ctx, js_key));
		Handle<Value> value = object->_ObjectGetProperty(name);
		#if 0
		if (value->IsObject())
		{
			printf("getting object %p %s as object %p\n", js_object, *String::Utf8Value(name), value->m_js_value);
		}
		else if (value->IsString())
		{
			String::Utf8Value value_str(value->ToString());
			const size_t length = 256; char buffer[length];
			strncpy(buffer, *value_str, length); buffer[length - 1] = '\0';
			if (strlen(*value_str) >= length) { strcpy(&buffer[length - 4], "..."); }
			printf("getting object %p %s as string %p \"%s\"\n", js_object, *String::Utf8Value(name), value->m_js_value, buffer);
		}
		else
		{
			printf("getting object %p %s as '%s'\n", js_object, *String::Utf8Value(name), *String::Utf8Value(value->ToString()));
		}
		#endif
		if (!value.IsEmpty())
		{
			return internal::ExportValue(value);
		}
	}
	return NULL;
}

/**
 * If this function returns false, the set request forwards to
 * object's statically declared properties, then its parent
 * class chain (which includes the default object class).
 */
/*static*/ bool Object::_JS_SetProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef js_value, JSValueRef* js_exception)
{
	#if 1 // V8LIKE_DEBUG
	JSDebugPrintScriptStack(js_ctx);
	#endif

	Handle<Object> object((Object*) JSObjectGetPrivate(js_object));
	assert(!object.IsEmpty());
	if (!object.IsEmpty())
	{
		JSValueRef js_key = JSValueMakeString(js_ctx, js_name);

		if (object->HasIndexedPropertiesInExternalArrayData())
		{
			double key = JSValueToNumber(js_ctx, js_key, NULL);
			double data = JSValueToNumber(js_ctx, js_value, NULL);
			double whole = 0.0;
			if ((std::modf(key, &whole) == 0.0) && (std::numeric_limits<uint32_t>::min() <= whole) && (whole <= std::numeric_limits<uint32_t>::max()) && (std::isnan(data) == 0))
			{
				uint32_t index = internal::DoubleToUint32(whole);
				int array_data_length = object->GetIndexedPropertiesExternalArrayDataLength();
				if (index < array_data_length)
				{
					ExternalArrayType array_data_type = object->GetIndexedPropertiesExternalArrayDataType();
					void* array_data = object->GetIndexedPropertiesExternalArrayData();
					switch (array_data_type)
					{
					case kExternalByteArray:			((char*)		   array_data)[index] = internal::DoubleToInt32 (data);	return true;
					case kExternalShortArray:   		((short*)   	   array_data)[index] = internal::DoubleToInt32 (data);	return true;
					case kExternalIntArray: 			((int*) 		   array_data)[index] = internal::DoubleToInt32 (data);	return true;
					case kExternalPixelArray:			((unsigned char*)  array_data)[index] = internal::DoubleToUint32(data);	return true;
					case kExternalUnsignedByteArray:	((unsigned char*)  array_data)[index] = internal::DoubleToUint32(data);	return true;
					case kExternalUnsignedShortArray:   ((unsigned short*) array_data)[index] = internal::DoubleToUint32(data);	return true;
					case kExternalUnsignedIntArray:		((unsigned int*)   array_data)[index] = internal::DoubleToUint32(data);	return true;
					case kExternalFloatArray:			((float*)   	   array_data)[index] = (float) 				 data;	return true;
					case kExternalDoubleArray:			((double*)  	   array_data)[index] = 						 data;	return true;
					default: assert(0); return false;
					}
				}

				return false;
			}
		}

		Handle<String> name = Handle<String>(new String(js_ctx, js_key));
		Handle<Value> value = internal::ImportValue(js_ctx, js_value);
		#if 0
		if (value->IsObject())
		{
			printf("setting object %p %s to object %p\n", js_object, *String::Utf8Value(name), value->m_js_value);
		}
		else if (value->IsString())
		{
			String::Utf8Value value_str(value->ToString());
			const size_t length = 256; char buffer[length];
			strncpy(buffer, *value_str, length); buffer[length - 1] = '\0';
			if (strlen(*value_str) >= length) { strcpy(&buffer[length - 4], "..."); }
			printf("setting object %p %s to string %p \"%s\"\n", js_object, *String::Utf8Value(name), value->m_js_value, buffer);
		}
		else
		{
			printf("setting object %p %s to '%s'\n", js_object, *String::Utf8Value(name), *String::Utf8Value(value->ToString()));
		}
		#endif
		return object->_ObjectSetProperty(name, value, None);
	}
	return false;
}

bool Object::_JS_DeleteProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef* js_exception)
{
	#if 1 // V8LIKE_DEBUG
	JSDebugPrintScriptStack(js_ctx);
	#endif

	Handle<Object> object((Object*) JSObjectGetPrivate(js_object));
	assert(!object.IsEmpty());
	if (!object.IsEmpty())
	{
		JSValueRef js_key = JSValueMakeString(js_ctx, js_name);
		Handle<String> name = Handle<String>(new String(js_ctx, js_key));
		return object->_ObjectDeleteProperty(name);
	}
	return false;
}

void Object::_JS_GetPropertyNames(JSContextRef js_ctx, JSObjectRef js_object, JSPropertyNameAccumulatorRef js_name_accumulator)
{
	#if 1 // V8LIKE_DEBUG
	JSDebugPrintScriptStack(js_ctx);
	#endif

	Handle<Object> object((Object*) JSObjectGetPrivate(js_object));
	assert(!object.IsEmpty());
	if (!object.IsEmpty())
	{
		object->_ObjectGetPropertyNames(js_name_accumulator);
	}
}

bool Object::_JS_HasInstance(JSContextRef js_ctx, JSObjectRef js_object, JSValueRef js_instance, JSValueRef* js_exception)
{
	#if 1 // V8LIKE_DEBUG
	JSDebugPrintScriptStack(js_ctx);
	#endif

	Handle<Object> object((Object*) JSObjectGetPrivate(js_object));
	assert(!object.IsEmpty());
	if (!object.IsEmpty())
	{
		if (JSValueIsObject(js_ctx, js_instance))
		{
			Handle<Object> instance = internal::ImportObject(js_ctx, JSValueToObject(js_ctx, js_instance, NULL));

			internal::Helper* instance_helper = instance->GetHelper();

			if (!instance_helper->m_function.IsEmpty())
			{
				return (instance_helper->m_function == object);
			}
		}
	}

	return false;
}

/*static*/ JSValueRef Object::_JS_ConvertToType(JSContextRef js_ctx, JSObjectRef js_object, JSType js_type, JSValueRef* js_exception)
{
	#if 1 // V8LIKE_DEBUG
	JSDebugPrintScriptStack(js_ctx);
	#endif

	Handle<Object> object((Object*) JSObjectGetPrivate(js_object));
	assert(!object.IsEmpty());
	if (!object.IsEmpty())
	{
	}

	return NULL;
}

/// v8::Array

RTTI_IMPLEMENT(v8::Array, v8::Object);

Array::Array(JSContextRef js_ctx, int length)
{
	JSValueRef js_exception = NULL;
	if (length > 0)
	{
		JSValueRef js_length = JSValueMakeNumber(js_ctx, length);
		const JSValueRef js_argv[] = { js_length };
		size_t js_argc = sizeof(js_argv) / sizeof(*js_argv);
		Object::AttachJSContextAndJSObject(js_ctx, JSObjectMakeArray(js_ctx, js_argc, js_argv, &js_exception));
	}
	else
	{
		Object::AttachJSContextAndJSObject(js_ctx, JSObjectMakeArray(js_ctx, 0, NULL, &js_exception));
	}
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
	assert(JSValueIsArray(js_ctx, m_js_value));
}

Array::Array(JSContextRef js_ctx, JSObjectRef js_object)
{
	Object::AttachJSContextAndJSObject(js_ctx, js_object);
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
	assert(JSValueIsArray(js_ctx, m_js_value));
}

uint32_t Array::Length() const
{
	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	JSValueRef js_length = JSObjectGetProperty(js_ctx, m_js_object, internal::JSStringWrap("length"), &js_exception);
	if (js_exception)
	{
		ThrowException(internal::ImportValue(js_ctx, js_exception));
	}
	if (js_length && JSValueIsNumber(js_ctx, js_length))
	{
		double length = JSValueToNumber(js_ctx, js_length, NULL);
		if (length >= 0)
		{
			return internal::DoubleToUint32(length);
		}
	}
	return 0;
}

Local<Object> Array::CloneElementAt(uint32_t index)
{
	TODO(); return Local<Object>();
}

////*static*/ JSClassRef Array::sm_js_class = NULL;

/*static*/ Local<Array> Array::New(int length)
{
	return Local<Array>(new Array(Context::GetCurrentJSContext(), length));
}

/*static*/ Array* Array::Cast(Value* value)
{
	if (RTTI_IsKindOf(Array, value))
	{
		return RTTI_StaticCast(Array, value);
	}

	if (value)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = value->m_js_value;
///		if (JSValueIsObjectOfClass(js_ctx, js_value, Array::GetJSClass()))
///		{
///			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
///			return (Array*) JSObjectGetPrivate(js_object);
///		}
		if (JSValueIsArray(js_ctx, js_value))
		{
			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
			assert(js_object == js_value);
			return new Array(js_ctx, js_object);
		}
	}
	return NULL;
}

////*static*/ JSClassRef Array::GetJSClass()
///{
///	if (!sm_js_class)
///	{
///		JSClassDefinition js_def = kJSClassDefinitionEmpty;
///		js_def.attributes = kJSClassAttributeNoAutomaticPrototype;
///		js_def.className = "Array";
///		js_def.parentClass = Object::GetJSClass();
///		sm_js_class = JSClassCreate(&js_def);
///	}
///	return sm_js_class;
///}

/// v8::Arguments

Arguments::Arguments(Isolate* isolate, Handle<Function> callee, Handle<Object> that, Handle<Object> holder, Handle<Value> data, int argc, Handle<Value>* argv, bool is_construct_call) :
	m_isolate(isolate), m_callee(callee), m_that(that), m_holder(holder), m_data(data), m_argc(argc), m_argv(argv), m_is_construct_call(is_construct_call) {}

Arguments::~Arguments()
{
	m_isolate = NULL;
	m_argc = 0;
	m_argv = NULL;
	m_is_construct_call = false;
}

Isolate* Arguments::GetIsolate() const
{
	return m_isolate;
}

int Arguments::Length() const
{
	return m_argc;
}

Local<Value> Arguments::operator[](int i) const
{
	if ((0 <= i) && (i < Length()))
	{
		return Local<Value>(m_argv[i]);
	}
	return Local<Value>(Undefined());
}

Local<Function> Arguments::Callee() const
{
	return Local<Function>(m_callee);
}

Local<Object> Arguments::This() const
{
	return Local<Object>(m_that);
}

Local<Object> Arguments::Holder() const
{
	return Local<Object>(m_holder);
}

bool Arguments::IsConstructCall() const
{
	return m_is_construct_call;
}

Local<Value> Arguments::Data() const
{
	return Local<Value>(m_data);
}

/// v8::Function

RTTI_IMPLEMENT(v8::Function, v8::Object);

Function::Function(JSContextRef js_ctx)
{
	Object::AttachJSContextAndJSObject(js_ctx, JSObjectMake(js_ctx, Function::GetJSClass(), this));
	assert(JSValueIsObjectOfClass(js_ctx, m_js_value, Function::GetJSClass()));
	assert(JSValueIsObjectOfClass(js_ctx, m_js_value, Object::GetJSClass()));
	assert(JSObjectGetPrivate(m_js_object) == this);

	// m_js_object.__proto__ = new Function("");
	//JSObjectRef js_prototype = JSObjectMakeFunction(js_ctx, NULL, 0, NULL, internal::JSStringWrap(""), NULL, 0, NULL);
	//JSObjectSetPrototype(js_ctx, m_js_object, js_prototype);

	// m_js_object.__proto__ = new Function();
	JSStringRef js_name = JSStringCreateWithUTF8CString("Function");
	JSObjectRef js_ctor = JSValueToObject(js_ctx, JSObjectGetProperty(js_ctx, Context::GetJSGlobalObject(js_ctx, JSContextGetGlobalObject(js_ctx)), js_name, NULL), NULL);
	JSObjectRef js_prototype = JSObjectCallAsConstructor(js_ctx, js_ctor, 0, NULL, NULL);
	JSObjectSetPrototype(js_ctx, m_js_object, js_prototype);
	JSStringRelease(js_name); js_name = NULL;

	assert(JSObjectGetPrivate(m_js_object) == this);
	assert(JSValueIsObject(js_ctx, m_js_value));
	assert(JSObjectIsFunction(js_ctx, m_js_object));
}

Function::Function(JSContextRef js_ctx, JSObjectRef js_object)
{
	Object::AttachJSContextAndJSObject(js_ctx, js_object);
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
	assert(JSValueIsFunction(js_ctx, m_js_object));
}

bool Function::_FunctionHasProperty(Handle<String> name)
{
	// check function template, for Objects created by FunctionTemplate::GetFunction()->NewInstance()
	if (!m_function_template.IsEmpty() && m_function_template->_FunctionTemplateHasProperty(Handle<Function>(this), name))
	{
		return true;
	}

	// check function base class
	if (Object::_ObjectHasProperty(name))
	{
		return true;
	}

	return false;
}

Handle<Value> Function::_FunctionGetProperty(Handle<String> name)
{
	Handle<Value> value;

	// check function template, for Objects created by FunctionTemplate::GetFunction()->NewInstance()
	if (!m_function_template.IsEmpty())
	{
		value = m_function_template->_FunctionTemplateGetProperty(Handle<Function>(this), name);
		if (!value.IsEmpty())
		{
			return value;
		}
	}

	// check function base class
	value = Object::_ObjectGetProperty(name);
	if (!value.IsEmpty())
	{
		return value;
	}

	return value;
}

bool Function::_FunctionSetProperty(Handle<String> name, Handle<Value> value, PropertyAttribute attrib)
{
	// check function template, for Objects created by FunctionTemplate::GetFunction()->NewInstance()
	if (!m_function_template.IsEmpty())
	{
		if (m_function_template->_FunctionTemplateSetProperty(Handle<Function>(this), name, value))
		{
			return true;
		}
	}

	// check function base class
	if (Object::_ObjectSetProperty(name, value, attrib))
	{
		return true;
	}

	return false;
}

bool Function::_FunctionDeleteProperty(Handle<String> name)
{
	// check function template, for Objects created by FunctionTemplate::GetFunction()->NewInstance()
	if (!m_function_template.IsEmpty() && m_function_template->_FunctionTemplateDeleteProperty(Handle<Function>(this), name))
	{
		return true;
	}

	// check function base class
	if (Object::_ObjectDeleteProperty(name))
	{
		return true;
	}

	return false;
}

void Function::_FunctionGetPropertyNames(JSPropertyNameAccumulatorRef js_name_accumulator)
{
	// check function template, for Objects created by FunctionTemplate::GetFunction()->NewInstance()
	if (!m_function_template.IsEmpty())
	{
		m_function_template->_FunctionTemplateGetPropertyNames(Handle<Function>(this), js_name_accumulator);
	}

	// check function base class
	Object::_ObjectGetPropertyNames(js_name_accumulator);
}

Local<Value> Function::Call(Handle<Object> that, int argc, Handle<Value> argv[])
{
	if (m_is_native)
	{
		if (!m_function_template.IsEmpty() && m_function_template->m_callback)
		{
			Arguments args(Isolate::GetCurrent(), Handle<Function>(this), that, that, m_function_template->m_data, argc, argv, false); // TODO: holder?
			Handle<Value> result = m_function_template->m_callback(args);
			return Local<Value>(result);
		}
		else
		{
			return Local<Value>(Undefined());
		}
	}
	else
	{
		//return Object::CallAsFunction(that, argc, argv);
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		assert(JSObjectIsFunction(js_ctx, m_js_object));
		JSObjectRef js_that = (that == Context::GetCurrent()->Global())?(NULL):(internal::ExportObject(that));
		size_t js_argc = (size_t) argc;
		JSValueRef* js_argv = (js_argc > 0)?(new JSValueRef[js_argc]):(NULL);
		for (size_t i = 0; i < js_argc; ++i)
		{
			js_argv[i] = internal::ExportValue(argv[i]);
		}
		JSValueRef js_value = JSObjectCallAsFunction(js_ctx, m_js_object, js_that, js_argc, js_argv, &js_exception);
		if (js_argv)
		{
			delete[] js_argv; js_argv = NULL;
		}
		if (js_exception)
		{
			ThrowException(internal::ImportValue(js_ctx, js_exception));
		}
		return Local<Value>(internal::ImportValue(js_ctx, js_value));
	}
}

Local<Object> Function::NewInstance(int argc, Handle<Value> argv[]) /*const*/
{
	if (m_is_native)
	{
		if (!m_function_template.IsEmpty())
		{
			Local<Object> that = m_function_template->InstanceTemplate()->NewInstance();

			//printf("%s: %s\n", __PRETTY_FUNCTION__, m_name.IsEmpty()?"anonymous":*String::Utf8Value(m_name));
			//printf("Function::NewInstance object %p\n", that->m_js_object);

			JSContextRef js_ctx = Context::GetCurrentJSContext();

			#if 0
			// that->m_js_object.__proto__ = m_js_object.__proto__;
			JSObjectSetPrototype(js_ctx, that->m_js_object, JSObjectGetPrototype(js_ctx, m_js_object));
			#endif

			#if 0
			// that->m_js_object.prototype = m_js_object.prototype;
			JSValueRef js_prototype = JSObjectGetProperty(js_ctx, m_js_object, internal::JSStringWrap("prototype"), NULL);
			JSObjectSetProperty(js_ctx, that->m_js_object, internal::JSStringWrap("prototype"), js_prototype, kJSPropertyAttributeDontEnum, NULL);
			#endif

			#if 1
			// that->m_js_object.__proto__ = m_js_object.prototype;
			JSValueRef js_prototype = JSObjectGetProperty(js_ctx, m_js_object, internal::JSStringWrap("prototype"), NULL);
			JSObjectSetPrototype(js_ctx, that->m_js_object, js_prototype);
			#endif

			internal::Helper* helper = that->GetHelper();
			helper->m_function = Handle<Function>(this);

			if (m_function_template->m_callback)
			{
				Arguments args(Isolate::GetCurrent(), Handle<Function>(this), that, that, m_function_template->m_data, argc, argv, true); // TODO: holder?
				Handle<Value> result = m_function_template->m_callback(args);
				if (!result.IsEmpty())
				{
					//return Local<Object>(Object::Cast(*result));
					return Local<Object>(result.As<Object>());
				}
			}

			return that;
		}
		else
		{
			return Object::New();
		}
	}
	else
	{
		//return Object::CallAsConstructor(argc, argv);
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		assert(JSObjectIsConstructor(js_ctx, m_js_object));
		size_t js_argc = (size_t) argc;
		JSValueRef* js_argv = (js_argc > 0)?(new JSValueRef[js_argc]):(NULL);
		for (size_t i = 0; i < js_argc; ++i)
		{
			js_argv[i] = internal::ExportValue(argv[i]);
		}
		JSObjectRef js_object = JSObjectCallAsConstructor(js_ctx, m_js_object, js_argc, js_argv, &js_exception);
		if (js_argv)
		{
			delete[] js_argv; js_argv = NULL;
		}
		if (js_exception)
		{
			ThrowException(internal::ImportValue(js_ctx, js_exception));
		}
		return Local<Object>(internal::ImportObject(js_ctx, js_object));
	}
}

/*static*/ JSClassRef Function::sm_js_class = NULL;

/*static*/ Function* Function::Cast(Value* value)
{
	if (RTTI_IsKindOf(Function, value))
	{
		return RTTI_StaticCast(Function, value);
	}

	if (value)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = value->m_js_value;
		if (JSValueIsObjectOfClass(js_ctx, js_value, Function::GetJSClass()))
		{
			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
			assert(js_object == js_value);
			assert(JSObjectGetPrivate(js_object) != NULL);
			return (Function*) JSObjectGetPrivate(js_object);
		}
		if (JSValueIsFunction(js_ctx, js_value))
		{
			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
			assert(js_object == js_value);
			return new Function(js_ctx, js_object);
		}
	}
	return NULL;

}

/*static*/ JSClassRef Function::GetJSClass()
{
	if (!sm_js_class)
	{
		JSClassDefinition js_def = kJSClassDefinitionEmpty;
		js_def.attributes = kJSClassAttributeNoAutomaticPrototype;
		js_def.className = "Function";
		js_def.parentClass = Object::GetJSClass();
		js_def.callAsFunction = Function::_JS_CallAsFunction;
		js_def.callAsConstructor = Function::_JS_CallAsConstructor;
		sm_js_class = JSClassCreate(&js_def);
	}
	return sm_js_class;
}

/*static*/ JSValueRef Function::_JS_CallAsFunction(JSContextRef js_ctx, JSObjectRef js_function, JSObjectRef js_that, size_t js_argc, const JSValueRef js_argv[], JSValueRef* js_exception)
{
	#if 1 // V8LIKE_DEBUG
	JSDebugPrintScriptStack(js_ctx);
	#endif

	Handle<Function> function((Function*) JSObjectGetPrivate(js_function));
	assert(!function.IsEmpty());
	if (!function.IsEmpty())
	{
		Handle<Object> that = internal::ImportObject(js_ctx, js_that);
		int argc = (int) js_argc;
		Handle<Value>* argv = (argc > 0)?(new Handle<Value>[argc]):(NULL);
		for (int i = 0; i < argc; ++i) { argv[i] = internal::ImportValue(js_ctx, js_argv[i]); }
		Context::GetCurrent()->m_js_ctx_stack.push_back(js_ctx);
		Handle<Value> result = function->Call(that, argc, argv);
		Context::GetCurrent()->m_js_ctx_stack.pop_back();
		if (argv) { delete[] argv; argv = NULL; }
		if (!sg_last_exception.IsEmpty())
		{
			if (js_exception != NULL)
			{
				*js_exception = internal::ExportValue(sg_last_exception);
			}
			sg_last_exception._Clear();
		}
		return internal::ExportValue(result);
	}
	return NULL;
}

/*static*/ JSObjectRef Function::_JS_CallAsConstructor(JSContextRef js_ctx, JSObjectRef js_function, size_t js_argc, const JSValueRef js_argv[], JSValueRef* js_exception)
{
	#if 1 // V8LIKE_DEBUG
	JSDebugPrintScriptStack(js_ctx);
	#endif

	Handle<Function> function((Function*) JSObjectGetPrivate(js_function));
	assert(!function.IsEmpty());
	if (!function.IsEmpty())
	{
		int argc = (int) js_argc;
		Handle<Value>* argv = (argc > 0)?(new Handle<Value>[argc]):(NULL);
		for (int i = 0; i < argc; ++i) { argv[i] = internal::ImportValue(js_ctx, js_argv[i]); }
		Context::GetCurrent()->m_js_ctx_stack.push_back(js_ctx);
		Handle<Object> result = function->NewInstance(argc, argv);
		Context::GetCurrent()->m_js_ctx_stack.pop_back();
		if (argv) { delete[] argv; argv = NULL; }
		if (!sg_last_exception.IsEmpty())
		{
			if (js_exception != NULL)
			{
				*js_exception = internal::ExportValue(sg_last_exception);
			}
			sg_last_exception._Clear();
		}
		return internal::ExportObject(result);
	}
	return NULL;
}

/// v8::Date

RTTI_IMPLEMENT(v8::Date, v8::Object);

Date::Date(JSContextRef js_ctx, double time)
{
	JSValueRef js_exception = NULL;
	JSValueRef js_time = JSValueMakeNumber(js_ctx, time);
	const JSValueRef js_argv[] = { js_time };
	size_t js_argc = sizeof(js_argv) / sizeof(*js_argv);
	Object::AttachJSContextAndJSObject(js_ctx, JSObjectMakeDate(js_ctx, js_argc, js_argv, &js_exception));
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
	assert(JSValueIsDate(js_ctx, m_js_object));
}

Date::Date(JSContextRef js_ctx, JSObjectRef js_object)
{
	Object::AttachJSContextAndJSObject(js_ctx, js_object);
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
	assert(JSValueIsDate(js_ctx, m_js_object));
}

////*static*/ JSClassRef Date::sm_js_class = NULL;

/*static*/ Local<Value> Date::New(double time)
{
	return Local<Value>(new Date(Context::GetCurrentJSContext(), time));
}

/*static*/ Date* Date::Cast(Value* value)
{
	if (RTTI_IsKindOf(Date, value))
	{
		return RTTI_StaticCast(Date, value);
	}

	if (value)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = value->m_js_value;
///		if (JSValueIsObjectOfClass(js_ctx, js_value, Date::GetJSClass()))
///		{
///			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
///			assert(JSObjectGetPrivate(js_object) != NULL);
///			return (Date*) JSObjectGetPrivate(js_object);
///		}
		if (JSValueIsDate(js_ctx, js_value))
		{
			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
			assert(js_object == js_value);
			return new Date(js_ctx, js_object);
		}
	}
	return NULL;
}

////*static*/ JSClassRef Date::GetJSClass()
///{
///	if (!sm_js_class)
///	{
///		JSClassDefinition js_def = kJSClassDefinitionEmpty;
///		js_def.attributes = kJSClassAttributeNoAutomaticPrototype;
///		js_def.className = "Date";
///		js_def.parentClass = Object::GetJSClass();
///		sm_js_class = JSClassCreate(&js_def);
///	}
///	return sm_js_class;
///}

/// v8::RegExp

RTTI_IMPLEMENT(v8::RegExp, v8::Object);

RegExp::RegExp(JSContextRef js_ctx, Handle<String> pattern, Flags flags)
{
	JSValueRef js_exception = NULL;
	JSValueRef js_pattern = pattern->m_js_value;
	char flags_str[4] = { '\0' };
	if (flags & kGlobal) { strcat(flags_str, "g"); }
	if (flags & kIgnoreCase) { strcat(flags_str, "i"); }
	if (flags & kMultiline) { strcat(flags_str, "m"); }
	JSStringRef js_flags_str = JSStringCreateWithUTF8CString(flags_str);
	JSValueRef js_flags = JSValueMakeString(js_ctx, js_flags_str);
	JSStringRelease(js_flags_str); js_flags_str = NULL;
	const JSValueRef js_argv[] = { js_pattern, js_flags };
	size_t js_argc = sizeof(js_argv) / sizeof(*js_argv);
	Object::AttachJSContextAndJSObject(js_ctx, JSObjectMakeRegExp(js_ctx, js_argc, js_argv, &js_exception));
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
	assert(JSValueIsRegExp(js_ctx, m_js_object));
}

RegExp::RegExp(JSContextRef js_ctx, JSObjectRef js_object)
{
	Object::AttachJSContextAndJSObject(js_ctx, js_object);
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
	assert(JSValueIsRegExp(js_ctx, m_js_object));
}

////*static*/ JSClassRef RegExp::sm_js_class = NULL;

/*static*/ Local<RegExp> RegExp::New(Handle<String> pattern, Flags flags)
{
	return Local<RegExp>(new RegExp(Context::GetCurrentJSContext(), pattern, flags));
}

/*static*/ RegExp* RegExp::Cast(Value* value)
{
	if (RTTI_IsKindOf(RegExp, value))
	{
		return RTTI_StaticCast(RegExp, value);
	}

	if (value)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = value->m_js_value;
///		if (JSValueIsObjectOfClass(js_ctx, js_value, RegExp::GetJSClass()))
///		{
///			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
///			assert(JSObjectGetPrivate(js_object) != NULL);
///			return (RegExp*) JSObjectGetPrivate(js_object);
///		}
		if (JSValueIsRegExp(js_ctx, js_value))
		{
			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
			assert(js_object == js_value);
			return new RegExp(js_ctx, js_object);
		}
	}
	return NULL;
}

////*static*/ JSClassRef RegExp::GetJSClass()
///{
///	if (!sm_js_class)
///	{
///		JSClassDefinition js_def = kJSClassDefinitionEmpty;
///		js_def.attributes = kJSClassAttributeNoAutomaticPrototype;
///		js_def.className = "RegExp";
///		js_def.parentClass = Object::GetJSClass();
///		sm_js_class = JSClassCreate(&js_def);
///	}
///	return sm_js_class;
///}

/// v8::BooleanObject

RTTI_IMPLEMENT(v8::BooleanObject, v8::Object);

BooleanObject::BooleanObject(JSContextRef js_ctx, bool value)
{
	// TODO
	Object::AttachJSContextAndJSObject(js_ctx, JSObjectMake(js_ctx, Object::GetJSClass(), this));
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
///	assert(JSValueIsBooleanObject(js_ctx, m_js_object));
}

BooleanObject::BooleanObject(JSContextRef js_ctx, JSObjectRef js_object)
{
	Object::AttachJSContextAndJSObject(js_ctx, js_object);
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
///	assert(JSValueIsBooleanObject(js_ctx, m_js_object));
}

/*static*/ Local<Value> BooleanObject::New(bool value)
{
	return Local<Value>(new BooleanObject(Context::GetCurrentJSContext(), value));
}

/*static*/ BooleanObject* BooleanObject::Cast(Value* value)
{
	if (RTTI_IsKindOf(BooleanObject, value))
	{
		return RTTI_StaticCast(BooleanObject, value);
	}

	// TODO
///	if (value)
///	{
///		JSContextRef js_ctx = Context::GetCurrentJSContext();
///		JSValueRef js_value = value->m_js_value;
///		if (JSValueIsBooleanObject(js_ctx, js_value))
///		{
///			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
///			assert(js_object == js_value);
///			return new BooleanObject(js_ctx, js_object);
///		}
///	}
	return NULL;
}

/// v8::NumberObject

RTTI_IMPLEMENT(v8::NumberObject, v8::Object);

NumberObject::NumberObject(JSContextRef js_ctx, double value)
{
	// TODO
	Object::AttachJSContextAndJSObject(js_ctx, JSObjectMake(js_ctx, Object::GetJSClass(), this));
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
///	assert(JSValueIsNumberObject(js_ctx, m_js_object));
}

NumberObject::NumberObject(JSContextRef js_ctx, JSObjectRef js_object)
{
	Object::AttachJSContextAndJSObject(js_ctx, js_object);
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
///	assert(JSValueIsNumberObject(js_ctx, m_js_object));
}

/*static*/ Local<Value> NumberObject::New(double value)
{
	return Local<Value>(new NumberObject(Context::GetCurrentJSContext(), value));
}

/*static*/ NumberObject* NumberObject::Cast(Value* value)
{
	if (RTTI_IsKindOf(NumberObject, value))
	{
		return RTTI_StaticCast(NumberObject, value);
	}

	// TODO
///	if (value)
///	{
///		JSContextRef js_ctx = Context::GetCurrentJSContext();
///		JSValueRef js_value = value->m_js_value;
///		if (JSValueIsNumberObject(js_ctx, js_value))
///		{
///			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
///			assert(js_object == js_value);
///			return new NumberObject(js_ctx, js_object);
///		}
///	}
	return NULL;
}

/// v8::StringObject

RTTI_IMPLEMENT(v8::StringObject, v8::Object);

StringObject::StringObject(JSContextRef js_ctx, Handle<String> value)
{
	// TODO
	Object::AttachJSContextAndJSObject(js_ctx, JSObjectMake(js_ctx, Object::GetJSClass(), this));
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
///	assert(JSValueIsStringObject(js_ctx, m_js_object));
}

StringObject::StringObject(JSContextRef js_ctx, JSObjectRef js_object)
{
	Object::AttachJSContextAndJSObject(js_ctx, js_object);
	assert(JSObjectGetPrivate(m_js_object) == NULL);
	assert(JSValueIsObject(js_ctx, m_js_value));
///	assert(JSValueIsStringObject(js_ctx, m_js_object));
}

/*static*/ Local<Value> StringObject::New(Handle<String> value)
{
	return Local<Value>(new StringObject(Context::GetCurrentJSContext(), value));
}

/*static*/ StringObject* StringObject::Cast(Value* value)
{
	if (RTTI_IsKindOf(StringObject, value))
	{
		return RTTI_StaticCast(StringObject, value);
	}

	// TODO
///	if (value)
///	{
///		JSContextRef js_ctx = Context::GetCurrentJSContext();
///		JSValueRef js_value = value->m_js_value;
///		if (JSValueIsStringObject(js_ctx, js_value))
///		{
///			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
///			assert(js_object == js_value);
///			return new StringObject(js_ctx, js_object);
///		}
///	}
	return NULL;
}

/// v8::External

RTTI_IMPLEMENT(v8::External, v8::Value);

External::External(JSContextRef js_ctx, void* data) : 
	m_js_object(NULL), m_data(data)
{
	External::AttachJSContextAndJSObject(js_ctx, JSObjectMake(js_ctx, External::GetJSClass(), this));
	assert(JSValueIsObjectOfClass(js_ctx, m_js_value, External::GetJSClass()));
	assert(JSObjectGetPrivate(m_js_object) == this);
	assert(JSValueIsObject(js_ctx, m_js_value));
}

External::~External()
{
	if (m_data)
	{
		m_data = NULL;
	}

	if (m_js_object)
	{
		assert(JSObjectGetPrivate(m_js_object) == this);
		bool ret = JSObjectSetPrivate(m_js_object, NULL);
		assert(ret);
		m_js_object = NULL;
	}
}

void External::AttachJSContextAndJSObject(JSContextRef js_ctx, JSObjectRef js_object)
{
	AttachJSContextAndJSValue(js_ctx, m_js_object = js_object);

	assert(JSValueIsObjectOfClass(js_ctx, m_js_value, External::GetJSClass()));
	assert(JSObjectGetPrivate(m_js_object) == this);

	SmartObject::IncRef(); // v8::External JSObjectRef private reference
}

void External::DetachJSContextAndJSObject()
{
	assert(m_js_object != NULL);
	assert(JSObjectGetPrivate(m_js_object) == this);
	bool ret = JSObjectSetPrivate(m_js_object, NULL);
	assert(ret);
	m_js_object = NULL;
	assert(m_js_object == NULL);

	DetachJSContextAndJSValue();

	SmartObject::DecRef(); // v8::External JSObjectRef private reference
}

void* External::Value() const
{
	return m_data;
}

/*static*/ JSClassRef External::sm_js_class = NULL;

/*static*/ Local<v8::Value> External::Wrap(void* data)
{
	return Local<v8::Value>(External::New(data));
}

/*static*/ void* External::Unwrap(Handle<v8::Value> value)
{
	External* external = External::Cast(*value);
	return (external)?(external->m_data):(NULL);
}

/*static*/ Local<External> External::New(void* data)
{
	return Local<External>(new External(Context::GetCurrentJSContext(), data));
}

/*static*/ External* External::Cast(v8::Value* value)
{
	if (RTTI_IsKindOf(External, value))
	{
		return RTTI_StaticCast(External, value);
	}

	if (value)
	{
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		JSValueRef js_value = value->m_js_value;
		if (JSValueIsObjectOfClass(js_ctx, js_value, External::GetJSClass()))
		{
			JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
			assert(js_object == js_value);
			assert(JSObjectGetPrivate(js_object) != NULL);
			return (External*) JSObjectGetPrivate(js_object);
		}
	}
	return NULL;
}

/*static*/ JSClassRef External::GetJSClass()
{
	if (!sm_js_class)
	{
		JSClassDefinition js_def = kJSClassDefinitionEmpty;
		js_def.attributes = kJSClassAttributeNoAutomaticPrototype;
		js_def.className = "External";
		js_def.initialize = External::_JS_Initialize;
		js_def.finalize = External::_JS_Finalize;
		sm_js_class = JSClassCreate(&js_def);
	}
	return sm_js_class;
}

/*static*/ void External::_JS_Initialize(JSContextRef js_ctx, JSObjectRef js_object)
{
}

/*static*/ void External::_JS_Finalize(JSObjectRef js_object)
{
//	Handle<External> external((External*) JSObjectGetPrivate(js_object));
//	assert(!external.IsEmpty());
//	if (!external.IsEmpty())
	External* external = (External*) JSObjectGetPrivate(js_object);
	assert(external != NULL);
	if (external != NULL)
	{
		external->DetachJSContextAndJSObject();
	}
}

/// v8::StackFrame

int StackFrame::GetLineNumber() const { TODO(); return 0; }
int StackFrame::GetColumn() const { TODO(); return 0; }
Local<String> StackFrame::GetScriptName() const { TODO(); return Local<String>(); }
Local<String> StackFrame::GetScriptNameOrSourceURL() const { TODO(); return Local<String>(); }
Local<String> StackFrame::GetFunctionName() const { TODO(); return Local<String>(); }
bool StackFrame::IsEval() const { TODO(); return false; }
bool StackFrame::IsConstructor() const { TODO(); return false; }

/// v8::StackTrace

Local<StackFrame> StackTrace::GetFrame(uint32_t index) const { TODO(); return Local<StackFrame>(); }
int StackTrace::GetFrameCount() const { TODO(); return 0; }
Local<Array> StackTrace::AsArray() { TODO(); return Local<Array>(); }
/*static*/ Local<StackTrace> StackTrace::CurrentStackTrace(int frame_limit, StackTraceOptions options) { TODO(); return Local<StackTrace>(); }

/// v8::Message

Local<String> Message::Get() const { TODO(); return Local<String>(); }
Local<String> Message::GetSourceLine() const { TODO(); return Local<String>(); }
Handle<Value> Message::GetScriptResourceName() const { TODO(); return Handle<Value>(); }
Handle<Value> Message::GetScriptData() const { TODO(); return Handle<Value>(); }
Handle<StackTrace> Message::GetStackTrace() const { TODO(); return Handle<StackTrace>(); }
int Message::GetLineNumber() const { TODO(); return 0; }
int Message::GetStartPosition() const { TODO(); return 0; }
int Message::GetEndPosition() const { TODO(); return 0; }
int Message::GetStartColumn() const { TODO(); return 0; }
int Message::GetEndColumn() const { TODO(); return 0; }
/*static*/ void Message::PrintCurrentStackTrace(FILE* out) { TODO(); }

/// v8::TryCatch

TryCatch::TryCatch() { /*TODO();*/ }
TryCatch::~TryCatch() { /*TODO();*/ }
bool TryCatch::HasCaught() const { /*TODO();*/ return false; }
bool TryCatch::CanContinue() const { TODO(); return true; }
Handle<Value> TryCatch::ReThrow() { TODO(); return Handle<Value>(); }
Local<Value> TryCatch::Exception() const { TODO(); return Local<Value>(); }
Local<Value> TryCatch::StackTrace() const { TODO(); return Local<Value>(); }
Local<Message> TryCatch::Message() const { TODO(); return Local<v8::Message>(); }
void TryCatch::Reset() { TODO(); }
void TryCatch::SetVerbose(bool value) { TODO(); }
void TryCatch::SetCaptureMessage(bool value) { TODO(); }

/// v8::Signature

Signature::Signature(Handle<FunctionTemplate> receiver, int argc, Handle<FunctionTemplate> argv[])
{
}

Local<Signature> Signature::New(Handle<FunctionTemplate> receiver, int argc, Handle<FunctionTemplate> argv[])
{
	return Local<Signature>(new Signature(receiver, argc, argv));
}

/// v8::AccessorSignature

AccessorSignature::AccessorSignature(Handle<FunctionTemplate> receiver)
{
}

Local<AccessorSignature> AccessorSignature::New(Handle<FunctionTemplate> receiver)
{
	return Local<AccessorSignature>(new AccessorSignature(receiver));
}

/// v8::Template

RTTI_IMPLEMENT(v8::Template, v8::Data);

Template::Template()
{
}

Template::~Template()
{
}

void Template::Set(Handle<String> name, Handle<Data> value, PropertyAttribute attrib)
{
	m_template_property_map[name] = internal::TemplateProperty(value, attrib);
}

void Template::Set(const char* name, Handle<Data> value)
{
	Set(String::NewSymbol(name), value);
}

void Template::ApplyToObject(Handle<Object> object)
{
	// copy each template property to object
	for (internal::TemplatePropertyMap::iterator template_property_it = m_template_property_map.begin(); template_property_it != m_template_property_map.end(); ++template_property_it)
	{
		const Handle<String>& name = template_property_it->first;
		const Handle<Data>& data = template_property_it->second.m_value;
		const PropertyAttribute& attrib = template_property_it->second.m_attrib;

		ObjectTemplate* object_template = RTTI_DynamicCast(ObjectTemplate, *data);
		if (object_template)
		{
			//printf("nested ObjectTemplate %s\n", *String::Utf8Value(name));
			object->Set(name, object_template->NewInstance(), attrib);
			continue;
		}

		FunctionTemplate* function_template = RTTI_DynamicCast(FunctionTemplate, *data);
		if (function_template)
		{
			//printf("nested FunctionTemplate %s\n", *String::Utf8Value(name));
			object->Set(name, function_template->GetFunction(), attrib);
			continue;
		}

		Value* value = RTTI_DynamicCast(Value, *data);
		assert(value);
		object->Set(name, Handle<Value>(value), attrib);
	}
}

/// v8::ObjectTemplate

RTTI_IMPLEMENT(v8::ObjectTemplate, v8::Template);

ObjectTemplate::ObjectTemplate() :
	m_named_property_getter(NULL),
	m_named_property_setter(NULL),
	m_named_property_query(NULL),
	m_named_property_deleter(NULL),
	m_named_property_enumerator(NULL),
	m_indexed_property_getter(NULL),
	m_indexed_property_setter(NULL),
	m_indexed_property_query(NULL),
	m_indexed_property_deleter(NULL),
	m_indexed_property_enumerator(NULL),
	m_call_as_function_invocation_callback(NULL),
	m_access_check_named_security_callback(NULL),
	m_access_check_indexed_security_callback(NULL),
	m_access_check_turned_on_by_default(true),
	m_internal_field_count(0),
	m_undetectable(false)
{
}

ObjectTemplate::~ObjectTemplate()
{
}

bool ObjectTemplate::_ObjectTemplateHasProperty(Handle<Object> object, Handle<String> name)
{
	// check object template accessor map
	internal::TemplateAccessorMap::iterator template_accessor_it = m_template_accessor_map.find(name);
	if (template_accessor_it != m_template_accessor_map.end())
	{
		return true;
	}

	// check object template named_property_query
	if (m_named_property_query)
	{
		AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, m_named_property_data); // TODO: holder?
		Handle<Integer> value = m_named_property_query(Local<String>(name), accessor_info);
		return !value.IsEmpty();
	}

	// check object template indexed_property_query
	if (m_indexed_property_query)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		uint32_t index = internal::DoubleToUint32(JSValueToNumber(js_ctx, name->m_js_value, &js_exception));
		if (js_exception == NULL)
		{
			AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, m_indexed_property_data); // TODO: holder?
			Handle<Integer> value = m_indexed_property_query(index, accessor_info);
			return !value.IsEmpty();
		}
	}

	// check object prototype template
	if (!m_prototype_template.IsEmpty() && m_prototype_template->_ObjectTemplateHasProperty(object, name))
	{
		return true;
	}

	return false;
}

Handle<Value> ObjectTemplate::_ObjectTemplateGetProperty(Handle<Object> object, Handle<String> name)
{
	Handle<Value> value;

	// check object template accessor map
	internal::TemplateAccessorMap::iterator template_accessor_it = m_template_accessor_map.find(name);
	if (template_accessor_it != m_template_accessor_map.end())
	{
		AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, template_accessor_it->second.m_data); // TODO: holder?
		return template_accessor_it->second.m_getter(Local<String>(name), accessor_info);
	}

	// check object template named_property_getter
	if (m_named_property_getter)
	{
		AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, m_named_property_data); // TODO: holder?
		return m_named_property_getter(Local<String>(name), accessor_info);
	}

	// check object template indexed_property_getter
	if (m_indexed_property_getter)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		uint32_t index = internal::DoubleToUint32(JSValueToNumber(js_ctx, name->m_js_value, &js_exception));
		if (js_exception == NULL)
		{
			AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, m_indexed_property_data); // TODO: holder?
			return m_indexed_property_getter(index, accessor_info);
		}
	}

	// check object prototype template
	if (!m_prototype_template.IsEmpty())
	{
		value = m_prototype_template->_ObjectTemplateGetProperty(object, name);
		if (!value.IsEmpty())
		{
			return value;
		}
	}

	return value;
}

bool ObjectTemplate::_ObjectTemplateSetProperty(Handle<Object> object, Handle<String> name, Handle<Value> value)
{
	// check object template accessor map
	internal::TemplateAccessorMap::iterator template_accessor_it = m_template_accessor_map.find(name);
	if (template_accessor_it != m_template_accessor_map.end())
	{
		AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, template_accessor_it->second.m_data); // TODO: holder?
		template_accessor_it->second.m_setter(Local<String>(name), Local<Value>(value), accessor_info);
		return true;
	}

	// check object template named_property_setter
	if (m_named_property_setter)
	{
		AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, m_named_property_data); // TODO: holder?
		if (value == m_named_property_setter(Local<String>(name), Local<Value>(value), accessor_info))
		{
			return true;
		}
	}

	// check object template indexed_property_setter
	if (m_indexed_property_setter)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		uint32_t index = internal::DoubleToUint32(JSValueToNumber(js_ctx, name->m_js_value, &js_exception));
		if (js_exception == NULL)
		{
			AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, m_indexed_property_data); // TODO: holder?
			if (value == m_indexed_property_setter(index, Local<Value>(value), accessor_info))
			{
				return true;
			}
		}
	}

	// check object prototype template
	if (!m_prototype_template.IsEmpty() && m_prototype_template->_ObjectTemplateSetProperty(object, name, value))
	{
		return true;
	}

	return false;
}

bool ObjectTemplate::_ObjectTemplateDeleteProperty(Handle<Object> object, Handle<String> name)
{
//	// check object template accessor map
//	internal::TemplateAccessorMap::iterator template_accessor_it = m_template_accessor_map.find(name);
//	if (template_accessor_it != m_template_accessor_map.end())
//	{
//		m_template_accessor_map.erase(template_accessor_it);
//		return true;
//	}

	// check object template named_property_deleter
	if (m_named_property_deleter)
	{
		AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, m_named_property_data); // TODO: holder?
		Handle<Boolean> value = m_named_property_deleter(Local<String>(name), accessor_info);
		return !value.IsEmpty() && value->Value();
	}

	// check object template indexed_property_deleter
	if (m_indexed_property_deleter)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = Context::GetCurrentJSContext();
		uint32_t index = internal::DoubleToUint32(JSValueToNumber(js_ctx, name->m_js_value, &js_exception));
		if (js_exception == NULL)
		{
			AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, m_indexed_property_data); // TODO: holder?
			Handle<Boolean> value = m_indexed_property_deleter(index, accessor_info);
			return !value.IsEmpty() && value->Value();
		}
	}

	// check object prototype template
	if (!m_prototype_template.IsEmpty() && m_prototype_template->_ObjectTemplateDeleteProperty(object, name))
	{
		return true;
	}

	return false;
}

void ObjectTemplate::_ObjectTemplateGetPropertyNames(Handle<Object> object, JSPropertyNameAccumulatorRef js_name_accumulator)
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();

	// check object template accessor map
	for (internal::TemplateAccessorMap::iterator template_accessor_it = m_template_accessor_map.begin(); template_accessor_it != m_template_accessor_map.end(); ++template_accessor_it)
	{
		if (!(template_accessor_it->second.m_attrib & DontEnum))
		{
			JSStringRef js_name = JSValueToStringCopy(js_ctx, template_accessor_it->first->m_js_value, NULL);
			JSPropertyNameAccumulatorAddName(js_name_accumulator, js_name);
			JSStringRelease(js_name);
		}
	}

	// check object template named_property_enumerator
	if (m_named_property_enumerator)
	{
		AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, m_named_property_data); // TODO: holder?
		Handle<Array> names = m_named_property_enumerator(accessor_info);
		if (!names.IsEmpty())
		{
			uint32_t length = names->Length();
			for (uint32_t index = 0; index < length; ++index)
			{
				Handle<Value> name = names->Get(index);
				JSStringRef js_name = JSValueToStringCopy(js_ctx, name->m_js_value, NULL);
				JSPropertyNameAccumulatorAddName(js_name_accumulator, js_name);
				JSStringRelease(js_name);
			}
		}
	}

	// check object template indexed_property_enumerator
	if (m_indexed_property_enumerator)
	{
		AccessorInfo accessor_info(Isolate::GetCurrent(), object, object, m_indexed_property_data); // TODO: holder?
		Handle<Array> names = m_indexed_property_enumerator(accessor_info);
		if (!names.IsEmpty())
		{
			uint32_t length = names->Length();
			for (uint32_t index = 0; index < length; ++index)
			{
				Handle<Value> name = names->Get(index);
				JSStringRef js_name = JSValueToStringCopy(js_ctx, name->m_js_value, NULL);
				JSPropertyNameAccumulatorAddName(js_name_accumulator, js_name);
				JSStringRelease(js_name);
			}
		}
	}

	// check object prototype template
	if (!m_prototype_template.IsEmpty())
	{
		m_prototype_template->_ObjectTemplateGetPropertyNames(object, js_name_accumulator);
	}
}

Local<Object> ObjectTemplate::NewInstance()
{
	Handle<Object> object = Object::New();
	//printf("ObjectTemplate::NewInstance object %p\n", object->m_js_object);
	ObjectTemplate::ApplyToObject(object);
	internal::Helper* helper = object->GetHelper();
	helper->m_object_template = Handle<ObjectTemplate>(this);
	helper->SetInternalFieldCount(m_internal_field_count);
	return Local<Object>(object);
}

void ObjectTemplate::SetAccessor(Handle<String> name, AccessorGetter getter, AccessorSetter setter, Handle<Value> data, AccessControl settings, PropertyAttribute attrib, Handle<AccessorSignature> signature)
{
	m_template_accessor_map[name] = internal::TemplateAccessor(getter, setter, data, settings, attrib, signature);
}

void ObjectTemplate::SetNamedPropertyHandler(NamedPropertyGetter getter, NamedPropertySetter setter, NamedPropertyQuery query, NamedPropertyDeleter deleter, NamedPropertyEnumerator enumerator, Handle<Value> data)
{
	m_named_property_getter = getter;
	m_named_property_setter = setter;
	m_named_property_query = query;
	m_named_property_deleter = deleter;
	m_named_property_enumerator = enumerator;
	m_named_property_data = data;
}

void ObjectTemplate::SetIndexedPropertyHandler(IndexedPropertyGetter getter, IndexedPropertySetter setter, IndexedPropertyQuery query, IndexedPropertyDeleter deleter, IndexedPropertyEnumerator enumerator, Handle<Value> data)
{
	m_indexed_property_getter = getter;
	m_indexed_property_setter = setter;
	m_indexed_property_query = query;
	m_indexed_property_deleter = deleter;
	m_indexed_property_enumerator = enumerator;
	m_indexed_property_data = data;
}

void ObjectTemplate::SetCallAsFunctionHandler(InvocationCallback callback, Handle<Value> data)
{
	m_call_as_function_invocation_callback = callback;
	m_call_as_function_data = data;
}

void ObjectTemplate::MarkAsUndetectable()
{
	m_undetectable = true;
}

void ObjectTemplate::SetAccessCheckCallbacks(NamedSecurityCallback named_handler, IndexedSecurityCallback indexed_handler, Handle<Value> data, bool turned_on_by_default)
{
	m_access_check_named_security_callback = named_handler;
	m_access_check_indexed_security_callback = indexed_handler;
	m_access_check_data = data;
	m_access_check_turned_on_by_default = turned_on_by_default;
}

int ObjectTemplate::InternalFieldCount()
{
	return m_internal_field_count;
}

void ObjectTemplate::SetInternalFieldCount(int value)
{
	m_internal_field_count = value;
}

void ObjectTemplate::SetPrototypeTemplate(Handle<ObjectTemplate> prototype_template)
{
	m_prototype_template = prototype_template;
}

void ObjectTemplate::ApplyToObject(Handle<Object> object)
{
	if (!m_prototype_template.IsEmpty())
	{
		// recurse up the prototype chain
		m_prototype_template->ApplyToObject(object);
	}
	Template::ApplyToObject(object);
}

/*static*/ Local<ObjectTemplate> ObjectTemplate::New()
{
	return Local<ObjectTemplate>(new ObjectTemplate());
}

/// v8::FunctionTemplate

RTTI_IMPLEMENT(v8::FunctionTemplate, v8::Template);

FunctionTemplate::FunctionTemplate(InvocationCallback callback, Handle<Value> data, Handle<Signature> signature) :
	m_callback(callback),
	m_data(data),
	m_signature(signature)
{
}

FunctionTemplate::~FunctionTemplate()
{
}

bool FunctionTemplate::_FunctionTemplateHasProperty(Handle<Function> function, Handle<String> name)
{
	// check instance object template, also checks prototype object template
	if (!m_instance_template.IsEmpty() && m_instance_template->_ObjectTemplateHasProperty(function, name))
	{
		return true;
	}

	return false;
}

Handle<Value> FunctionTemplate::_FunctionTemplateGetProperty(Handle<Function> function, Handle<String> name)
{
	Handle<Value> value;

	// check instance object template, also checks prototype object template
	if (!m_instance_template.IsEmpty())
	{
		value = m_instance_template->_ObjectTemplateGetProperty(function, name);
		if (!value.IsEmpty())
		{
			return value;
		}
	}

	return value;
}

bool FunctionTemplate::_FunctionTemplateSetProperty(Handle<Function> function, Handle<String> name, Handle<Value> value)
{
	// check instance object template, also checks prototype object template
	if (!m_instance_template.IsEmpty() && m_instance_template->_ObjectTemplateSetProperty(function, name, value))
	{
		return true;
	}

	return false;
}

bool FunctionTemplate::_FunctionTemplateDeleteProperty(Handle<Function> function, Handle<String> name)
{
	// check instance object template, also checks prototype object template
	if (!m_instance_template.IsEmpty() && m_instance_template->_ObjectTemplateDeleteProperty(function, name))
	{
		return true;
	}

	return false;
}

void FunctionTemplate::_FunctionTemplateGetPropertyNames(Handle<Function> function, JSPropertyNameAccumulatorRef js_name_accumulator)
{
	// check instance object template, also checks prototype object template
	if (!m_instance_template.IsEmpty())
	{
		m_instance_template->_ObjectTemplateGetPropertyNames(function, js_name_accumulator);
	}
}

Local<Function> FunctionTemplate::GetFunction()
{
	if (m_function.IsEmpty())
	{
		m_function = new Function(Context::GetCurrentJSContext());
		Template::ApplyToObject(m_function);
		m_function->m_function_template = Handle<FunctionTemplate>(this);
		m_function->SetName(m_class_name);
	}
	return Local<Function>(m_function);
}

void FunctionTemplate::Inherit(Handle<FunctionTemplate> parent)
{
	InstanceTemplate()->m_prototype_template = parent->InstanceTemplate()->m_prototype_template;
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate()
{
	if (m_instance_template.IsEmpty())
	{
		m_instance_template = ObjectTemplate::New();
		m_instance_template->m_prototype_template = ObjectTemplate::New();
	}
	return Local<ObjectTemplate>(m_instance_template);
}

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate()
{
	return Local<ObjectTemplate>(InstanceTemplate()->m_prototype_template);
}

void FunctionTemplate::SetCallHandler(InvocationCallback callback, Handle<Value> data)
{
	m_callback = callback;
	m_data = data;
}

bool FunctionTemplate::HasInstance(Handle<Value> value)
{
	if (value->IsObject())
	{
		Handle<Object> object(value.As<Object>());
		internal::Helper* helper = object->GetHelper();
		Handle<Function> function(GetFunction());
		if (!helper->m_function.IsEmpty() && !function.IsEmpty())
		{
			return helper->m_function == function;
		}
	}
	return false;
}

/*static*/ Local<FunctionTemplate> FunctionTemplate::New(InvocationCallback callback, Handle<Value> data, Handle<Signature> signature)
{
	return Local<FunctionTemplate>(new FunctionTemplate(callback, data, signature));
}

/// v8::Script

Script::Script(Handle<String> source, Handle<Value> file_name, Handle<Integer> line_number, Handle<String> script_data) :
	m_source(source), m_file_name(file_name), m_line_number(line_number), m_script_data(script_data)
{
}

bool Script::CheckSyntax()
{
	//printf("%s\n>>>\n%s\n<<<\n", __PRETTY_FUNCTION__, *String::Utf8Value(m_source));

	Handle<Context> context = (!m_context.IsEmpty())?(m_context):(Context::GetCurrent());

	Context::Scope context_scope(context);

	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = context->m_js_global_ctx;
	JSStringRef js_script = JSValueToStringCopy(js_ctx, m_source->m_js_value, NULL);
	JSStringRef js_file = NULL; int js_line = 0;

	if (!m_file_name.IsEmpty())
	{
		js_file = JSValueToStringCopy(js_ctx, m_file_name->m_js_value, NULL);
	}

	if (!m_line_number.IsEmpty())
	{
		js_line = (int) internal::DoubleToInt32(JSValueToNumber(js_ctx, m_line_number->m_js_value, NULL));
	}

	bool check = JSCheckScriptSyntax(js_ctx, js_script, js_file, js_line, &js_exception);

	if (js_file)
	{
		JSStringRelease(js_file); js_file = NULL;
	}

	if (js_script)
	{
		JSStringRelease(js_script); js_script = NULL;
	}

	if (js_exception)
	{
		ThrowException(internal::ImportValue(js_ctx, js_exception));
	}

	if (!check)
	{
		printf("%s\n>>>\n%s\n<<<\n", __PRETTY_FUNCTION__, *String::Utf8Value(m_source));
		printf("note: check script syntax fail\n");
	}

	return check;
}

Local<Value> Script::Run()
{
	//printf("%s\n>>>\n%s\n<<<\n", __PRETTY_FUNCTION__, *String::Utf8Value(m_source));

	Handle<Context> context = (!m_context.IsEmpty())?(m_context):(Context::GetCurrent());

	Context::Scope context_scope(context);

	Handle<Object> global_object = context->Global();

	JSValueRef js_exception = NULL;
	JSContextRef js_ctx = context->m_js_global_ctx;
	JSValueRef js_source = m_source->m_js_value;

	#if 1 // V8LIKE_NODE_BUFFER_WRAP_CTOR
	internal::JSStringWrap js_name_arg0("source");
	const JSStringRef js_name_argv[] = { js_name_arg0 };
	unsigned js_name_argc = sizeof(js_name_argv) / sizeof(*js_name_argv);
	const char* body = "return source.replace(/new\\s+Buffer/g, 'new global.Buffer');";
	JSObjectRef js_patch = JSObjectMakeFunction(js_ctx, NULL, js_name_argc, js_name_argv, internal::JSStringWrap(body), NULL, 0, NULL);
	js_source = JSObjectCallAsFunction(js_ctx, js_patch, NULL, 1, &js_source, NULL);
	#endif

	JSStringRef js_script = JSValueToStringCopy(js_ctx, js_source, NULL);
	JSObjectRef js_that = internal::ExportObject(global_object);
	JSStringRef js_file = NULL; int js_line = 0;

	if (!m_file_name.IsEmpty())
	{
		js_file = JSValueToStringCopy(js_ctx, m_file_name->m_js_value, NULL);
	}

	if (!m_line_number.IsEmpty())
	{
		js_line = (int) internal::DoubleToInt32(JSValueToNumber(js_ctx, m_line_number->m_js_value, NULL));
	}

	JSValueRef js_value = JSEvaluateScript(js_ctx, js_script, js_that, js_file, js_line, &js_exception);

	if (js_file)
	{
		JSStringRelease(js_file); js_file = NULL;
	}

	if (js_script)
	{
		JSStringRelease(js_script); js_script = NULL;
	}

	if (js_exception)
	{
		printf("%s\n>>>\n%s\n<<<\n", __PRETTY_FUNCTION__, *String::Utf8Value(m_source));
		ThrowException(internal::ImportValue(js_ctx, js_exception));
	}

	return Local<Value>(internal::ImportValue(js_ctx, js_value));
}

/*static*/ Local<Script> Script::New(Handle<String> source, ScriptOrigin* origin, ScriptData* pre_data, Handle<String> script_data)
{
	Handle<Script> script(new Script(source, origin->m_resource_name, origin->m_resource_line_offset, script_data));
	script->CheckSyntax();
	return Local<Script>(script);
}

/*static*/ Local<Script> Script::New(Handle<String> source, Handle<Value> file_name)
{
	ScriptOrigin origin(file_name);
	return Script::New(source, &origin);
}

/*static*/ Local<Script> Script::Compile(Handle<String> source, ScriptOrigin* origin, ScriptData* pre_data, Handle<String> script_data)
{
	Handle<Script> script(new Script(source, origin->m_resource_name, origin->m_resource_line_offset, script_data));
	script->m_context = Context::GetCurrent(); // bind to current context
	script->CheckSyntax();
	return Local<Script>(script);
}

/*static*/ Local<Script> Script::Compile(Handle<String> source, Handle<Value> file_name, Handle<String> script_data)
{
	ScriptOrigin origin(file_name);
	return Script::Compile(source, &origin, NULL, script_data);
}

/// v8::Context

Context::Context()
{
	if (sm_initial_js_group != NULL)
	{
		// take ownership of the initial global group
		m_js_group = sm_initial_js_group;
		sm_initial_js_group = NULL;
	}
	else
	{
		// create the global group (maybe move this to v8::Isolate)
		m_js_group = JSContextGroupCreate();
		JSContextGroupRetain(m_js_group);
	}

	if (sm_initial_js_global_ctx != NULL)
	{
		// take ownership of the initial global context
		m_js_global_ctx = sm_initial_js_global_ctx;
		sm_initial_js_global_ctx = NULL;
	}
	else
	{
		// create the global context using the object class
		m_js_global_ctx = JSGlobalContextCreateInGroup(m_js_group, Context::GetJSClass());
		JSGlobalContextRetain(m_js_global_ctx);
	}

	// push global context into the JSContextRef stack
	m_js_ctx_stack.push_back(m_js_global_ctx);

	// initialize global object
	JSObjectRef js_global_object = Context::GetJSGlobalObject(m_js_global_ctx);
	m_global_object = Handle<Object>(new Object(m_js_global_ctx, js_global_object));
	assert(JSValueIsObjectOfClass(m_js_global_ctx, js_global_object, Context::GetJSClass()));
	//printf("global %p\n", js_global_object);
}

Context::~Context()
{
	// TODO
}

Local<Object> Context::Global()
{
	if (!m_external_global_object.IsEmpty())
	{
		return Local<Object>(m_external_global_object);
	}

	return Local<Object>(m_global_object);
}

void Context::Enter()
{
	m_previous_context = Persistent<Context>(Context::GetCurrent());
	sm_calling_context = Persistent<Context>(Context::GetCurrent());
	sm_entered_context = Persistent<Context>(this);
	sm_current_context = Persistent<Context>(this);
}

void Context::Exit()
{
	sm_entered_context._Clear();
	sm_current_context = Persistent<Context>(m_previous_context);
	sm_calling_context._Clear();
	m_previous_context._Clear();
}

/*static*/ JSClassRef Context::sm_js_class = NULL;
/*static*/ JSContextGroupRef Context::sm_initial_js_group = NULL;
/*static*/ JSGlobalContextRef Context::sm_initial_js_global_ctx = NULL;
/*static*/ std::map<JSContextRef, JSObjectRef> Context::sm_js_global_object_map;
/*static*/ Persistent<Context> Context::sm_initial_context;
/*static*/ Persistent<Context> Context::sm_entered_context;
/*static*/ Persistent<Context> Context::sm_current_context;
/*static*/ Persistent<Context> Context::sm_calling_context;

/*static*/ Local<Context> Context::GetEntered()
{
	assert(0);
	return Local<Context>(sm_entered_context);
}

/*static*/ Local<Context> Context::GetCurrent()
{
	if (!sm_current_context.IsEmpty())
	{
		return Local<Context>(sm_current_context);
	}

	return Local<Context>(sm_initial_context);
}

/*static*/ Local<Context> Context::GetCalling()
{
	assert(0);
	return Local<Context>(sm_calling_context);
}

/*static*/ JSClassRef Context::GetJSClass()
{
	if (!sm_js_class)
	{
		JSClassDefinition js_def = kJSClassDefinitionEmpty;
		js_def.attributes = kJSClassAttributeNoAutomaticPrototype;
		js_def.className = "Context";
		js_def.parentClass = Object::GetJSClass();
		//js_def.staticValues = NULL;
		//js_def.staticFunctions = NULL;
		js_def.initialize = Context::_JS_Initialize;
		js_def.finalize = Context::_JS_Finalize;
		js_def.hasProperty = Context::_JS_HasProperty;
		js_def.getProperty = Context::_JS_GetProperty;
		js_def.setProperty = Context::_JS_SetProperty;
		js_def.deleteProperty = Context::_JS_DeleteProperty;
		js_def.getPropertyNames = Context::_JS_GetPropertyNames;
		//js_def.callAsFunction = Context::_JS_CallAsFunction;
		//js_def.callAsConstructor = Context::_JS_CallAsConstructor;
		//js_def.hasInstance = Context::_JS_HasInstance;
		//js_def.convertToType = Context::_JS_ConvertToType;
		sm_js_class = JSClassCreate(&js_def);
	}
	return sm_js_class;
}

/*static*/ JSGlobalContextRef Context::GetCurrentJSGlobalContext()
{
	if (!sm_current_context.IsEmpty())
	{
		return sm_current_context->m_js_global_ctx;
	}
	else if (!sm_initial_context.IsEmpty())
	{
		return sm_initial_context->m_js_global_ctx;
	}
	else
	{
		if (sm_initial_js_group == NULL)
		{
			sm_initial_js_group = JSContextGroupCreate();
			JSContextGroupRetain(sm_initial_js_group);
		}

		if (sm_initial_js_global_ctx == NULL)
		{
			sm_initial_js_global_ctx = JSGlobalContextCreateInGroup(sm_initial_js_group, Context::GetJSClass());
			JSGlobalContextRetain(sm_initial_js_global_ctx);
		}

		return sm_initial_js_global_ctx;
	}
}

/*static*/ JSContextRef Context::GetCurrentJSContext()
{
	if (!sm_current_context.IsEmpty())
	{
		return sm_current_context->m_js_ctx_stack.back();
	}
	else if (!sm_initial_context.IsEmpty())
	{
		return sm_initial_context->m_js_ctx_stack.back();
	}
	else
	{
		if (sm_initial_js_group == NULL)
		{
			sm_initial_js_group = JSContextGroupCreate();
			JSContextGroupRetain(sm_initial_js_group);
		}

		if (sm_initial_js_global_ctx == NULL)
		{
			sm_initial_js_global_ctx = JSGlobalContextCreateInGroup(sm_initial_js_group, Context::GetJSClass());
			JSGlobalContextRetain(sm_initial_js_global_ctx);
		}

		return sm_initial_js_global_ctx;
	}
}

/*static*/ JSObjectRef Context::GetJSGlobalObject(JSContextRef js_ctx, JSObjectRef js_object /*= NULL*/)
{
	JSObjectRef js_global_object = Context::sm_js_global_object_map[js_ctx];
	if (js_global_object == NULL)
	{
		js_global_object = js_object;
	}
	if (js_global_object == NULL)
	{
		js_global_object = JSContextGetGlobalObject(js_ctx);
	}
	return js_global_object;
}

static bool sg_expose_gc = true;

static JSValueRef GC(JSContextRef js_ctx, JSObjectRef js_function, JSObjectRef js_this, size_t js_argc, const JSValueRef js_argv[], JSValueRef* js_exception)
{
	if ((js_argc > 0) && (JSValueIsNumber(js_ctx, js_argv[0])) && (JSValueToNumber(js_ctx, js_argv[0], NULL) > 0.0))
	{
		JSGarbageCollect(Context::GetCurrentJSGlobalContext()); // async
	}
	else
	{
		JSSynchronousGarbageCollectForDebugging(Context::GetCurrentJSGlobalContext());
	}
	return NULL;
}

/*static*/ Persistent<Context> Context::New(ExtensionConfiguration* extensions, Handle<ObjectTemplate> global_template, Handle<Value> global_value)
{
	Persistent<Context> context = Persistent<Context>(new Context());

	if (sm_initial_context.IsEmpty())
	{
		sm_initial_context = context;
	}

	Handle<Object> global_object;

	if (!global_value.IsEmpty() && global_value->IsObject())
	{
		// use the external global object
		global_object = context->m_external_global_object = global_value->ToObject();
	}
	else
	{
		// use the internal global object
		global_object = context->m_global_object;
	}

	if (!global_template.IsEmpty() && !global_object.IsEmpty())
	{
		// apply global object template to global object
		global_template->ApplyToObject(global_object);
		internal::Helper* helper = global_object->GetHelper();
		helper->m_object_template = global_template;
		helper->SetInternalFieldCount(global_template->m_internal_field_count);
	}

	// global.gc
	if (sg_expose_gc)
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = context->m_js_global_ctx;
		JSObjectRef js_global_object = global_object->m_js_object;
		JSObjectRef js_function = JSObjectMakeFunctionWithCallback(js_ctx, internal::JSStringWrap("gc"), GC);
		JSPropertyAttributes js_attrib = kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete;
		JSObjectSetProperty(js_ctx, js_global_object, internal::JSStringWrap("gc"), js_function, js_attrib, &js_exception);
	}

	// Error.captureStackTrace
	{
		JSValueRef js_exception = NULL;
		JSContextRef js_ctx = context->m_js_global_ctx;
		JSObjectRef js_global_object = global_object->m_js_object;
		JSObjectRef js_constructor = JSValueToObject(js_ctx, JSObjectGetProperty(js_ctx, js_global_object, internal::JSStringWrap("Error"), &js_exception), &js_exception);
		JSValueRef js_function = JSObjectGetProperty(js_ctx, js_constructor, internal::JSStringWrap("captureStackTrace"), &js_exception);
		if (js_function == NULL)
		{
			js_function = JSObjectMakeFunction(js_ctx, internal::JSStringWrap("captureStackTrace"), 0, NULL, internal::JSStringWrap(""), NULL, 0, &js_exception);
			JSPropertyAttributes js_attrib = kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete;
			JSObjectSetProperty(js_ctx, js_constructor, internal::JSStringWrap("captureStackTrace"), js_function, js_attrib, &js_exception);
		}
	}

	return Persistent<Context>(context);
}

#if 1 // V8LIKE_NODE_BUFFER_WRAP_CTOR

static JSValueRef NativeObjectCreate(JSContextRef js_ctx, JSObjectRef js_function, JSObjectRef js_this, size_t js_argc, const JSValueRef js_argv[], JSValueRef* js_exception)
{
	Handle<Object> object = Object::New();
	JSObjectRef js_object = internal::ExportObject(object);
	if ((js_argc > 0) && (JSValueIsObject(js_ctx, js_argv[0]) || JSValueIsNull(js_ctx, js_argv[0])))
	{
		JSObjectSetPrototype(js_ctx, js_object, js_argv[0]);
	}
	return js_object;
}

static JSValueRef NativeWrapConstructor(JSContextRef js_ctx, JSValueRef js_ctor)
{
	JSValueRef js_exception = NULL;
	internal::JSStringWrap js_name_arg0("ctor");
	internal::JSStringWrap js_name_arg1("native_object_create");
	const JSStringRef js_name_argv[] = { js_name_arg0, js_name_arg1 };
	unsigned js_name_argc = sizeof(js_name_argv) / sizeof(*js_name_argv);
	const char* body =
		//	function NativeWrapConstructor(ctor, native_object_create)
		//	{
		//		// so wrap_ctor.name == ctor.name
		"		var name_function = function (name, fn)\n"
		"		{\n"
		"			return (new Function('return function (call) { return function ' + name + ' () { return call(this, arguments) }; };')())(Function.apply.bind(fn));\n"
		"		};\n"
		"\n"
		"		var wrap_ctor = name_function(ctor.name, function ()\n"
		"		{\n"
		"			var args = Array.prototype.slice.call(arguments, 0);\n"
		"\n"
		//			// in case constructor called without new keyword
		"			if (!(this instanceof wrap_ctor))\n"
		"			{\n"
		"				function F() { return wrap_ctor.apply(this, args); }\n"
		"				F.prototype = Object.create(wrap_ctor.prototype);\n"
		"				return new F();\n"
		"			}\n"
		"\n"
		//			// wrap the script object in a native object
		"			var that = native_object_create(this);\n"
		"\n"
		//			// call ctor class constructor
		"			ctor.prototype.constructor.apply(that, args);\n"
		"\n"
		"			return that;\n"
		"		});\n"
		"\n"
		//		// inherit from ctor class
		"		wrap_ctor.prototype = Object.create(ctor.prototype);\n"
		"		wrap_ctor.prototype.constructor = wrap_ctor;\n"
		"\n"
		//		// copy ctor class statics
		"		for (var i in ctor)\n"
		"		{\n"
		"			wrap_ctor[i] = ctor[i];\n"
		"		}\n"
		"\n"
		"		return wrap_ctor;\n"
		//	}
		"";
	internal::JSStringWrap js_body(body);
	JSObjectRef js_native_wrap_constructor = JSObjectMakeFunction(js_ctx, internal::JSStringWrap("NativeWrapConstructor"), js_name_argc, js_name_argv, js_body, NULL, 0, &js_exception);
	JSObjectRef js_native_object_create = JSObjectMakeFunctionWithCallback(js_ctx, internal::JSStringWrap("NativeObjectCreate"), NativeObjectCreate);
	const JSValueRef js_argv[] = { js_ctor, js_native_object_create };
	size_t js_argc = sizeof(js_argv) / sizeof(*js_argv);
	return JSObjectCallAsFunction(js_ctx, js_native_wrap_constructor, NULL, js_argc, js_argv, &js_exception);
}

static JSValueRef sg_node_buffer_constructor = NULL;

#endif

/*static*/ void Context::_JS_Initialize(JSContextRef js_ctx, JSObjectRef js_object)
{
	assert(js_ctx != NULL);
	assert(js_object != NULL);
	assert(sm_js_global_object_map[js_ctx] == NULL);
	sm_js_global_object_map[js_ctx] = js_object;
}

/*static*/ void Context::_JS_Finalize(JSObjectRef js_object)
{
	JSContextRef js_ctx = NULL;

	for (std::map<JSContextRef, JSObjectRef>::iterator it = sm_js_global_object_map.begin(); it != sm_js_global_object_map.end(); ++it)
	{
		if (it->second == js_object)
		{
			assert(js_ctx == NULL);
			js_ctx = it->first;
		}
	}

	assert(js_ctx != NULL);
	assert(js_object != NULL);
	assert(sm_js_global_object_map[js_ctx] != NULL);
	sm_js_global_object_map.erase(js_ctx);
}

/**
 * If this function returns false, the hasProperty request
 * forwards to object's statically declared properties, then its
 * parent class chain (which includes the default object class),
 * then its prototype chain.
 */
/*static*/ bool Context::_JS_HasProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name)
{
	assert(JSValueIsStrictEqual(js_ctx, Context::GetJSGlobalObject(js_ctx, js_object), js_object));

	#if 1 // V8LIKE_NODE_BUFFER_WRAP_CTOR
	if (JSStringIsEqualToUTF8CString(js_name, "Buffer"))
	{
		return (sg_node_buffer_constructor != NULL);
	}
	#endif

	return false;
}

/**
 * If this function returns NULL, the get request forwards to
 * object's statically declared properties, then its parent
 * class chain (which includes the default object class), then
 * its prototype chain.
 */
/*static*/ JSValueRef Context::_JS_GetProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef* js_exception)
{
	assert(JSValueIsStrictEqual(js_ctx, Context::GetJSGlobalObject(js_ctx, js_object), js_object));

	#if 1 // V8LIKE_NODE_BUFFER_WRAP_CTOR
	if (JSStringIsEqualToUTF8CString(js_name, "Buffer"))
	{
		if (sg_node_buffer_constructor != NULL)
		{
			return sg_node_buffer_constructor;
		}
	}
	#endif

	return NULL;
}

/**
 * If this function returns false, the set request forwards to
 * object's statically declared properties, then its parent
 * class chain (which includes the default object class).
 */
/*static*/ bool Context::_JS_SetProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef js_value, JSValueRef* js_exception)
{
	assert(JSValueIsStrictEqual(js_ctx, Context::GetJSGlobalObject(js_ctx, js_object), js_object));

	#if 1
	// for tracing JS before console.log is ready
	// __trace__ = "message";
	if (JSStringIsEqualToUTF8CString(js_name, "__trace__"))
	{
		Handle<Value> value = internal::ImportValue(js_ctx, js_value);
		printf("trace %p: %s\n", js_value, *String::Utf8Value(value->ToString()));
		return true;
	}
	#endif

	#if 1 // V8LIKE_NODE_BUFFER_WRAP_CTOR
	if (JSStringIsEqualToUTF8CString(js_name, "Buffer"))
	{
		if (sg_node_buffer_constructor != js_value)
		{
			if (JSValueIsObject(js_ctx, js_value))
			{
				JSObjectRef js_object = JSValueToObject(js_ctx, js_value, NULL);
				assert(js_object == js_value);
				if (JSObjectIsConstructor(js_ctx, js_object))
				{
					sg_node_buffer_constructor = NativeWrapConstructor(js_ctx, js_value);
				}
			}
		}
		return true;
	}
	#endif

	return false;
}

bool Context::_JS_DeleteProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef* js_exception)
{
	assert(JSValueIsStrictEqual(js_ctx, Context::GetJSGlobalObject(js_ctx, js_object), js_object));

	#if 1 // V8LIKE_NODE_BUFFER_WRAP_CTOR
	if (JSStringIsEqualToUTF8CString(js_name, "Buffer"))
	{
		if (sg_node_buffer_constructor != NULL)
		{
			sg_node_buffer_constructor = NULL;
		}
		return true;
	}
	#endif

	return false;
}

void Context::_JS_GetPropertyNames(JSContextRef js_ctx, JSObjectRef js_object, JSPropertyNameAccumulatorRef js_name_accumulator)
{
	assert(JSValueIsStrictEqual(js_ctx, Context::GetJSGlobalObject(js_ctx, js_object), js_object));

	#if 1 // V8LIKE_NODE_BUFFER_WRAP_CTOR
	if (sg_node_buffer_constructor != NULL)
	{
		JSStringRef js_name = JSStringCreateWithUTF8CString("Buffer");
		JSPropertyNameAccumulatorAddName(js_name_accumulator, js_name);
		JSStringRelease(js_name);
	}
	#endif
}

/// v8::Unlocker

/// v8::Locker

/*static*/ void Locker::StartPreemption(int every_n_ms)
{
	TODO();
}

/*static*/ void Locker::StopPreemption()
{
	TODO();
}

/*static*/ bool Locker::IsLocked(Isolate* isolate)
{
	TODO(); return false;
}

/*static*/ bool Locker::IsActive()
{
	TODO(); return false;
}

/// v8

/*static*/ Handle<Primitive> Undefined()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return Handle<Primitive>(new Primitive(js_ctx, JSValueMakeUndefined(js_ctx)));
}

/*static*/ Handle<Primitive> Null()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return Handle<Primitive>(new Primitive(js_ctx, JSValueMakeNull(js_ctx)));
}

/*static*/ Handle<Boolean> True()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return Handle<Boolean>(new Boolean(js_ctx, JSValueMakeBoolean(js_ctx, true)));
}

/*static*/ Handle<Boolean> False()
{
	JSContextRef js_ctx = Context::GetCurrentJSContext();
	return Handle<Boolean>(new Boolean(js_ctx, JSValueMakeBoolean(js_ctx, false)));
}

/// v8::ResourceConstraints

ResourceConstraints::ResourceConstraints() :
	max_young_space_size_(0), max_old_space_size_(0), max_executable_size_(0), stack_limit_(NULL)
{}

/// v8

bool SetResourceConstraints(ResourceConstraints* constraints)
{
	TODO(); return false;
}

Handle<Value> ThrowException(Handle<Value> exception)
{
	#if 1 // V8LIKE_DEBUG
	if (strstr(*String::Utf8Value(exception->ToString()), "ENOENT") == NULL)
	{
		printf("exception: %s\n", *String::Utf8Value(exception->ToString()));
		if (exception->IsObject())
		{
			Handle<Value> stack = exception.As<Object>()->Get(String::NewSymbol("stack"));
			printf("%s\n", *String::Utf8Value(stack->ToString()));
		}
	}
	#endif
	sg_last_exception = exception;
	return exception;
}

/// v8::Exception

/*static*/ Local<Value> Exception::RangeError(Handle<String> message)
{
	return Local<Value>(message);
}

/*static*/ Local<Value> Exception::ReferenceError(Handle<String> message)
{
	return Local<Value>(message);
}

/*static*/ Local<Value> Exception::SyntaxError(Handle<String> message)
{
	return Local<Value>(message);
}

/*static*/ Local<Value> Exception::TypeError(Handle<String> message)
{
	return Local<Value>(message);
}

/*static*/ Local<Value> Exception::Error(Handle<String> message)
{
	return Local<Value>(message);
}

/// v8::HeapStatistics

HeapStatistics::HeapStatistics() :
	total_heap_size_(0), total_heap_size_executable_(0), used_heap_size_(0), heap_size_limit_()
{
}

/// v8::V8

/*static*/ void V8::SetFatalErrorHandler(FatalErrorCallback that) { /*TODO();*/ }
/*static*/ void V8::SetAllowCodeGenerationFromStringsCallback(AllowCodeGenerationFromStringsCallback that) { TODO(); }
/*static*/ void V8::IgnoreOutOfMemoryException() { TODO(); }
/*static*/ bool V8::IsDead() { TODO(); return false; }
/*static*/ StartupData::CompressionAlgorithm V8::GetCompressedStartupDataAlgorithm() { TODO(); return StartupData::kUncompressed; }
/*static*/ int V8::GetCompressedStartupDataCount() { TODO(); return 0; }
/*static*/ void V8::GetCompressedStartupData(StartupData* compressed_data) { TODO(); }
/*static*/ void V8::SetDecompressedStartupData(StartupData* decompressed_data) { TODO(); }
/*static*/ bool V8::AddMessageListener(MessageCallback that, Handle<Value> data) { TODO(); return false; }
/*static*/ void V8::RemoveMessageListeners(MessageCallback that) { TODO(); }
/*static*/ void V8::SetCaptureStackTraceForUncaughtExceptions(bool capture, int frame_limit, StackTrace::StackTraceOptions options) { TODO(); }
/*static*/ void V8::SetFlagsFromString(const char* str, int length) { /*TODO(); printf("%d:%s\n", length, str);*/ }
/*static*/ void V8::SetFlagsFromCommandLine(int* argc, char** argv, bool remove_flags) { /*TODO(); printf("argc: %d\n", *argc); for (int i = 0; i < *argc; ++i) { printf("argv[%i]: %s\n", i, argv[i]); }*/ }
/*static*/ const char* V8::GetVersion() { return "3.14.5.9"; }
/*static*/ void V8::SetCounterFunction(CounterLookupCallback) { TODO(); }
/*static*/ void V8::SetCreateHistogramFunction(CreateHistogramCallback) { TODO(); }
/*static*/ void V8::SetAddHistogramSampleFunction(AddHistogramSampleCallback) { TODO(); }
/*static*/ void V8::EnableSlidingStateWindow() { TODO(); }
/*static*/ void V8::SetFailedAccessCheckCallbackFunction(FailedAccessCheckCallback) { TODO(); }
/*static*/ void V8::AddGCPrologueCallback(GCPrologueCallback callback, GCType gc_type_filter) { TODO(); }
/*static*/ void V8::RemoveGCPrologueCallback(GCPrologueCallback callback) { TODO(); }
/*static*/ void V8::SetGlobalGCPrologueCallback(GCCallback) { TODO(); }
/*static*/ void V8::AddGCEpilogueCallback(GCEpilogueCallback callback, GCType gc_type_filter) { TODO(); }
/*static*/ void V8::RemoveGCEpilogueCallback(GCEpilogueCallback callback) { TODO(); }
/*static*/ void V8::SetGlobalGCEpilogueCallback(GCCallback) { TODO(); }
/*static*/ void V8::AddMemoryAllocationCallback(MemoryAllocationCallback callback, ObjectSpace space, AllocationAction action) { TODO(); }
/*static*/ void V8::RemoveMemoryAllocationCallback(MemoryAllocationCallback callback) { TODO(); }
/*static*/ void V8::AddCallCompletedCallback(CallCompletedCallback callback) { TODO(); }
/*static*/ void V8::RemoveCallCompletedCallback(CallCompletedCallback callback) { TODO(); }
/*static*/ void V8::AddObjectGroup(Persistent<Value>* objects, size_t length, RetainedObjectInfo* info) { TODO(); }
/*static*/ void V8::AddImplicitReferences(Persistent<Object> parent, Persistent<Value>* children, size_t length) { TODO(); }
/*static*/ bool V8::Initialize() { /*TODO();*/ return false; }
/*static*/ void V8::SetEntropySource(EntropySource source) { TODO(); }
/*static*/ void V8::SetReturnAddressLocationResolver(ReturnAddressLocationResolver return_address_resolver) { TODO(); }
/*static*/ bool V8::SetFunctionEntryHook(FunctionEntryHook entry_hook) { TODO(); return false; }
/*static*/ void V8::SetJitCodeEventHandler(JitCodeEventOptions options, JitCodeEventHandler event_handler) { TODO(); }
/*static*/ intptr_t V8::AdjustAmountOfExternalAllocatedMemory(intptr_t change_in_bytes)
{
	static intptr_t _bytes = 0;

	//printf("%s: %ld + %ld -> %ld\n", __PRETTY_FUNCTION__, _bytes, change_in_bytes, _bytes + change_in_bytes);

	_bytes += change_in_bytes;

	static intptr_t _inc_bytes = 0;
	const intptr_t _max_inc_bytes = 16*1024;
	_inc_bytes += change_in_bytes;
	if ((_inc_bytes <= -_max_inc_bytes) || (_max_inc_bytes <= _inc_bytes))
	{
		_inc_bytes %= _max_inc_bytes;
		printf("%s: %ld\n", __PRETTY_FUNCTION__, _bytes);
	}

	JSGarbageCollect(Context::GetCurrentJSGlobalContext());

	return _bytes;
}
/*static*/ void V8::PauseProfiler() { TODO(); }
/*static*/ void V8::ResumeProfiler() { TODO(); }
/*static*/ bool V8::IsProfilerPaused() { TODO(); return false; }
/*static*/ int V8::GetCurrentThreadId() { TODO(); return 0; }
/*static*/ void V8::TerminateExecution(int thread_id) { TODO(); }
/*static*/ void V8::TerminateExecution(Isolate* isolate) { TODO(); }
/*static*/ bool V8::IsExecutionTerminating(Isolate* isolate) { TODO(); return false; }
/*static*/ bool V8::Dispose() { TODO(); return false; }
/*static*/ void V8::GetHeapStatistics(HeapStatistics* heap_statistics) { TODO(); }
/*static*/ void V8::VisitExternalResources(ExternalResourceVisitor* visitor) { TODO(); }
/*static*/ void V8::VisitHandlesWithClassIds(PersistentHandleVisitor* visitor) { TODO(); }
/*static*/ bool V8::IdleNotification(int hint) { TODO(); return false; }
/*static*/ void V8::LowMemoryNotification() { TODO(); }
/*static*/ int V8::ContextDisposedNotification() { TODO(); return 0; }

} // namespace v8

/// v8-debug.cc

#include "v8-debug.h"

namespace v8 {

/// v8::Debug

/*static*/ bool Debug::SetDebugEventListener(EventCallback that, Handle<Value> data) { TODO(); return false; }
/*static*/ bool Debug::SetDebugEventListener2(EventCallback2 that, Handle<Value> data) { TODO(); return false; }
/*static*/ bool Debug::SetDebugEventListener(v8::Handle<v8::Object> that, Handle<Value> data) { TODO(); return false; }
/*static*/ void Debug::DebugBreak(Isolate* isolate) { TODO(); }
/*static*/ void Debug::CancelDebugBreak(Isolate* isolate) { TODO(); }
/*static*/ void Debug::DebugBreakForCommand(ClientData* data, Isolate* isolate) { TODO(); }
/*static*/ void Debug::SetMessageHandler(MessageHandler handler, bool message_handler_thread) { TODO(); }
/*static*/ void Debug::SetMessageHandler2(MessageHandler2 handler) { TODO(); }
/*static*/ void Debug::SendCommand(const uint16_t* command, int length, ClientData* client_data, Isolate* isolate) { TODO(); }
/*static*/ void Debug::SetHostDispatchHandler(HostDispatchHandler handler, int period ) { TODO(); }
/*static*/ void Debug::SetDebugMessageDispatchHandler(DebugMessageDispatchHandler handler, bool provide_locker) { TODO(); }
/*static*/ Local<Value> Debug::Call(v8::Handle<v8::Function> fun, Handle<Value> data) { TODO(); return Local<Value>(); }
/*static*/ Local<Value> Debug::GetMirror(v8::Handle<v8::Value> obj) { TODO(); return Local<Value>(); }
/*static*/ bool Debug::EnableAgent(const char* name, int port, bool wait_for_connection) { TODO(); return false; }
/*static*/ void Debug::DisableAgent() { TODO(); }
/*static*/ void Debug::ProcessDebugMessages() { TODO(); }
/*static*/ Local<Context> Debug::GetDebugContext() { TODO(); return Local<Context>(); }
/*static*/ void Debug::SetLiveEditEnabled(bool enable, Isolate* isolate) { TODO(); }

} // namespace v8

/// v8-preparser.cc

#include "v8-preparser.h"

namespace v8 {

/// v8::UnicodeInputStream

UnicodeInputStream::~UnicodeInputStream() {}

/// v8

PreParserData Preparse(UnicodeInputStream* input, size_t max_stack_size) { TODO(); return PreParserData(0, NULL); }

} // namespace v8

/// v8-profiler.cc

#include "v8-profiler.h"

namespace v8 {

/// v8::HeapProfiler

/*static*/ int HeapProfiler::GetSnapshotsCount() { TODO(); return 0; }
/*static*/ const HeapSnapshot* HeapProfiler::GetSnapshot(int index) { TODO(); return NULL; }
/*static*/ const HeapSnapshot* HeapProfiler::FindSnapshot(unsigned uid) { TODO(); return NULL; }
/*static*/ SnapshotObjectId HeapProfiler::GetSnapshotObjectId(Handle<Value> value) { TODO(); return kUnknownObjectId; }
/*static*/ const HeapSnapshot* HeapProfiler::TakeSnapshot(Handle<String> title, HeapSnapshot::Type type, ActivityControl* control) { TODO(); return NULL; }
/*static*/ void HeapProfiler::StartHeapObjectsTracking() { TODO(); }
/*static*/ SnapshotObjectId HeapProfiler::PushHeapObjectsStats(OutputStream* stream) { TODO(); return kUnknownObjectId; }
/*static*/ void HeapProfiler::StopHeapObjectsTracking() { TODO(); }
/*static*/ void HeapProfiler::DeleteAllSnapshots() { TODO(); }
/*static*/ void HeapProfiler::DefineWrapperClass(uint16_t class_id, WrapperInfoCallback callback) { /*TODO();*/ }
/*static*/ int HeapProfiler::GetPersistentHandleCount() { TODO(); return 0; }
/*static*/ size_t HeapProfiler::GetMemorySizeUsedByProfiler() { TODO(); return 0; }

} // namespace v8

/// v8-testing.cc

#include "v8-testing.h"

namespace v8 {

/// v8::Testing

/*static*/ void Testing::SetStressRunType(StressType type) { TODO(); }
/*static*/ int Testing::GetStressRuns() { TODO(); return 0; }
/*static*/ void Testing::PrepareStressRun(int run) { TODO(); }
/*static*/ void Testing::DeoptimizeAll() { TODO(); }

} // namespace v8

/// v8-internal.cc

#include "v8-internal.cc" // borrowed *.cc from v8/src/...

