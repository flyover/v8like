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

#ifndef V8LIKE_H_
#define V8LIKE_H_

#define Handle JSC_Handle
#define Boolean JSC_Boolean
#include <JavaScriptCore/JavaScriptCore.h>
#undef Handle
#undef Boolean

#include "v8stdint.h"

#include <vector>
#include <map>

#ifdef _WIN32

#if defined(BUILDING_V8_SHARED) && defined(USING_V8_SHARED)
#error both BUILDING_V8_SHARED and USING_V8_SHARED are set
#endif

#ifdef BUILDING_V8_SHARED
#define V8EXPORT __declspec(dllexport)
#elif USING_V8_SHARED
#define V8EXPORT __declspec(dllimport)
#else
#define V8EXPORT
#endif  // BUILDING_V8_SHARED

#else  // _WIN32

#if defined(__GNUC__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)) && defined(V8_SHARED)
#ifdef BUILDING_V8_SHARED
#define V8EXPORT __attribute__ ((visibility("default")))
#else
#define V8EXPORT
#endif
#else
#define V8EXPORT
#endif

#endif  // _WIN32

namespace v8 {

#define TODO() printf("%s:%d TODO: %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__)

class Context;
class Isolate;
template <class T> class Handle;
template <class T> class Local;
template <class T> class Persistent;
class HandleScope;
class Data;
class Value;
class Primitive;
class Boolean;
class Number;
class String;
class Object;
class Array;
class Integer;
class Int32;
class Uint32;
class Function;
class BooleanObject;
class NumberObject;
class StringObject;
class External;
class Signature;
class AccessorSignature;
class Template;
class ObjectTemplate;
class FunctionTemplate;
class StackFrame;
class StackTrace;

namespace internal {

/// v8::internal::RTTI

class RTTI
{
public:
	RTTI(const char* name, const RTTI* base_rtti) : 
		m_name(name), m_base_rtti(base_rtti) {}

	const char* GetName() const { return m_name; }
	const RTTI* GetBaseRTTI() const { return m_base_rtti; }

protected:
	const char* m_name;
	const RTTI* m_base_rtti;
};

// add to root class declaration
#define RTTI_DECLARE_ROOT(classname) \
	public: \
		static const internal::RTTI m_rtti; \
		virtual const internal::RTTI* GetRTTI() const { return &m_rtti; } \
		virtual const char* GetTypeName() const { return m_rtti.GetName(); } \
		static bool IsExactKindOf(const internal::RTTI* rtti, const classname* object) \
		{ \
			return (object != NULL)?(object->IsExactKindOf(rtti)):(false); \
		} \
		bool IsExactKindOf(const internal::RTTI* rtti) const \
		{ \
			return (GetRTTI() == rtti); \
		} \
		static bool IsKindOf(const internal::RTTI* rtti, const classname* object) \
		{ \
			return (object != NULL)?(object->IsKindOf(rtti)):(false); \
		} \
		bool IsKindOf(const internal::RTTI* rtti) const \
		{ \
			const internal::RTTI* tmp = GetRTTI(); \
			while (tmp != NULL) \
			{ \
				if (tmp == rtti) \
				{ \
					return true; \
				} \
				tmp = tmp->GetBaseRTTI(); \
			} \
			return false; \
		} \
		static classname* DynamicCast(const internal::RTTI* rtti, const classname* object) \
		{ \
			return (object != NULL)?(object->DynamicCast(rtti)):(NULL); \
		} \
		classname* DynamicCast(const internal::RTTI* rtti) const \
		{ \
			return (IsKindOf(rtti) ? (classname*) this : NULL ); \
		}

// add to class declaration
#define RTTI_DECLARE() \
	public: \
		static const internal::RTTI m_rtti; \
		virtual const internal::RTTI* GetRTTI() const { return &m_rtti; } \
		virtual const char* GetTypeName() const { return m_rtti.GetName(); }

// add to root class source file
#define RTTI_IMPLEMENT_ROOT(rootclassname) \
	const internal::RTTI rootclassname::m_rtti(#rootclassname, NULL);

// add to class source file
#define RTTI_IMPLEMENT(classname, baseclassname) \
	const internal::RTTI classname::m_rtti(#classname, &baseclassname::m_rtti);

#define RTTI_GetRTTI(classname) \
	(&classname::m_rtti)

#define RTTI_GetTypeName(classname) \
	(classname::m_rtti.GetName())

#define RTTI_IsExactKindOf(classname, object) \
	classname::IsExactKindOf(&classname::m_rtti, object)

#define RTTI_IsKindOf(classname, object) \
	classname::IsKindOf(&classname::m_rtti, object)

#define RTTI_StaticCast(classname, object) \
	((classname *) object)

#define RTTI_DynamicCast(classname, object) \
	((classname *) classname::DynamicCast(&classname::m_rtti, object))

/// v8::internal::SmartObject

class SmartObject
{
private:
	unsigned int m_ref;

public:
	SmartObject() : m_ref(0)
	{
		//printf("ctor %p\n", this);
	}
	virtual ~SmartObject()
	{
		assert(m_ref == 0);
		//printf("dtor %p\n", this);
	}
	virtual void IncRef();
	virtual void DecRef();
	unsigned int GetRef() const { return m_ref; }

public:
	virtual void _OnCreate() {}
	virtual void _OnDelete() {}
	virtual void _MakeWeak(void* parameter, void (*callback)(Persistent<Value> object, void* parameter)) {}
	virtual void _ClearWeak() {}
	virtual void _MarkIndependent() {}
	virtual bool _IsIndependent() const { return false; }
	virtual bool _IsNearDeath() const { return false; }
	virtual bool _IsWeak() const { return false; }
	virtual void _SetWrapperClassId(uint16_t class_id) {}
	virtual uint16_t _WrapperClassId() const { return 0; }
};

/// v8::internal::SmartPointer

// Note: class T must be derived from SmartObject for reference counting
template <class T> class SmartPointer
{
private:
	T* m_ptr; // the managed pointer (derived from SmartObject)

public:
	// construction and destruction
	SmartPointer(T* ptr = (T*) 0) : m_ptr(ptr)
	{
		if (m_ptr) { m_ptr->IncRef(); }
	}
	SmartPointer(const SmartPointer& other) : m_ptr(other.m_ptr)
	{
		if (m_ptr) { m_ptr->IncRef(); }
	}
	virtual ~SmartPointer()
	{
		if (m_ptr) { m_ptr->DecRef(); m_ptr = (T*) 0; }
	}

	// implicit conversions
	operator T*() const { return m_ptr; }
	T& operator*() const { return *m_ptr; }
	T* operator->() const { return m_ptr; }

	// assignment
	SmartPointer& operator=(T* ptr)
	{
		if (m_ptr != ptr)
		{
			if (m_ptr) { m_ptr->DecRef(); }
			m_ptr = ptr;
			if (m_ptr) { m_ptr->IncRef(); }
		}
		return *this;
	}
	SmartPointer& operator=(const SmartPointer& other)
	{
		if (m_ptr != other.m_ptr)
		{
			if (m_ptr) { m_ptr->DecRef(); }
			m_ptr = other.m_ptr;
			if (m_ptr) { m_ptr->IncRef(); }
		}
		return *this;
	}

	// comparisons
	bool operator==(T* ptr) const { return (m_ptr == ptr); }
	bool operator!=(T* ptr) const { return (m_ptr != ptr); }
	bool operator==(const SmartPointer& other) const { return (m_ptr == other.m_ptr); }
	bool operator!=(const SmartPointer& other) const { return (m_ptr != other.m_ptr); }

	T* GetPointer() const { return m_ptr; }
	bool IsEmpty() const { return (m_ptr == ((T*) 0)); }
	void _Clear() { if (m_ptr) { m_ptr->DecRef(); m_ptr = (T*) 0; } }
};

class JSStringWrap
{
private:
	JSStringRef m_js_string;

public:
	JSStringWrap(const char* utf8_cstring) : m_js_string(NULL)
	{
		if (utf8_cstring)
		{
			m_js_string = JSStringCreateWithUTF8CString(utf8_cstring);
		}
	}

	~JSStringWrap()
	{
		if (m_js_string != NULL)
		{
			JSStringRelease(m_js_string);
			m_js_string = NULL;
		}
	}

	operator JSStringRef() const
	{
		return m_js_string;
	}
};

JSValueRef ExportValue(Handle<Value> value);
JSObjectRef ExportObject(Handle<Object> object);
Handle<Value> ImportValue(JSContextRef js_ctx, JSValueRef js_value);
Handle<Object> ImportObject(JSContextRef js_ctx, JSObjectRef js_object);

} // namespace internal

/// v8::Isolate

class V8EXPORT Isolate
{
public:
	class V8EXPORT Scope
	{
	private:
		Isolate* const m_isolate;
	public:
		explicit Scope(Isolate* isolate) : m_isolate(isolate) { m_isolate->Enter(); }
		~Scope() { m_isolate->Exit(); }
	private:
		// Prevent copying of Scope objects.
		Scope(const Scope&);
		Scope& operator=(const Scope&);
	};

private:
	Isolate* m_previous_isolate;
	void* m_data;

private:
	Isolate() : m_previous_isolate(NULL), m_data(NULL) {}
	~Isolate() {}

public:
	void Enter();
	void Exit();

	void Dispose();

	void SetData(void* data) { m_data = data; }
	void* GetData() { return m_data; }

private:
	static Isolate* sm_current_isolate;

public:
	static Isolate* New();
	static Isolate* GetCurrent();
};

/// v8::Handle<T>

#define TYPE_CHECK(T, S) while (false) { *(static_cast<T* volatile*>(0)) = static_cast<S*>(0); }

template <class T> class Handle : public internal::SmartPointer<T>
{
public:
	Handle(T* ptr = (T*) 0) : internal::SmartPointer<T>(ptr) {}
	Handle(const Handle<T>& other) : internal::SmartPointer<T>(other) {}
	template <class S> Handle(const Handle<S>& other) : internal::SmartPointer<T>(reinterpret_cast<T*>(other.GetPointer())) { TYPE_CHECK(T, S); }
	virtual ~Handle() {}

	// override dereference operator so SmartPointer behaves like a Handle
	T* operator*() const { return internal::SmartPointer<T>::GetPointer(); }

	template <class S> inline Handle<S> As() { return Handle<S>::Cast(*this); }

public:
	template <class S> static inline Handle<T> Cast(Handle<S> that) { return Handle<T>(T::Cast(*that)); }
};

/// v8::Local<T>

template <class T> class Local : public Handle<T>
{
public:
	Local(T* ptr = (T*) 0) : Handle<T>(ptr) {}
	Local(const Local<T>& other) : Handle<T>(other) {}
	template <class S> Local(const Local<S>& other) : Handle<T>(reinterpret_cast<T*>(other.GetPointer())) { TYPE_CHECK(T, S); }
	virtual ~Local() {}

	template <class S> inline Local<S> As() { return Local<S>::Cast(*this); }

public:
	static Local<T> New(Handle<T> that) { return Local<T>(that); }
	template <class S> static inline Local<T> Cast(Local<S> that) { return Local<T>(T::Cast(*that)); }
};

/// v8::Persistent<T>

typedef void (*WeakReferenceCallback)(Persistent<Value> object, void* parameter);

template <class T> class Persistent : public Handle<T>
{
public:
	Persistent(T* ptr = (T*) 0) : Handle<T>(ptr) {}
	Persistent(const Persistent<T>& other) : Handle<T>(other) {}
	template <class S> Persistent(const Persistent<S>& other) : Handle<T>(reinterpret_cast<T*>(other.GetPointer())) { TYPE_CHECK(T, S); }
	virtual ~Persistent() {}

	template <class S> inline Persistent<S> As() { return Persistent<S>::Cast(*this); }

	void Dispose()
	{
		internal::SmartPointer<T>::_Clear();
	}
	void Clear() {}
	void MakeWeak(void* parameter, WeakReferenceCallback callback) { internal::SmartPointer<T>::GetPointer()->_MakeWeak(parameter, callback); }
	void ClearWeak() { internal::SmartPointer<T>::GetPointer()->_ClearWeak(); }
	void MarkIndependent() { internal::SmartPointer<T>::GetPointer()->_MarkIndependent(); }
	bool IsIndependent() const { return internal::SmartPointer<T>::GetPointer()->_IsIndependent(); }
	bool IsNearDeath() const { return internal::SmartPointer<T>::GetPointer()->_IsNearDeath(); }
	bool IsWeak() const { return internal::SmartPointer<T>::GetPointer()->_IsWeak(); }
	void SetWrapperClassId(uint16_t class_id) { internal::SmartPointer<T>::GetPointer()->_SetWrapperClassId(class_id); }
	uint16_t WrapperClassId() const { return internal::SmartPointer<T>::GetPointer()->_WrapperClassId(); }

public:
	static Persistent<T> New(Handle<T> that) { return Persistent<T>(that); }
	template <class S> static inline Persistent<T> Cast(Persistent<S> that) { return Persistent<T>(T::Cast(*that)); }
};

/// v8::HandleScope

class HandleScope
{
public:
	HandleScope() {}
	~HandleScope() {}

	template <class T> Local<T> Close(Handle<T> value) { return Local<T>(value); }
};

/// v8::Data

class V8EXPORT Data : public internal::SmartObject
{
	RTTI_DECLARE_ROOT(v8::Data);
public:
	V8EXPORT Data() {}
	virtual ~Data() {}
};

/// v8::Value

class V8EXPORT Value : public Data
{
	RTTI_DECLARE();
public:
	JSContextRef m_js_ctx;
	JSValueRef m_js_value;
	unsigned int m_js_ref;
	bool m_js_is_protected;
	bool m_is_weak;
	WeakReferenceCallback m_weak_callback;
	void* m_weak_parameter;
	bool m_is_independent;
	uint16_t m_class_id;
	bool m_is_near_death;
public:
	V8EXPORT Value();
	V8EXPORT Value(JSContextRef js_ctx, JSValueRef js_value);
	virtual ~Value();

public:
	virtual void IncRef();
	virtual void DecRef();
	virtual void _OnCreate();
	virtual void _OnDelete();
	virtual void _MakeWeak(void* parameter, void (*callback)(Persistent<Value> object, void* parameter));
	virtual void _ClearWeak();
	virtual void _MarkIndependent();
	virtual bool _IsIndependent() const;
	virtual bool _IsNearDeath() const;
	virtual bool _IsWeak() const;
	virtual void _SetWrapperClassId(uint16_t class_id);
	virtual uint16_t _WrapperClassId() const;

public:
	//JSValueRef GetJSValue() { return m_js_value; }
	void AttachJSContextAndJSValue(JSContextRef js_ctx, JSValueRef js_value);
	void DetachJSContextAndJSValue();
	void IncJSRef();
	void DecJSRef();
	unsigned int GetJSRef() { return m_js_ref; }

public:
	bool IsUndefined();
	bool IsNull();
	bool IsBoolean();
	bool IsFalse();
	bool IsTrue();
	bool IsNumber();
	bool IsInt32();
	bool IsUint32();
	bool IsString();
	bool IsObject();
	bool IsArray();
	bool IsFunction();
	bool IsExternal();
	bool IsRegExp();
	bool IsNativeError();
	bool IsBooleanObject();
	bool IsNumberObject();
	bool IsStringObject();

	Local<Boolean> ToBoolean() const;
	Local<Number> ToNumber() const;
	Local<Integer> ToInteger() const;
	Local<Int32> ToInt32() const;
	Local<Uint32> ToUint32() const;
	Local<Uint32> ToArrayIndex() const;
	Local<String> ToString() const;
	Local<String> ToDetailString() const;
	Local<Object> ToObject() const;

	bool BooleanValue() const;
	double NumberValue() const;
	int64_t IntegerValue() const;
	int32_t Int32Value() const;
	uint32_t Uint32Value() const;
	bool Equals(Handle<Value> that) const;
	bool StrictEquals(Handle<Value> that) const;
};

/// v8::Primitive

class V8EXPORT Primitive : public Value
{
	RTTI_DECLARE();
public:
	V8EXPORT Primitive() {}
	V8EXPORT Primitive(JSContextRef js_ctx, JSValueRef js_value) : Value(js_ctx, js_value) {}
	virtual ~Primitive() {}
};

/// v8::Boolean

class V8EXPORT Boolean : public Primitive
{
	RTTI_DECLARE();
public:
	V8EXPORT Boolean(JSContextRef js_ctx, JSValueRef js_value) : Primitive(js_ctx, js_value) {}
	virtual ~Boolean() {}
public:
	V8EXPORT bool Value() const;
public:
	V8EXPORT static Handle<Boolean> New(bool value);
	V8EXPORT static Boolean* Cast(v8::Value* value);
};

/// v8::Number

class V8EXPORT Number : public Primitive
{
	RTTI_DECLARE();
public:
	V8EXPORT Number(JSContextRef js_ctx, JSValueRef js_value) : Primitive(js_ctx, js_value) {}
	virtual ~Number() {}
public:
	V8EXPORT double Value() const;
public:
	V8EXPORT static Local<Number> New(double value);
	V8EXPORT static Number* Cast(v8::Value* value);
};

/// v8::Integer

class V8EXPORT Integer : public Number
{
	RTTI_DECLARE();
public:
	V8EXPORT Integer(JSContextRef js_ctx, JSValueRef js_value) : Number(js_ctx, js_value) {}
	virtual ~Integer() {}
public:
	V8EXPORT int64_t Value() const;
public:
	V8EXPORT static Local<Integer> New(int32_t value);
	V8EXPORT static Local<Integer> NewFromUnsigned(uint32_t value);
	//V8EXPORT static Local<Integer> New(int32_t value, Isolate*);
	//V8EXPORT static Local<Integer> NewFromUnsigned(uint32_t value, Isolate*);
	V8EXPORT static Integer* Cast(v8::Value* value);
};

/// v8::Int32

class V8EXPORT Int32 : public Integer
{
	RTTI_DECLARE();
public:
	V8EXPORT Int32(JSContextRef js_ctx, JSValueRef js_value) : Integer(js_ctx, js_value) {}
	virtual ~Int32() {}
public:
	V8EXPORT int32_t Value() const;
};

/// v8::Uint32

class V8EXPORT Uint32 : public Integer
{
	RTTI_DECLARE();
public:
	V8EXPORT Uint32(JSContextRef js_ctx, JSValueRef js_value) : Integer(js_ctx, js_value) {}
	virtual ~Uint32() {}
public:
	V8EXPORT uint32_t Value() const;
};

/// v8::String

class V8EXPORT String : public Primitive
{
	RTTI_DECLARE();
public:
	enum Encoding
	{
		UNKNOWN_ENCODING = 0x1,
		TWO_BYTE_ENCODING = 0x0,
		ASCII_ENCODING = 0x4
	};

	enum WriteOptions
	{
		NO_OPTIONS = 0,
		HINT_MANY_WRITES_EXPECTED = 1,
		NO_NULL_TERMINATION = 2,
		PRESERVE_ASCII_NULL = 4,
		// Used by WriteUtf8 to replace orphan surrogate code units with the
		// unicode replacement character. Needs to be set to guarantee valid UTF-8
		// output.
		REPLACE_INVALID_UTF8 = 8
	};

public:
	class V8EXPORT Utf8Value
	{
	private:
		char* m_str;
		int m_length;
	public:
		Utf8Value(Handle<Value> value);
		~Utf8Value();
	public:
		char* operator*() { return m_str; }
		const char* operator*() const { return m_str; }
		int length() const { return m_length; }
	private:
		// Disallow copying and assigning.
		Utf8Value(const Utf8Value&);
		void operator=(const Utf8Value&);
	};

public:
	class V8EXPORT AsciiValue
	{
	private:
		char* m_str;
		int m_length;
	public:
		AsciiValue(Handle<Value> value);
		~AsciiValue();
	public:
		char* operator*() { return m_str; }
		const char* operator*() const { return m_str; }
		int length() const { return m_length; }
	private:
		// Disallow copying and assigning.
		AsciiValue(const AsciiValue&);
		void operator=(const AsciiValue&);
	};

public:
	class V8EXPORT Value
	{
	private:
		uint16_t* m_str;
		int m_length;
	public:
		Value(Handle<v8::Value> value);
		~Value();
	public:
		uint16_t* operator*() { return m_str; }
		const uint16_t* operator*() const { return m_str; }
		int length() const { return m_length; }
	private:
		// Disallow copying and assigning.
		Value(const Value&);
		void operator=(const Value&);
	};

	class V8EXPORT ExternalStringResourceBase
	{
	public:
		virtual ~ExternalStringResourceBase() {}
	protected:
		ExternalStringResourceBase() {}
		virtual void Dispose() { delete this; }
	private:
		// Disallow copying and assigning.
		ExternalStringResourceBase(const ExternalStringResourceBase&);
		void operator=(const ExternalStringResourceBase&);
		//friend class v8::internal::Heap;
	};

	class V8EXPORT ExternalStringResource : public ExternalStringResourceBase
	{
	public:
		virtual ~ExternalStringResource() {}
		virtual const uint16_t* data() const = 0;
		virtual size_t length() const = 0;
	protected:
		ExternalStringResource() {}
	};

	class V8EXPORT ExternalAsciiStringResource : public ExternalStringResourceBase
	{
	public:
		virtual ~ExternalAsciiStringResource() {}
		virtual const char* data() const = 0;
		virtual size_t length() const = 0;
	protected:
		ExternalAsciiStringResource() {}
	};

public:
	V8EXPORT String(JSContextRef js_ctx, JSValueRef js_value);
	V8EXPORT virtual ~String();

public:
	V8EXPORT int Length();
	V8EXPORT int Utf8Length();
	V8EXPORT bool MayContainNonAscii() const;

	V8EXPORT int Write(uint16_t* buffer, int start = 0, int length = -1, int options = NO_OPTIONS) const;
	V8EXPORT int WriteAscii(char* buffer, int start = 0, int length = -1, int options = NO_OPTIONS) const;
	V8EXPORT int WriteUtf8(char* buffer, int length = -1, int* nchars_ref = NULL, int options = NO_OPTIONS) const;

	V8EXPORT bool IsExternal() const;
	V8EXPORT bool IsExternalAscii() const;
	V8EXPORT ExternalStringResourceBase* GetExternalStringResourceBase(Encoding* encoding_out) const;
	V8EXPORT ExternalStringResource* GetExternalStringResource() const;
	V8EXPORT const ExternalAsciiStringResource* GetExternalAsciiStringResource() const;

	V8EXPORT bool MakeExternal(ExternalStringResource* resource);
	V8EXPORT bool MakeExternal(ExternalAsciiStringResource* resource);
	V8EXPORT bool CanMakeExternal();

public:
	V8EXPORT static Local<String> Empty();
    V8EXPORT static String* Cast(v8::Value* value);

public:
	V8EXPORT static Local<String> New(const char* data, int length = -1);
	V8EXPORT static Local<String> New(const uint16_t* data, int length = -1);
	V8EXPORT static Local<String> NewSymbol(const char* data, int length = -1);
	V8EXPORT static Local<String> Concat(Handle<String> left, Handle<String> right);
	V8EXPORT static Local<String> NewExternal(ExternalStringResource* resource);
	V8EXPORT static Local<String> NewExternal(ExternalAsciiStringResource* resource);
	V8EXPORT static Local<String> NewUndetectable(const char* data, int length = -1);
	V8EXPORT static Local<String> NewUndetectable(const uint16_t* data, int length = -1);
};

/// v8::AccessorInfo

class V8EXPORT AccessorInfo
{
public:
	Isolate* m_isolate;
	Handle<Object> m_that;
	Handle<Object> m_holder;
	Handle<Value> m_data;
public:
	V8EXPORT AccessorInfo(Isolate* isolate, Handle<Object> that, Handle<Object> holder, Handle<Value> data);
	V8EXPORT ~AccessorInfo();
public:
	Isolate* GetIsolate() const;
	Local<Object> This() const;
	Local<Object> Holder() const;
	Local<Value> Data() const;
};

///

typedef Handle<Value> (*NamedPropertyGetter)(Local<String> property, const AccessorInfo& info);
typedef Handle<Value> (*NamedPropertySetter)(Local<String> property, Local<Value> value, const AccessorInfo& info);
typedef Handle<Integer> (*NamedPropertyQuery)(Local<String> property, const AccessorInfo& info);
typedef Handle<Boolean> (*NamedPropertyDeleter)(Local<String> property, const AccessorInfo& info);
typedef Handle<Array> (*NamedPropertyEnumerator)(const AccessorInfo& info);

typedef Handle<Value> (*IndexedPropertyGetter)(uint32_t index, const AccessorInfo& info);
typedef Handle<Value> (*IndexedPropertySetter)(uint32_t index, Local<Value> value, const AccessorInfo& info);
typedef Handle<Integer> (*IndexedPropertyQuery)(uint32_t index, const AccessorInfo& info);
typedef Handle<Boolean> (*IndexedPropertyDeleter)(uint32_t index, const AccessorInfo& info);
typedef Handle<Array> (*IndexedPropertyEnumerator)(const AccessorInfo& info);

enum AccessType
{
	ACCESS_GET,
	ACCESS_SET,
	ACCESS_HAS,
	ACCESS_DELETE,
	ACCESS_KEYS
};

typedef bool (*NamedSecurityCallback)(Local<Object> host, Local<Value> key, AccessType type, Local<Value> data);

typedef bool (*IndexedSecurityCallback)(Local<Object> host, uint32_t index, AccessType type, Local<Value> data);

enum PropertyAttribute
{
	None	   = 0,
	ReadOnly   = 1 << 0,
	DontEnum   = 1 << 1,
	DontDelete = 1 << 2
};

enum ExternalArrayType
{
	kExternalByteArray = 1,
	kExternalUnsignedByteArray,
	kExternalShortArray,
	kExternalUnsignedShortArray,
	kExternalIntArray,
	kExternalUnsignedIntArray,
	kExternalFloatArray,
	kExternalDoubleArray,
	kExternalPixelArray
};

typedef Handle<Value> (*AccessorGetter)(Local<String> property, const AccessorInfo& info);
typedef void (*AccessorSetter)(Local<String> property, Local<Value> value, const AccessorInfo& info);

enum AccessControl
{
	DEFAULT 			  = 0,
	ALL_CAN_READ		  = 1,
	ALL_CAN_WRITE   	  = 1 << 1,
	PROHIBITS_OVERWRITING = 1 << 2
};

namespace internal {

class Property
{
public:
	Handle<Value> m_value;
	PropertyAttribute m_attrib;
public:
	Property() : m_attrib(None) {}
	Property(Handle<Value> value, PropertyAttribute attrib) :
		m_value(value), m_attrib(attrib) {}
	Property(const Property& other) : 
		m_value(other.m_value), m_attrib(other.m_attrib) {}
	Property& operator=(const Property& other)
	{
		m_value = other.m_value;
		m_attrib = other.m_attrib;
		return *this;
	}
};

class Accessor
{
public:
	AccessorGetter m_getter;
	AccessorSetter m_setter;
	Handle<Value> m_data;
	AccessControl m_settings;
	PropertyAttribute m_attrib;
public:
	Accessor() : m_getter(NULL), m_setter(NULL), m_settings(DEFAULT), m_attrib(None) {}
	Accessor(AccessorGetter getter, AccessorSetter setter, Handle<Value> data,
			 AccessControl settings, PropertyAttribute attrib) :
		m_getter(getter), m_setter(setter), m_data(data),
		m_settings(settings), m_attrib(attrib) {}
	Accessor(const Accessor& other) : 
		m_getter(other.m_getter), m_setter(other.m_setter), m_data(other.m_data),
		m_settings(other.m_settings), m_attrib(other.m_attrib) {}
	Accessor& operator=(const Accessor& other)
	{
		m_getter = other.m_getter;
		m_setter = other.m_setter;
		m_data = other.m_data;
		m_settings = other.m_settings;
		m_attrib = other.m_attrib;
		return *this;
	}
};

class TemplateProperty
{
public:
	Handle<Data> m_value;
	PropertyAttribute m_attrib;
public:
	TemplateProperty() : m_attrib(None) {}
	TemplateProperty(Handle<Data> value, PropertyAttribute attrib) :
		m_value(value), m_attrib(attrib) {}
	TemplateProperty(const TemplateProperty& other) : 
		m_value(other.m_value), m_attrib(other.m_attrib) {}
	TemplateProperty& operator=(const TemplateProperty& other)
	{
		m_value = other.m_value;
		m_attrib = other.m_attrib;
		return *this;
	}
};

class TemplateAccessor
{
public:
	AccessorGetter m_getter;
	AccessorSetter m_setter;
	Handle<Value> m_data;
	AccessControl m_settings;
	PropertyAttribute m_attrib;
	Handle<AccessorSignature> m_signature;
public:
	TemplateAccessor() {}
	TemplateAccessor(AccessorGetter getter, AccessorSetter setter, Handle<Value> data, 
					 AccessControl settings, PropertyAttribute attrib, 
					 Handle<AccessorSignature> signature) :
		m_getter(getter), m_setter(setter), m_data(data),
		m_settings(settings), m_attrib(attrib), 
		m_signature(signature) {}
	TemplateAccessor(const TemplateAccessor& other) : 
		m_getter(other.m_getter), m_setter(other.m_setter), m_data(other.m_data),
		m_settings(other.m_settings), m_attrib(other.m_attrib), 
		m_signature(other.m_signature) {}
	TemplateAccessor& operator=(const TemplateAccessor& other)
	{
		m_getter = other.m_getter;
		m_setter = other.m_setter;
		m_data = other.m_data;
		m_settings = other.m_settings;
		m_attrib = other.m_attrib;
		m_signature = other.m_signature;
		return *this;
	}
};

struct StringCompare { bool operator()(const Handle<String>& key1, const Handle<String>& key2) const; };

#if 0 // V8LIKE_PROPERTY_MAP
class PropertyMap : public std::map<Handle<String>, Property, StringCompare> {};
#endif

class AccessorMap : public std::map<Handle<String>, Accessor, StringCompare> {};

class TemplatePropertyMap : public std::map<Handle<String>, TemplateProperty, StringCompare> {};

class TemplateAccessorMap : public std::map<Handle<String>, TemplateAccessor, StringCompare> {};

class Helper
{
private:
	class InternalField
	{
	public:
		Handle<Value> m_value;
	};

public:
	Handle<ObjectTemplate> m_object_template;	// for Objects created by ObjectTemplate::NewInstance
	Handle<Function> m_function;				// for Objects created by Function::NewInstance

	#if 0 // V8LIKE_PROPERTY_MAP
	internal::PropertyMap m_property_map;
	#endif

	internal::AccessorMap m_accessor_map;

	InternalField* m_internal_field_array;
	size_t m_internal_field_count;

	uint8_t* m_pixel_data;
	int m_pixel_data_length;

	void* m_external_array_data;
	ExternalArrayType m_external_array_data_type;
	int m_external_array_data_length;

public:
	Helper();
	virtual ~Helper();

public:
	void SetInternalFieldCount(int internal_field_count);

public:
	static void WeakFree(Persistent<Value> object, void* parameter);
};

} // namespace internal

/// v8::Object

class V8EXPORT Object : public Primitive
{
	RTTI_DECLARE();

public:
	JSObjectRef m_js_object;
	bool m_is_native;
	internal::Helper* m_helper;

public:
	Object();
	Object(JSContextRef js_ctx);
	Object(JSContextRef js_ctx, JSObjectRef js_object);
	virtual ~Object();

public:
	//JSObjectRef GetJSObject() { return m_js_object; }
	void AttachJSContextAndJSObject(JSContextRef js_ctx, JSObjectRef js_object);
	void DetachJSContextAndJSObject();
	internal::Helper* GetHelper();

public:
	bool _ObjectHasProperty(Handle<String> name);
	Handle<Value> _ObjectGetProperty(Handle<String> name);
	bool _ObjectSetProperty(Handle<String> name, Handle<Value> value, PropertyAttribute attrib);
	bool _ObjectDeleteProperty(Handle<String> name);
	void _ObjectGetPropertyNames(JSPropertyNameAccumulatorRef js_name_accumulator);

public:
	V8EXPORT bool Has(Handle<String> key);
	V8EXPORT Local<Value> Get(Handle<Value> key);
	V8EXPORT bool Set(Handle<Value> key, Handle<Value> value, PropertyAttribute attrib = None);
	V8EXPORT bool ForceSet(Handle<Value> key, Handle<Value> value, PropertyAttribute attrib = None);
	V8EXPORT bool Delete(Handle<String> key);
	V8EXPORT bool ForceDelete(Handle<Value> key);

	V8EXPORT PropertyAttribute GetPropertyAttributes(Handle<Value> key);

	V8EXPORT bool Has(uint32_t index);
	V8EXPORT Local<Value> Get(uint32_t index);
	V8EXPORT bool Set(uint32_t index, Handle<Value> value);
	V8EXPORT bool Delete(uint32_t index);

	V8EXPORT bool SetAccessor(Handle<String> name, AccessorGetter getter, AccessorSetter setter = 0, Handle<Value> data = Handle<Value>(), AccessControl settings = DEFAULT, PropertyAttribute attrib = None);

	V8EXPORT Local<Array> GetPropertyNames();
	V8EXPORT Local<Array> GetOwnPropertyNames();

	V8EXPORT Local<Value> GetPrototype();
	V8EXPORT bool SetPrototype(Handle<Value> prototype);

	V8EXPORT Local<Object> FindInstanceInPrototypeChain(Handle<FunctionTemplate> tmpl);
	V8EXPORT Local<String> ObjectProtoToString();
	V8EXPORT Local<Value> GetConstructor();
	V8EXPORT Local<String> GetConstructorName();

	V8EXPORT int InternalFieldCount();
	V8EXPORT Local<Value> GetInternalField(int index);
	V8EXPORT void SetInternalField(int index, Handle<Value> value);

	V8EXPORT void* GetPointerFromInternalField(int index);
	V8EXPORT void SetPointerInInternalField(int index, void* value);

	V8EXPORT bool HasOwnProperty(Handle<String> key);
	V8EXPORT bool HasRealNamedProperty(Handle<String> key);
	V8EXPORT bool HasRealIndexedProperty(uint32_t index);
	V8EXPORT bool HasRealNamedCallbackProperty(Handle<String> key);
	V8EXPORT Local<Value> GetRealNamedPropertyInPrototypeChain(Handle<String> key);
	V8EXPORT Local<Value> GetRealNamedProperty(Handle<String> key);

	V8EXPORT bool HasNamedLookupInterceptor();
	V8EXPORT bool HasIndexedLookupInterceptor();

	V8EXPORT void TurnOnAccessCheck();

	V8EXPORT int GetIdentityHash();

	V8EXPORT bool SetHiddenValue(Handle<String> key, Handle<Value> value);
	V8EXPORT Local<Value> GetHiddenValue(Handle<String> key);
	V8EXPORT bool DeleteHiddenValue(Handle<String> key);

	V8EXPORT bool IsDirty();

	V8EXPORT Local<Object> Clone();

	V8EXPORT Local<Context> CreationContext();

	V8EXPORT void SetIndexedPropertiesToPixelData(uint8_t* data, int length);
	V8EXPORT bool HasIndexedPropertiesInPixelData();
	V8EXPORT uint8_t* GetIndexedPropertiesPixelData();
	V8EXPORT int GetIndexedPropertiesPixelDataLength();

	V8EXPORT void SetIndexedPropertiesToExternalArrayData(void* data, ExternalArrayType array_type, int number_of_elements);
	V8EXPORT bool HasIndexedPropertiesInExternalArrayData();
	V8EXPORT void* GetIndexedPropertiesExternalArrayData();
	V8EXPORT ExternalArrayType GetIndexedPropertiesExternalArrayDataType();
	V8EXPORT int GetIndexedPropertiesExternalArrayDataLength();

	V8EXPORT bool IsCallable();
	V8EXPORT Local<Value> CallAsFunction(Handle<Object> that, int argc, Handle<Value> argv[]);
	V8EXPORT Local<Value> CallAsConstructor(int argc, Handle<Value> argv[]);

public:
	static JSClassRef sm_js_class;

public:
	V8EXPORT static Local<Object> New();
	V8EXPORT static Object* Cast(Value* value);
public:
	static JSClassRef GetJSClass();

private:
	static void _JS_Initialize(JSContextRef js_ctx, JSObjectRef js_object);
	static void _JS_Finalize(JSObjectRef js_object);
	static bool _JS_HasProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name);
	static JSValueRef _JS_GetProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef *js_exception);
	static bool _JS_SetProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef js_value, JSValueRef *js_exception);
	static bool _JS_DeleteProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef* js_exception);
	static void _JS_GetPropertyNames(JSContextRef js_ctx, JSObjectRef js_object, JSPropertyNameAccumulatorRef js_names);
	static bool _JS_HasInstance(JSContextRef js_ctx, JSObjectRef js_constructor, JSValueRef js_instance, JSValueRef* js_exception);
	static JSValueRef _JS_ConvertToType(JSContextRef js_ctx, JSObjectRef js_object, JSType js_type, JSValueRef* js_exception);
};

/// v8::Array

class V8EXPORT Array : public Object
{
	RTTI_DECLARE();
public:
	V8EXPORT Array(JSContextRef js_ctx, int length = 0);
	V8EXPORT Array(JSContextRef js_ctx, JSObjectRef js_object);
	virtual ~Array() {}
public:
	V8EXPORT uint32_t Length() const;
	V8EXPORT Local<Object> CloneElementAt(uint32_t index);
///public:
///	static JSClassRef sm_js_class;
public:
	V8EXPORT static Local<Array> New(int length = 0);
	V8EXPORT static Array* Cast(Value* value);
///public:
///	static JSClassRef GetJSClass();
};

/// v8::ScriptOrigin

class V8EXPORT ScriptOrigin
{
public:
	Handle<Value> m_resource_name;
	Handle<Integer> m_resource_line_offset;
	Handle<Integer> m_resource_column_offset;

public:
	ScriptOrigin(Handle<Value> resource_name, Handle<Integer> resource_line_offset = Handle<Integer>(), Handle<Integer> resource_column_offset = Handle<Integer>()) : 
		m_resource_name(resource_name), 
		m_resource_line_offset(resource_line_offset), 
		m_resource_column_offset(resource_column_offset)
	{}

	Handle<Value> ResourceName() const { return m_resource_name; }
	Handle<Integer> ResourceLineOffset() const { return m_resource_line_offset; }
	Handle<Integer> ResourceColumnOffset() const { return m_resource_column_offset; }
};

/// v8::ScriptData

class V8EXPORT ScriptData
{
};

/// v8::Arguments

class V8EXPORT Arguments
{
public:
	Isolate* m_isolate;
	Handle<Function> m_callee;
	Handle<Object> m_that;
	Handle<Object> m_holder;
	Handle<Value> m_data;
	int m_argc;
	Handle<Value>* m_argv;
	bool m_is_construct_call;

public:
	V8EXPORT Arguments(Isolate* isolate, Handle<Function> callee, Handle<Object> that, Handle<Object> holder, Handle<Value> data, int argc, Handle<Value>* argv, bool is_construct_call);
	~Arguments();

public:
	Isolate* GetIsolate() const;
	int Length() const;
	Local<Value> operator[](int i) const;
	Local<Function> Callee() const;
	Local<Object> This() const;
	Local<Object> Holder() const;
	bool IsConstructCall() const;
	Local<Value> Data() const;
};

/// v8::Function

typedef Handle<Value> (*InvocationCallback)(const Arguments& args);

class V8EXPORT Function : public Object
{
	RTTI_DECLARE();
public:
	Handle<FunctionTemplate> m_function_template;
	Handle<String> m_name;
public:
	Function(JSContextRef js_ctx);
	Function(JSContextRef js_ctx, JSObjectRef js_object);
	virtual ~Function() {}

public:
	bool _FunctionHasProperty(Handle<String> name);
	Handle<Value> _FunctionGetProperty(Handle<String> name);
	bool _FunctionSetProperty(Handle<String> name, Handle<Value> value, PropertyAttribute attrib);
	bool _FunctionDeleteProperty(Handle<String> name);
	void _FunctionGetPropertyNames(JSPropertyNameAccumulatorRef js_name_accumulator);

public:
	V8EXPORT Local<Value> Call(Handle<Object> that, int argc, Handle<Value> argv[]);

	V8EXPORT Local<Object> NewInstance() /*const*/ { return NewInstance(0, NULL); }
	V8EXPORT Local<Object> NewInstance(int argc, Handle<Value> argv[]) /*const*/;

	V8EXPORT void SetName(Handle<String> name)
	{
		m_name = name;
	}
	V8EXPORT Handle<Value> GetName() const
	{
		return Handle<Value>(m_name);
	}
	V8EXPORT Handle<Value> GetInferredName() const
	{
		TODO(); return Handle<Value>(m_name);
	}

	V8EXPORT int GetScriptLineNumber() const
	{
		TODO(); return 0;
	}
	V8EXPORT int GetScriptColumnNumber() const
	{
		TODO(); return 0;
	}
	V8EXPORT Handle<Value> GetScriptId() const
	{
		TODO(); return Handle<Value>();
	}
	V8EXPORT ScriptOrigin GetScriptOrigin() const
	{
		return ScriptOrigin(GetName(), Integer::New(GetScriptLineNumber()), Integer::New(GetScriptColumnNumber()));
	}

public:
	static JSClassRef sm_js_class;

public:
	V8EXPORT static Function* Cast(Value* value);
	V8EXPORT static const int kLineOffsetNotFound;
public:
	static JSClassRef GetJSClass();
	static JSValueRef _JS_CallAsFunction(JSContextRef js_ctx, JSObjectRef js_function, JSObjectRef js_that, size_t js_argc, const JSValueRef js_argv[], JSValueRef *js_exception);
	static JSObjectRef _JS_CallAsConstructor(JSContextRef js_ctx, JSObjectRef js_function, size_t js_argc, const JSValueRef js_argv[], JSValueRef *js_exception);
};

/// v8::Date

class V8EXPORT Date : public Object
{
	RTTI_DECLARE();
public:
	Date(JSContextRef js_ctx, double time);
	Date(JSContextRef js_ctx, JSObjectRef js_object);
	virtual ~Date() {}
///public:
///	static JSClassRef sm_js_class;
public:
	V8EXPORT static Local<Value> New(double time);
	V8EXPORT static Date* Cast(Value* value);
///public:
///	static JSClassRef GetJSClass();
};

/// v8::RegExp

class V8EXPORT RegExp : public Object
{
	RTTI_DECLARE();
public:
	enum Flags
	{
		kNone = 0,
		kGlobal = 1,
		kIgnoreCase = 2,
		kMultiline = 4
	};
public:
	Local<String> m_source;
	Flags m_flags;
public:
	RegExp(JSContextRef js_ctx, Handle<String> pattern, Flags flags);
	RegExp(JSContextRef js_ctx, JSObjectRef js_object);
	virtual ~RegExp() {}
public:
	V8EXPORT Local<String> GetSource() const
	{
		return m_source;
	}
	V8EXPORT Flags GetFlags() const
	{
		return m_flags;
	}

///public:
///	static JSClassRef sm_js_class;
public:
	V8EXPORT static Local<RegExp> New(Handle<String> pattern, Flags flags);
	V8EXPORT static RegExp* Cast(Value* value);
///public:
///	static JSClassRef GetJSClass();
};

/// v8::BooleanObject

class V8EXPORT BooleanObject : public Object
{
	RTTI_DECLARE();
public:
	BooleanObject(JSContextRef js_ctx, bool value);
	BooleanObject(JSContextRef js_ctx, JSObjectRef js_object);
	virtual ~BooleanObject() {}
public:
	V8EXPORT static Local<Value> New(bool value);
	V8EXPORT static BooleanObject* Cast(Value* value);
};

/// v8::NumberObject

class V8EXPORT NumberObject : public Object
{
	RTTI_DECLARE();
public:
	NumberObject(JSContextRef js_ctx, double value);
	NumberObject(JSContextRef js_ctx, JSObjectRef js_object);
	virtual ~NumberObject() {}
public:
	V8EXPORT static Local<Value> New(double value);
	V8EXPORT static NumberObject* Cast(Value* value);
};

/// v8::StringObject

class V8EXPORT StringObject : public Object
{
	RTTI_DECLARE();
public:
	StringObject(JSContextRef js_ctx, Handle<String> value);
	StringObject(JSContextRef js_ctx, JSObjectRef js_object);
	virtual ~StringObject() {}
public:
	V8EXPORT static Local<Value> New(Handle<String> value);
	V8EXPORT static StringObject* Cast(Value* value);
};

/// v8::External
	
class V8EXPORT External : public Value
{
	RTTI_DECLARE();
public:
	JSObjectRef m_js_object;
	void* m_data;
public:
	V8EXPORT External(JSContextRef js_ctx, void* data);
	V8EXPORT virtual ~External();
public:
	//JSObjectRef GetJSObject() { return m_js_object; }
	void AttachJSContextAndJSObject(JSContextRef js_ctx, JSObjectRef js_object);
	void DetachJSContextAndJSObject();
public:
	V8EXPORT void* Value() const;
public:
	static JSClassRef sm_js_class;
public:
	V8EXPORT static Local<v8::Value> Wrap(void* data);
	V8EXPORT static void* Unwrap(Handle<v8::Value> obj);
	V8EXPORT static Local<External> New(void* value);
	V8EXPORT static External* Cast(v8::Value* value);
public:
	static JSClassRef GetJSClass();
private:
	static void _JS_Initialize(JSContextRef js_ctx, JSObjectRef js_object);
	static void _JS_Finalize(JSObjectRef js_object);
};

/// v8::StackFrame

class V8EXPORT StackFrame : public internal::SmartObject
{
public:
	virtual ~StackFrame() {}
public:
	int GetLineNumber() const;
	int GetColumn() const;
	Local<String> GetScriptName() const;
	Local<String> GetScriptNameOrSourceURL() const;
	Local<String> GetFunctionName() const;
	bool IsEval() const;
	bool IsConstructor() const;
};

/// v8::StackTrace

class V8EXPORT StackTrace : public internal::SmartObject
{
public:
	enum StackTraceOptions
	{
		kLineNumber = 1,
		kColumnOffset = 1 << 1 | kLineNumber,
		kScriptName = 1 << 2,
		kFunctionName = 1 << 3,
		kIsEval = 1 << 4,
		kIsConstructor = 1 << 5,
		kScriptNameOrSourceURL = 1 << 6,
		kOverview = kLineNumber | kColumnOffset | kScriptName | kFunctionName,
		kDetailed = kOverview | kIsEval | kIsConstructor | kScriptNameOrSourceURL
	};

public:
	virtual ~StackTrace() {}
public:
	Local<StackFrame> GetFrame(uint32_t index) const;
	int GetFrameCount() const;
	Local<Array> AsArray();

	static Local<StackTrace> CurrentStackTrace(int frame_limit, StackTraceOptions options = kOverview);
};

/// v8::Message

class V8EXPORT Message : public internal::SmartObject
{
public:
	virtual ~Message() {}
public:
	Local<String> Get() const;
	Local<String> GetSourceLine() const;
	Handle<Value> GetScriptResourceName() const;
	Handle<Value> GetScriptData() const;
	Handle<StackTrace> GetStackTrace() const;
	int GetLineNumber() const;
	int GetStartPosition() const;
	int GetEndPosition() const;
	int GetStartColumn() const;
	int GetEndColumn() const;
public:
	static void PrintCurrentStackTrace(FILE* out);
	static const int kNoLineNumberInfo = 0;
	static const int kNoColumnInfo = 0;
};

/// v8::TryCatch

class V8EXPORT TryCatch
{
public:
	TryCatch();
	~TryCatch();
public:
	bool HasCaught() const;
	bool CanContinue() const;
	Handle<Value> ReThrow();
	Local<Value> Exception() const;
	Local<Value> StackTrace() const;
	Local<Message> Message() const;
	void Reset();
	void SetVerbose(bool value);
	void SetCaptureMessage(bool value);
};

/// v8::Signature

class V8EXPORT Signature : public internal::SmartObject
{
public:
	Signature(Handle<FunctionTemplate> receiver = Handle<FunctionTemplate>(), int argc = 0, Handle<FunctionTemplate> argv[] = 0);
	virtual ~Signature() {}
public:
	static Local<Signature> New(Handle<FunctionTemplate> receiver = Handle<FunctionTemplate>(), int argc = 0, Handle<FunctionTemplate> argv[] = 0);
};

/// v8::AccessorSignature

class V8EXPORT AccessorSignature : public internal::SmartObject
{
public:
	AccessorSignature(Handle<FunctionTemplate> receiver);
	virtual ~AccessorSignature() {}
public:
	static Local<AccessorSignature> New(Handle<FunctionTemplate> receiver = Handle<FunctionTemplate>());
};

/// v8::Template

class V8EXPORT Template : public Data
{
	RTTI_DECLARE();

public:
	internal::TemplatePropertyMap m_template_property_map;

public:
	Template();
	virtual ~Template();

public:
	void Set(Handle<String> name, Handle<Data> value, PropertyAttribute attrib = None);
	void Set(const char* name, Handle<Data> value);

	virtual void ApplyToObject(Handle<Object> object);
};

/// v8::ObjectTemplate

class V8EXPORT ObjectTemplate : public Template
{
	RTTI_DECLARE();

public:
	Handle<ObjectTemplate> m_prototype_template;

	internal::TemplateAccessorMap m_template_accessor_map;

	NamedPropertyGetter m_named_property_getter;
	NamedPropertySetter m_named_property_setter;
	NamedPropertyQuery m_named_property_query;
	NamedPropertyDeleter m_named_property_deleter;
	NamedPropertyEnumerator m_named_property_enumerator;
	Handle<Value> m_named_property_data;

	IndexedPropertyGetter m_indexed_property_getter;
	IndexedPropertySetter m_indexed_property_setter;
	IndexedPropertyQuery m_indexed_property_query;
	IndexedPropertyDeleter m_indexed_property_deleter;
	IndexedPropertyEnumerator m_indexed_property_enumerator;
	Handle<Value> m_indexed_property_data;

	InvocationCallback m_call_as_function_invocation_callback;
	Handle<Value> m_call_as_function_data;

	NamedSecurityCallback m_access_check_named_security_callback;
	IndexedSecurityCallback m_access_check_indexed_security_callback;
	Handle<Value> m_access_check_data;
	bool m_access_check_turned_on_by_default;

	int m_internal_field_count;

	bool m_undetectable;

public:
	ObjectTemplate();
	virtual ~ObjectTemplate();

public:
	bool _ObjectTemplateHasProperty(Handle<Object> object, Handle<String> name);
	Handle<Value> _ObjectTemplateGetProperty(Handle<Object> object, Handle<String> name);
	bool _ObjectTemplateSetProperty(Handle<Object> object, Handle<String> name, Handle<Value> value);
	bool _ObjectTemplateDeleteProperty(Handle<Object> object, Handle<String> name);
	void _ObjectTemplateGetPropertyNames(Handle<Object> object, JSPropertyNameAccumulatorRef js_name_accumulator);

public:
	V8EXPORT Local<Object> NewInstance();
	void SetAccessor(Handle<String> name, AccessorGetter getter, AccessorSetter setter = 0, Handle<Value> data = Handle<Value>(), AccessControl settings = DEFAULT, PropertyAttribute attrib = None, Handle<AccessorSignature> signature = Handle<AccessorSignature>());
	void SetNamedPropertyHandler(NamedPropertyGetter getter, NamedPropertySetter setter = 0, NamedPropertyQuery query = 0, NamedPropertyDeleter deleter = 0, NamedPropertyEnumerator enumerator = 0, Handle<Value> data = Handle<Value>());
	void SetIndexedPropertyHandler(IndexedPropertyGetter getter, IndexedPropertySetter setter = 0, IndexedPropertyQuery query = 0, IndexedPropertyDeleter deleter = 0, IndexedPropertyEnumerator enumerator = 0, Handle<Value> data = Handle<Value>());
	void SetCallAsFunctionHandler(InvocationCallback callback, Handle<Value> data = Handle<Value>());
	void MarkAsUndetectable();
	void SetAccessCheckCallbacks(NamedSecurityCallback named_handler, IndexedSecurityCallback indexed_handler, Handle<Value> data = Handle<Value>(), bool turned_on_by_default = true);
	int InternalFieldCount();
	void SetInternalFieldCount(int value);
	void SetPrototypeTemplate(Handle<ObjectTemplate> prototype_template);

	virtual void ApplyToObject(Handle<Object> object);

public:
	static Local<ObjectTemplate> New();
};

/// v8::FunctionTemplate

class V8EXPORT FunctionTemplate : public Template
{
	RTTI_DECLARE();

public:
	Handle<ObjectTemplate> m_instance_template;

	InvocationCallback m_callback;
	Handle<Value> m_data;
	Handle<Signature> m_signature;

	Handle<String> m_class_name;

	Handle<Function> m_function;

public:
	FunctionTemplate(
	   InvocationCallback callback = 0,
	   Handle<Value> data = Handle<Value>(),
	   Handle<Signature> signature = Handle<Signature>());
	virtual ~FunctionTemplate();

public:
	bool _FunctionTemplateHasProperty(Handle<Function> function, Handle<String> name);
	Handle<Value> _FunctionTemplateGetProperty(Handle<Function> function, Handle<String> name);
	bool _FunctionTemplateSetProperty(Handle<Function> function, Handle<String> name, Handle<Value> value);
	bool _FunctionTemplateDeleteProperty(Handle<Function> function, Handle<String> name);
	void _FunctionTemplateGetPropertyNames(Handle<Function> function, JSPropertyNameAccumulatorRef js_name_accumulator);

public:
	Local<Function> GetFunction();
	void Inherit(Handle<FunctionTemplate> parent);
	Local<ObjectTemplate> InstanceTemplate();
	Local<ObjectTemplate> PrototypeTemplate();
	void SetCallHandler(InvocationCallback callback, Handle<Value> data = Handle<Value>());
	void SetClassName(Handle<String> name) { m_class_name = name; }
	void SetHiddenPrototype(bool value) { TODO(); }
	void ReadOnlyPrototype() { TODO(); }
	bool HasInstance(Handle<Value> object);

public:
	static Local<FunctionTemplate> New(
	   InvocationCallback callback = 0,
	   Handle<Value> data = Handle<Value>(),
	   Handle<Signature> signature = Handle<Signature>());
};

/// v8::Script

class V8EXPORT Script : public internal::SmartObject
{
public:
	Handle<Context> m_context;
	Handle<String> m_source;
	Handle<Value> m_file_name;
	Handle<Integer> m_line_number;
	Handle<String> m_script_data;

public:
	Script(Handle<String> source, Handle<Value> file_name, Handle<Integer> line_unmber, Handle<String> script_data = Handle<String>());
	virtual ~Script() {}

public:
	bool CheckSyntax();
public:
	Local<Value> Run();

	Local<Value> Id() { TODO(); return Local<Value>(); }
	void SetData(Handle<String> data) { m_script_data = data; }

public:
	static Local<Script> New(Handle<String> source, ScriptOrigin* origin = NULL, ScriptData* pre_data = NULL, Handle<String> script_data = Handle<String>());
	static Local<Script> New(Handle<String> source, Handle<Value> file_name);
	static Local<Script> Compile(Handle<String> source, ScriptOrigin* origin = NULL, ScriptData* pre_data = NULL, Handle<String> script_data = Handle<String>());
	static Local<Script> Compile(Handle<String> source, Handle<Value> file_name, Handle<String> script_data = Handle<String>());
};

class V8EXPORT ExtensionConfiguration
{
};

/// v8::Context

class V8EXPORT Context : public internal::SmartObject
{
public:
	class Scope
	{
	private:
		Handle<Context> m_context;
	public:
		explicit Scope(Handle<Context> context) : m_context(context) { m_context->Enter(); }
		~Scope() { m_context->Exit(); }
	private:
		// Prevent copying of Scope objects.
		Scope(const Scope&);
		Scope& operator=(const Scope&);
	};

public:
	Handle<Context> m_previous_context;
	Handle<Object> m_global_object;
	Handle<Object> m_external_global_object;

	JSContextGroupRef m_js_group;
	JSGlobalContextRef m_js_global_ctx;
	std::vector<JSContextRef> m_js_ctx_stack;

public:
	Context();
	virtual ~Context();

public:
	Local<Object> Global();
	void Enter();
	void Exit();

public:
	static JSClassRef sm_js_class;
	static JSContextGroupRef sm_initial_js_group;
	static JSGlobalContextRef sm_initial_js_global_ctx;
	static Persistent<Context> sm_initial_context;
private:
	static Persistent<Context> sm_entered_context;
	static Persistent<Context> sm_current_context;
	static Persistent<Context> sm_calling_context;

public:
	static JSClassRef GetJSClass();
	static JSGlobalContextRef GetCurrentJSGlobalContext();
	static JSContextRef GetCurrentJSContext();
public:
	static Local<Context> GetEntered();
	static Local<Context> GetCurrent();
	static Local<Context> GetCalling();
	static Persistent<Context> New(ExtensionConfiguration* extensions = NULL, Handle<ObjectTemplate> global_template = Handle<ObjectTemplate>(), Handle<Value> global_value = Handle<Value>());
private:
	static bool _JS_HasProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name);
	static JSValueRef _JS_GetProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef *js_exception);
	static bool _JS_SetProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef js_value, JSValueRef *js_exception);
	static bool _JS_DeleteProperty(JSContextRef js_ctx, JSObjectRef js_object, JSStringRef js_name, JSValueRef* js_exception);
	static void _JS_GetPropertyNames(JSContextRef js_ctx, JSObjectRef js_object, JSPropertyNameAccumulatorRef js_names);
};

/// v8::Unlocker

class V8EXPORT Unlocker
{
public:
	explicit Unlocker(Isolate* isolate = NULL) {}
	~Unlocker() {}
private:
	// Disallow copying and assigning.
	Unlocker(const Unlocker&);
	void operator=(const Unlocker&);
};

/// v8::Locker

class V8EXPORT Locker
{
public:
	explicit Locker(Isolate* isolate = NULL) {}
	~Locker() {}
	static void StartPreemption(int every_n_ms);
	static void StopPreemption();
	static bool IsLocked(Isolate* isolate = NULL);
	static bool IsActive();
private:
	// Disallow copying and assigning.
	Locker(const Locker&);
	void operator=(const Locker&);
};

struct HeapStatsUpdate;

class V8EXPORT OutputStream
{
public:
	enum OutputEncoding
	{
		kAscii = 0  // 7-bit ASCII.
	};
	enum WriteResult
	{
		kContinue = 0,
		kAbort = 1
	};
	virtual ~OutputStream() {}
	virtual void EndOfStream() = 0;
	virtual int GetChunkSize() { return 1024; }
	virtual OutputEncoding GetOutputEncoding() { return kAscii; }
	virtual WriteResult WriteAsciiChunk(char* data, int size) = 0;
	virtual WriteResult WriteHeapStatsChunk(HeapStatsUpdate* data, int count)
	{
		return kAbort;
	};
};

class V8EXPORT ActivityControl
{
public:
	enum ControlOption
	{
		kContinue = 0,
		kAbort = 1
	};
	virtual ~ActivityControl() {}
	virtual ControlOption ReportProgressValue(int done, int total) = 0;
};

/// v8

Handle<Primitive> V8EXPORT Undefined();
Handle<Primitive> V8EXPORT Null();
Handle<Boolean> V8EXPORT True();
Handle<Boolean> V8EXPORT False();

/// v8::ResourceConstraints

class V8EXPORT ResourceConstraints
{
public:
	ResourceConstraints();
	int max_young_space_size() const { return max_young_space_size_; }
	void set_max_young_space_size(int value) { max_young_space_size_ = value; }
	int max_old_space_size() const { return max_old_space_size_; }
	void set_max_old_space_size(int value) { max_old_space_size_ = value; }
	int max_executable_size() { return max_executable_size_; }
	void set_max_executable_size(int value) { max_executable_size_ = value; }
	uint32_t* stack_limit() const { return stack_limit_; }
	void set_stack_limit(uint32_t* value) { stack_limit_ = value; }
private:
	int max_young_space_size_;
	int max_old_space_size_;
	int max_executable_size_;
	uint32_t* stack_limit_;
};

/// v8

bool V8EXPORT SetResourceConstraints(ResourceConstraints* constraints);

typedef void (*FatalErrorCallback)(const char* location, const char* message);
typedef void (*MessageCallback)(Handle<Message> message, Handle<Value> error);

Handle<Value> V8EXPORT ThrowException(Handle<Value> exception);

/// v8::Exception

class V8EXPORT Exception
{
public:
	static Local<Value> RangeError(Handle<String> message);
	static Local<Value> ReferenceError(Handle<String> message);
	static Local<Value> SyntaxError(Handle<String> message);
	static Local<Value> TypeError(Handle<String> message);
	static Local<Value> Error(Handle<String> message);
};

typedef int* (*CounterLookupCallback)(const char* name);
typedef void* (*CreateHistogramCallback)(const char* name, int min, int max, size_t buckets);
typedef void (*AddHistogramSampleCallback)(void* histogram, int sample);

enum ObjectSpace
{
	kObjectSpaceNewSpace = 1 << 0,
	kObjectSpaceOldPointerSpace = 1 << 1,
	kObjectSpaceOldDataSpace = 1 << 2,
	kObjectSpaceCodeSpace = 1 << 3,
	kObjectSpaceMapSpace = 1 << 4,
	kObjectSpaceLoSpace = 1 << 5,

	kObjectSpaceAll = kObjectSpaceNewSpace | kObjectSpaceOldPointerSpace |
		kObjectSpaceOldDataSpace | kObjectSpaceCodeSpace | kObjectSpaceMapSpace |
		kObjectSpaceLoSpace
};

enum AllocationAction
{
	kAllocationActionAllocate = 1 << 0,
	kAllocationActionFree = 1 << 1,
	kAllocationActionAll = kAllocationActionAllocate | kAllocationActionFree
};

typedef void (*MemoryAllocationCallback)(ObjectSpace space, AllocationAction action, int size);

typedef void (*CallCompletedCallback)();
typedef void (*FailedAccessCheckCallback)(Local<Object> target, AccessType type, Local<Value> data);
typedef bool (*AllowCodeGenerationFromStringsCallback)(Local<Context> context);

enum GCType
{
	kGCTypeScavenge = 1 << 0,
	kGCTypeMarkSweepCompact = 1 << 1,
	kGCTypeAll = kGCTypeScavenge | kGCTypeMarkSweepCompact
};

enum GCCallbackFlags
{
	kNoGCCallbackFlags = 0,
	kGCCallbackFlagCompacted = 1 << 0
};

typedef void (*GCPrologueCallback)(GCType type, GCCallbackFlags flags);
typedef void (*GCEpilogueCallback)(GCType type, GCCallbackFlags flags);

typedef void (*GCCallback)();

/// v8::HeapStatistics

class V8EXPORT HeapStatistics
{
public:
	HeapStatistics();
	size_t total_heap_size() { return total_heap_size_; }
	size_t total_heap_size_executable() { return total_heap_size_executable_; }
	size_t used_heap_size() { return used_heap_size_; }
	size_t heap_size_limit() { return heap_size_limit_; }

private:
	void set_total_heap_size(size_t size) { total_heap_size_ = size; }
	void set_total_heap_size_executable(size_t size)
	{
		total_heap_size_executable_ = size;
	}
	void set_used_heap_size(size_t size) { used_heap_size_ = size; }
	void set_heap_size_limit(size_t size) { heap_size_limit_ = size; }

	size_t total_heap_size_;
	size_t total_heap_size_executable_;
	size_t used_heap_size_;
	size_t heap_size_limit_;

	friend class V8;
};

class RetainedObjectInfo;

class StartupData
{
public:
	enum CompressionAlgorithm
	{
		kUncompressed,
		kBZip2
	};
	const char* data;
	int compressed_size;
	int raw_size;
};

class V8EXPORT StartupDataDecompressor
{
public:
	StartupDataDecompressor();
	virtual ~StartupDataDecompressor();
	int Decompress();
protected:
	virtual int DecompressData(char* raw_data, int* raw_data_size, const char* compressed_data, int compressed_data_size) = 0;
private:
	char** raw_data;
};

typedef bool (*EntropySource)(unsigned char* buffer, size_t length);

typedef uintptr_t (*ReturnAddressLocationResolver)(uintptr_t return_addr_location);

typedef void (*FunctionEntryHook)(uintptr_t function, uintptr_t return_addr_location);

struct JitCodeEvent
{
	enum EventType
	{
		CODE_ADDED,
		CODE_MOVED,
		CODE_REMOVED
	};

	EventType type;
	void* code_start;
	size_t code_len;

	union
	{
		struct { const char* str; size_t len; } name;
		void* new_code_start;
	};
};

enum JitCodeEventOptions
{
	kJitCodeEventDefault = 0,
	kJitCodeEventEnumExisting = 1
};

typedef void (*JitCodeEventHandler)(const JitCodeEvent* event);

class V8EXPORT ExternalResourceVisitor
{
public:
	virtual ~ExternalResourceVisitor() {}
	virtual void VisitExternalString(Handle<String> string) {}
};

class V8EXPORT PersistentHandleVisitor
{
public:
	virtual ~PersistentHandleVisitor() {}
	virtual void VisitPersistentHandle(Persistent<Value> value, uint16_t class_id) {}
};

/// v8::V8

class V8EXPORT V8
{
public:
	static void SetFatalErrorHandler(FatalErrorCallback that);
	static void SetAllowCodeGenerationFromStringsCallback(AllowCodeGenerationFromStringsCallback that);
	static void IgnoreOutOfMemoryException();
	static bool IsDead();
	static StartupData::CompressionAlgorithm GetCompressedStartupDataAlgorithm();
	static int GetCompressedStartupDataCount();
	static void GetCompressedStartupData(StartupData* compressed_data);
	static void SetDecompressedStartupData(StartupData* decompressed_data);
	static bool AddMessageListener(MessageCallback that, Handle<Value> data = Handle<Value>());
	static void RemoveMessageListeners(MessageCallback that);
	static void SetCaptureStackTraceForUncaughtExceptions(bool capture, int frame_limit = 10, StackTrace::StackTraceOptions options = StackTrace::kOverview);
	static void SetFlagsFromString(const char* str, int length);
	static void SetFlagsFromCommandLine(int* argc, char** argv, bool remove_flags);
	static const char* GetVersion();
	static void SetCounterFunction(CounterLookupCallback);
	static void SetCreateHistogramFunction(CreateHistogramCallback);
	static void SetAddHistogramSampleFunction(AddHistogramSampleCallback);
	static void EnableSlidingStateWindow();
	static void SetFailedAccessCheckCallbackFunction(FailedAccessCheckCallback);
	static void AddGCPrologueCallback(GCPrologueCallback callback, GCType gc_type_filter = kGCTypeAll);
	static void RemoveGCPrologueCallback(GCPrologueCallback callback);
	static void SetGlobalGCPrologueCallback(GCCallback);
	static void AddGCEpilogueCallback(GCEpilogueCallback callback, GCType gc_type_filter = kGCTypeAll);
	static void RemoveGCEpilogueCallback(GCEpilogueCallback callback);
	static void SetGlobalGCEpilogueCallback(GCCallback);
	static void AddMemoryAllocationCallback(MemoryAllocationCallback callback, ObjectSpace space, AllocationAction action);
	static void RemoveMemoryAllocationCallback(MemoryAllocationCallback callback);
	static void AddCallCompletedCallback(CallCompletedCallback callback);
	static void RemoveCallCompletedCallback(CallCompletedCallback callback);
	static void AddObjectGroup(Persistent<Value>* objects, size_t length, RetainedObjectInfo* info = NULL);
	static void AddImplicitReferences(Persistent<Object> parent, Persistent<Value>* children, size_t length);
	static bool Initialize();
	static void SetEntropySource(EntropySource source);
	static void SetReturnAddressLocationResolver(ReturnAddressLocationResolver return_address_resolver);
	static bool SetFunctionEntryHook(FunctionEntryHook entry_hook);
	static void SetJitCodeEventHandler(JitCodeEventOptions options, JitCodeEventHandler event_handler);
	static intptr_t AdjustAmountOfExternalAllocatedMemory(intptr_t change_in_bytes);
	static void PauseProfiler();
	static void ResumeProfiler();
	static bool IsProfilerPaused();
	static int GetCurrentThreadId();
	static void TerminateExecution(int thread_id);
	static void TerminateExecution(Isolate* isolate = NULL);
	static bool IsExecutionTerminating(Isolate* isolate = NULL);
	static bool Dispose();
	static void GetHeapStatistics(HeapStatistics* heap_statistics);
	static void VisitExternalResources(ExternalResourceVisitor* visitor);
	static void VisitHandlesWithClassIds(PersistentHandleVisitor* visitor);
	static bool IdleNotification(int hint = 1000);
	static void LowMemoryNotification();
	static int ContextDisposedNotification();
};

} // namespace v8

#endif // V8LIKE_H_

