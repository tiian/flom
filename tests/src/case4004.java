/*
 * Copyright (c) 2013-2021, Christian Ferrari <tiian@users.sourceforge.net>
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



public class case4004 {
    
    private static void sequenceResourceTest() {
        try {
            int retCod;
            /* used for non transactional resource */
            FlomHandle myHandle1 = new FlomHandle();
            /* used for transactional resource */
            FlomHandle myHandle2 = new FlomHandle();

            /* First step: non transactional resource */
            /* set a new resource name */
            if (FlomErrorCodes.FLOM_RC_OK != (
                    retCod = myHandle1.setResourceName(
                        "_s_nontransactional[1]"))) {
                System.err.println("FlomHandle.setResourceName() returned " +
                                   retCod + " '" +
                                   FlomErrorCodes.getText(retCod) + "'");
                System.exit(1);
            }
            /* set a new value for resource idle lifespan */
            myHandle1.setResourceIdleLifespan(60000);
            /* lock acquisition */
            myHandle1.lock();
            System.out.println("locked element is " +
                               myHandle1.getLockedElement());
            /* lock release & rollback: the resource is not transactional, the
             * function must return a warning condition */
            try {
                /* this operation must return an exception because the
                   resource is not transactional */
                myHandle1.unlockRollback();
            } catch(FlomException e) {
                if (FlomErrorCodes.FLOM_RC_RESOURCE_IS_NOT_TRANSACTIONAL !=
                    e.getReturnCode())
                    throw(e);
            }
            /* lock acquisition */
            myHandle1.lock();
            System.out.println("locked element is " +
                               myHandle1.getLockedElement());
            /* the resource associated to myHandle1 is intentionally not
             * unlocked to check the behavior in case of abort */
            
            /* Second step: non transactional resource */
            /* set a new resource name */
            if (FlomErrorCodes.FLOM_RC_OK != (
                    retCod = myHandle2.setResourceName(
                        "_S_nontransactional[1]"))) {
                System.err.println("FlomHandle.setResourceName() returned " +
                                   retCod + " '" +
                                   FlomErrorCodes.getText(retCod) + "'");
                System.exit(1);
            }
            /* set a new value for resource idle lifespan */
            myHandle2.setResourceIdleLifespan(60000);
            /* lock acquisition */
            myHandle2.lock();
            System.out.println("locked element is " +
                               myHandle2.getLockedElement());
            /* lock release & rollback: the resource is transactional, the
             * function must NOT return a warning condition */
            myHandle2.unlockRollback();
            /* lock acquisition */
            myHandle2.lock();
            System.out.println("locked element is " +
                               myHandle2.getLockedElement());
            /* lock release */
            myHandle2.unlock();
            /* lock acquisition */
            myHandle2.lock();
            System.out.println("locked element is " +
                               myHandle2.getLockedElement());
            /* interrupt execution to verify transactionality (the program
               must be restarted */
            System.exit(0);
            /* this point will be never reached! */

            
        } catch(FlomException e) {
            System.out.println("FlomException: ReturnCode=" +
                               e.getReturnCode() +
                               " (" + e.getMessage() + ")");
            System.exit(1);
        }
    }
    
    public static void main(String[] args) {
        sequenceResourceTest();
    }
}
