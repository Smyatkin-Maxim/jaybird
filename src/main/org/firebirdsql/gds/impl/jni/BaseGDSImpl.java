/*
 * $Id$
 *
 * Firebird Open Source JavaEE Connector - JDBC Driver
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
 * can be obtained from a source control history command.
 *
 * All rights reserved.
 */
package org.firebirdsql.gds.impl.jni;

import org.firebirdsql.gds.*;
import org.firebirdsql.gds.impl.*;
import org.firebirdsql.logging.Logger;
import org.firebirdsql.logging.LoggerFactory;

import java.io.UnsupportedEncodingException;

public abstract class BaseGDSImpl extends AbstractGDS {
    
    // TODO Synchronization seems to be inconsistent: sometimes on dbhandle, sometimes on this (and sometimes on blobhandle)
    // TODO Checking for validity of dbhandle is inconsistent (sometimes only null check, sometimes also .isValid())

    private static Logger log = LoggerFactory.getLogger(BaseGDSImpl.class);

    private static final String WARNING_CONNECT_TIMEOUT_NATIVE = 
            "WARNING: The native driver does not apply connectTimeout for establishing the socket connection (only for protocol negotiation with the Firebird server), " + 
            "it will not detect unreachable hosts within the specified timeout";

    public int isc_api_handle;
    
    public BaseGDSImpl() {
        super();
    }

    public BaseGDSImpl(GDSType gdsType) {
        super(gdsType);
    }

    protected abstract String getServerUrl(String file_name) throws GDSException;

    public DatabaseParameterBuffer createDatabaseParameterBuffer() {
        return new DatabaseParameterBufferImp();
    }

    public IscDbHandle createIscDbHandle() {
        return new isc_db_handle_impl();
    }

    public IscSvcHandle createIscSvcHandle() {
        return new isc_svc_handle_impl();
    }

    public ServiceParameterBuffer createServiceParameterBuffer() {
        return new ServiceParameterBufferImp();
    }

    public ServiceRequestBuffer createServiceRequestBuffer(int taskIdentifier) {
        return new ServiceRequestBufferImp(taskIdentifier);
    }

    // isc_attach_database
    // ---------------------------------------------------------------------------------------------
    public void iscAttachDatabase(String file_name, IscDbHandle db_handle,
            DatabaseParameterBuffer databaseParameterBuffer)
            throws GDSException {
        if (db_handle == null) { 
            throw new GDSException(ISCConstants.isc_bad_db_handle); 
        }

        final byte[] dpbBytes;
        final String filenameCharset;
        if (databaseParameterBuffer != null) {
            DatabaseParameterBuffer cleanDPB = ((DatabaseParameterBufferExtension)databaseParameterBuffer).removeExtensionParams();
            if (cleanDPB.hasArgument(DatabaseParameterBuffer.CONNECT_TIMEOUT)) {
                // For the native driver isc_dpb_connect_timeout is not a socket connect timeout
                // It only applies to the steps for op_accept (negotiating protocol, etc)
                if (log != null) {
                    log.warn(WARNING_CONNECT_TIMEOUT_NATIVE);
                }
                db_handle.addWarning(new GDSWarning(WARNING_CONNECT_TIMEOUT_NATIVE));
            }

            dpbBytes = cleanDPB.toBytes();
            filenameCharset = databaseParameterBuffer.getArgumentAsString(DatabaseParameterBufferExtension.FILENAME_CHARSET);
        } else {
            dpbBytes = null;
            filenameCharset = null;
        }

        String serverUrl = getServerUrl(file_name);
        
        byte[] urlData;
        try {
            if (filenameCharset != null)
                urlData = serverUrl.getBytes(filenameCharset);
            else
                urlData = serverUrl.getBytes();
            
            byte[] nullTerminated = new byte[urlData.length + 1];
            System.arraycopy(urlData, 0, nullTerminated, 0, urlData.length);
            urlData = nullTerminated;
        } catch(UnsupportedEncodingException ex) {
            throw new GDSException(ISCConstants.isc_bad_dpb_content);
        }
        
        synchronized (db_handle) {
            throw new UnsupportedOperationException();
            //native_isc_attach_database(urlData, db_handle, dpbBytes);
        }

//        parseAttachDatabaseInfo(iscDatabaseInfo(db_handle,
//                AbstractGDS.DESCRIBE_DATABASE_INFO_BLOCK, 1024), db_handle);
    }

    // isc_attach_database
    // ---------------------------------------------------------------------------------------------
    public byte[] iscDatabaseInfo(IscDbHandle db_handle, byte[] items,
            int buffer_length) throws GDSException {
        synchronized (db_handle) {
            final byte[] returnValue = new byte[buffer_length];

            throw new UnsupportedOperationException();
//            native_isc_database_info(db_handle, items.length, items,
//                    buffer_length, returnValue);
//
//            return returnValue;
        }
    }

    // isc_detach_database
    // ---------------------------------------------------------------------------------------------
    public void iscDetachDatabase(IscDbHandle db_handle) throws GDSException {
        if (db_handle == null) { throw new GDSException(ISCConstants.isc_bad_db_handle); }

        synchronized (db_handle) {
            throw new UnsupportedOperationException();
//            native_isc_detach_database(db_handle);
//            try {
//                db_handle.invalidate();
//            } catch (Exception e) {
//                // Actual implementation does not throw exception
//                // TODO : Invalidate should throw GDSException?
//                throw new GDSException(ISCConstants.isc_network_error, e);
//            }
        }
    }

    // Services API

    public void iscServiceAttach(String service, IscSvcHandle serviceHandle,
            ServiceParameterBuffer serviceParameterBuffer) throws GDSException {
        final byte[] serviceParameterBufferBytes = serviceParameterBuffer == null ? null
                : serviceParameterBuffer.toBytesWithType();

        synchronized (serviceHandle) {
            if (serviceHandle.isValid())
                throw new GDSException("serviceHandle is already attached.");

            throw new UnsupportedOperationException();
//            native_isc_service_attach(service, serviceHandle,
//                    serviceParameterBufferBytes);
        }
    }

    public void iscServiceDetach(IscSvcHandle serviceHandle) throws GDSException {
        synchronized (serviceHandle) {
            if (serviceHandle.isNotValid())
                throw new GDSException("serviceHandle is not attached.");

            throw new UnsupportedOperationException();
//            native_isc_service_detach(serviceHandle);
        }
    }

    public void iscServiceQuery(IscSvcHandle serviceHandle,
            ServiceParameterBuffer serviceParameterBuffer,
            ServiceRequestBuffer serviceRequestBuffer, byte[] resultBuffer)
            throws GDSException {
        final byte[] serviceParameterBufferBytes = serviceParameterBuffer == null ? null
                : serviceParameterBuffer.toBytesWithType();

        final byte[] serviceRequestBufferBytes = serviceRequestBuffer == null ? null : serviceRequestBuffer.toBytes();

        synchronized (serviceHandle) {
            if (serviceHandle.isNotValid())
                throw new GDSException("serviceHandle is not attached.");

            throw new UnsupportedOperationException();
//            native_isc_service_query(serviceHandle,
//                    serviceParameterBufferBytes, serviceRequestBufferBytes,
//                    resultBuffer);
        }
    }

    public void iscServiceStart(IscSvcHandle serviceHandle,
            ServiceRequestBuffer serviceRequestBuffer) throws GDSException {
        final byte[] serviceRequestBufferBytes = serviceRequestBuffer == null ? null : serviceRequestBuffer.toBytes();

        synchronized (serviceHandle) {
            if (serviceHandle.isNotValid())
                throw new GDSException("serviceHandle is not attached.");

            throw new UnsupportedOperationException();
//            native_isc_service_start(serviceHandle, serviceRequestBufferBytes);
        }
    }

    public TransactionParameterBuffer newTransactionParameterBuffer() {
        return new TransactionParameterBufferImpl();
    }

    public int iscQueueEvents(IscDbHandle dbHandle, 
            EventHandle eventHandle, EventHandler eventHandler) 
            throws GDSException {
        
        EventHandleImp eventHandleImp = (EventHandleImp)eventHandle;
        if (!eventHandleImp.isValid()){
            throw new IllegalStateException(
                    "Can't queue events on an invalid EventHandle");
        }
        if (eventHandleImp.isCancelled()){
            throw new IllegalStateException(
                    "Can't queue events on a cancelled EventHandle");
        }
        synchronized (dbHandle) {
            throw new UnsupportedOperationException();
//            return native_isc_que_events(
//                    dbHandle, eventHandleImp, eventHandler);
        }
    }

    public void iscEventBlock(EventHandle eventHandle) 
            throws GDSException {
        
        EventHandleImp eventHandleImp = (EventHandleImp)eventHandle;
        throw new UnsupportedOperationException();
//        native_isc_event_block(
//                eventHandleImp, eventHandle.getEventName());
    }

    public void iscEventCounts(EventHandle eventHandle)
            throws GDSException {

        EventHandleImp eventHandleImp = (EventHandleImp)eventHandle;
        if (!eventHandleImp.isValid()){
            throw new IllegalStateException(
                    "Can't get counts on an invalid EventHandle");
        }
        throw new UnsupportedOperationException();
//        native_isc_event_counts(eventHandleImp);
    }


    public void iscCancelEvents(IscDbHandle dbHandle, EventHandle eventHandle)
            throws GDSException {

        EventHandleImp eventHandleImp = (EventHandleImp)eventHandle;
        if (!eventHandleImp.isValid()){
            throw new IllegalStateException(
                    "Can't cancel an invalid EventHandle");
        }
        if (eventHandleImp.isCancelled()){
            throw new IllegalStateException(
                    "Can't cancel a previously cancelled EventHandle");
        }
        eventHandleImp.cancel();
        synchronized (dbHandle){
            throw new UnsupportedOperationException();
//            native_isc_cancel_events(dbHandle, eventHandleImp);
        }
    }

    public EventHandle createEventHandle(String eventName){
        return new EventHandleImp(eventName);
    }
    
}
