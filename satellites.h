/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006, 2007, 2008, 2009 Winfried Koehler 
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



/* this file is shared between w_scan and the VDR plugin wirbelscan.
 * For details on both of them see http://wirbel.htpc-forum.de
 */

#ifndef __SATELLITES_H__
#define __SATELLITES_H__

#include "extended_frontend.h"
#include <stdint.h>
#include <unistd.h>


int choose_satellite(const char * satellite, int * channellist);
int txt_to_satellite (const char * id);
int sat_count();
const char * satellite_to_short_name(int idx);
const char * satellite_to_full_name(int idx);
int rotor_position_to_sat_list_index(int rotor_position);
void print_satellites(void);
//int get_frontend_param(uint16_t satellite, uint16_t table_index,
//                       struct tuning_parameters * param);

/******************************************************************************
 * only used for storage of data
 *
 *****************************************************************************/
struct __sat_transponder {
        fe_delivery_system_t           modulation_system;
        uint32_t                       intermediate_frequency;
        fe_polarization_t              polarization;
        uint32_t                       symbol_rate;
        fe_code_rate_t                 fec_inner;
        fe_rolloff_t                   rolloff;
        fe_modulation_t                modulation_type;
};
#define SAT_TRANSPONDER_COUNT(x) (sizeof(x)/sizeof(struct __sat_transponder))


struct cSat {
        const char                     * short_name;
        const int                      id;
        const char                     * full_name;
        const struct __sat_transponder * items;
        const int                      item_count;
        const fe_west_east_flag_t      west_east_flag;
        const uint16_t                 orbital_position;
        int                            rotor_position;     // Note: *not* const
        const char                     * source_id;        // VDR sources.conf
};
#define SAT_COUNT(x) (sizeof(x)/sizeof(struct cSat))

extern struct cSat sat_list[];


#endif
