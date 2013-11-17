/*
 * Copyright (c) 2013, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLOM.
 *
 * FLOM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLOM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLOM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#include "flom_config.h"
#include "flom_errors.h"
#include "flom_regex.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_REGEX



/* global static objects */
regex_t global_res_name_preg;



int global_res_name_preg_init()
{
    enum Exception { SNPRINTF_ERROR
                     , REGCOMP_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("global_res_name_preg_init\n"));
    TRY {
        int reg_error;
        char reg_errbuf[200];
        char reg_expr[1000];

        if (sizeof(reg_expr) <= snprintf(
                reg_expr, sizeof(reg_expr),
                "^"
                "%s|"
                "([[:alpha:][:digit:]])+"
                "$", DEFAULT_RESOURCE_NAME))
            THROW(SNPRINTF_ERROR);

        FLOM_TRACE(("global_res_name_preg_init: regular expression is '%s'\n",
                    reg_expr));
        reg_error = regcomp(
            &global_res_name_preg, reg_expr,
            REG_EXTENDED|REG_NOSUB|REG_NEWLINE);
        if (0 != reg_error) {
            regerror(reg_error, &global_res_name_preg, reg_errbuf,
                     sizeof(reg_errbuf));
            FLOM_TRACE(("global_res_name_preg_init: regcomp returned %d "
                        "('%s') instead of 0\n", reg_error, reg_errbuf));
            THROW(REGCOMP_ERROR);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SNPRINTF_ERROR:
                ret_cod = FLOM_RC_SNPRINTF_ERROR;
                break;
            case REGCOMP_ERROR:
                ret_cod = FLOM_RC_REGCOMP_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("global_res_name_preg_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

