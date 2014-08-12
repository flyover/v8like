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

#ifndef V8LIKE_DEBUG_H_
#define V8LIKE_DEBUG_H_

#include "v8like.h"

#ifdef _WIN32

typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;  // NOLINT
typedef long long int64_t;  // NOLINT

#if defined(BUILDING_V8_SHARED) && defined(USING_V8_SHARED)
#error both BUILDING_V8_SHARED and USING_V8_SHARED are set
#endif

#ifdef BUILDING_V8_SHARED
#define EXPORT __declspec(dllexport)
#elif USING_V8_SHARED
#define EXPORT __declspec(dllimport)
#else
#define EXPORT
#endif

#else  // _WIN32

#if defined(__GNUC__) && (__GNUC__ >= 4) && defined(V8_SHARED)
#define EXPORT __attribute__ ((visibility("default")))
#else
#define EXPORT
#endif

#endif  // _WIN32

namespace v8 {

enum DebugEvent
{
	Break = 1,
	Exception = 2,
	NewFunction = 3,
	BeforeCompile = 4,
	AfterCompile  = 5,
	ScriptCollected = 6,
	BreakForCommand = 7
};

class EXPORT Debug
{
public:
	class ClientData
	{
	public:
		virtual ~ClientData() {}
	};

	class Message
	{
	public:
		virtual bool IsEvent() const = 0;
		virtual bool IsResponse() const = 0;
		virtual DebugEvent GetEvent() const = 0;
		virtual bool WillStartRunning() const = 0;
		virtual Handle<Object> GetExecutionState() const = 0;
		virtual Handle<Object> GetEventData() const = 0;
		virtual Handle<String> GetJSON() const = 0;
		virtual Handle<Context> GetEventContext() const = 0;
		virtual ClientData* GetClientData() const = 0;
		virtual ~Message() {}
	};

	class EventDetails {
	public:
		virtual DebugEvent GetEvent() const = 0;
		virtual Handle<Object> GetExecutionState() const = 0;
		virtual Handle<Object> GetEventData() const = 0;
		virtual Handle<Context> GetEventContext() const = 0;
		virtual Handle<Value> GetCallbackData() const = 0;
		virtual ClientData* GetClientData() const = 0;
		virtual ~EventDetails() {}
	};

	typedef void (*EventCallback)(DebugEvent event, Handle<Object> exec_state, Handle<Object> event_data, Handle<Value> data);
	typedef void (*EventCallback2)(const EventDetails& event_details);
	typedef void (*MessageHandler)(const uint16_t* message, int length, ClientData* client_data);
	typedef void (*MessageHandler2)(const Message& message);
	typedef void (*HostDispatchHandler)();
	typedef void (*DebugMessageDispatchHandler)();
	static bool SetDebugEventListener(EventCallback that, Handle<Value> data = Handle<Value>());
	static bool SetDebugEventListener2(EventCallback2 that, Handle<Value> data = Handle<Value>());
	static bool SetDebugEventListener(v8::Handle<v8::Object> that, Handle<Value> data = Handle<Value>());
	static void DebugBreak(Isolate* isolate = NULL);
	static void CancelDebugBreak(Isolate* isolate = NULL);
	static void DebugBreakForCommand(ClientData* data = NULL, Isolate* isolate = NULL);
	static void SetMessageHandler(MessageHandler handler, bool message_handler_thread = false);
	static void SetMessageHandler2(MessageHandler2 handler);
	static void SendCommand(const uint16_t* command, int length, ClientData* client_data = NULL, Isolate* isolate = NULL);
	static void SetHostDispatchHandler(HostDispatchHandler handler, int period = 100);
	static void SetDebugMessageDispatchHandler(DebugMessageDispatchHandler handler, bool provide_locker = false);
	static Local<Value> Call(v8::Handle<v8::Function> fun, Handle<Value> data = Handle<Value>());
	static Local<Value> GetMirror(v8::Handle<v8::Value> obj);
	static bool EnableAgent(const char* name, int port, bool wait_for_connection = false);
	static void DisableAgent();
	static void ProcessDebugMessages();
	static Local<Context> GetDebugContext();
	static void SetLiveEditEnabled(bool enable, Isolate* isolate = NULL);
};

}  // namespace v8

#undef EXPORT

#endif  // V8LIKE_DEBUG_H_

