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

#ifndef V8LIKE_PROFILER_H_
#define V8LIKE_PROFILER_H_

#include "v8like.h"

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
#endif

#else  // _WIN32

#if defined(__GNUC__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)) && defined(V8_SHARED)
#define V8EXPORT __attribute__ ((visibility("default")))
#else
#define V8EXPORT
#endif

#endif  // _WIN32

namespace v8 {

typedef uint32_t SnapshotObjectId;

class V8EXPORT CpuProfileNode
{
public:
	Handle<String> GetFunctionName() const;
	Handle<String> GetScriptResourceName() const;
	int GetLineNumber() const;
	double GetTotalTime() const;
	double GetSelfTime() const;
	double GetTotalSamplesCount() const;
	double GetSelfSamplesCount() const;
	unsigned GetCallUid() const;
	int GetChildrenCount() const;
	const CpuProfileNode* GetChild(int index) const;
	static const int kNoLineNumberInfo = Message::kNoLineNumberInfo;
};

class V8EXPORT CpuProfile
{
public:
	unsigned GetUid() const;
	Handle<String> GetTitle() const;
	const CpuProfileNode* GetBottomUpRoot() const;
	const CpuProfileNode* GetTopDownRoot() const;
	void Delete();
};

class V8EXPORT CpuProfiler
{
public:
	static int GetProfilesCount();
	static const CpuProfile* GetProfile(int index, Handle<Value> security_token = Handle<Value>());
	static const CpuProfile* FindProfile(unsigned uid, Handle<Value> security_token = Handle<Value>());
	static void StartProfiling(Handle<String> title);
	static const CpuProfile* StopProfiling(Handle<String> title, Handle<Value> security_token = Handle<Value>());
	static void DeleteAllProfiles();
};

class HeapGraphNode;

class V8EXPORT HeapGraphEdge
{
public:
	enum Type
	{
		kContextVariable = 0,
		kElement = 1,
		kProperty = 2,
		kInternal = 3,
		kHidden = 4,
		kShortcut = 5,
		kWeak = 6
	};
	Type GetType() const;
	Handle<Value> GetName() const;
	const HeapGraphNode* GetFromNode() const;
	const HeapGraphNode* GetToNode() const;
};

class V8EXPORT HeapGraphNode
{
public:
	enum Type
	{
		kHidden = 0,
		kArray = 1,
		kString = 2,
		kObject = 3,
		kCode = 4,
		kClosure = 5,
		kRegExp = 6,
		kHeapNumber = 7,
		kNative = 8,
		kSynthetic = 9
	};
	Type GetType() const;
	Handle<String> GetName() const;
	SnapshotObjectId GetId() const;
	int GetSelfSize() const;
	int GetChildrenCount() const;
	const HeapGraphEdge* GetChild(int index) const;
	Handle<Value> GetHeapValue() const;
};

class V8EXPORT HeapSnapshot
{
public:
	enum Type
	{
		kFull = 0
	};
	enum SerializationFormat
	{
		kJSON = 0
	};
	Type GetType() const;
	unsigned GetUid() const;
	Handle<String> GetTitle() const;
	const HeapGraphNode* GetRoot() const;
	const HeapGraphNode* GetNodeById(SnapshotObjectId id) const;
	int GetNodesCount() const;
	const HeapGraphNode* GetNode(int index) const;
	SnapshotObjectId GetMaxSnapshotJSObjectId() const;
	void Delete();
	void Serialize(OutputStream* stream, SerializationFormat format) const;
};

class RetainedObjectInfo;

class V8EXPORT HeapProfiler
{
public:
	typedef RetainedObjectInfo* (*WrapperInfoCallback)(uint16_t class_id, Handle<Value> wrapper);
	static int GetSnapshotsCount();
	static const HeapSnapshot* GetSnapshot(int index);
	static const HeapSnapshot* FindSnapshot(unsigned uid);
	static SnapshotObjectId GetSnapshotObjectId(Handle<Value> value);
	static const SnapshotObjectId kUnknownObjectId = 0;
	static const HeapSnapshot* TakeSnapshot(Handle<String> title, HeapSnapshot::Type type = HeapSnapshot::kFull, ActivityControl* control = NULL);
	static void StartHeapObjectsTracking();
	static SnapshotObjectId PushHeapObjectsStats(OutputStream* stream);
	static void StopHeapObjectsTracking();
	static void DeleteAllSnapshots();
	static void DefineWrapperClass(uint16_t class_id, WrapperInfoCallback callback);
	static const uint16_t kPersistentHandleNoClassId = 0;
	static int GetPersistentHandleCount();
	static size_t GetMemorySizeUsedByProfiler();
};

class V8EXPORT RetainedObjectInfo
{
public:
	virtual void Dispose() = 0;
	virtual bool IsEquivalent(RetainedObjectInfo* other) = 0;
	virtual intptr_t GetHash() = 0;
	virtual const char* GetLabel() = 0;
	virtual const char* GetGroupLabel() { return GetLabel(); }
	virtual intptr_t GetElementCount() { return -1; }
	virtual intptr_t GetSizeInBytes() { return -1; }
protected:
	RetainedObjectInfo() {}
	virtual ~RetainedObjectInfo() {}
private:
	RetainedObjectInfo(const RetainedObjectInfo&);
	RetainedObjectInfo& operator=(const RetainedObjectInfo&);
};

struct HeapStatsUpdate
{
	HeapStatsUpdate(uint32_t index, uint32_t count, uint32_t size) :
		index(index), count(count), size(size) {}
	uint32_t index;
	uint32_t count;
	uint32_t size;
};

} // namespace v8

#undef V8EXPORT

#endif // V8LIKE_PROFILER_H_

