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

#include "platform.h"

#include "entrypoints_custom.h"
#include "entrypoints_generated.h"

#include "fb_binding.h"
#include "fb_helpers.h"
#include "handle_wrappers.h"
#include "jni_helpers.h"
#include "xsqlda_wrapper.h"

#include "interface_manager.h"

#include "ibase.h"

#include "jni.h"

// Dll Entrypoints

// First some basic helper functions for error handling and a macro to use for the
// catch block in each JNI entrypoint.

// Must be initialized in Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_nativeInitilize
JClassBinding  sInternalErrorClassBinding;
JClassBinding  sOutOfMemoryErrorClassBinding;
JFieldBinding  isc_api_handle;
InterfaceManager interfaceManager;
EventStructManager eventStructManager;

JNIEXPORT void EnsureJavaExceptionIssued(JNIEnv * javaEnvironment, InternalException& exception)
    {
    if( javaEnvironment->ExceptionCheck() == false ) 
        {
        JString messageJString(javaEnvironment, exception.getMessage());

        javaEnvironment->Throw( (jthrowable)sInternalErrorClassBinding.CreateNewInstance(javaEnvironment, "(Ljava/lang/String;)V", messageJString.AsJString()) );
        }
    }

JNIEXPORT void EnsureJavaExceptionIssued(JNIEnv * javaEnvironment)
    {
    if( javaEnvironment->ExceptionCheck() == false ) 
        {
        JString messageJString(javaEnvironment, "Unexpected exception caught.");

        javaEnvironment->Throw( (jthrowable)sInternalErrorClassBinding.CreateNewInstance(javaEnvironment, "(Ljava/lang/String;)V", messageJString.AsJString()) );
        }
    }

JNIEXPORT void MaybeIssueOutOfMemory(JNIEnv * javaEnvironment, std::bad_alloc& badAlloc)
    {
    if( javaEnvironment->ExceptionCheck() == false ) 
        {
        javaEnvironment->Throw( (jthrowable)sOutOfMemoryErrorClassBinding.CreateNewInstance(javaEnvironment, "()V") );
        }
    }

#define ENTER_PROTECTED_BLOCK try {
                                 
#define LEAVE_PROTECTED_BLOCK   }                                                               \
                                    catch(std::bad_alloc& badAlloc)                                 \
                                        {                                                           \
                                        MaybeIssueOutOfMemory(javaEnvironment, badAlloc);           \
                                        }                                                           \
                                    catch(InternalException& exception)                             \
                                        {                                                           \
                                        EnsureJavaExceptionIssued( javaEnvironment, exception );    \
                                        }                                                           \
                                    catch( ... )                                                    \
                                        {                                                           \
                                        EnsureJavaExceptionIssued( javaEnvironment );               \
                                        } 

// A hack to ensure that nativeInitilize can be called multiple times
// until a client library is located.
bool sHasMostInitilizationBeenDone = false;
static JavaVM *jvm;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
        jvm = vm;
        JNIEnv *javaEnvironment;
        if(vm->GetEnv((void**)&javaEnvironment,JNI_VERSION_1_2)!=JNI_OK)
            return JNI_EVERSION;
        ENTER_PROTECTED_BLOCK
            // Todo : If these fail then the exception handling for this method will not work.
            sInternalErrorClassBinding    = JClassBinding( javaEnvironment, "org/firebirdsql/gds/impl/jni/InternalError" );
            sOutOfMemoryErrorClassBinding = JClassBinding( javaEnvironment, "java/lang/OutOfMemoryError" );
    
            JIscDatabaseHandle::Initilize(javaEnvironment);
            JIscTransactionHandle::Initilize(javaEnvironment);
            JIscStatementHandle::Initilize(javaEnvironment);
            JIscBlobHandle::Initilize(javaEnvironment);
            JIscServiceHandle::Initilize(javaEnvironment);
			JEventHandle::Initialize(javaEnvironment);
            JEventHandler::Initialize(javaEnvironment);
            JXSqlda::Initilize(javaEnvironment);
            FirebirdStatusVector::Initilize(javaEnvironment);

            JClassBinding classBinding(javaEnvironment, "org/firebirdsql/gds/impl/jni/JniGDSImpl" );
            isc_api_handle= classBinding.GetFieldBinding(javaEnvironment,"isc_api_handle","I");
            sHasMostInitilizationBeenDone = true;
        LEAVE_PROTECTED_BLOCK
    return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_nativeInitilize
  (JNIEnv *javaEnvironment, jobject jThis, jstring firebirdDllName)
    {
    ENTER_PROTECTED_BLOCK
        JString fileName( javaEnvironment, firebirdDllName );
        jint isc_api_handle=interfaceManager.LoadInterface(fileName.AsCString());
        ::isc_api_handle.SetInt(javaEnvironment,jThis,isc_api_handle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1create_1database
  (JNIEnv * javaEnvironment, jobject jThis, jbyteArray jFileName, jobject jDatabaseHandle, jbyteArray jDpb)
    {
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JByteArray dpb( javaEnvironment, jDpb );
        JByteArray fileName(javaEnvironment, jFileName);

        FirebirdStatusVector status;
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();

        CALL_API(isc_create_database)( status.RawAccess(), 0, fileName.Read(), &rawDatabaseHandle, dpb.Size(), dpb.Read(), SQL_DIALECT_V6 );

        databaseHandle.SetHandleValue( rawDatabaseHandle );

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1attach_1database
  (JNIEnv * javaEnvironment, jobject jThis, jbyteArray jFileName, jobject jDatabaseHandle, jbyteArray jDpb)
    {
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JByteArray dpb( javaEnvironment, jDpb );
        JByteArray fileName(javaEnvironment, jFileName);

        FirebirdStatusVector status;
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();

        CALL_API(isc_attach_database)( status.RawAccess(), 0, fileName.Read(), &rawDatabaseHandle, dpb.Size(), dpb.Read() );

        databaseHandle.SetHandleValue( rawDatabaseHandle );

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1database_1info
  (JNIEnv * javaEnvironment, jobject jThis, jobject jDatabaseHandle, jint jItemLength, jbyteArray jItems, jint jBufferLength, jbyteArray jBuffer)
    {
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JByteArray items( javaEnvironment, jItems );
        JByteArray buffer( javaEnvironment, jBuffer );

        FirebirdStatusVector status;
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();

        CALL_API(isc_database_info)( status.RawAccess(), &rawDatabaseHandle, (short)jItemLength, items.Read(), (short)jBufferLength, buffer.Read() );

        databaseHandle.SetHandleValue( rawDatabaseHandle );

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1detach_1database
  (JNIEnv * javaEnvironment, jobject jThis, jobject jDatabaseHandle)
    {
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);

        FirebirdStatusVector status;
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();

        CALL_API(isc_detach_database)( status.RawAccess(), &rawDatabaseHandle );

        databaseHandle.SetHandleValue( rawDatabaseHandle );

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1drop_1database
  (JNIEnv * javaEnvironment, jobject jThis, jobject jDatabaseHandle)
    {
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);

        FirebirdStatusVector status;
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();

        CALL_API(isc_drop_database)( status.RawAccess(), &rawDatabaseHandle );

        databaseHandle.SetHandleValue( rawDatabaseHandle );

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1start_1transaction
  (JNIEnv * javaEnvironment, jobject jThis, jobject jTransactionHandle, jobject jDatabaseHandle, jbyteArray jTpb)
    {
    ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JByteArray tpb( javaEnvironment, jTpb );

        FirebirdStatusVector status;
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();

        CALL_API(isc_start_transaction)( status.RawAccess(), &rawTransactionHandle, 1, &rawDatabaseHandle, tpb.Size(), tpb.Read()  );
        
        databaseHandle.SetHandleValue( rawDatabaseHandle );
        transactionHandle.SetHandleValue(rawTransactionHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1reconnect_1transaction
  (JNIEnv * javaEnvironment, jobject jThis, jobject jDatabaseHandle, jobject jTransactionHandle, jbyteArray jTransactionId)
	{
    ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JByteArray transactionId(javaEnvironment, jTransactionId);

        FirebirdStatusVector status;
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();

        CALL_API(isc_reconnect_transaction)( status.RawAccess(), &rawDatabaseHandle, &rawTransactionHandle, transactionId.Size(), transactionId.Read()  );
        
        transactionHandle.SetHandleValue(rawTransactionHandle);
        databaseHandle.SetHandleValue( rawDatabaseHandle );

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
	}

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1commit_1transaction
  (JNIEnv * javaEnvironment, jobject jThis, jobject jTransactionHandle)
    {
    ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);

        FirebirdStatusVector status;
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();
        
        CALL_API(isc_commit_transaction)( status.RawAccess(), &rawTransactionHandle );
        
        transactionHandle.SetHandleValue(rawTransactionHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, transactionHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1commit_1retaining
  (JNIEnv * javaEnvironment, jobject jThis, jobject jTransactionHandle)
    {
    ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);

        FirebirdStatusVector status;
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();
        
        CALL_API(isc_commit_retaining)( status.RawAccess(), &rawTransactionHandle );
        
        transactionHandle.SetHandleValue(rawTransactionHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, transactionHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1prepare_1transaction
 (JNIEnv * javaEnvironment, jobject jThis, jobject jTransactionHandle)
    {
    ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);

        FirebirdStatusVector status;
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();
        
        CALL_API(isc_prepare_transaction2 )( status.RawAccess(), &rawTransactionHandle, 0, NULL );
        
        transactionHandle.SetHandleValue(rawTransactionHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, transactionHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1prepare_1transaction2
  (JNIEnv * javaEnvironment, jobject jThis, jobject jTransactionHandle, jbyteArray jBytes)
    {
    ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);

        FirebirdStatusVector status;
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();
        
        JByteArray tpb( javaEnvironment, jBytes );

        CALL_API(isc_prepare_transaction2)( status.RawAccess(), &rawTransactionHandle, tpb.Size(), (unsigned char*)tpb.Read() );
        
        transactionHandle.SetHandleValue(rawTransactionHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, transactionHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1rollback_1transaction
  (JNIEnv * javaEnvironment, jobject jThis, jobject jTransactionHandle)
    {
    ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);

        FirebirdStatusVector status;
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();
        
        CALL_API(isc_rollback_transaction)( status.RawAccess(), &rawTransactionHandle );
        
        transactionHandle.SetHandleValue(rawTransactionHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, transactionHandle);
    LEAVE_PROTECTED_BLOCK
    }
    
JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1rollback_1retaining
 (JNIEnv * javaEnvironment, jobject jThis, jobject jTransactionHandle)
    {
    ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);

        FirebirdStatusVector status;
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();
        
        CALL_API(isc_rollback_retaining)( status.RawAccess(), &rawTransactionHandle );
        
        transactionHandle.SetHandleValue(rawTransactionHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, transactionHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1allocate_1statement
  (JNIEnv * javaEnvironment, jobject jThis, jobject jDatabaseHandle, jobject jStatementHandle )
    {
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JIscStatementHandle statementHandle(javaEnvironment, jStatementHandle);

        FirebirdStatusVector status;
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();
        isc_stmt_handle rawStatementHandle = statementHandle.GetHandleValue();
        CALL_API(isc_dsql_allocate_statement)( status.RawAccess(), &rawDatabaseHandle, &rawStatementHandle );

        databaseHandle.SetHandleValue(rawDatabaseHandle);
        statementHandle.SetHandleValue(rawStatementHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1free_1statement
  (JNIEnv *javaEnvironment, jobject jThis, jobject jStatementHandle, jint jValue)
    {
    ENTER_PROTECTED_BLOCK
        JIscStatementHandle statementHandle(javaEnvironment, jStatementHandle);

        FirebirdStatusVector status;
        
        isc_stmt_handle rawStatementHandle = statementHandle.GetHandleValue();
        
        CALL_API(isc_dsql_free_statement)( status.RawAccess(), &rawStatementHandle, jValue );
        
        statementHandle.SetHandleValue(rawStatementHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, statementHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1alloc_1statement2
  (JNIEnv * javaEnvironment, jobject jThis, jobject jDatabaseHandle, jobject jStatementHandle )
    {
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JIscStatementHandle statementHandle(javaEnvironment, jStatementHandle);

        FirebirdStatusVector status;
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();
        isc_stmt_handle rawStatementHandle = statementHandle.GetHandleValue();
        
        CALL_API(isc_dsql_alloc_statement2)( status.RawAccess(), &rawDatabaseHandle, &rawStatementHandle );

        databaseHandle.SetHandleValue(rawDatabaseHandle);
        statementHandle.SetHandleValue(rawStatementHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT jobject JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1prepare
  (JNIEnv *javaEnvironment, jobject jThis, jobject jTransactionHandle, jobject jStatementHandle, jbyteArray statement, jint dialect)
	{
    ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);
        JIscStatementHandle statementHandle(javaEnvironment, jStatementHandle);
        JByteArray statementStringBytes( javaEnvironment, statement );

        FirebirdStatusVector status;
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();
        isc_stmt_handle rawStatementHandle = statementHandle.GetHandleValue();

		JXSqlda xsqlda(javaEnvironment);
        
        CALL_API(isc_dsql_prepare)( status.RawAccess(), &rawTransactionHandle, &rawStatementHandle, 0, statementStringBytes.Read(), dialect, xsqlda.RawAccess() );

        if(xsqlda.RawAccess()->sqln != xsqlda.RawAccess()->sqld )
            {
            xsqlda.Resize( xsqlda.RawAccess()->sqld );
            
            // Re-describe the statement. 
            CALL_API(isc_dsql_describe)( status.RawAccess(), &rawStatementHandle, dialect, xsqlda.RawAccess() );
            }

		transactionHandle.SetHandleValue(rawTransactionHandle);
        statementHandle.SetHandleValue(rawStatementHandle);

        jobject returnValue = xsqlda.AllocateJavaXSqlda(javaEnvironment);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, statementHandle);

        return returnValue;

    LEAVE_PROTECTED_BLOCK

	return NULL;
    }

JNIEXPORT jobject JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1describe
  (JNIEnv * javaEnvironment, jobject jThis, jobject jStatementHandle, jint jDaVersion)
    {
    ENTER_PROTECTED_BLOCK
        JIscStatementHandle statementHandle(javaEnvironment, jStatementHandle);
    
        FirebirdStatusVector status;
        isc_stmt_handle rawStatementHandle = statementHandle.GetHandleValue();

        JXSqlda xsqlda(javaEnvironment);
        
        CALL_API(isc_dsql_describe)( status.RawAccess(), &rawStatementHandle, jDaVersion, xsqlda.RawAccess() );

		if ( xsqlda.RawAccess()->sqln != xsqlda.RawAccess()->sqld )
			{
			xsqlda.Resize( xsqlda.RawAccess()->sqld );

			isc_dsql_describe( status.RawAccess(), &rawStatementHandle, jDaVersion, xsqlda.RawAccess() );
			}

        statementHandle.SetHandleValue(rawStatementHandle);

        jobject returnValue = xsqlda.AllocateJavaXSqlda(javaEnvironment);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, statementHandle);

        return returnValue;
    LEAVE_PROTECTED_BLOCK

    return NULL;
    }

JNIEXPORT jobject JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1describe_1bind
  (JNIEnv * javaEnvironment, jobject jThis, jobject jStatementHandle, jint jDaVersion)
    {
    ENTER_PROTECTED_BLOCK
        JIscStatementHandle statementHandle(javaEnvironment, jStatementHandle);

        FirebirdStatusVector status;
        isc_stmt_handle rawStatementHandle = statementHandle.GetHandleValue();

        JXSqlda xsqlda(javaEnvironment);
        
        CALL_API(isc_dsql_describe_bind)( status.RawAccess(), &rawStatementHandle, jDaVersion, xsqlda.RawAccess() );

		if ( xsqlda.RawAccess()->sqln != xsqlda.RawAccess()->sqld )
			{
			xsqlda.Resize( xsqlda.RawAccess()->sqld );

			isc_dsql_describe_bind( status.RawAccess(), &rawStatementHandle, jDaVersion, xsqlda.RawAccess() );
			}

        statementHandle.SetHandleValue(rawStatementHandle);

        jobject returnValue = xsqlda.AllocateJavaXSqlda(javaEnvironment);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, statementHandle);

        return returnValue;
    LEAVE_PROTECTED_BLOCK

    return NULL;
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1execute2
(JNIEnv * javaEnvironment, jobject jThis, jobject jTransactionHandle, jobject jStatementHandle, jint jDaVersion, jobject jInXSqlda, jobject jOutXSqlda)
    {
    ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);
    
        JIscStatementHandle statementHandle(javaEnvironment, jStatementHandle);
        
        JXSqlda in_xsqlda( javaEnvironment, jInXSqlda );
        JXSqlda out_xsqlda( javaEnvironment, jOutXSqlda );

        FirebirdStatusVector status;
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();
        isc_stmt_handle rawStatementHandle = statementHandle.GetHandleValue();

        CALL_API(isc_dsql_execute2)( status.RawAccess(), &rawTransactionHandle, &rawStatementHandle, jDaVersion, in_xsqlda.RawAccess(), out_xsqlda.RawAccess() );

		transactionHandle.SetHandleValue(rawTransactionHandle);
        statementHandle.SetHandleValue(rawStatementHandle);

        in_xsqlda.Resync(javaEnvironment);
        out_xsqlda.Resync(javaEnvironment);

		status.IssueExceptionsAndOrAddWarnings(javaEnvironment, statementHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT jbyteArray JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1sql_1info
  (JNIEnv *javaEnvironment, jobject jThis, jobject jStatementHandle, jbyteArray jItemsArray, jint jBufferLength)
{

ENTER_PROTECTED_BLOCK
        JIscStatementHandle statementHandle(javaEnvironment, jStatementHandle);

        JByteArray itemsArray( javaEnvironment, jItemsArray );

        JByteArray buffer( javaEnvironment, jBufferLength );
    
        FirebirdStatusVector status;
        isc_stmt_handle rawStatementHandle = statementHandle.GetHandleValue();

        CALL_API(isc_dsql_sql_info)( status.RawAccess(), &rawStatementHandle, itemsArray.Size(), itemsArray.Read(), buffer.Size(), buffer.Read() );

        statementHandle.SetHandleValue(rawStatementHandle);

        jbyteArray returnValue = buffer.GetHandle();
            
        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, statementHandle);

        return returnValue;
    LEAVE_PROTECTED_BLOCK

    return NULL;
    }


JNIEXPORT jbyteArray JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1transaction_1info
  (JNIEnv * javaEnvironment, jobject jThis, jobject jTransactionHandle, jbyteArray jItemsArray, jint jBufferLength)
	{
	ENTER_PROTECTED_BLOCK
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);

        JByteArray itemsArray( javaEnvironment, jItemsArray );
        JByteArray buffer( javaEnvironment, jBufferLength );
    
        FirebirdStatusVector status;
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();

        CALL_API(isc_transaction_info)( status.RawAccess(), &rawTransactionHandle, itemsArray.Size(), itemsArray.Read(), buffer.Size(), buffer.Read() );
    
        transactionHandle.SetHandleValue(rawTransactionHandle);

        jbyteArray returnValue = buffer.GetHandle();
            
        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, transactionHandle);

        return returnValue;
    LEAVE_PROTECTED_BLOCK

    return NULL;
    }


JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1exec_1immed2
  (JNIEnv *javaEnvironment, jobject jThis, jobject jDatabaseHandle, jobject jTransactionHandle, jbyteArray jStatement, jint jDialect, jobject jInXsqlda, jobject jOutXsqlda)
	{
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransactionHandle);
        JByteArray statementStringBytes( javaEnvironment, jStatement );

        JXSqlda in_xsqlda( javaEnvironment, jInXsqlda );
        JXSqlda out_xsqlda( javaEnvironment, jOutXsqlda );

        FirebirdStatusVector status;

        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();

        CALL_API(isc_dsql_exec_immed2)( status.RawAccess(), &rawDatabaseHandle, &rawTransactionHandle, 0, statementStringBytes.Read(), jDialect, in_xsqlda.RawAccess(), out_xsqlda.RawAccess() );
    
        databaseHandle.SetHandleValue(rawDatabaseHandle);
        transactionHandle.SetHandleValue(rawTransactionHandle);

        in_xsqlda.Resync(javaEnvironment);
        out_xsqlda.Resync(javaEnvironment);
        
        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }


JNIEXPORT jboolean JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1fetch
  (JNIEnv *javaEnvironment, jobject jThis, jobject jStatementHandle, jint jDaVersion, jobject jXsqlda, jint jFetchSize)
{
ENTER_PROTECTED_BLOCK
        JIscStatementHandle statementHandle(javaEnvironment, jStatementHandle);
        
        JXSqlda out_xsqlda( javaEnvironment, jXsqlda, true );

        FirebirdStatusVector status;
        
        isc_stmt_handle rawStatementHandle = statementHandle.GetHandleValue();
    
        DEF_CALL_API(isc_dsql_fetch)
        ISC_STATUS fetch_stat = isc_dsql_fetch( status.RawAccess(), &rawStatementHandle, jDaVersion, out_xsqlda.RawAccess() );

        statementHandle.SetHandleValue(rawStatementHandle);
    
        out_xsqlda.Resync(javaEnvironment);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, statementHandle);

        if(fetch_stat == 100L)
            return JNI_FALSE;
        else
            return JNI_TRUE;
    LEAVE_PROTECTED_BLOCK

    return JNI_FALSE;
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1dsql_1set_1cursor_1name
  (JNIEnv *javaEnvironment, jobject jThis, jobject jStatementHandle , jstring jCursorName, jint jType)
	{
    ENTER_PROTECTED_BLOCK
        JIscStatementHandle statementHandle(javaEnvironment, jStatementHandle);
        JString cursornameString( javaEnvironment, jCursorName );
    
        FirebirdStatusVector status;
        
        const char* const cursorname = cursornameString.AsCString();
        
        isc_stmt_handle rawStatementHandle = statementHandle.GetHandleValue();

        CALL_API(isc_dsql_set_cursor_name)( status.RawAccess(), &rawStatementHandle, const_cast<char*>(cursorname), jType );
    
        statementHandle.SetHandleValue(rawStatementHandle);
        
        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, statementHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1create_1blob2
  (JNIEnv *javaEnvironment, jobject jThis, jobject jDatabaseHandle, jobject jTransctionHandle, jobject jBlobHandle, jbyteArray jClumpetBytes)
    {
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransctionHandle);
        JIscBlobHandle blobHandle(javaEnvironment, jBlobHandle);
        JByteArray clumpetBytes(javaEnvironment, jClumpetBytes);
    
        FirebirdStatusVector status;
        
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();
        isc_blob_handle rawBlobHandle = blobHandle.GetHandleValue();
        ISC_QUAD rawBlobId = blobHandle.GetId();

        CALL_API(isc_create_blob2)( status.RawAccess(), &rawDatabaseHandle, &rawTransactionHandle, &rawBlobHandle, &rawBlobId, clumpetBytes.Size(), clumpetBytes.Read() );
    
        databaseHandle.SetHandleValue(rawDatabaseHandle);
        transactionHandle.SetHandleValue(rawTransactionHandle);
        blobHandle.SetHandleValue(rawBlobHandle);
        blobHandle.SetId(rawBlobId);
        
        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1open_1blob2
  (JNIEnv *javaEnvironment, jobject jThis, jobject jDatabaseHandle, jobject jTransctionHandle, jobject jBlobHandle, jbyteArray jClumpetBytes)
    {
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JIscTransactionHandle transactionHandle(javaEnvironment, jTransctionHandle);
        JIscBlobHandle blobHandle(javaEnvironment, jBlobHandle);
        JByteArray clumpetBytes(javaEnvironment, jClumpetBytes);
    
        FirebirdStatusVector status;
        
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();
        isc_tr_handle rawTransactionHandle = transactionHandle.GetHandleValue();
        isc_blob_handle rawBlobHandle = blobHandle.GetHandleValue();
        ISC_QUAD rawBlobId = blobHandle.GetId();

        CALL_API(isc_open_blob2)( status.RawAccess(), &rawDatabaseHandle, &rawTransactionHandle, &rawBlobHandle, &rawBlobId, clumpetBytes.Size(), (unsigned char*)clumpetBytes.Read() );
    
        databaseHandle.SetHandleValue(rawDatabaseHandle);
        transactionHandle.SetHandleValue(rawTransactionHandle);
        blobHandle.SetHandleValue(rawBlobHandle);
        blobHandle.SetId(rawBlobId);
        
        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT jbyteArray JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1get_1segment
  (JNIEnv *javaEnvironment, jobject jThis, jobject jBlobHandle, jint jMaxRead)
{
    ENTER_PROTECTED_BLOCK
        JIscBlobHandle blobHandle(javaEnvironment, jBlobHandle);
    
        FirebirdStatusVector status;
        
        isc_blob_handle rawBlobHandle = blobHandle.GetHandleValue();
        ISC_QUAD rawBlobId = blobHandle.GetId();
    
        Buffer buffer(jMaxRead);

        unsigned short lengthRead = 0;
        DEF_CALL_API(isc_get_segment);
        ISC_STATUS statusPart = isc_get_segment( status.RawAccess(), &rawBlobHandle, &lengthRead, jMaxRead, buffer.access() );

        JByteArray returnBytes(javaEnvironment, buffer.access(), lengthRead);

        blobHandle.SetHandleValue(rawBlobHandle);
        blobHandle.SetId(rawBlobId);

        jbyteArray returnValue = returnBytes.GetHandle();

        if( statusPart == isc_segstr_eof )
            blobHandle.SetIsEndOfFile(true);
        else 
            {
            blobHandle.SetIsEndOfFile(false);
            
            if( statusPart != isc_segment )
                status.IssueExceptionsAndOrAddWarnings(javaEnvironment, blobHandle);
            }

        return returnValue;
    LEAVE_PROTECTED_BLOCK

    return NULL;
    }   

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1put_1segment
  (JNIEnv *javaEnvironment, jobject jThis, jobject jBlobHandle, jbyteArray jBytesToWrite)
    {
    ENTER_PROTECTED_BLOCK
        JIscBlobHandle blobHandle(javaEnvironment, jBlobHandle);
        JByteArray bytesToWrite(javaEnvironment, jBytesToWrite);

        FirebirdStatusVector status;
        
        isc_blob_handle rawBlobHandle = blobHandle.GetHandleValue();
        ISC_QUAD rawBlobId = blobHandle.GetId();
        
        CALL_API(isc_put_segment)( status.RawAccess(), &rawBlobHandle, bytesToWrite.Size(), bytesToWrite.Read() );

        blobHandle.SetHandleValue(rawBlobHandle);
        blobHandle.SetId(rawBlobId);
            
        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, blobHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1close_1blob
  (JNIEnv *javaEnvironment, jobject jThis, jobject jBlobHandle)
    {
    ENTER_PROTECTED_BLOCK
        JIscBlobHandle blobHandle(javaEnvironment, jBlobHandle);
    
        FirebirdStatusVector status;
        
        isc_blob_handle rawBlobHandle = blobHandle.GetHandleValue();
        ISC_QUAD rawBlobId = blobHandle.GetId();
        
        CALL_API(isc_close_blob)( status.RawAccess(), &rawBlobHandle );
    
        blobHandle.SetHandleValue(rawBlobHandle);
        blobHandle.SetId(rawBlobId);
            
        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, blobHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT jbyteArray JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1blob_1info
  (JNIEnv *javaEnvironment, jobject jThis, jobject jBlobHandle, jbyteArray jItemsArrayHandle, jint jBufferLength)
    {
    ENTER_PROTECTED_BLOCK
        JIscBlobHandle blobHandle(javaEnvironment, jBlobHandle);
        JByteArray bytesToWrite(javaEnvironment, jItemsArrayHandle);
    
        FirebirdStatusVector status;
        
        isc_blob_handle rawBlobHandle = blobHandle.GetHandleValue();
        ISC_QUAD rawBlobId = blobHandle.GetId();

        char* resultBuffer = (char*)alloca(jBufferLength);
        
        CALL_API(isc_blob_info)( status.RawAccess(), &rawBlobHandle, bytesToWrite.Size(), bytesToWrite.Read(), jBufferLength, resultBuffer );

        blobHandle.SetHandleValue(rawBlobHandle);
        blobHandle.SetId(rawBlobId);

        JByteArray returnBytes(javaEnvironment, resultBuffer, jBufferLength);
            
        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, blobHandle);

        return returnBytes.GetHandle();
    LEAVE_PROTECTED_BLOCK

	return NULL;
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1seek_1blob
  (JNIEnv *javaEnvironment, jobject jThis, jobject jBlobHandle, jint position, jint mode)
    {
    ENTER_PROTECTED_BLOCK
        JIscBlobHandle blobHandle(javaEnvironment, jBlobHandle);
    
        FirebirdStatusVector status;
        
        isc_blob_handle rawBlobHandle = blobHandle.GetHandleValue();
        ISC_QUAD rawBlobId = blobHandle.GetId();

        ISC_LONG result;
        
        CALL_API(isc_seek_blob)( status.RawAccess(), &rawBlobHandle, mode, position, &result );
    
        blobHandle.SetHandleValue(rawBlobHandle);
        blobHandle.SetId(rawBlobId);
            
        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, blobHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1service_1attach
  (JNIEnv *javaEnvironment, jobject jThis, jstring jServiceString, jobject jServiceHandle, jbyteArray jServiceParameterBuffer)
    {
    ENTER_PROTECTED_BLOCK
        JIscServiceHandle serviceHandle(javaEnvironment, jServiceHandle);
        JString serviceString(javaEnvironment, jServiceString);
        JByteArray serviceParameterBuffer(javaEnvironment, jServiceParameterBuffer);

        FirebirdStatusVector status;

        isc_svc_handle rawServiceHandle = serviceHandle.GetHandleValue();

        CALL_API(isc_service_attach)( status.RawAccess(), serviceString.GetLength(), (char*)serviceString.AsCString(),
            &rawServiceHandle, serviceParameterBuffer.Size(), serviceParameterBuffer.Read() );

        serviceHandle.SetHandleValue(rawServiceHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, serviceHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1service_1detach
  (JNIEnv *javaEnvironment, jobject jThis, jobject jServiceHandle)
    {
    ENTER_PROTECTED_BLOCK
        JIscServiceHandle serviceHandle(javaEnvironment, jServiceHandle);

        FirebirdStatusVector status;

        isc_svc_handle rawServiceHandle = serviceHandle.GetHandleValue();

        CALL_API(isc_service_detach)( status.RawAccess(), &rawServiceHandle );

        serviceHandle.SetHandleValue(rawServiceHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, serviceHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1service_1start
  (JNIEnv *javaEnvironment, jobject jThis, jobject jServiceHandle, jbyteArray jServiceParameterBuffer)
    {
    ENTER_PROTECTED_BLOCK
        JIscServiceHandle serviceHandle(javaEnvironment, jServiceHandle);
        JByteArray serviceParameterBuffer(javaEnvironment, jServiceParameterBuffer);

        FirebirdStatusVector status;

        isc_svc_handle rawServiceHandle = serviceHandle.GetHandleValue();

        CALL_API(isc_service_start)( status.RawAccess(), &rawServiceHandle, NULL, serviceParameterBuffer.Size(), serviceParameterBuffer.Read() );

        serviceHandle.SetHandleValue(rawServiceHandle);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, serviceHandle);
    LEAVE_PROTECTED_BLOCK
    }

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1service_1query
  (JNIEnv *javaEnvironment, jobject jThis, jobject jServiceHandle, jbyteArray jSendServiceParameterBuffer, 
   jbyteArray jRequestServiceParameterBuffer, jbyteArray jResultBuffer)
    {
    ENTER_PROTECTED_BLOCK
        JIscServiceHandle serviceHandle(javaEnvironment, jServiceHandle);

        JByteArray sendParameterBuffer(javaEnvironment, jSendServiceParameterBuffer);
        JByteArray requestParameterBuffer(javaEnvironment, jRequestServiceParameterBuffer);

        JByteArray resultBuffer(javaEnvironment, jResultBuffer);

        FirebirdStatusVector status;

        isc_svc_handle rawServiceHandle = serviceHandle.GetHandleValue();

        CALL_API(isc_service_query)( status.RawAccess(), &rawServiceHandle, NULL, sendParameterBuffer.Size(), sendParameterBuffer.Read(),
            requestParameterBuffer.Size(), requestParameterBuffer.Read(), resultBuffer.Size(), resultBuffer.Read());

        serviceHandle.SetHandleValue(rawServiceHandle);

		status.IssueExceptionsAndOrAddWarnings(javaEnvironment, serviceHandle);
	LEAVE_PROTECTED_BLOCK
	}

/*
 * Callback for events
 */
isc_callback event_function(event_struct* es, short length, char* updated)
	{
    JNIEnv* javaEnvironment;
    jint attachment = jvm->GetEnv((void**)&javaEnvironment, JNI_VERSION_1_1);
    if (attachment == JNI_EDETACHED)
		{
        if (jvm->AttachCurrentThread((void **)&javaEnvironment, NULL) != JNI_OK)
			{
            fprintf(stderr, "Attach thread failed\n");
            return 0;
			}
		}

    ENTER_PROTECTED_BLOCK
        if (es->state == EVENT_UNINITIALIZED)
			{
            es->state = EVENT_ACTIVE;
			} 

        JEventHandler eventHandler(javaEnvironment, es->handler);
        JEventHandle eventHandle(javaEnvironment, es->eventHandle);
        bool freeGlobals = false;

		long eventStructPos = eventHandle.GetEventStructHandle();

        if (es->state != EVENT_CANCELLED)
			{
			char* buffer = es->resultBuffer;
            if (buffer != 0)
				{
                while(length--)
					{
                    buffer[length] = updated[length];
					}
                eventHandler.EventOccurred();
				}
            else
				{
                freeGlobals = true;
				}
			} 
        else
			{
            freeGlobals = true;
			}

        if (freeGlobals)
			{
            javaEnvironment->DeleteGlobalRef(es->handler);
            javaEnvironment->DeleteGlobalRef(es->eventHandle);

			eventStructManager.releaseEventStruct(eventStructPos);
			}
    LEAVE_PROTECTED_BLOCK

    if (attachment == JNI_EDETACHED)
		{
        if (jvm->DetachCurrentThread() != JNI_OK)
			{
            fprintf(stderr, "Detach thread failed\n");
			}
		}
    return 0;
	}


JNIEXPORT jint JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1que_1events
  (JNIEnv * javaEnvironment, jobject jThis, jobject jDatabaseHandle, jobject jEventHandle, jobject eventHandler)
	{
    ISC_LONG eventId = -1;
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JEventHandle eventHandle(javaEnvironment, jEventHandle);
        FirebirdStatusVector status;

        isc_db_handle rawDatabaseHandle = 
            databaseHandle.GetHandleValue();

		long eventStructPos = eventHandle.GetEventStructHandle();
		event_struct* es = eventStructManager.getEventStruct(eventStructPos);

		if (es->eventHandle == 0) 
			{
			es->state = EVENT_UNINITIALIZED;

			jobject globalEventHandler = 
                javaEnvironment->NewGlobalRef(eventHandler);

			jobject globalEventHandle = 
                javaEnvironment->NewGlobalRef(jEventHandle);
            
            es->handler = globalEventHandler;
            es->eventHandle = globalEventHandle;
			} 
		else 
			{
			es->state = EVENT_ACTIVE;
			}

        CALL_API(isc_que_events)(
                status.RawAccess(), 
                &rawDatabaseHandle, 
                &eventId,
                eventHandle.GetSize(),
                es->eventBuffer, 
                (isc_callback)event_function, 
                (void*)es);
        
        eventHandle.SetEventId(eventId);

        status.IssueExceptionsAndOrAddWarnings(
                javaEnvironment, 
                databaseHandle);
    LEAVE_PROTECTED_BLOCK
    return eventId;
	}


JNIEXPORT jlong JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1event_1block
  (JNIEnv * javaEnvironment, jobject jThis, jobject jEventHandle, jstring eventName)
	{
    ENTER_PROTECTED_BLOCK

        JString jEventName(javaEnvironment, eventName);
        JEventHandle eventHandle(javaEnvironment, jEventHandle);
        const char* const event_name = jEventName.AsCString();

		long eventStructPos = eventStructManager.addEventStruct();
		event_struct* eventStructPtr = eventStructManager.getEventStruct(eventStructPos);

		eventHandle.SetEventStructHandle(eventStructPos);
        
        DEF_CALL_API(isc_event_block)
        
        long length = isc_event_block(
				&eventStructPtr->eventBuffer,
                &eventStructPtr->resultBuffer,
                1,
                const_cast<char*>(event_name));

        eventHandle.SetSize(length);
		
        return length;
    LEAVE_PROTECTED_BLOCK
    return -1;
	}

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1event_1counts
  (JNIEnv * javaEnvironment, jobject jThis, jobject jEventHandle)
	{
    ENTER_PROTECTED_BLOCK
        ISC_STATUS stat[20];
        FirebirdStatusVector status;
        JEventHandle eventHandle(javaEnvironment, jEventHandle);
		long eventStructPos = eventHandle.GetEventStructHandle();
		event_struct* es = eventStructManager.getEventStruct(eventStructPos);

        CALL_API(isc_event_counts)( 
                                    stat, 
                                    eventHandle.GetSize(),
                                    es->eventBuffer,
                                    es->resultBuffer);
        eventHandle.SetEventCount(stat[0]);
    LEAVE_PROTECTED_BLOCK
	}


JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1cancel_1events
  (JNIEnv * javaEnvironment, jobject jThis, jobject jDatabaseHandle, jobject jEventHandle)
	{
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);
        JEventHandle eventHandle(javaEnvironment, jEventHandle);

		long eventStructPos = eventHandle.GetEventStructHandle();

		event_struct* es = eventStructManager.getEventStruct(eventStructPos);
        es->state = EVENT_CANCELLED;

		char* event_buffer = es->eventBuffer;
		char* result_buffer = es->resultBuffer;

        FirebirdStatusVector status;
        ISC_LONG eventId = eventHandle.GetEventId(); 
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();
        CALL_API(isc_cancel_events)(
                status.RawAccess(), 
                &rawDatabaseHandle, 
                &eventId);

        CALL_API(isc_free)(event_buffer);
        isc_free(result_buffer);

        status.IssueExceptionsAndOrAddWarnings(
                javaEnvironment, databaseHandle);
    LEAVE_PROTECTED_BLOCK
	}

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1isc_1finalize
  (JNIEnv *, jobject, jint isc_api)
	{
    interfaceManager.ReleaseInterface(isc_api);
	}

JNIEXPORT void JNICALL Java_org_firebirdsql_gds_impl_jni_JniGDSImpl_native_1fb_1cancel_1operation
  (JNIEnv * javaEnvironment, jobject jThis, jobject jDatabaseHandle, jint jKind)
	{
    ENTER_PROTECTED_BLOCK
        JIscDatabaseHandle databaseHandle(javaEnvironment, jDatabaseHandle);

		FirebirdStatusVector status;
        isc_db_handle rawDatabaseHandle = databaseHandle.GetHandleValue();

		CALL_API(fb_cancel_operation)(status.RawAccess(), &rawDatabaseHandle, (short)jKind);

        status.IssueExceptionsAndOrAddWarnings(javaEnvironment, databaseHandle);

    LEAVE_PROTECTED_BLOCK
	}
