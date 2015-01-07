/*
 * Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM and libflom (FLoM API client library)
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * This file is part of libflom too and you can redistribute it and/or modify
 * it under the terms of one of the following licences:
 * - GNU General Public License version 2.0
 * - GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License and
 * GNU Lesser General Public License along with FLoM.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif
#ifdef HAVE_REGEX_H
# include <regex.h>
#endif



#include "flom_config.h"
#include "flom_errors.h"
#include "flom_rsrc.h"
#include "flom_resource_hier.h"
#include "flom_resource_numeric.h"
#include "flom_resource_set.h"
#include "flom_resource_simple.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_RSRC



/* global static objects */
regex_t global_res_name_preg[FLOM_RSRC_TYPE_N];



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
        flom_rsrc_type_t i;
        const char *reg_str[FLOM_RSRC_TYPE_N] = {
            "^_$" /* this is a dummy value */ ,
            "^%s$|^([[:alpha:]][[:alpha:][:digit:]]*)$" ,
            "^([[:alpha:]][[:alpha:][:digit:]]*)\\[([[:digit:]]+)\\]$",
            "^([[:alpha:]][[:alpha:][:digit:]]*)(\\%s[[:alpha:]][[:alpha:][:digit:]]*)+$",
            "^\\%s[^\\%s]+(\\%s[^\\%s]+)*$"
        };

        memset(global_res_name_preg, 0, sizeof(global_res_name_preg));
        for (i=FLOM_RSRC_TYPE_NULL; i<FLOM_RSRC_TYPE_N; ++i) {
            int printed;
            /* preparing regular expression */
            switch (i) {
                case FLOM_RSRC_TYPE_SIMPLE:
                    printed = snprintf(
                        reg_expr, sizeof(reg_expr), reg_str[i],
                        DEFAULT_RESOURCE_NAME);
                    break;
                case FLOM_RSRC_TYPE_SET:
                    printed = snprintf(
                        reg_expr, sizeof(reg_expr), reg_str[i],
                        FLOM_RESOURCE_SET_SEPARATOR);
                    break;
                case FLOM_RSRC_TYPE_HIER:
                    printed = snprintf(
                        reg_expr, sizeof(reg_expr), reg_str[i],
                        FLOM_HIER_RESOURCE_SEPARATOR,
                        FLOM_HIER_RESOURCE_SEPARATOR,
                        FLOM_HIER_RESOURCE_SEPARATOR,
                        FLOM_HIER_RESOURCE_SEPARATOR);
                    break;
                default:
                    printed = snprintf(
                        reg_expr, sizeof(reg_expr), reg_str[i],
                        FLOM_EMPTY_STRING);
                    break;
            } /* switch (i) */
            if (sizeof(reg_expr) <= printed)
                THROW(SNPRINTF_ERROR);
            FLOM_TRACE(("global_res_name_preg_init: regular expression for "
                        "type %d is '%s'\n", i, reg_expr));
            reg_error = regcomp(global_res_name_preg+i, reg_expr,
                                REG_EXTENDED|REG_NEWLINE);
            if (0 != reg_error) {
                regerror(reg_error, global_res_name_preg+i,
                         reg_errbuf, sizeof(reg_errbuf));
                FLOM_TRACE(("global_res_name_preg_init: regcomp returned %d "
                            "('%s') instead of 0\n", reg_error, reg_errbuf));
                THROW(REGCOMP_ERROR);
            }
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



void global_res_name_preg_free()
{
    flom_rsrc_type_t i;
    
    FLOM_TRACE(("global_res_name_preg_free\n"));
    for (i=FLOM_RSRC_TYPE_NULL; i<FLOM_RSRC_TYPE_N; ++i)
        regfree(global_res_name_preg+i);
    memset(global_res_name_preg, 0, sizeof(global_res_name_preg));
}



flom_rsrc_type_t flom_rsrc_get_type(const gchar *resource_name)
{
    int reg_error;
    char reg_errbuf[200];
    flom_rsrc_type_t i;
    flom_rsrc_type_t ret_cod = FLOM_RSRC_TYPE_NULL;
    
    FLOM_TRACE(("flom_rsrc_get_type\n"));

    for (i=FLOM_RSRC_TYPE_NULL+1; i<FLOM_RSRC_TYPE_N; ++i) {
        reg_error = regexec(global_res_name_preg+i,
                            resource_name, 0, NULL, 0);
        regerror(reg_error, global_res_name_preg+i, reg_errbuf,
                 sizeof(reg_errbuf));
        FLOM_TRACE(("flom_rsrc_get_type: regexec returned "
                    "%d ('%s') for string '%s'\n",
                    reg_error, reg_errbuf, resource_name));
        if (0 == reg_error) {
            ret_cod = i;
            break;
        } /* if (0 == reg_error) */
    } /* for (i=FLOM_RSRC_RES_TYPE_NULL+1; ... */
    FLOM_TRACE(("flom_rsrc_get_type/ret_cod=%d\n", ret_cod));
    return ret_cod;
}



int flom_rsrc_get_number(const gchar *resource_name, gint *number)
{
    enum Exception { REGEXEC_ERROR
                     , INVALID_RESOURCE_NAME
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_rsrc_get_number\n"));
    TRY {
        int reg_error, i;
        char reg_errbuf[200];
        regmatch_t regmatch[3];
        for (i=0; i<sizeof(regmatch)/sizeof(regmatch_t); ++i)
            regmatch[i].rm_so = regmatch[i].rm_eo = -1;
        reg_error = regexec(global_res_name_preg+FLOM_RSRC_TYPE_NUMERIC,
                            resource_name, sizeof(regmatch)/sizeof(regmatch_t),
                            regmatch, 0);
        regerror(reg_error, global_res_name_preg+FLOM_RSRC_TYPE_NUMERIC,
                 reg_errbuf, sizeof(reg_errbuf));
        FLOM_TRACE(("flom_rsrc_get_number: regexec returned "
                    "%d ('%s') for string '%s'\n",
                    reg_error, reg_errbuf, resource_name));
        if (0 != reg_error)
            THROW(REGEXEC_ERROR);
        /* number must be in position 2 */
        if (-1 == regmatch[2].rm_so) {
            THROW(INVALID_RESOURCE_NAME);
        } else {
            char buffer[1000];
            size_t delta = regmatch[2].rm_eo - regmatch[2].rm_so;
            if (delta >= sizeof(buffer))
                delta = sizeof(buffer)-1;
            memcpy(buffer, resource_name+regmatch[2].rm_so, delta);
            buffer[delta] = '\0';
            /* value is always interpreted using decimal base */
            *number = strtol(buffer, NULL, 10);
            FLOM_TRACE(("flom_rsrc_get_number: regmatch[2]='%s', "
                        "number=%d\n", buffer, *number));
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case REGEXEC_ERROR:
                ret_cod = FLOM_RC_REGEXEC_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_rsrc_get_number/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_rsrc_get_elements(const gchar *resource_name, GArray *elements)
{
    enum Exception { G_STRSPLIT_ERROR
                     , G_STRDUP_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    gchar **names = NULL;
    
    FLOM_TRACE(("flom_rsrc_get_elements\n"));
    TRY {
        gchar **name;
        
        /* resource_name is a VALID resource set (verified by regular
           expression before this function is called! */
        if (NULL == (names = g_strsplit(resource_name,
                                        FLOM_RESOURCE_SET_SEPARATOR, 0)))
            THROW(G_STRSPLIT_ERROR);
        /* loop on list of splitted strings */
        for (name = names; *name; name++) {
            struct flom_rsrc_data_set_element_s element;
            memset(&element, 0, sizeof(element));
            if (NULL == (element.name = g_strdup(*name)))
                THROW(G_STRDUP_ERROR);
            element.conn = NULL;
            FLOM_TRACE(("flom_rsrc_get_elements: adding element '%s' to "
                        "resource set\n", element.name));
            g_array_append_val(elements, element);
        } /* for (name = names; *name; name++) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRSPLIT_ERROR:
                ret_cod = FLOM_RC_G_STRSPLIT_ERROR;
                break;
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* release storage */
    if (NULL != names)
        g_strfreev(names);
    FLOM_TRACE(("flom_rsrc_get_elements/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



struct flom_rsrc_conn_lock_s *flom_rsrc_conn_lock_new(void)
{
    struct flom_rsrc_conn_lock_s *cl;
    if (NULL == (cl = g_try_malloc(sizeof(struct flom_rsrc_conn_lock_s)))) {
        FLOM_TRACE(("flom_rsrc_conn_lock_new: unable to allocate a struct "
                    "of type 'flom_rsrc_conn_lock_s' with g_try_malloc\n"));
        return NULL;
    }
    memset(cl, 0, sizeof(struct flom_rsrc_conn_lock_s));
    return cl;
}



void flom_rsrc_conn_lock_delete(struct flom_rsrc_conn_lock_s *frcl)
{
    if (NULL == frcl) {
        FLOM_TRACE(("flom_rsrc_conn_lock_delete: trying to delete a NULL "
                    "struct pointer\n"));
        return;
    }
    if (NULL != frcl->name)
        g_free(frcl->name);
    g_free(frcl);
}



int flom_resource_init(flom_resource_t *resource,
                       flom_rsrc_type_t type, const gchar *name)
{
    enum Exception { OUT_OF_RANGE
                     , UNKNOW_RESOURCE
                     , RESOURCE_INIT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_init\n"));
    TRY {
        if (FLOM_RSRC_TYPE_NULL >= type || FLOM_RSRC_TYPE_N <= type)
            THROW(OUT_OF_RANGE);

        /* reset resource memory space */
        memset(resource, 0, sizeof(flom_resource_t));

        /* "virtual" methods */
        switch (type) {
            case FLOM_RSRC_TYPE_SIMPLE:
                resource->init = flom_resource_simple_init;
                resource->inmsg = flom_resource_simple_inmsg;
                resource->clean = flom_resource_simple_clean;
                resource->free = flom_resource_simple_free;
                resource->compare_name = flom_resource_compare_name;
                break;
            case FLOM_RSRC_TYPE_NUMERIC:
                resource->init = flom_resource_numeric_init;
                resource->inmsg = flom_resource_numeric_inmsg;
                resource->clean = flom_resource_numeric_clean;
                resource->free = flom_resource_numeric_free;
                resource->compare_name = flom_resource_compare_name;
                break;
            case FLOM_RSRC_TYPE_SET:
                resource->init = flom_resource_set_init;
                resource->inmsg = flom_resource_set_inmsg;
                resource->clean = flom_resource_set_clean;
                resource->free = flom_resource_set_free;
                resource->compare_name = flom_resource_compare_name;
                break;
            case FLOM_RSRC_TYPE_HIER:
                resource->init = flom_resource_hier_init;
                resource->inmsg = flom_resource_hier_inmsg;
                resource->clean = flom_resource_hier_clean;
                resource->free = flom_resource_hier_free;
                resource->compare_name = flom_resource_hier_compare_name;
                break;
            default:
                THROW(UNKNOW_RESOURCE);
        } /* switch (resource->type) */
        /* set resource type and initialize the new resource */
        resource->type = type;
        if (FLOM_RC_OK != (ret_cod = resource->init(resource, name)))
            THROW(RESOURCE_INIT_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OUT_OF_RANGE:
                ret_cod = FLOM_RC_OUT_OF_RANGE;
                break;
            case UNKNOW_RESOURCE:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case RESOURCE_INIT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_resource_free(flom_resource_t *resource)
{
    FLOM_TRACE(("flom_resource_free\n"));
    /* calling the destructor */
    if (NULL != resource->free)
        resource->free(resource);
    else
        FLOM_TRACE(("flom_resource_free: resource->free can not be called "
                    "because it's null\n"));
}



int flom_resource_compare_name(const flom_resource_t *resource,
                               const gchar *name)
{
    FLOM_TRACE(("flom_resource_compare_name: resource->name='%s', "
                "name='%s'\n", resource->name,
                NULL != name ? name : "null"));
    return g_strcmp0(resource->name, name);
}
