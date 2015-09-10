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

// A macro used to make better inlining. Don't bother for debug builds.
// Use like:
//   V8_INLINE int GetZero() { return 0; }
#if !defined(DEBUG) && V8_HAS_ATTRIBUTE_ALWAYS_INLINE
# define V8_INLINE inline __attribute__((always_inline))
#elif !defined(DEBUG) && V8_HAS___FORCEINLINE
# define V8_INLINE __forceinline
#else
# define V8_INLINE inline
#endif

#include "v8stdint.h"

#include "assert.h"

#include <vector>
#include <map>
#include <memory>

#define V8EXPORT

// A macro to mark classes or functions as deprecated.
#if defined(V8_DEPRECATION_WARNINGS) && V8_HAS_ATTRIBUTE_DEPRECATED_MESSAGE
# define V8_DEPRECATED(message, declarator) \
declarator __attribute__((deprecated(message)))
#elif defined(V8_DEPRECATION_WARNINGS) && V8_HAS_ATTRIBUTE_DEPRECATED
# define V8_DEPRECATED(message, declarator) \
declarator __attribute__((deprecated))
#elif defined(V8_DEPRECATION_WARNINGS) && V8_HAS_DECLSPEC_DEPRECATED
# define V8_DEPRECATED(message, declarator) __declspec(deprecated) declarator
#else
# define V8_DEPRECATED(message, declarator) declarator
#endif

namespace v8 {

#define TODO() printf("%s:%d TODO: %s\n", __FILE__, __LINE__, __FUNCTION__)

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
class Value;
class HeapStatistics;
class Platform;

namespace internal {

	//added 6/6
	const int kApiPointerSize = sizeof(void*);  // NOLINT
	const int kApiIntSize = sizeof(int);  // NOLINT
	const int kApiInt64Size = sizeof(int64_t);  // NOLINT

	// Tag information for HeapObject.
	const int kHeapObjectTag = 1;
	const int kHeapObjectTagSize = 2;
	const intptr_t kHeapObjectTagMask = (1 << kHeapObjectTagSize) - 1;

	// Tag information for Smi.
	const int kSmiTag = 0;
	const int kSmiTagSize = 1;
	const intptr_t kSmiTagMask = (1 << kSmiTagSize) - 1;

	template <size_t ptr_size> struct SmiTagging;

	class Internals {
	public:
		// These values match non-compiler-dependent values defined within
		// the implementation of v8.
		static const int kHeapObjectMapOffset = 0;
		static const int kMapInstanceTypeAndBitFieldOffset =
			1 * kApiPointerSize + kApiIntSize;
		static const int kStringResourceOffset = 3 * kApiPointerSize;

		static const int kOddballKindOffset = 3 * kApiPointerSize;
		static const int kForeignAddressOffset = kApiPointerSize;
		static const int kJSObjectHeaderSize = 3 * kApiPointerSize;
		static const int kFixedArrayHeaderSize = 2 * kApiPointerSize;
		static const int kContextHeaderSize = 2 * kApiPointerSize;
		static const int kContextEmbedderDataIndex = 95;
		static const int kFullStringRepresentationMask = 0x07;
		static const int kStringEncodingMask = 0x4;
		static const int kExternalTwoByteRepresentationTag = 0x02;
		static const int kExternalAsciiRepresentationTag = 0x06;

		static const int kIsolateEmbedderDataOffset = 0 * kApiPointerSize;
		static const int kAmountOfExternalAllocatedMemoryOffset =
			4 * kApiPointerSize;
		static const int kAmountOfExternalAllocatedMemoryAtLastGlobalGCOffset =
			kAmountOfExternalAllocatedMemoryOffset + kApiInt64Size;
		static const int kIsolateRootsOffset =
			kAmountOfExternalAllocatedMemoryAtLastGlobalGCOffset + kApiInt64Size +
			kApiPointerSize;
		static const int kUndefinedValueRootIndex = 5;
		static const int kNullValueRootIndex = 7;
		static const int kTrueValueRootIndex = 8;
		static const int kFalseValueRootIndex = 9;
		static const int kEmptyStringRootIndex = 164;

		// The external allocation limit should be below 256 MB on all architectures
		// to avoid that resource-constrained embedders run low on memory.
		static const int kExternalAllocationLimit = 192 * 1024 * 1024;

		static const int kNodeClassIdOffset = 1 * kApiPointerSize;
		static const int kNodeFlagsOffset = 1 * kApiPointerSize + 3;
		static const int kNodeStateMask = 0xf;
		static const int kNodeStateIsWeakValue = 2;
		static const int kNodeStateIsPendingValue = 3;
		static const int kNodeStateIsNearDeathValue = 4;
		static const int kNodeIsIndependentShift = 4;
		static const int kNodeIsPartiallyDependentShift = 5;

		static const int kJSObjectType = 0xbc;
		static const int kFirstNonstringType = 0x80;
		static const int kOddballType = 0x83;
		static const int kForeignType = 0x88;

		static const int kUndefinedOddballKind = 5;
		static const int kNullOddballKind = 3;

		static const uint32_t kNumIsolateDataSlots = 4;

	};


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
	virtual void _MakeWeak(void* parameter, void * callback) {}
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

protected:
	void setPtr(T* ptr)
	{
		if (m_ptr != ptr)
		{
			if (m_ptr) { m_ptr->DecRef(); }
			m_ptr = ptr;
			if (m_ptr) { m_ptr->IncRef(); }
		}
	}
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
		setPtr(ptr);
		return *this;
	}
	SmartPointer& operator=(const SmartPointer& other)
	{
		setPtr(other.m_ptr);
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
	void Clear(){ _Clear(); }
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


class HeapProfiler;
class CpuProfiler;

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

typedef void(*GCPrologueCallback)(Isolate* isolate,GCType type, GCCallbackFlags flags);
typedef void(*GCEpilogueCallback)(Isolate* isolate,GCType type, GCCallbackFlags flags);

typedef void(*GCCallback)();

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
	HeapProfiler* heapProfiler;
	CpuProfiler* cpuProfiler;
	void* embedder_data_[internal::Internals::kNumIsolateDataSlots];
private:
	Isolate();
	~Isolate();

public:
	typedef bool (*abort_on_uncaught_exception_t)();
	void SetAbortOnUncaughtException(abort_on_uncaught_exception_t callback);
	void GetHeapStatistics(HeapStatistics* heap_statistics);
	void AddGCPrologueCallback(
		GCPrologueCallback callback, GCType gc_type_filter = kGCTypeAll);
	void AddGCEpilogueCallback(
		GCEpilogueCallback callback, GCType gc_type_filter = kGCTypeAll);
	void RemoveGCPrologueCallback(GCPrologueCallback callback);
	void RemoveGCEpilogueCallback(GCEpilogueCallback callback);
	Local<Value> ThrowException(Local<Value> exception);
	void Enter();
	void Exit();
	Local<Context> GetCurrentContext();
	void Dispose();
	HeapProfiler* GetHeapProfiler();
	CpuProfiler* GetCpuProfiler();

	void RunMicrotasks();
	void SetAutorunMicrotasks(bool autorun);

	//void SetData(void* data) { m_data = data; }
	//void* GetData() { return m_data; }

	void SetData(uint32_t slot, void* data) {
		if (slot < internal::Internals::kNumIsolateDataSlots)
			embedder_data_[slot] = data;
	}
	void* GetData(uint32_t slot) {
		if (slot < internal::Internals::kNumIsolateDataSlots)
			return embedder_data_[slot];
		return nullptr;
	}
	V8_INLINE int64_t
		AdjustAmountOfExternalAllocatedMemory(int64_t change_in_bytes)
	{
			return 0;
	}
    
    bool IdleNotification(int hint)
    {
        return true;
    }
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
	template <class S> Handle(const Local<S>& other) : internal::SmartPointer<T>(reinterpret_cast<T*>(other.GetPointer())) { TYPE_CHECK(T, S); }
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
	Local(const Handle<T>& other) : Handle<T>(other) {}
	template <class S> Local(const Local<S>& other) : Handle<T>(reinterpret_cast<T*>(other.GetPointer())) { TYPE_CHECK(T, S); }
	template <class S> Local(const Handle<S>& other) : Handle<T>(reinterpret_cast<T*>(other.GetPointer())) { TYPE_CHECK(T, S); }
	virtual ~Local() {}

	template <class S> inline Local<S> As() { return Local<S>::Cast(*this); }

public:
	static Local<T> New(Handle<T> that) { return Local<T>(that); }
	static Local<T> New(Isolate* isolate, Handle<T> that) { return New(that); }
	template <class S> static inline Local<T> Cast(Local<S> that) { return Local<T>(T::Cast(*that)); }
};

/// v8::Persistent<T>

/// WeakCallbackData
template<class T, class P>
class WeakCallbackData {
public:
	typedef void(*Callback)(const WeakCallbackData<T, P>& data);

	V8_INLINE Isolate* GetIsolate() const { return isolate_; }
	V8_INLINE Local<T> GetValue() const { return handle_; }
	V8_INLINE P* GetParameter() const { return parameter_; }

	WeakCallbackData(Isolate* isolate, Local<T> handle, P* parameter)
		: isolate_(isolate), handle_(handle), parameter_(parameter) { }
	Isolate* isolate_;
	Local<T> handle_;
	P* parameter_;
};

typedef WeakCallbackData<v8::Value, void>::Callback WeakCallback;

typedef WeakCallback WeakReferenceCallback;

template <class T> class Persistent : public Handle<T>
{
public:
	Persistent(T* ptr = (T*) 0) : Handle<T>(ptr) {}
	Persistent(const Persistent<T>& other) : Handle<T>(other) {}
	Persistent(Isolate* isolate,const Handle<T>& other) : Handle<T>(other) {}
	template <class S> Persistent(const Persistent<S>& other) : Handle<T>(reinterpret_cast<T*>(other.GetPointer())) { TYPE_CHECK(T, S); }
	virtual ~Persistent() {}

	template <class S> inline Persistent<S> As() { return Persistent<S>::Cast(*this); }

	void Dispose()
	{
		internal::SmartPointer<T>::_Clear();
	}
	template <class S>
	void Reset(Isolate* isolate, const Handle<S>& other) {
		TYPE_CHECK(T, S);
		if (other.IsEmpty()) return;
        internal::SmartPointer<T>::setPtr(other.GetPointer());
	}
	void Clear() { internal::SmartPointer<T>::_Clear(); }
	void Reset() {
		if (this->IsEmpty()) return;
		internal::SmartPointer<T>::_Clear();
	}
	
	void MakeWeak(void* parameter, WeakReferenceCallback callback) { internal::SmartPointer<T>::GetPointer()->_MakeWeak(parameter, (void*)callback); }

	template<typename P>
	V8_INLINE void SetWeak(
		P* parameter,
		typename WeakCallbackData<T, P>::Callback callback)
	{
		MakeWeak((void*)parameter, (WeakReferenceCallback)callback);
	}
//	V8_INLINE void SetWeak(
//		void* parameter,
//		WeakReferenceCallback callback)
//	{
//		MakeWeak(parameter, callback);
//	}
	void ClearWeak() { internal::SmartPointer<T>::GetPointer()->_ClearWeak(); }
    
    
    template<typename P>
    V8_INLINE P* ClearWeak()
    {
        P* p = (P*)internal::SmartPointer<T>::GetPointer()->m_weak_parameter;
        internal::SmartPointer<T>::GetPointer()->_ClearWeak();
        return p;
    }
    
    
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










template<class T>
class UniquePersistent : public Persistent<T> {
	struct RValue {
		V8_INLINE explicit RValue(UniquePersistent* obj) : object(obj) {}
		UniquePersistent* object;
	};

public:
	/**
	* A UniquePersistent with no storage cell.
	*/
	V8_INLINE UniquePersistent() : Persistent<T>(0) { }
	/**
	* Construct a UniquePersistent from a Handle.
	* When the Handle is non-empty, a new storage cell is created
	* pointing to the same object, and no flags are set.
	*/
	template <class S>
	V8_INLINE UniquePersistent(Isolate* isolate, Handle<S> that)
		: Persistent<T>(Persistent<T>::New(isolate, *that)) {
			TYPE_CHECK(T, S);
		}
	/**
	* Construct a UniquePersistent from a PersistentBase.
	* When the Persistent is non-empty, a new storage cell is created
	* pointing to the same object, and no flags are set.
	*/
	template <class S>
	V8_INLINE UniquePersistent(Isolate* isolate, const Persistent<S>& that)
		: Persistent<T>(Persistent<T>::New(isolate, that.m_ptr)) {
			TYPE_CHECK(T, S);
		}
	/**
	* Move constructor.
	*/
	V8_INLINE UniquePersistent(RValue rvalue)
		: Persistent<T>(rvalue.object->m_ptr) {
			rvalue.object->m_ptr = 0;
		}
	V8_INLINE ~UniquePersistent() { this->Reset(); }
	/**
	* Move via assignment.
	*/
	template<class S>
	V8_INLINE UniquePersistent& operator=(UniquePersistent<S> rhs) {
		TYPE_CHECK(T, S);
		this->Reset();
		this->m_ptr = rhs.m_ptr;
		rhs.m_ptr = 0;
		return *this;
	}
	/**
	* Cast operator for moves.
	*/
	V8_INLINE operator RValue() { return RValue(this); }
	/**
	* Pass allows returning uniques from functions, etc.
	*/
	UniquePersistent Pass() { return UniquePersistent(RValue(this)); }

private:
	UniquePersistent(const UniquePersistent&);
	void operator=(UniquePersistent&);
};




/// v8::HandleScope

class HandleScope
{
public:
	HandleScope(Isolate *isolate = 0) {}
	~HandleScope() {}

	template <class T> Local<T> Close(Handle<T> value) { return Local<T>(value); }
};

class EscapableHandleScope : public HandleScope
{
public:
	EscapableHandleScope(Isolate* isolate){}
	EscapableHandleScope() {}
	~EscapableHandleScope() {}

	template <class T> Local<T> Escape(Handle<T> value) { return Local<T>(value); }
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
	virtual void _MakeWeak(void* parameter, void*);
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
	virtual bool IsTypedArray();
	virtual bool IsArrayBuffer();

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


	V8EXPORT static Value* Cast(v8::Value* value);
};

/// v8::Primitive

class V8EXPORT Primitive : public Value
{
	RTTI_DECLARE();
public:
	V8EXPORT Primitive() {}
	V8EXPORT Primitive(const Value & other):Value(other) {}
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
	V8EXPORT static Handle<Boolean> New(Isolate* isolate, bool value)
	{
		return New(value);
	}
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
	V8EXPORT static Local<Number> New(Isolate* isolate, double value)
	{
		return New(value);
	}
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
	V8EXPORT static Local<Integer> New(v8::Isolate* isolate, int32_t value)
	{
		return New(value);
	}
	V8EXPORT static Local<Integer> New(int32_t value);
	V8EXPORT static Local<Integer> NewFromUnsigned(uint32_t value);
	V8EXPORT static Local<Integer> NewFromUnsigned(Isolate* isolate, uint32_t value)
	{
		return New(value);
	}
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
	V8EXPORT String(const Primitive & other) :Primitive(other){};
	V8EXPORT virtual ~String();

public:
	V8EXPORT int Length();
	V8EXPORT int Utf8Length();
	V8EXPORT bool MayContainNonAscii() const;

	V8EXPORT int Write(uint16_t* buffer, int start = 0, int length = -1, int options = NO_OPTIONS) const;
	V8EXPORT int WriteAscii(char* buffer, int start = 0, int length = -1, int options = NO_OPTIONS) const;

	// One byte characters.
	int WriteOneByte(uint8_t* buffer,
		int start = 0,
		int length = -1,
		int options = NO_OPTIONS) const
	{
		return WriteAscii((char*)buffer, start, length, options);
	}
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
	V8EXPORT static Local<String> Empty(Isolate* isolate = nullptr);
    V8EXPORT static String* Cast(v8::Value* value);

public:
	V8EXPORT static Local<String> New(const char* data, int length = -1);
	V8EXPORT static Local<String> New(const uint16_t* data, int length = -1);
	V8EXPORT static Local<String> NewSymbol(const char* data, int length = -1);
	V8EXPORT static Local<String> Concat(Handle<String> left, Handle<String> right);
	V8EXPORT static Local<String> NewExternal(Isolate* isolate, ExternalStringResource* resource);
	V8EXPORT static Local<String> NewExternal(Isolate* isolate, ExternalAsciiStringResource* resource);
	V8EXPORT static Local<String> NewExternal(ExternalStringResource* resource);
	V8EXPORT static Local<String> NewExternal(ExternalAsciiStringResource* resource);
	V8EXPORT static Local<String> NewUndetectable(const char* data, int length = -1);
	V8EXPORT static Local<String> NewUndetectable(const uint16_t* data, int length = -1);


	enum NewStringType {
		kNormalString, kInternalizedString, kUndetectableString
	};
	/** Allocates a new string from UTF-8 data.*/
	static Local<String> NewFromUtf8(Isolate* isolate,
		const char* data,
		NewStringType type = kNormalString,
		int length = -1)
	{
		return New(data, length);
	}

	/** Allocates a new string from Latin-1 data.*/
	static Local<String> NewFromOneByte(
		Isolate* isolate,
		const uint8_t* data,
		NewStringType type = kNormalString,
		int length = -1)
	{
		return New((const char*)data, length);
	}

	/** Allocates a new string from UTF-16 data.*/
	static Local<String> NewFromTwoByte(
		Isolate* isolate,
		const uint16_t* data,
		NewStringType type = kNormalString,
		int length = -1)
	{
		return New(data, length);
	}

};
/// v8

Local<Primitive> V8EXPORT Undefined(v8::Isolate* isolate = 0);
Local<Primitive> V8EXPORT Null(v8::Isolate* isolate = 0);
Local<Boolean> V8EXPORT True(v8::Isolate* isolate = 0);
Local<Boolean> V8EXPORT False(v8::Isolate* isolate = 0);

template<typename T>
class ReturnValue {
public:
	class SmartValue
	{
	public:
		SmartValue() 
			:_val(0)
		{
			
		}
		SmartValue(T * val)
			: _val(val)
		{
			retain();
		}
		~SmartValue()
		{
			release();
		}
		T *get()const
		{
			return _val;
		}
		void set(T* val)
		{
			release();
			_val = val;
			retain();
		}
	private:
		T *_val;
		void release()
		{
			if (_val)
			{
				static_cast<Data*>(_val)->DecRef();
			}
		}
		void retain()
		{
			if (_val)
			{
				static_cast<Data*>(_val)->IncRef();
			}
		}
	};


	ReturnValue()
		: value_(new std::shared_ptr<SmartValue>())
	{
		initDefaultValue();
	}
	~ReturnValue()
	{
	}


	template <class S> ReturnValue(const ReturnValue<S>& that)
		: value_(that.value_) {
		TYPE_CHECK(T, S);
		if (!(*value_).get())
		{
			initDefaultValue();
		}
	}

	ReturnValue(const ReturnValue<T>& that)
		: value_(that.value_) {
		if (!(*value_).get())
		{
			initDefaultValue();
		}
	}

	// Handle setters
	template <typename S> void Set(const Persistent<S>& handle){
		TYPE_CHECK(T, S);
		T *v = static_cast<T*>(handle.GetPointer());
		(*value_).reset(new SmartValue(v));
	}

	template <typename S> void Set(const Local<S> handle) {
		TYPE_CHECK(T, S);
		T *v = static_cast<T*>(handle.GetPointer());
		(*value_).reset(new SmartValue(v));
	}

	template <typename S> void Set(const Handle<S> handle) {
		TYPE_CHECK(T, S);
		T *v = static_cast<T*>(handle.GetPointer());
		(*value_).reset(new SmartValue(v));
	}
	// Fast primitive setters
	void Set(bool value) {
		TYPE_CHECK(T, Boolean);
		Set(Boolean::New(value));
	}

	void Set(double i){
		TYPE_CHECK(T, Number);
		Set(Number::New(i));
	}

	void Set(int32_t i){
		TYPE_CHECK(T, Int32);
		Set(Int32::New(i));
	}
	void Set(uint32_t i){
		TYPE_CHECK(T, Uint32);
		Set(Uint32::NewFromUnsigned(i));
	}
	// Fast JS primitive setters
	void SetNull(){
		TYPE_CHECK(T, Primitive);
		Set(Null());
	}

	void SetUndefined(){
		TYPE_CHECK(T, Primitive);
		Set(Undefined());
	}

	void SetEmptyString(){
		TYPE_CHECK(T, String);
		Set(String::Empty());
	}

	Local<T> getValue()
	{
		SmartValue * v = value_.get()->get();
		return (v && v->get()) ? v->get() : _defaultValue.get();
	}

	void initDefaultValue()
	{
		_defaultValue.set(static_cast<T*>(Undefined().GetPointer()));
	}

	void SetDefaultValue(T* val)
	{
		_defaultValue.set(val);
	}

	// Convenience getter for Isolate
	//	Isolate* GetIsolate(){
	//		// Isolate is always the pointer below the default value on the stack.
	//		return *reinterpret_cast<Isolate**>(&value_[-2]);
	//	}

	// Pointer setter: Uncompilable to prevent inadvertent misuse.
	template <typename S>
	void Set(S* whatever){
		// Uncompilable to prevent inadvertent misuse.
		TYPE_CHECK(S*, Primitive);
	}

private:
	std::shared_ptr< std::shared_ptr<SmartValue> > value_;
	SmartValue _defaultValue;
};


template<typename T>
class PropertyCallbackInfo {
	Isolate* m_isolate;
	Handle<Object> m_that;
	Handle<Object> m_holder;
	Handle<Value> m_data;
	ReturnValue<T> m_return_value;
	typename ReturnValue<T>::SmartValue defaultRetValue;
public:

	V8_INLINE Isolate* GetIsolate() const{
		return m_isolate;
	}
	V8_INLINE Local<Value> Data() const{
		return m_data;
	}
	V8_INLINE Local<Object> This() const{
		return m_that;
	}
	V8_INLINE Local<Object> Holder() const{
		return m_holder;
	}
	V8_INLINE ReturnValue<T> GetReturnValue() const{
		return m_return_value;
	}
	// This shouldn't be public, but the arm compiler needs it.
	static const int kArgsLength = 6;
	V8EXPORT PropertyCallbackInfo(Isolate* isolate, Handle<Object> that, Handle<Object> holder, Handle<Value> data)
	{
		m_isolate = isolate;
		m_that = that;
		m_holder = holder;
		m_data = data;
	}
//	V8_INLINE PropertyCallbackInfo(void** args)
//	{
//		args_ = args;
//		m_holder = Local<Object>(reinterpret_cast<Object*>(&args[kHolderIndex]));
//		m_isolate = *reinterpret_cast<Isolate**>(&args[kIsolateIndex]);
////		defaultRetValue.set(reinterpret_cast<T*>(&args[kReturnValueDefaultValueIndex]));
//		m_return_value = ReturnValue<T>(&args[kReturnValueIndex]);
//		m_data = Local<Value>(reinterpret_cast<Value*>(&args[kDataIndex]));
//		m_that = Local<Object>(reinterpret_cast<Object*>(&args[kThisIndex]));
//
//		m_return_value.SetDefaultValue(reinterpret_cast<T*>(&args[kReturnValueDefaultValueIndex]));
//	}
protected:
	static const int kHolderIndex = 0;
	static const int kIsolateIndex = 1;
	static const int kReturnValueDefaultValueIndex = 2;
	static const int kReturnValueIndex = 3;
	static const int kDataIndex = 4;
	static const int kThisIndex = 5;

	void ** args_;
};

/// v8::AccessorInfo
/*
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
*/ 

typedef PropertyCallbackInfo<Value> AccessorInfo;
///

typedef void (*NamedPropertyGetter)(Local<String> property, const AccessorInfo& info);
typedef void (*NamedPropertySetter)(Local<String> property, Local<Value> value, const AccessorInfo& info);
typedef void (*NamedPropertyQuery)(Local<String> property, const PropertyCallbackInfo<Integer>& info);
typedef void(*NamedPropertyDeleter)(Local<String> property, const PropertyCallbackInfo<Boolean>& info);
typedef void(*NamedPropertyEnumerator)(const PropertyCallbackInfo<Array>& info);

typedef void (*IndexedPropertyGetter)(uint32_t index, const AccessorInfo& info);
typedef void (*IndexedPropertySetter)(uint32_t index, Local<Value> value, const AccessorInfo& info);
typedef void (*IndexedPropertyQuery)(uint32_t index, const AccessorInfo& info);
typedef void (*IndexedPropertyDeleter)(uint32_t index, const AccessorInfo& info);
typedef void (*IndexedPropertyEnumerator)(const AccessorInfo& info);

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
	kExternalInt8Array = 1,
	kExternalUint8Array,
	kExternalInt16Array,
	kExternalUint16Array,
	kExternalInt32Array,
	kExternalUint32Array,
	kExternalFloat32Array,
	kExternalFloat64Array,
	kExternalUint8ClampedArray,

	// Legacy constant names
	kExternalByteArray = kExternalInt8Array,
	kExternalUnsignedByteArray = kExternalUint8Array,
	kExternalShortArray = kExternalInt16Array,
	kExternalUnsignedShortArray = kExternalUint16Array,
	kExternalIntArray = kExternalInt32Array,
	kExternalUnsignedIntArray = kExternalUint32Array,
	kExternalFloatArray = kExternalFloat32Array,
	kExternalDoubleArray = kExternalFloat64Array,
	kExternalPixelArray = kExternalUint8ClampedArray
};

//typedef Handle<Value> (*AccessorGetter)(Local<String> property, const AccessorInfo& info);
typedef void (*AccessorGetter)(Local<String> property, const AccessorInfo& info);
typedef void(*AccessorSetter)(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info);

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
		static void WeakFree(const WeakCallbackData<Value, internal::Helper>& data);
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
	V8EXPORT void* GetAlignedPointerFromInternalField(int index)
	{
		return GetPointerFromInternalField(index);
	}

	V8EXPORT void SetPointerInInternalField(int index, void* value);
	V8EXPORT void SetAlignedPointerInInternalField(int index, void* value)
	{
		SetPointerInInternalField(index, value);
	}

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
	V8EXPORT Local<Value> CallAsFunction(Handle<Value> that, int argc, Handle<Value> argv[]);
	//V8EXPORT Local<Value> CallAsConstructor(int argc, Handle<Value> argv[]);

public:
	static JSClassRef sm_js_class;

public:
	V8EXPORT static Local<Object> New(Isolate* isolate = 0);
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
	V8EXPORT static Local<Array> New(v8::Isolate* isolate, int length = 0)
	{
		return New(length);
	}
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

/**/
#ifndef V8_ARRAY_BUFFER_INTERNAL_FIELD_COUNT
// The number of required internal fields can be defined by embedder.
#define V8_ARRAY_BUFFER_INTERNAL_FIELD_COUNT 2
#endif

class V8EXPORT ArrayBuffer : public Primitive {
public:

	class V8EXPORT Allocator { // NOLINT
	public:
		virtual ~Allocator() {}

		virtual void* Allocate(size_t length) = 0;

		virtual void* AllocateUninitialized(size_t length) = 0;
	
		virtual void Free(void* data, size_t length) = 0;
	};

	class V8EXPORT Contents { // NOLINT
	public:
		Contents() : data_(NULL), byte_length_(0), isOwnData(false) {}
		~Contents();

		void* Data() const { return data_; }
		size_t ByteLength() const { return byte_length_; }

	private:
		void setData(size_t byte_length, void* data = nullptr);
		void* data_;
		size_t byte_length_;
		bool isOwnData;
		friend class ArrayBuffer;
	};

	size_t ByteLength() const;

	static Local<ArrayBuffer> New(Isolate* isolate, size_t byte_length);

	static Local<ArrayBuffer> New(Isolate* isolate, void* data,
		size_t byte_length);

	bool IsExternal() const;

	void Neuter(){}

	Contents Externalize();

	static ArrayBuffer* Cast(Value* obj);

	static const int kInternalFieldCount = V8_ARRAY_BUFFER_INTERNAL_FIELD_COUNT;

	virtual bool IsArrayBuffer() override;
private:
	Contents content;
	ArrayBuffer(){};
	static void CheckCast(Value* obj){}
};




/// v8::Arguments

//template<typename T>
///**
//* The argument information given to function call callbacks.  This
//* class provides access to information about the context of the call,
//* including the receiver, the number and values of arguments, and
//* the holder of the function.
//*/
//template<typename T>
//class FunctionCallbackInfo {
//public:
//	Isolate* m_isolate;
//	Handle<Function> m_callee;
//	Handle<Object> m_that;
//	Handle<Object> m_holder;
//	Handle<Value> m_data;
//	int m_argc;
//	Handle<Value>* m_argv;
//	bool m_is_construct_call;
//
//	ReturnValue<T> m_returnValue;
//public:
//	FunctionCallbackInfo(Isolate* isolate, Handle<Function> callee, Handle<Object> that, Handle<Object> holder, Handle<Value> data, int argc, Handle<Value>* argv, bool is_construct_call) :
//		m_isolate(isolate), m_callee(callee), m_that(that), m_holder(holder), m_data(data), m_argc(argc), m_argv(argv), m_is_construct_call(is_construct_call) {}
//	~FunctionCallbackInfo()
//	{
//		m_isolate = NULL;
//		m_argc = 0;
//		m_argv = NULL;
//		m_is_construct_call = false;
//	}
//	int Length() const
//	{
//		return m_argc;
//	}
//	Local<Value> operator[](int i) const
//	{
//		if ((0 <= i) && (i < Length()))
//		{
//			return Local<Value>(m_argv[i]);
//		}
//		return Local<Value>(Undefined());
//	}
//	Local<Function> Callee() const
//	{
//		return Local<Function>(m_callee);
//	}
//	Local<Object> This() const
//	{
//		return Local<Object>(m_that);
//	}
//	Local<Object> Holder() const
//	{
//		return Local<Object>(m_holder);
//	}
//
//	bool IsConstructCall() const
//	{
//		return m_is_construct_call;
//	}
//
//	Local<Value> Data() const
//	{
//		return Local<Value>(m_data);
//	}
//
//	Isolate* GetIsolate() const
//	{
//		return m_isolate;
//	}
//	ReturnValue<T> GetReturnValue() const
//	{
//		return m_returnValue;
//	}
//}





template<typename T>
class V8EXPORT FunctionCallbackInfo
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
	Isolate* GetIsolate() const
	{
		return m_isolate;
	}
	int Length() const
	{
		return m_argc;
	}
	Local<Value> operator[](int i) const
	{
		if ((0 <= i) && (i < Length()))
		{
			return Local<Value>(m_argv[i]);
		}
		return Local<Value>(Undefined());
	}
	Local<Function> Callee() const
	{
		return Local<Function>(m_callee);
	}
	Local<Object> This() const
	{
		return Local<Object>(m_that);
	}
	Local<Object> Holder() const
	{
		return Local<Object>(m_holder);
	}
	bool IsConstructCall() const
	{
		return m_is_construct_call;
	}
	Local<Value> Data() const
	{
		return Local<Value>(m_data);
	}
public:
	ReturnValue<T> m_returnValue;
public:
	V8EXPORT FunctionCallbackInfo(Isolate* isolate, Handle<Function> callee, Handle<Object> that, Handle<Object> holder, Handle<Value> data, int argc, Handle<Value>* argv, bool is_construct_call) :
		m_isolate(isolate), m_callee(callee), m_that(that), m_holder(holder), m_data(data), m_argc(argc), m_argv(argv), m_is_construct_call(is_construct_call)
	{
		if (m_is_construct_call)
		{
			m_returnValue.Set(holder);
		}
	}
	~FunctionCallbackInfo()
	{
		m_isolate = NULL;
		m_argc = 0;
		m_argv = NULL;
		m_is_construct_call = false;
	}

public:
	ReturnValue<T> GetReturnValue() const
	{
		return m_returnValue;
	}
};
/*
class V8EXPORT Arguments : FunctionCallbackInfo<Value>
{
public:
	V8EXPORT Arguments(Isolate* isolate, Handle<Function> callee, Handle<Object> that, Handle<Object> holder, Handle<Value> data, int argc, Handle<Value>* argv, bool is_construct_call);
	~Arguments();
};
*/
typedef FunctionCallbackInfo<Value> Arguments;

//template<typename T>
//using FunctionCallbackInfo = Arguments<T>;

/// v8::Function

//typedef Handle<Value> (*InvocationCallback)(const Arguments& args);

typedef void(*FunctionCallback)(const FunctionCallbackInfo<Value>& info);
typedef void(*InvocationCallback)(const Arguments& info);


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
	V8EXPORT Local<Value> Call(Handle<Value> that, int argc, Handle<Value> argv[]);

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
	V8EXPORT static Local<External> New(Isolate* isolate, void* value)
	{
		return New(value);
	}
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
    static Local<StackTrace> CurrentStackTrace(Isolate *iso , int frame_limit, StackTraceOptions options = kOverview)
    {
        return CurrentStackTrace(frame_limit , options);
    }
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
	bool HasTerminated() const;
};

/// v8::Signature

class V8EXPORT Signature : public internal::SmartObject
{
public:
	Signature(Handle<FunctionTemplate> receiver = Handle<FunctionTemplate>(), int argc = 0, Handle<FunctionTemplate> argv[] = 0);
	virtual ~Signature() {}
public:
	static Local<Signature> New(Handle<FunctionTemplate> receiver = Handle<FunctionTemplate>(), int argc = 0, Handle<FunctionTemplate> argv[] = 0);
	static Local<Signature> New(Isolate *iso, Handle<FunctionTemplate> receiver = Handle<FunctionTemplate>(), int argc = 0, Handle<FunctionTemplate> argv[] = 0)
	{
		return New(receiver , argc , argv);
	}
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
	void Set(Isolate* isolate, Handle<String> name, Handle<Data> value, PropertyAttribute attrib = None)
	{
		Set(name, value, attrib);
	}
	void Set(const char* name, Handle<Data> value);
	void Set(Isolate* isolate, const char* name, Handle<Data> value)
	{
		Set(name, value);
	}

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
	static Local<ObjectTemplate> New(Isolate* isolate = 0);
};

/// v8::FunctionTemplate

class V8EXPORT FunctionTemplate : public Template
{
	RTTI_DECLARE();

public:
	Handle<ObjectTemplate> m_instance_template;
	Handle<FunctionTemplate> m_inherit_template;

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
	void _ApplyMemberFunctionToObject(Handle<Object> prototype);

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

	static Local<FunctionTemplate> New(
		Isolate* isolate,
		FunctionCallback callback = 0,
		Handle<Value> data = Handle<Value>(),
		Handle<Signature> signature = Handle<Signature>(),
		int length = 0);
	///** Creates a function template.*/
	//static Local<FunctionTemplate> New(
	//	Isolate* isolate,
	//	FunctionCallback callback = 0,
	//	Handle<Value> data = Handle<Value>(),
	//	Handle<Signature> signature = Handle<Signature>(),
	//	int length = 0)
	//{
	//	return New(callback, data, signature, length);
	//}
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

	void bindContext();
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
	void** embedder_data_;
	int _length;
	Handle<Value> token;
	void resetEmbedderData(int length)
	{
		length++;
		if (length > _length)
		{
			if (_length * 2 > length)
				_length = _length * 2;
			else
				_length = length;
			embedder_data_ = (void**)(realloc(embedder_data_, _length * sizeof(void*)));
		}
	}

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
	V8_INLINE Isolate* GetIsolate() const { return Isolate::GetCurrent(); }

	void SetAlignedPointerInEmbedderData(int index, void* value) {
		resetEmbedderData(index);
		embedder_data_[index] = value;
	}

	V8_INLINE void* GetAlignedPointerFromEmbedderData(int index)
	{
		return embedder_data_[index];
	}

	void SetSecurityToken(Handle<Value> _token)
	{
		token = _token;
	}

	void UseDefaultSecurityToken()
	{
		token = Null();
	}

	Handle<Value> GetSecurityToken()
	{
		return token;
	}
	/*
	void SetData(uint32_t slot, void* data) {
		if (slot < internal::Internals::kNumIsolateDataSlots)
			embedder_data_[slot] = data;
	}
	void* GetData(uint32_t slot) {
		if (slot < internal::Internals::kNumIsolateDataSlots)
			return embedder_data_[slot];
		return nullptr;
	}
	*/
public:
	Local<Object> Global();
	void Enter();
	void Exit();

public:
	static JSClassRef sm_js_class;
	static JSContextGroupRef sm_initial_js_group;
	static JSGlobalContextRef sm_initial_js_global_ctx;
	static std::map<JSContextRef, JSObjectRef> sm_js_global_object_map;
	static Persistent<Context> sm_initial_context;
private:
	static Persistent<Context> sm_entered_context;
	static Persistent<Context> sm_current_context;
	static Persistent<Context> sm_calling_context;

public:
	static JSClassRef GetJSClass();
	static JSGlobalContextRef GetCurrentJSGlobalContext();
	static JSContextRef GetCurrentJSContext();
	static JSObjectRef GetJSGlobalObject(JSContextRef js_ctx, JSObjectRef js_object = NULL);
public:
	static Local<Context> GetEntered();
	static Local<Context> GetCurrent();
	static Local<Context> GetCalling();
	static Persistent<Context> New(ExtensionConfiguration* extensions = NULL, Handle<ObjectTemplate> global_template = Handle<ObjectTemplate>(), Handle<Value> global_value = Handle<Value>());
	static Persistent<Context> New(Isolate* isolate,ExtensionConfiguration* extensions = NULL, Handle<ObjectTemplate> global_template = Handle<ObjectTemplate>(), Handle<Value> global_value = Handle<Value>())
	{
		return New(extensions, global_template, global_value);
	};
private:
	static void _JS_Initialize(JSContextRef js_ctx, JSObjectRef js_object);
	static void _JS_Finalize(JSObjectRef js_object);
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


class V8EXPORT UnboundScript: public Data
{
	Local<Script> script;
public:
	UnboundScript(Handle<String> source, ScriptOrigin* origin = NULL, ScriptData* pre_data = NULL, Handle<String> script_data = Handle<String>())
	{
		script = Script::New(source, origin, pre_data, script_data);
	}

	/**
	* Binds the script to the currently entered context.
	*/
	Local<Script> BindToCurrentContext()
	{
		script->bindContext();
		return script;
	}

	int GetId()
	{
		return script->Id()->ToInteger()->Value();
	}

	Handle<Value> GetScriptName()
	{
		return script->m_file_name;
	}

	/**
	* Data read from magic sourceURL comments.
	*/
	Handle<Value> GetSourceURL()
	{
		return script->m_source;
	}
	/**
	* Data read from magic sourceMappingURL comments.
	*/
	Handle<Value> GetSourceMappingURL()
	{
		return script->m_source;
	}

	/**
	* Returns zero based line number of the code_pos location in the script.
	* -1 will be returned if no information available.
	*/
	int GetLineNumber(int code_pos)
	{
		return script->m_line_number->Value();
	}

	static const int kNoScriptId = 0;
};

class V8EXPORT ScriptCompiler
{
	struct V8EXPORT CachedData {
		enum BufferPolicy {
			BufferNotOwned,
			BufferOwned
		};

		CachedData() : data(NULL), length(0), buffer_policy(BufferNotOwned) {}

		// If buffer_policy is BufferNotOwned, the caller keeps the ownership of
		// data and guarantees that it stays alive until the CachedData object is
		// destroyed. If the policy is BufferOwned, the given data will be deleted
		// (with delete[]) when the CachedData object is destroyed.
		CachedData(const uint8_t* data, int length,
			BufferPolicy buffer_policy = BufferNotOwned) : data(NULL),
			length(0),
			buffer_policy(BufferNotOwned) {}
		~CachedData() {
			if (buffer_policy == BufferOwned) {
				delete[] data;
			}
		}
		// TODO(marja): Async compilation; add constructors which take a callback
		// which will be called when V8 no longer needs the data.
		const uint8_t* data;
		int length;
		BufferPolicy buffer_policy;

	private:
		// Prevent copying. Not implemented.
		CachedData(const CachedData&);
		CachedData& operator=(const CachedData&);
	};
public:
	class Source {
	public:
		// Source takes ownership of CachedData.
		V8_INLINE Source(Local<String> source_string, const ScriptOrigin& origin,
			CachedData* _cached_data = NULL) :source_string(source_string),
			resource_name(origin.ResourceName()),
			resource_line_offset(origin.ResourceLineOffset()),
			resource_column_offset(origin.ResourceColumnOffset()),
			cached_data(_cached_data) {}
		V8_INLINE Source(Local<String> source_string,
			CachedData* cached_data = NULL) : source_string(source_string), cached_data(cached_data) {}
		V8_INLINE ~Source() {
			delete cached_data;
		}

		// Ownership of the CachedData or its buffers is *not* transferred to the
		// caller. The CachedData object is alive as long as the Source object is
		// alive.
		V8_INLINE const CachedData* GetCachedData() const {
			return cached_data;
		}

	private:
		friend class ScriptCompiler;
		// Prevent copying. Not implemented.
		Source(const Source&);
		Source& operator=(const Source&);

		Local<String> source_string;

		// Origin information
		Handle<Value> resource_name;
		Handle<Integer> resource_line_offset;
		Handle<Integer> resource_column_offset;
		Handle<Boolean> resource_is_shared_cross_origin;

		// Cached data from previous compilation (if a kConsume*Cache flag is
		// set), or hold newly generated cache data (kProduce*Cache flags) are
		// set when calling a compile method.
		CachedData* cached_data;
	};

	enum CompileOptions {
		kNoCompileOptions = 0,
		kProduceParserCache,
		kConsumeParserCache,
		kProduceCodeCache,
		kConsumeCodeCache,

		// Support the previous API for a transition period.
		kProduceDataToCache
	};

	static Local<UnboundScript> CompileUnbound(
		Isolate* isolate, Source* source,
		CompileOptions options = kNoCompileOptions)
	{
		ScriptOrigin org(source->resource_name, source->resource_line_offset, source->resource_column_offset);
		return Local<UnboundScript>(new UnboundScript(source->source_string, &org));
	}

	static Local<Script> Compile(
		Isolate* isolate, Source* source,
		CompileOptions options = kNoCompileOptions)
	{
		ScriptOrigin org(source->resource_name,source->resource_line_offset,source->resource_column_offset);
		return Script::Compile(source->source_string, &org);
	}
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
	static Local<String> RangeError(Handle<String> message);
	static Local<String> ReferenceError(Handle<String> message);
	static Local<String> SyntaxError(Handle<String> message);
	static Local<String> TypeError(Handle<String> message);
	static Local<String> Error(Handle<String> message);
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

/// v8::HeapStatistics

class V8EXPORT HeapStatistics
{
public:
	HeapStatistics();
	size_t total_heap_size() { return total_heap_size_; }
	size_t total_heap_size_executable() { return total_heap_size_executable_; }
	size_t used_heap_size() { return used_heap_size_; }
	size_t heap_size_limit() { return heap_size_limit_; }
	size_t total_physical_size() { return total_physical_size_; }

private:
	void set_total_heap_size(size_t size) { total_heap_size_ = size; }
	void set_total_heap_size_executable(size_t size)
	{
		total_heap_size_executable_ = size;
	}
	void set_used_heap_size(size_t size) { used_heap_size_ = size; }
	void set_heap_size_limit(size_t size) { heap_size_limit_ = size; }
	void set_total_physical_size(size_t size) { total_physical_size_ = size; }

	size_t total_heap_size_;
	size_t total_heap_size_executable_;
	size_t used_heap_size_;
	size_t heap_size_limit_;
	size_t total_physical_size_;

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
	static void SetArrayBufferAllocator(ArrayBuffer::Allocator* allocator);
	static void CancelTerminateExecution(Isolate* isolate);
	static void InitializePlatform(Platform* platform);
	static void ShutdownPlatform();
	static void Eternalize(Isolate* isolate,
		Value* handle,
		int* index);
	static Local<Value> GetEternal(Isolate* isolate, int index);
private:
	static int eternalIndex;
	static std::vector<Persistent<Value>> eternalVec;
};

template <class T> class Eternal {
public:
	V8_INLINE Eternal() : index_(kInitialValue) { }
	template<class S>
	V8_INLINE Eternal(Isolate* isolate, Local<S> handle) : index_(kInitialValue) {
		Set(isolate, handle);
	}
	// Can only be safely called if already set.
	V8_INLINE Local<T> Get(Isolate* isolate){
		return Local<T>(reinterpret_cast<T*>(*V8::GetEternal(isolate, index_)));
	}
	V8_INLINE bool IsEmpty() { return index_ == kInitialValue; }
	template<class S> V8_INLINE void Set(Isolate* isolate, Local<S> handle)
	{
		TYPE_CHECK(T, S);
		V8::Eternalize(isolate, reinterpret_cast<Value*>(*handle), &this->index_);
	}

private:
	static const int kInitialValue = -1;
	int index_;
};
} // namespace v8

#endif // V8LIKE_H_

#include "v8like-platform.h"