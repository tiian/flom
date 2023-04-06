/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM, Free Lock Manager
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
#include <iostream>
#include <string>

#include "flom.hh"

using namespace flom;



/*
 * Non default values used for tests
 */
const string ndSocketName("/tmp/flom_socket_name");
const string ndTraceFilename("/tmp/flom.trc");
const string ndResourceName("red.blue.green");
const string ndUnicastAddress("127.0.0.1");
const string ndMulticastAddress("224.0.0.1");
const string ndTlsCertificate("CA1/peer1_CA1_cert.pem");
const string ndTlsPrivateKey("CA1/peer1_CA1_key.pem");
const string ndTlsCaCertificate("CA1/cacert.pem");


/*
 * Happy path usage with a static handle
 */
void staticHandleHappyPath(const string ndNetworkInterface) {
    int retCod;
    FlomHandle myHandle;

    /* get current AF_UNIX/PF_LOCAL socket_name */
    cout << "FlomHandle.getSocketName() = '" << myHandle.getSocketName() <<
        "'" << endl;
    /* set a new AF_UNIX/PF_LOCAL socket_name */
    if (FLOM_RC_OK != (retCod = myHandle.setSocketName(ndSocketName))) {
        cerr << "FlomHandle.setSocketName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* get new AF_UNIX/PF_LOCAL socket_name */
    cout << "FlomHandle.getSocketName() = '" << myHandle.getSocketName() <<
        "'" << endl;
    /* check socket name */
    if (ndSocketName.compare(myHandle.getSocketName())) {
        cerr << "Unexpected result from FlomHandle.set/getSocketName" <<
            endl;
        exit(1);
    }

    /* we don't get current trace filename because it can be altered by a
       global config file */
    /* set a new trace filename */
    myHandle.setTraceFilename(ndTraceFilename);
    /* get new trace filename */
    cout << "FlomHandle.getTraceFilename() = '" << myHandle.getTraceFilename()
         << "'" << endl;
    /* check trace filename */
    if (ndTraceFilename.compare(myHandle.getTraceFilename())) {
        cerr << "Unexpected result from FlomHandle.set/getTraceFilename" <<
            endl;
        exit(1);
    }
    
    /* get current resource name */
    cout << "FlomHandle.getResourceName() = '" << myHandle.getResourceName()
         << "'" << endl;
    /* set a new resource name */
    if (FLOM_RC_OK != (retCod = myHandle.setResourceName(ndResourceName))) {
        cerr << "FlomHandle.setResourceName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* get new resource name */
    cout << "FlomHandle.getResourceName() = '" << myHandle.getResourceName()
         << "'" << endl;
    /* check resource name */
    if (ndResourceName.compare(myHandle.getResourceName())) {
        cerr << "Unexpected result from FlomFandle.set/getResourceName" <<
            endl;
        exit(1);
    }
    
    /* get current value for resource create property */
    cout << "FlomHandle.getResourceCreate() = " << myHandle.getResourceCreate()
         << endl;
    /* set a new value for resource create property */
    myHandle.setResourceCreate(FALSE);
    /* get new value for resource create property */
    cout << "FlomHandle.getResourceCreate() = " << myHandle.getResourceCreate()
         << endl;
    /* check resource create 1/2 */
    if (myHandle.getResourceCreate()) {
        cerr << "Unexpected result from FlomHandle.set/getResourceCreate" <<
            endl;
        exit(1);
    }
    /* set a new value for resource create property */
    myHandle.setResourceCreate(TRUE);
    /* get new value for resource create property */
    cout << "FlomHandle.getResourceCreate() = " << myHandle.getResourceCreate()
         << endl;
    /* check resource create 2/2 */
    if (!myHandle.getResourceCreate()) {
        cerr << "Unexpected result from FlomHandle.set/getResourceCreate" <<
            endl;
        exit(1);
    }
    
    /* get current value for resource timeout property */
    cout << "FlomHandle.getResourceTimeout() = " <<
        myHandle.getResourceTimeout() << endl;
    /* set a new value for resource timeout property */
    myHandle.setResourceTimeout(-1);
    /* get new value for resource timeout property */
    cout << "FlomHandle.getResourceTimeout() = " <<
        myHandle.getResourceTimeout() << endl;
    /* check resource timeout */
    if (-1 != myHandle.getResourceTimeout()) {
        cerr << "Unexpected result from FlomHandle.set/getResourceTimeout" <<
            endl;
        exit(1);
    }
    
    /* get current value for resource quantity property */
    cout << "FlomHandle.getResourceQuantity() = " <<
        myHandle.getResourceQuantity() << endl;
    /* set a new value for resource quantity property */
    myHandle.setResourceQuantity(3);
    /* get new value for resource quantity property */
    cout << "FlomHandle.getResourceQuantity() = " <<
        myHandle.getResourceQuantity() << endl;
    /* check resource quantity */
    if (3 != myHandle.getResourceQuantity()) {
        cerr << "Unexpected result from FlomHandle.set/getResourceQuantity" <<
            endl;
        exit(1);
    }
    
    /* get current value for resource lock mode property */
    cout << "FlomHandle.getLockMode() = " <<
        myHandle.getLockMode() << endl;
    /* set a new value for resource lock mode property */
    myHandle.setLockMode(FLOM_LOCK_MODE_PW);
    /* get new value for resource lock mode property */
    cout << "FlomHandle.getLockMode() = " <<
        myHandle.getLockMode() << endl;
    /* check resource lock mode */
    if (FLOM_LOCK_MODE_PW != myHandle.getLockMode()) {
        cerr << "Unexpected result from FlomHandle.set/getLockMode" <<
            endl;
        exit(1);
    }
    
    /* get current value for resource idle lifespan */
    cout << "FlomHandle.getResourceIdleLifespan() = " <<
        myHandle.getResourceIdleLifespan() << endl;
    /* set a new value for resource idle lifespan */
    myHandle.setResourceIdleLifespan(10000);
    /* get new value for resource idle lifespan */
    cout << "FlomHandle.getResourceIdleLifespan() = " <<
        myHandle.getResourceIdleLifespan() << endl;
    /* check resource idle lifespan */
    if (10000 != myHandle.getResourceIdleLifespan()) {
        cerr << "Unexpected result from FlomHandle.set/getResourceIdleLifespan"
             << endl;
        exit(1);
    }
    
    /* get current unicast address */
    cout << "FlomHandle.getUnicastAddress() = '" <<
        myHandle.getUnicastAddress() << "'" << endl;
    /* set a new unicast_address */
    myHandle.setUnicastAddress(ndUnicastAddress);
    /* get new unicast address */
    cout << "FlomHandle.getUnicastAddress() = '" <<
        myHandle.getUnicastAddress() << "'" << endl;
    /* check unicast address */
    if (ndUnicastAddress.compare(myHandle.getUnicastAddress())) {
        cerr << "Unexpected result from FlomHandle.set/getUnicastAddress"
             << endl;
        exit(1);
    }
    
    /* get current multicast address */
    cout << "FlomHandle.getMulticastAddress() = '" <<
        myHandle.getMulticastAddress() << "'" << endl;
    /* set a new multicast_address */
    myHandle.setMulticastAddress(ndMulticastAddress);
    /* get new multicast address */
    cout << "FlomHandle.getMulticastAddress() = '" <<
        myHandle.getMulticastAddress() << "'" << endl;
    /* check multicast address */
    if (ndMulticastAddress.compare(myHandle.getMulticastAddress())) {
        cerr << "Unexpected result from FlomHandle.set/getMulticastAddress"
             << endl;
        exit(1);
    }
    
    /* get current value for unicast port */
    cout << "FlomHandle.getUnicastPort() = " <<
        myHandle.getUnicastPort() << endl;
    /* set a new value for unicast_port */
    myHandle.setUnicastPort(7777);
    /* get new value for unicast port */
    cout << "FlomHandle.getUnicastPort() = " <<
        myHandle.getUnicastPort() << endl;
    /* check unicast port */
    if (7777 != myHandle.getUnicastPort()) {
        cerr << "Unexpected result from FlomHandle.set/getUnicastPort"
             << endl;
        exit(1);
    }
    
    /* get current value for multicast port */
    cout << "FlomHandle.getMulticastPort() = " <<
        myHandle.getMulticastPort() << endl;
    /* set a new value for multicast_port */
    myHandle.setMulticastPort(8888);
    /* get new value for multicast port */
    cout << "FlomHandle.getMulticastPort() = " <<
        myHandle.getMulticastPort() << endl;
    /* check multicast port */
    if (8888 != myHandle.getMulticastPort()) {
        cerr << "Unexpected result from FlomHandle.set/getMulticastPort"
             << endl;
        exit(1);
    }
    
    /* get current value for discovery attempts property */
    cout << "FlomHandle.getDiscoveryAttempts() = " <<
        myHandle.getDiscoveryAttempts() << endl;
    /* set a new value for discovery attempts property */
    myHandle.setDiscoveryAttempts(5);
    /* get new value for discovery attempts */
    cout << "FlomHandle.getDiscoveryAttempts() = " <<
        myHandle.getDiscoveryAttempts() << endl;
    /* check discovery attempts */
    if (5 != myHandle.getDiscoveryAttempts()) {
        cerr << "Unexpected result from FlomHandle.set/getDiscoveryAttempts"
             << endl;
        exit(1);
    }
    
    /* get current value for discovery timeout property */
    cout << "FlomHandle.getDiscoveryTimeout() = " <<
        myHandle.getDiscoveryTimeout() << endl;
    /* set a new value for discovery timeout property */
    myHandle.setDiscoveryTimeout(750);
    /* get new value for discovery timeout */
    cout << "FlomHandle.getDiscoveryTimeout() = " <<
        myHandle.getDiscoveryTimeout() << endl;
    /* check discovery timeout */
    if (750 != myHandle.getDiscoveryTimeout()) {
        cerr << "Unexpected result from FlomHandle.set/getDiscoveryTimeout"
             << endl;
        exit(1);
    }
    
    /* get current value for discovery ttl property */
    cout << "FlomHandle.getDiscoveryTtl() = " <<
        myHandle.getDiscoveryTtl() << endl;
    /* set a new value for discovery ttl property */
    myHandle.setDiscoveryTtl(2);
    /* get new value for discovery ttl */
    cout << "FlomHandle.getDiscoveryTtl() = " <<
        myHandle.getDiscoveryTtl() << endl;
    /* check discovery ttl */
    if (2 != myHandle.getDiscoveryTtl()) {
        cerr << "Unexpected result from FlomHandle.set/getDiscoveryTtl"
             << endl;
        exit(1);
    }
    
    /* get current value for TLS certificate */
    cerr << "FlomHandle.getTlsCertificate() = '"
         << myHandle.getTlsCertificate() << "'" << endl;
    /* set a new TLS certificate */
    if (FLOM_RC_OK != (retCod = myHandle.setTlsCertificate(
                           ndTlsCertificate))) {
        FlomException excp(retCod);
        cerr << "FlomHandle.setTlsCertificate() returned "
             << retCod << ", '" << excp.getReturnCodeText() << "'" << endl;
        exit(1);
    }
    /* get new TLS certificate */
    cerr << "FlomHandle.getTlsCertificate() = '"
         << myHandle.getTlsCertificate() << "'" << endl;

    /* get current value for TLS private key */
    cerr << "FlomHandle.getTlsPrivateKey() = '"
         << myHandle.getTlsPrivateKey() << "'" << endl;
    /* set a new TLS private key */
    if (FLOM_RC_OK != (retCod = myHandle.setTlsPrivateKey(
                           ndTlsPrivateKey))) {
        FlomException excp(retCod);
        cerr << "FlomHandle.setTlsPrivateKey() returned "
             << retCod << ", '" << excp.getReturnCodeText() << "'" << endl;
        exit(1);
    }
    /* get new TLS private key */
    cerr << "FlomHandle.getTlsPrivateKey() = '"
         << myHandle.getTlsPrivateKey() << "'" << endl;

    /* get current value for TLS CA certificate */
    cerr << "FlomHandle.getTlsCaCertificate() = '"
         << myHandle.getTlsCaCertificate() << "'" << endl;
    /* set a new TLS CA certificate */
    if (FLOM_RC_OK != (retCod = myHandle.setTlsCaCertificate(
                           ndTlsCaCertificate))) {
        FlomException excp(retCod);
        cerr << "FlomHandle.setTlsCaCertificate() returned "
             << retCod << ", '" << excp.getReturnCodeText() << "'" << endl;
        exit(1);
    }
    /* get new TLS CA certificate */
    cerr << "FlomHandle.getTlsCaCertificate() = '"
         << myHandle.getTlsCaCertificate() << "'" << endl;

    /* get current value for TLS check peer ID property */
    cout << "FlomHandle.getTlsCheckPeerId() = " << myHandle.getTlsCheckPeerId()
         << endl;
    /* set a new value for TLS check peer ID property */
    myHandle.setTlsCheckPeerId(FALSE);
    /* get new value for TLS check peer ID property */
    cout << "FlomHandle.getTlsCheckPeerId() = " << myHandle.getTlsCheckPeerId()
         << endl;
    /* check TLS check peer ID 1/2 */
    if (myHandle.getTlsCheckPeerId()) {
        cerr << "Unexpected result from FlomHandle.set/getTlsCheckPeerId" <<
            endl;
        exit(1);
    }
    /* set a new value for TLS check peer ID property */
    myHandle.setTlsCheckPeerId(TRUE);
    /* get new value for TLS check peer ID property */
    cout << "FlomHandle.getTlsCheckPeerId() = " << myHandle.getTlsCheckPeerId()
         << endl;
    /* check TLS check peer ID 2/2 */
    if (!myHandle.getTlsCheckPeerId()) {
        cerr << "Unexpected result from FlomHandle.set/getTlsCheckPeerId" <<
            endl;
        exit(1);
    }
    
    /* get current network interface */
    cout << "FlomHandle.getNetworkInterface() = '"
         << myHandle.getNetworkInterface() << "'" << endl;
    /* set a new network interface */
    if (FLOM_RC_OK == myHandle.setNetworkInterface(ndNetworkInterface)) {
        /* check network interface */
        if (ndNetworkInterface.compare(myHandle.getNetworkInterface())) {
            cerr << "Unexpected result from FlomHandle/getNetworkInterface: "
                 << "expected was '" << ndNetworkInterface << "'"
                 << "obtained is '" << myHandle.getNetworkInterface() << "'"
                 << endl;
            exit(1);
        }
    } else {
        cerr << "'" << ndNetworkInterface << "' is not a valid IPv6 network "
             << "interface for this system" << endl;
        exit(1);
    }
    
    /* set AF_UNIX/PF_LOCAL socket_name again */
    if (FLOM_RC_OK != (retCod = myHandle.setSocketName(ndSocketName))) {
        cerr << "FlomHandle.setSocketName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle.lock())) {
        cerr << "FlomHandle.lock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    } else {
        cout << "staticHandleHappyPath locked element is " <<
            myHandle.getLockedElement() << endl;
    }   
    /* lock release */
    if (FLOM_RC_OK != (retCod = myHandle.unlock())) {
        cerr << "FlomHandle.unlock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
}



/*
 * Happy path usage with a dynamic handle
 */
void dynamicHandleHappyPath(const string ndNetworkInterface) {
    int retCod;
    FlomHandle *myHandle = NULL;

    /* create a new handle */
    if (NULL == (myHandle = new FlomHandle())) {
        cerr << "FlomHandle() returned %p" << myHandle << endl;
        exit(1);
    }

    /* get current AF_UNIX/PF_LOCAL socket_name */
    cout << "FlomHandle->getSocketName() = '" << myHandle->getSocketName() <<
        "'" << endl;
    /* set a new AF_UNIX/PF_LOCAL socket_name */
    if (FLOM_RC_OK != (retCod = myHandle->setSocketName(ndSocketName))) {
        cerr << "FlomHandle->setSocketName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* get new AF_UNIX/PF_LOCAL socket_name */
    cout << "FlomHandle->getSocketName() = '" << myHandle->getSocketName() <<
        "'" << endl;
    /* check socket name */
    if (ndSocketName.compare(myHandle->getSocketName())) {
        cerr << "Unexpected result from FlomHandle->set/getSocketName" <<
            endl;
        exit(1);
    }

    /* we don't get current trace filename because it can be altered by a
       global config file */
    /* set a new trace filename */
    myHandle->setTraceFilename(ndTraceFilename);
    /* get new trace filename */
    cout << "FlomHandle->getTraceFilename() = '"
         << myHandle->getTraceFilename()
         << "'" << endl;
    /* check trace filename */
    if (ndTraceFilename.compare(myHandle->getTraceFilename())) {
        cerr << "Unexpected result from FlomHandle->set/getTraceFilename" <<
            endl;
        exit(1);
    }
    
    /* get current resource name */
    cout << "FlomHandle->getResourceName() = '" << myHandle->getResourceName()
         << "'" << endl;
    /* set a new resource name */
    if (FLOM_RC_OK != (retCod = myHandle->setResourceName(ndResourceName))) {
        cerr << "FlomHandle->setResourceName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* get new resource name */
    cout << "FlomHandle->getResourceName() = '" << myHandle->getResourceName()
         << "'" << endl;
    /* check resource name */
    if (ndResourceName.compare(myHandle->getResourceName())) {
        cerr << "Unexpected result from FlomFandle.set/getResourceName" <<
            endl;
        exit(1);
    }
    
    /* get current value for resource create property */
    cout << "FlomHandle->getResourceCreate() = " <<
        myHandle->getResourceCreate() << endl;
    /* set a new value for resource create property */
    myHandle->setResourceCreate(FALSE);
    /* get new value for resource create property */
    cout << "FlomHandle->getResourceCreate() = " <<
        myHandle->getResourceCreate() << endl;
    /* check resource create 1/2 */
    if (myHandle->getResourceCreate()) {
        cerr << "Unexpected result from FlomHandle->set/getResourceCreate" <<
            endl;
        exit(1);
    }
    /* set a new value for resource create property */
    myHandle->setResourceCreate(TRUE);
    /* get new value for resource create property */
    cout << "FlomHandle->getResourceCreate() = " <<
        myHandle->getResourceCreate() << endl;
    /* check resource create 2/2 */
    if (!myHandle->getResourceCreate()) {
        cerr << "Unexpected result from FlomHandle->set/getResourceCreate" <<
            endl;
        exit(1);
    }
    
    /* get current value for resource timeout property */
    cout << "FlomHandle->getResourceTimeout() = " <<
        myHandle->getResourceTimeout() << endl;
    /* set a new value for resource timeout property */
    myHandle->setResourceTimeout(-1);
    /* get new value for resource timeout property */
    cout << "FlomHandle->getResourceTimeout() = " <<
        myHandle->getResourceTimeout() << endl;
    /* check resource timeout */
    if (-1 != myHandle->getResourceTimeout()) {
        cerr << "Unexpected result from FlomHandle->set/getResourceTimeout" <<
            endl;
        exit(1);
    }
    
    /* get current value for resource quantity property */
    cout << "FlomHandle->getResourceQuantity() = " <<
        myHandle->getResourceQuantity() << endl;
    /* set a new value for resource quantity property */
    myHandle->setResourceQuantity(3);
    /* get new value for resource quantity property */
    cout << "FlomHandle->getResourceQuantity() = " <<
        myHandle->getResourceQuantity() << endl;
    /* check resource quantity */
    if (3 != myHandle->getResourceQuantity()) {
        cerr << "Unexpected result from FlomHandle->set/getResourceQuantity" <<
            endl;
        exit(1);
    }
    
    /* get current value for resource lock mode property */
    cout << "FlomHandle->getLockMode() = " <<
        myHandle->getLockMode() << endl;
    /* set a new value for resource lock mode property */
    myHandle->setLockMode(FLOM_LOCK_MODE_PW);
    /* get new value for resource lock mode property */
    cout << "FlomHandle->getLockMode() = " <<
        myHandle->getLockMode() << endl;
    /* check resource lock mode */
    if (FLOM_LOCK_MODE_PW != myHandle->getLockMode()) {
        cerr << "Unexpected result from FlomHandle->set/getLockMode" <<
            endl;
        exit(1);
    }
    
    /* get current value for resource idle lifespan */
    cout << "FlomHandle->getResourceIdleLifespan() = " <<
        myHandle->getResourceIdleLifespan() << endl;
    /* set a new value for resource idle lifespan */
    myHandle->setResourceIdleLifespan(10000);
    /* get new value for resource idle lifespan */
    cout << "FlomHandle->getResourceIdleLifespan() = " <<
        myHandle->getResourceIdleLifespan() << endl;
    /* check resource idle lifespan */
    if (10000 != myHandle->getResourceIdleLifespan()) {
        cerr << "Unexpected result from FlomHandle->set/getResourceIdleLifespan"
             << endl;
        exit(1);
    }
    
    /* get current unicast address */
    cout << "FlomHandle->getUnicastAddress() = '" <<
        myHandle->getUnicastAddress() << "'" << endl;
    /* set a new unicast_address */
    myHandle->setUnicastAddress(ndUnicastAddress);
    /* get new unicast address */
    cout << "FlomHandle->getUnicastAddress() = '" <<
        myHandle->getUnicastAddress() << "'" << endl;
    /* check unicast address */
    if (ndUnicastAddress.compare(myHandle->getUnicastAddress())) {
        cerr << "Unexpected result from FlomHandle->set/getUnicastAddress"
             << endl;
        exit(1);
    }
    
    /* get current multicast address */
    cout << "FlomHandle->getMulticastAddress() = '" <<
        myHandle->getMulticastAddress() << "'" << endl;
    /* set a new multicast_address */
    myHandle->setMulticastAddress(ndMulticastAddress);
    /* get new multicast address */
    cout << "FlomHandle->getMulticastAddress() = '" <<
        myHandle->getMulticastAddress() << "'" << endl;
    /* check multicast address */
    if (ndMulticastAddress.compare(myHandle->getMulticastAddress())) {
        cerr << "Unexpected result from FlomHandle->set/getMulticastAddress"
             << endl;
        exit(1);
    }
    
    /* get current value for unicast port */
    cout << "FlomHandle->getUnicastPort() = " <<
        myHandle->getUnicastPort() << endl;
    /* set a new value for unicast_port */
    myHandle->setUnicastPort(7777);
    /* get new value for unicast port */
    cout << "FlomHandle->getUnicastPort() = " <<
        myHandle->getUnicastPort() << endl;
    /* check unicast port */
    if (7777 != myHandle->getUnicastPort()) {
        cerr << "Unexpected result from FlomHandle->set/getUnicastPort"
             << endl;
        exit(1);
    }
    
    /* get current value for multicast port */
    cout << "FlomHandle->getMulticastPort() = " <<
        myHandle->getMulticastPort() << endl;
    /* set a new value for multicast_port */
    myHandle->setMulticastPort(8888);
    /* get new value for multicast port */
    cout << "FlomHandle->getMulticastPort() = " <<
        myHandle->getMulticastPort() << endl;
    /* check multicast port */
    if (8888 != myHandle->getMulticastPort()) {
        cerr << "Unexpected result from FlomHandle->set/getMulticastPort"
             << endl;
        exit(1);
    }
    
    /* get current value for discovery attempts property */
    cout << "FlomHandle->getDiscoveryAttempts() = " <<
        myHandle->getDiscoveryAttempts() << endl;
    /* set a new value for discovery attempts property */
    myHandle->setDiscoveryAttempts(5);
    /* get new value for discovery attempts */
    cout << "FlomHandle->getDiscoveryAttempts() = " <<
        myHandle->getDiscoveryAttempts() << endl;
    /* check discovery attempts */
    if (5 != myHandle->getDiscoveryAttempts()) {
        cerr << "Unexpected result from FlomHandle->set/getDiscoveryAttempts"
             << endl;
        exit(1);
    }
    
    /* get current value for discovery timeout property */
    cout << "FlomHandle->getDiscoveryTimeout() = " <<
        myHandle->getDiscoveryTimeout() << endl;
    /* set a new value for discovery timeout property */
    myHandle->setDiscoveryTimeout(750);
    /* get new value for discovery timeout */
    cout << "FlomHandle->getDiscoveryTimeout() = " <<
        myHandle->getDiscoveryTimeout() << endl;
    /* check discovery timeout */
    if (750 != myHandle->getDiscoveryTimeout()) {
        cerr << "Unexpected result from FlomHandle->set/getDiscoveryTimeout"
             << endl;
        exit(1);
    }
    
    /* get current value for discovery ttl property */
    cout << "FlomHandle->getDiscoveryTtl() = " <<
        myHandle->getDiscoveryTtl() << endl;
    /* set a new value for discovery ttl property */
    myHandle->setDiscoveryTtl(2);
    /* get new value for discovery ttl */
    cout << "FlomHandle->getDiscoveryTtl() = " <<
        myHandle->getDiscoveryTtl() << endl;
    /* check discovery ttl */
    if (2 != myHandle->getDiscoveryTtl()) {
        cerr << "Unexpected result from FlomHandle->set/getDiscoveryTtl"
             << endl;
        exit(1);
    }
    
    /* get current value for TLS certificate */
    cerr << "FlomHandle->getTlsCertificate() = '"
         << myHandle->getTlsCertificate() << "'" << endl;
    /* set a new TLS certificate */
    if (FLOM_RC_OK != (retCod = myHandle->setTlsCertificate(
                           ndTlsCertificate))) {
        FlomException excp(retCod);
        cerr << "FlomHandle->setTlsCertificate() returned "
             << retCod << ", '" << excp.getReturnCodeText() << "'" << endl;
        exit(1);
    }
    /* get new TLS certificate */
    cerr << "FlomHandle->getTlsCertificate() = '"
         << myHandle->getTlsCertificate() << "'" << endl;

    /* get current value for TLS private key */
    cerr << "FlomHandle->getTlsPrivateKey() = '"
         << myHandle->getTlsPrivateKey() << "'" << endl;
    /* set a new TLS private key */
    if (FLOM_RC_OK != (retCod = myHandle->setTlsPrivateKey(
                           ndTlsPrivateKey))) {
        FlomException excp(retCod);
        cerr << "FlomHandle->setTlsPrivateKey() returned "
             << retCod << ", '" << excp.getReturnCodeText() << "'" << endl;
        exit(1);
    }
    /* get new TLS private key */
    cerr << "FlomHandle->getTlsPrivateKey() = '"
         << myHandle->getTlsPrivateKey() << "'" << endl;

    /* get current value for TLS CA certificate */
    cerr << "FlomHandle->getTlsCaCertificate() = '"
         << myHandle->getTlsCaCertificate() << "'" << endl;
    /* set a new TLS CA certificate */
    if (FLOM_RC_OK != (retCod = myHandle->setTlsCaCertificate(
                           ndTlsCaCertificate))) {
        FlomException excp(retCod);
        cerr << "FlomHandle->setTlsCaCertificate() returned "
             << retCod << ", '" << excp.getReturnCodeText() << "'" << endl;
        exit(1);
    }
    /* get new TLS CA certificate */
    cerr << "FlomHandle->getTlsCaCertificate() = '"
         << myHandle->getTlsCaCertificate() << "'" << endl;

    /* get current value for TLS check peer ID property */
    cout << "FlomHandle->getTlsCheckPeerId() = " <<
        myHandle->getTlsCheckPeerId() << endl;
    /* set a new value for TLS check peer ID property */
    myHandle->setTlsCheckPeerId(FALSE);
    /* get new value for TLS check peer ID property */
    cout << "FlomHandle->getTlsCheckPeerId() = " <<
        myHandle->getTlsCheckPeerId() << endl;
    /* check TLS check peer ID 1/2 */
    if (myHandle->getTlsCheckPeerId()) {
        cerr << "Unexpected result from FlomHandle->set/getTlsCheckPeerId" <<
            endl;
        exit(1);
    }
    /* set a new value for TLS check peer ID property */
    myHandle->setTlsCheckPeerId(TRUE);
    /* get new value for TLS check peer ID property */
    cout << "FlomHandle->getTlsCheckPeerId() = " <<
        myHandle->getTlsCheckPeerId() << endl;
    /* check TLS check peer ID 2/2 */
    if (!myHandle->getTlsCheckPeerId()) {
        cerr << "Unexpected result from FlomHandle->set/getTlsCheckPeerId" <<
            endl;
        exit(1);
    }
    
    /* get current network interface */
    cout << "FlomHandle->getNetworkInterface() = '"
         << myHandle->getNetworkInterface() << "'" << endl;
    /* set a new network interface */
    if (FLOM_RC_OK == myHandle->setNetworkInterface(ndNetworkInterface)) {
        /* get new network interface */
        /*
        cerr << "FlomHandle->getNetworkInterface() = '" 
             << myHandle->getNetworkInterface() << "'" << endl;
        */
        /* check network interface */
        if (ndNetworkInterface.compare(myHandle->getNetworkInterface())) {
            cerr << "Unexpected result from FlomHandle/getNetworkInterface: "
                 << "expected was '" << ndNetworkInterface << "'"
                 << "obtained is '" << myHandle->getNetworkInterface() << "'"
                 << endl;
            exit(1);
        }
    } else {
        cerr << "'" << ndNetworkInterface << "' is not a valid IPv6 network "
             << "interface for this system" << endl;
        exit(1);
    }
    
    /* set AF_UNIX/PF_LOCAL socket_name again */
    if (FLOM_RC_OK != (retCod = myHandle->setSocketName(ndSocketName))) {
        cerr << "FlomHandle->setSocketName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle->lock())) {
        cerr << "FlomHandle->lock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    } else {
        cout << "dynamicHandleHappyPath locked element is " <<
            myHandle->getLockedElement() << endl;
    } 
    /* lock release */
    if (FLOM_RC_OK != (retCod = myHandle->unlock())) {
        cerr << "FlomHandle->unlock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    
    /* delete the handle */
    delete myHandle;
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "First argument must be a valid IPv6 network interface"
             << endl;
        exit(1);
    }
    string NetworkInterface = string(argv[1]);
    /* static handle tests */
    staticHandleHappyPath(NetworkInterface);
    /* dynamic handle test */
    dynamicHandleHappyPath(NetworkInterface);
    /* exit */
    return 0;
}
