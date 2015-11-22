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
    private final static String ndUnicastAddress = "127.0.0.1";
    private final static String ndMulticastAddress = "224.0.0.1";
    
    private static void HappyPath(String ndNetworkInterface) {
        try {
            FlomHandle myHandle = new FlomHandle();
            int retCod;

            // get current AF_UNIX/PF_LOCAL socket_name
            System.out.println("FlomHandle.getSocketName() = '" +
                               myHandle.getSocketName() + "'");
            // set a new AF_UNIX/PF_LOCAL socket_name
            if (FlomErrorCodes.FLOM_RC_OK != (
                    retCod = myHandle.setSocketName(ndSocketName))) {
                System.err.println("FlomHandle.setSocketName() returned " +
                                   retCod + " '" +
                                   FlomErrorCodes.getText(retCod) + "'");
                System.exit(1);
            }
            // get new AF_UNIX/PF_LOCAL socket_name
            System.out.println("FlomHandle.getSocketName() = '" +
                               myHandle.getSocketName() + "'");
            // check socket name
            if (!ndSocketName.equals(myHandle.getSocketName())) {
                System.err.println("Unexpected result from " +
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
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getTraceFilename");
                System.exit(1);
            }

            /* get current resource name */
            System.out.println("FlomHandle.getResourceName() = '" +
                               myHandle.getResourceName() + "'");
            /* set a new resource name */
            if (FlomErrorCodes.FLOM_RC_OK != (
                    retCod = myHandle.setResourceName(ndResourceName))) {
                System.err.println("FlomHandle.setResourceName() returned " +
                                   retCod + " '" +
                                   FlomErrorCodes.getText(retCod) + "'");
                System.exit(1);
            }
            /* get new resource name */
            System.out.println("FlomHandle.getResourceName() = '" +
                               myHandle.getResourceName() + "'");
            /* check resource name */
            if (!ndResourceName.equals(myHandle.getResourceName())) {
                System.err.println("Unexpected result from " +
                                   "FlomFandle.set/getResourceName");
                System.exit(1);
            }
    
            /* get current value for resource create property */
            System.out.println("FlomHandle.getResourceCreate() = " +
                               myHandle.getResourceCreate());
            /* set a new value for resource create property */
            myHandle.setResourceCreate(false);
            /* get new value for resource create property */
            System.out.println("FlomHandle.getResourceCreate() = " +
                               myHandle.getResourceCreate());
            /* check resource create 1/2 */
            if (myHandle.getResourceCreate()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getResourceCreate");
                System.exit(1);
            }
            /* set a new value for resource create property */
            myHandle.setResourceCreate(true);
            /* get new value for resource create property */
            System.out.println("FlomHandle.getResourceCreate() = " +
                               myHandle.getResourceCreate());
            /* check resource create 2/2 */
            if (!myHandle.getResourceCreate()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getResourceCreate");
                System.exit(1);
            }

            /* get current value for resource timeout property */
            System.out.println("FlomHandle.getResourceTimeout() = " +
                               myHandle.getResourceTimeout());
            /* set a new value for resource timeout property */
            myHandle.setResourceTimeout(-1);
            /* get new value for resource timeout property */
            System.out.println("FlomHandle.getResourceTimeout() = " +
                               myHandle.getResourceTimeout());
            /* check resource timeout */
            if (-1 != myHandle.getResourceTimeout()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getResourceTimeout");
                System.exit(1);
            }
    
            /* get current value for resource quantity property */
            System.out.println("FlomHandle.getResourceQuantity() = " +
                               myHandle.getResourceQuantity());
            /* set a new value for resource quantity property */
            myHandle.setResourceQuantity(3);
            /* get new value for resource quantity property */
            System.out.println("FlomHandle.getResourceQuantity() = " +
                               myHandle.getResourceQuantity());
            /* check resource quantity */
            if (3 != myHandle.getResourceQuantity()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getResourceQuantity");
                System.exit(1);
            }
            
            /* get current value for resource lock mode property */
            System.out.println("FlomHandle.getLockMode() = " +
                               myHandle.getLockMode());
            /* set a new value for resource lock mode property */
            myHandle.setLockMode(FlomLockModes.FLOM_LOCK_MODE_PW);
            /* get new value for resource lock mode property */
            System.out.println("FlomHandle.getLockMode() = " +
                               myHandle.getLockMode());
            /* check resource lock mode */
            if (FlomLockModes.FLOM_LOCK_MODE_PW != myHandle.getLockMode()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getLockMode");
                System.exit(1);
            }
    
            /* get current value for resource idle lifespan */
            System.out.println("FlomHandle.getResourceIdleLifespan() = " +
                               myHandle.getResourceIdleLifespan());
            /* set a new value for resource idle lifespan */
            myHandle.setResourceIdleLifespan(10000);
            /* get new value for resource idle lifespan */
            System.out.println("FlomHandle.getResourceIdleLifespan() = " +
                               myHandle.getResourceIdleLifespan());
            /* check resource idle lifespan */
            if (10000 != myHandle.getResourceIdleLifespan()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getResourceIdleLifespan");
                System.exit(1);
            }
    
            /* get current unicast address */
            System.out.println("FlomHandle.getUnicastAddress() = '" +
                               myHandle.getUnicastAddress() + "'");
            /* set a new unicast_address */
            myHandle.setUnicastAddress(ndUnicastAddress);
            /* get new unicast address */
            System.out.println("FlomHandle.getUnicastAddress() = '" +
                               myHandle.getUnicastAddress() + "'");
            /* check unicast address */
            if (!ndUnicastAddress.equals(myHandle.getUnicastAddress())) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getUnicastAddress");
                System.exit(1);
            }
    
            /* get current multicast address */
            System.out.println("FlomHandle.getMulticastAddress() = '" +
                               myHandle.getMulticastAddress() + "'");
            /* set a new multicast_address */
            myHandle.setMulticastAddress(ndMulticastAddress);
            /* get new multicast address */
            System.out.println("FlomHandle.getMulticastAddress() = '" +
                               myHandle.getMulticastAddress() + "'");
            /* check multicast address */
            if (!ndMulticastAddress.equals(myHandle.getMulticastAddress())) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getMulticastAddress");
                System.exit(1);
            }
    
            /* get current value for unicast port */
            System.out.println("FlomHandle.getUnicastPort() = " +
                               myHandle.getUnicastPort());
            /* set a new value for unicast_port */
            myHandle.setUnicastPort(7777);
            /* get new value for unicast port */
            System.out.println("FlomHandle.getUnicastPort() = " +
                               myHandle.getUnicastPort());
            /* check unicast port */
            if (7777 != myHandle.getUnicastPort()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getUnicastPort");
                System.exit(1);
            }
    
            /* get current value for multicast port */
            System.out.println("FlomHandle.getMulticastPort() = " +
                               myHandle.getMulticastPort());
            /* set a new value for multicast_port */
            myHandle.setMulticastPort(8888);
            /* get new value for multicast port */
            System.out.println("FlomHandle.getMulticastPort() = " +
                               myHandle.getMulticastPort());
            /* check multicast port */
            if (8888 != myHandle.getMulticastPort()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getMulticastPort");
                System.exit(1);
            }
    
            /* get current value for discovery attempts property */
            System.out.println("FlomHandle.getDiscoveryAttempts() = " +
                               myHandle.getDiscoveryAttempts());
            /* set a new value for discovery attempts property */
            myHandle.setDiscoveryAttempts(5);
            /* get new value for discovery attempts */
            System.out.println("FlomHandle.getDiscoveryAttempts() = " +
                               myHandle.getDiscoveryAttempts());
            /* check discovery attempts */
            if (5 != myHandle.getDiscoveryAttempts()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getDiscoveryAttempts");
                    System.exit(1);
            }
    
            /* get current value for discovery timeout property */
            System.out.println("FlomHandle.getDiscoveryTimeout() = " +
                               myHandle.getDiscoveryTimeout());
            /* set a new value for discovery timeout property */
            myHandle.setDiscoveryTimeout(750);
            /* get new value for discovery timeout */
            System.out.println("FlomHandle.getDiscoveryTimeout() = " +
                               myHandle.getDiscoveryTimeout());
            /* check discovery timeout */
            if (750 != myHandle.getDiscoveryTimeout()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getDiscoveryTimeout");
                System.exit(1);
            }
    
            /* get current value for discovery ttl property */
            System.out.println("FlomHandle.getDiscoveryTtl() = " +
                               myHandle.getDiscoveryTtl());
            /* set a new value for discovery ttl property */
            myHandle.setDiscoveryTtl(2);
            /* get new value for discovery ttl */
            System.out.println("FlomHandle.getDiscoveryTtl() = " +
                               myHandle.getDiscoveryTtl());
            /* check discovery ttl */
            if (2 != myHandle.getDiscoveryTtl()) {
                System.err.println("Unexpected result from " +
                                   "FlomHandle.set/getDiscoveryTtl");
                System.exit(1);
            }
    
            /* get current network interface */
            System.err.println("FlomHandle.getNetworkInterface() = '" +
                               myHandle.getNetworkInterface() + "'");
            /* set a new network interface */
            if (FlomErrorCodes.FLOM_RC_OK ==
                myHandle.setNetworkInterface(ndNetworkInterface)) {
                /* get new metwork interface */
                System.err.println("FlomHandle.getNetworkInterface() = '" +
                                   myHandle.getNetworkInterface() + "'");
                /* check network interface */
                if (!ndNetworkInterface.equals(
                        myHandle.getNetworkInterface())) {
                    System.err.println("Unexpected result from " +
                                       "FlomHandle.set/getNetworkInterface");
                    System.exit(1);
                }
            } else {
                System.err.println("'" + ndNetworkInterface + "' is not a " +
                                   "valid IPv6 network interface for this " +
                                   "system");
                System.exit(1);
            }
    
            // set AF_UNIX/PF_LOCAL socket_name
            if (FlomErrorCodes.FLOM_RC_OK != (
                    retCod = myHandle.setSocketName(ndSocketName))) {
                System.err.println("FlomHandle.setSocketName() returned " +
                                   retCod + " '" +
                                   FlomErrorCodes.getText(retCod) + "'");
                System.exit(1);
            }
            
            myHandle.lock();
            System.out.println("HappyPath locked element is " +
                               myHandle.getLockedElement());
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
        if (args.length < 1) {
            System.err.println("First argument must be a valid IPv6 " +
                               "network interface");
            System.exit(1);
        }
        HappyPath(args[0]);
        HappyPath(args[0]);
    }
}
