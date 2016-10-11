/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Winfried Koehler 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * Or, point your browser to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 *
 * The author can be reached at: w_scan AT gmx-topmail DOT de
 *
 * The project's page is http://wirbel.htpc-forum.de/w_scan/index2.html
 */

#ifndef __SCAN_H__
#define __SCAN_H__

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "extended_frontend.h"
#include <sys/types.h>
#include <stdint.h>
#include "tools.h"
#include "descriptors.h"
#include "emulate.h"



/******************************************************************************
 * internal definitions.
 *****************************************************************************/

struct w_scan_flags {
   uint32_t    version;
   scantype_t  scantype;
   uint8_t     atsc_type;
   uint8_t     need_2g_fe;
   uint32_t    list_id;
   uint8_t     tuning_timeout;
   uint8_t     filter_timeout;
   uint8_t     get_other_nits;
   uint8_t     add_frequencies;
   uint8_t     dump_provider;
   uint8_t     vdr_version;
   uint8_t     qam_no_auto;
   uint8_t     ca_select;
   int         rotor_position;
   uint16_t    api_version;
   uint16_t    sw_pos;
   uint16_t    codepage;
   uint8_t     print_pmt;
   uint8_t     emulate;
};


struct service * find_service (struct transponder * t, uint16_t service_id);
struct service * alloc_service(struct transponder * t, uint16_t service_id);

struct transponder * alloc_transponder(uint32_t frequency, uint8_t type, uint8_t polarization);

/* write transponder data to dest. no memory allocating,
 * so dest has to be big enough - think about before use!
 */
void print_transponder(char * dest, struct transponder * t);


#endif
