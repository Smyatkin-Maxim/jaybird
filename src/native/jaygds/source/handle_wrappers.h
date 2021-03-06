/*
 * Firebird Open Source J2ee connector - jdbc driver
 *
 * Distributable under LGPL license.
 * You may obtain a copy of the License at http://www.gnu.org/copyleft/lgpl.html
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * LGPL License for more details.
 *
 * This file was created by members of the firebird development team.
 * All individual contributions remain the Copyright (C) of those
 * individuals.  Contributors to this file are either listed here or
 * can be obtained from a CVS history command.
 *
 * All rights reserved.
 */

#ifndef _JNGDS__HandleWrappers
#define _JNGDS__HandleWrappers

#include "jni_helpers.h"

#include "ibase.h"
#include "jni.h"

// Event API definitions
#define EVENT_UNINITIALIZED 0
#define EVENT_ACTIVE 1
#define EVENT_CANCELLED 2

class JIscDatabaseHandle
	{
	public:

	JIscDatabaseHandle( JNIEnv* jEnv, jobject handle );

	JIscDatabaseHandle( JNIEnv* jEnv );
	
	virtual ~JIscDatabaseHandle();

	void SetHandleValue( isc_db_handle handle );
	
	isc_db_handle GetHandleValue();

	void AddWarning( jthrowable warning );

	static void	Initilize( JNIEnv* jEnv );
		
	private:
	JNIEnv* mJavaEnvironment;
	jobject mJavaObjectHandle;

	// static
	static JClassBinding  sClassBinding;
	static JMethodBinding sMethodBinding_GetHandle;
	static JMethodBinding sMethodBinding_SetHandle;
	static JMethodBinding sMethodBinding_AddWarning;
	static bool	sIsInitilized;
	};


class JIscTransactionHandle
	{
	public:

	JIscTransactionHandle( JNIEnv* jEnv, jobject handle );

	JIscTransactionHandle( JNIEnv* jEnv );
	
	virtual ~JIscTransactionHandle();

	void SetHandleValue( isc_tr_handle handle );
	
	isc_tr_handle GetHandleValue();

	void AddWarning( jthrowable warning );
	
	static void	Initilize( JNIEnv* jEnv );

	private:
	JNIEnv* mJavaEnvironment;
	jobject mJavaObjectHandle;

	// static
	static JClassBinding sClassBinding;
	static JMethodBinding sMethodBinding_GetHandle;
	static JMethodBinding sMethodBinding_SetHandle;
	static JMethodBinding sMethodBinding_AddWarning;
	static bool sIsInitilized;
	};

class JIscStatementHandle
	{
	public:
	
	JIscStatementHandle( JNIEnv* jEnv, jobject handle );

	JIscStatementHandle( JNIEnv* jEnv );
	
	virtual ~JIscStatementHandle();

	void SetHandleValue( isc_stmt_handle handle );
	
	isc_stmt_handle GetHandleValue();

	static void Initilize( JNIEnv* jEnv );

	void AddWarning( jthrowable warning );

	private:
	
	JNIEnv* mJavaEnvironment;
	jobject mJavaObjectHandle;

	// static
	static JClassBinding  sClassBinding;
	static JMethodBinding sMethodBinding_GetHandle;
	static JMethodBinding sMethodBinding_SetHandle;
	static JMethodBinding sMethodBinding_AddWarning;
	static bool		sIsInitilized;
	};

class JIscBlobHandle
	{
	public:
	
	JIscBlobHandle( JNIEnv* jEnv, jobject handle );

	JIscBlobHandle( JNIEnv* jEnv );
	
	virtual ~JIscBlobHandle();

	void SetHandleValue( isc_blob_handle handle );
	
	isc_blob_handle GetHandleValue();

	void SetId( ISC_QUAD handle );
	
	ISC_QUAD GetId();

	void SetIsEndOfFile( bool isEnd);

	void AddWarning( jthrowable warning );

	static void Initilize( JNIEnv* jEnv );
		
	private:

	/* 
	 * The primary purpose of this method is to ensure that the byte ordering in the jlong
	 * is the same across all platforms - the java code may set this into the sqldata field
	 * on an XSQLDAVar structure so it must always be LSB first.
	 */
	jlong GetJLongFromIscQuad(ISC_QUAD value);

	/* 
	 * The inverse of the above method.
	 */
	ISC_QUAD GetIscQuadFromJavaLong(jlong value);

	bool IsLittleEndianByteOrdering();

	JNIEnv* mJavaEnvironment;
	jobject mJavaObjectHandle;

	// static
	static JClassBinding sClassBinding;
	static JMethodBinding sMethodBinding_GetHandle;
	static JMethodBinding sMethodBinding_SetHandle;
	static JMethodBinding sMethodBinding_GetId;
	static JMethodBinding sMethodBinding_SetId;
	static JFieldBinding sFieldBinding_IsEof;
	static JMethodBinding sMethodBinding_AddWarning;
	static bool sIsInitilized;
	};

class JIscServiceHandle
	{
	public:
	
	JIscServiceHandle( JNIEnv* jEnv, jobject handle );

	JIscServiceHandle( JNIEnv* jEnv );
	
	virtual ~JIscServiceHandle();

	void SetHandleValue( isc_svc_handle handle );

	void AddWarning( jthrowable warning );
	
	isc_svc_handle GetHandleValue();

	static void Initilize( JNIEnv* jEnv );

	private:
	JNIEnv* mJavaEnvironment;
	jobject mJavaObjectHandle;

	// static
	static JClassBinding  sClassBinding;
	static JMethodBinding sMethodBinding_GetHandle;
	static JMethodBinding sMethodBinding_SetHandle;
	static JMethodBinding sMethodBinding_AddWarning;
	static bool sIsInitilized;
	};

class JEventHandle
	{
	public:
	
	JEventHandle( JNIEnv* jEnv, jobject handle );

	virtual ~JEventHandle();

    void SetSize(int size);

    int GetSize();

    void SetEventCount(int count);

    void SetEventId(int eventId);

    int GetEventId();

    int GetEventStructHandle();

    void SetEventStructHandle(int handle);
        
	static void	Initialize( JNIEnv* jEnv );
	
	private:
	JNIEnv* mJavaEnvironment;
	jobject mJavaObjectHandle;

	// static
	static JClassBinding  sClassBinding;
    static JMethodBinding sMethodBinding_SetSize;
    static JMethodBinding sMethodBinding_GetSize;
    static JMethodBinding sMethodBinding_SetEventCount;
    static JMethodBinding sMethodBinding_SetEventId;
    static JMethodBinding sMethodBinding_GetEventId;
    static JMethodBinding sMethodBinding_GetEventStructHandle;
    static JMethodBinding sMethodBinding_SetEventStructHandle;
	static bool sIsInitialized;
	};

class JEventHandler
	{
	public:
	
	JEventHandler( JNIEnv* jEnv, jobject handler );

	virtual ~JEventHandler();

    void EventOccurred();

	static void Initialize( JNIEnv* jEnv );

	private:
	JNIEnv* mJavaEnvironment;
	jobject mJavaObjectHandle;

	// static
	static JClassBinding  sClassBinding;
	static JMethodBinding sMethodBinding_EventOccurred;
	static bool		sIsInitialized;
	};

struct event_struct {
    jobject handler;
    jobject eventHandle;
    int state;
	char* eventBuffer;
	char* resultBuffer;
};

class EventStructManager
	{
	public:

	EventStructManager();

	~EventStructManager();

	long addEventStruct();

	event_struct* getEventStruct(long position);

	void releaseEventStruct(long position);

	private:

	void grow();

	event_struct** eventStructPtr;
	long size;
	long lastPosition;
	long increment;
	};

#endif
