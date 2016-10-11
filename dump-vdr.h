/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006, 2007, 2008, 2009, 2011 Winfried Koehler 
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

#ifndef __DUMP_VDR_H__
#define __DUMP_VDR_H__

/* 20110702 --wk */

#include <stdint.h>
#include "scan.h"
#include "extended_frontend.h"
#include "si_types.h"

const char * vdr_inversion_name(int inversion);
const char * vdr_fec_name(int fec);
const char * vdr17_modulation_name(int modulation);
const char * vdr_modulation_name(int modulation);
const char * vdr_bandwidth_name(int bandwidth);                                     
const char * vdr_transmission_mode_name(int transmission_mode);
const char * vdr_guard_name(int guard_interval);
const char * vdr_hierarchy_name(int hierarchy);
const char * vdr_name_to_short_name(const char * satname);

void vdr_dump_service_parameter_set (FILE * f,
                                struct service * s,
                                struct transponder * t,
                                struct w_scan_flags * flags);

#endif

