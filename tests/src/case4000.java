/*
 * Copyright (c) 2013-2020, Christian Ferrari <tiian@users.sourceforge.net>
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



public class case4000 {
    public static void main(String[] args) {
        System.out.println("Hello world from case4000 program");
        try {
            FlomHandle fh = new FlomHandle();
            
            fh.lock();
            fh.unlock();
            fh.free();
        } catch(FlomException e) {
            System.out.println("FlomException: ReturnCode=" +
                               e.getReturnCode() +
                               " (" + e.getMessage() + ")");
        }
        System.out.println("Hello world from case4000 program, again");
    }
}
