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
/*
 * The Original Code is the Firebird Java GDS implementation.
 *
 * The Initial Developer of the Original Code is Alejandro Alberola.
 * Portions created by Alejandro Alberola are Copyright (C) 2001
 * Boix i Oltra, S.L. All Rights Reserved.
 */


package org.firebirdsql.gds.impl.wire;

import java.io.IOException;

import org.firebirdsql.gds.EventHandler;
import org.firebirdsql.gds.GDSException;

/**
 * Interface for handing pure java event callbacks. 
 */
public interface EventCoordinator {

    void close() throws IOException;
    void queueEvents(isc_db_handle_impl mainDb, 
            EventHandleImp eventHandleImp, 
            EventHandler handler) throws GDSException;
    boolean cancelEvents(EventHandleImp eventHandleImp);

}
