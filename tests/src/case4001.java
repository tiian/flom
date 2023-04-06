/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
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



public class case4001 {
    private static void HappyPath() {
        System.out.println("Hello world from case4001 class, " +
                           "HappyPath method");
        try {
            FlomHandle fh = new FlomHandle();
            
            fh.lock();
            fh.unlock();
            fh.free();
        } catch(FlomException e) {
            System.out.println("FlomException: ReturnCode=" +
                               e.getReturnCode() +
                               " (" + e.getMessage() + ")");
            System.exit(1);
        }
        System.out.println("Hello world from case4001 class, " +
                           "HappyPath method, again");
    }
    private static void MissingLock() {
        System.out.println("Hello world from case4001 class, " +
                           "MissingLock method");
        try {
            FlomHandle fh = new FlomHandle();

            try {
                fh.unlock();
            } catch(FlomException e) {
                if (FlomErrorCodes.FLOM_RC_API_INVALID_SEQUENCE !=
                    e.getReturnCode())
                    throw(e);
            }
            fh.free();
        } catch(FlomException e) {
            System.out.println("FlomException: ReturnCode=" +
                               e.getReturnCode() +
                               " (" + e.getMessage() + ")");
            System.exit(1);
        }
        System.out.println("Hello world from case4001 class, " +
                           "MissingLock method, again");
    }
    private static void MissingUnlock() {
        System.out.println("Hello world from case4001 class, " +
                           "MissingUnlock method");
        try {
            FlomHandle fh = new FlomHandle();
            
            fh.lock();
            fh.free();
        } catch(FlomException e) {
            System.out.println("FlomException: ReturnCode=" +
                               e.getReturnCode() +
                               " (" + e.getMessage() + ")");
            System.exit(1);
        }
        System.out.println("Hello world from case4001 class, " +
                           "MissingUnlock method, again");
    }
    
    public static void main(String[] args) {
        HappyPath();
        MissingLock();
        MissingUnlock();
    }
}
