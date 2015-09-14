/*
 * Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
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



public class case4002 {
    private final static String ndSocketName = "/tmp/flom_socket_name";
    private final static String ndTraceFilename = "/tmp/flom.trc";
    private final static String ndResourceName = "red.blue.green";
    
    private static void HappyPath() {
        try {
            FlomHandle myHandle = new FlomHandle();
            int retCod;

            // get current AF_UNIX/PF_LOCAL socket_name
            System.out.println("FlomHandle.getSocketName() = '" +
                               myHandle.getSocketName() + "'");
            // set a new AF_UNIX/PF_LOCAL socket_name
            if (FlomErrorCodes.FLOM_RC_OK != (
                    retCod = myHandle.setSocketName(ndSocketName))) {
                System.out.println("FlomHandle.setSocketName() returned " +
                                   retCod + " '" +
                                   FlomErrorCodes.getText(retCod) + "'");
                System.exit(1);
            }
            // get new AF_UNIX/PF_LOCAL socket_name
            System.out.println("FlomHandle.getSocketName() = '" +
                               myHandle.getSocketName() + "'");
            // check socket name
            if (!ndSocketName.equals(myHandle.getSocketName())) {
                System.out.println("Unexpected result from " +
                                   "FlomHandle.set/getSocketName");
                System.exit(1);
            }
            
            /* we don't get current trace filename because it can be altered
               by a global config file */
            /* set a new trace filename */
            myHandle.setTraceFilename(ndTraceFilename);
            /* get new trace filename */
            System.out.println("FlomHandle.getTraceFilename() = '" +
                               myHandle.getTraceFilename() + "'");
            /* check trace filename */
            if (!ndTraceFilename.equals(myHandle.getTraceFilename())) {
                System.out.println("Unexpected result from " +
                                   "FlomHandle.set/getTraceFilename");
                System.exit(1);
            }

            /* get current resource name */
            System.out.println("FlomHandle.getResourceName() = '" +
                               myHandle.getResourceName() + "'");
            /* set a new resource name */
            if (FlomErrorCodes.FLOM_RC_OK != (
                    retCod = myHandle.setResourceName(ndResourceName))) {
                System.out.println("FlomHandle.setResourceName() returned " +
                                   retCod + " '" +
                                   FlomErrorCodes.getText(retCod) + "'");
                System.exit(1);
            }
            /* get new resource name */
            System.out.println("FlomHandle.getResourceName() = '" +
                               myHandle.getResourceName() + "'");
            /* check resource name */
            if (!ndResourceName.equals(myHandle.getResourceName())) {
                System.out.println("Unexpected result from " +
                                   "FlomFandle.set/getResourceName");
                System.exit(1);
            }
    

            myHandle.lock();
            myHandle.unlock();
            myHandle.free();
        } catch(FlomException e) {
            System.out.println("FlomException: ReturnCode=" +
                               e.getReturnCode() +
                               " (" + e.getMessage() + ")");
            System.exit(1);
        }
        System.out.println("Hello world from case4001 class, " +
                           "HappyPath method, again");
    }
    
    public static void main(String[] args) {
        HappyPath();
    }
}
