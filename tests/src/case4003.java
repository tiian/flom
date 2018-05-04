/*
 * Copyright (c) 2013-2018, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */



import org.tiian.flom.*;



public class case4003 {
    private static void HappyPath() {
        try {
            FlomHandle myHandle = new FlomHandle();
            boolean thrown = false;
            int retCod;

            // get current AF_UNIX/PF_LOCAL socket_name
            System.out.println("FlomHandle.getSocketName() = '" +
                               myHandle.getSocketName() + "'");
            // set a null value for AF_UNIX/PF_LOCAL socket_name
            try {
                thrown = false;
                myHandle.setSocketName(null);
            } catch (FlomException e) {
                thrown = true;
                if (FlomErrorCodes.FLOM_RC_NULL_OBJECT != e.getReturnCode())
                    throw(e);
            }
            if (!thrown) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.setSocketName");
                System.exit(1);
            }
            // get current AF_UNIX/PF_LOCAL socket_name
            System.out.println("FlomHandle.getSocketName() = '" +
                               myHandle.getSocketName() + "'");
            
            /* set a null value for trace filename */
            try {
                thrown = false;
                myHandle.setTraceFilename(null);
            } catch (FlomException e) {
                thrown = true;
                if (FlomErrorCodes.FLOM_RC_NULL_OBJECT != e.getReturnCode())
                    throw(e);
            }
            if (!thrown) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.setTraceFilename");
                System.exit(1);
            }
            /* get current trace filename */
            System.out.println("FlomHandle.getTraceFilename() = '" +
                               myHandle.getTraceFilename() + "'");

            /* get current resource name */
            System.out.println("FlomHandle.getResourceName() = '" +
                               myHandle.getResourceName() + "'");
            /* set a null resource name */
            try {
                thrown = false;
                myHandle.setResourceName(null);
            } catch (FlomException e) {
                thrown = true;
                if (FlomErrorCodes.FLOM_RC_NULL_OBJECT != e.getReturnCode())
                    throw(e);
            }
            if (!thrown) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.setResourceName");
                System.exit(1);
            }
            /* get current resource name */
            System.out.println("FlomHandle.getResourceName() = '" +
                               myHandle.getResourceName() + "'");
    
            /* get current unicast address */
            System.out.println("FlomHandle.getUnicastAddress() = '" +
                               myHandle.getUnicastAddress() + "'");
            /* set a null unicast_address */
            try {
                thrown = false;
                myHandle.setUnicastAddress(null);
            } catch (FlomException e) {
                thrown = true;
                if (FlomErrorCodes.FLOM_RC_NULL_OBJECT != e.getReturnCode())
                    throw(e);
            }
            if (!thrown) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.setUnicastAddress");
                System.exit(1);
            }        
            /* get new unicast address */
            System.out.println("FlomHandle.getUnicastAddress() = '" +
                               myHandle.getUnicastAddress() + "'");
    
            /* get current multicast address */
            System.out.println("FlomHandle.getMulticastAddress() = '" +
                               myHandle.getMulticastAddress() + "'");
            /* set a null multicast_address */
            try {
                thrown = false;
                myHandle.setMulticastAddress(null);
            } catch (FlomException e) {
                thrown = true;
                if (FlomErrorCodes.FLOM_RC_NULL_OBJECT != e.getReturnCode())
                    throw(e);
            }
            if (!thrown) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.setMulticastAddress");
                System.exit(1);
            }        
            /* get new multicast address */
            System.out.println("FlomHandle.getMulticastAddress() = '" +
                               myHandle.getMulticastAddress() + "'");
        
            myHandle.lock();
            myHandle.unlock();
            myHandle.free();
        } catch(FlomException e) {
            System.out.println("FlomException: ReturnCode=" +
                               e.getReturnCode() +
                               " (" + e.getMessage() + ")");
            System.exit(1);
        }
    }
    
    public static void main(String[] args) {
        HappyPath();
    }
}
