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
 */

/* this file is shared between w_scan2 and the VDR plugin wirbelscan.
 * For details on the latter see http://wirbel.htpc-forum.de
 */

#ifndef __SATELLITES_H__
#define __SATELLITES_H__

#include "extended_frontend.h"
#include <stdint.h>
#include <unistd.h>

int choose_satellite(char const *satellite, int *channellist);
int txt_to_satellite(char const *id);
int sat_count();
char const *satellite_to_short_name(int idx);
char const *satellite_to_full_name(int idx);
int rotor_position_to_sat_list_index(int rotor_position);
void print_satellites(void);
// int get_frontend_param(uint16_t satellite, uint16_t table_index,
//                        struct tuning_parameters * param);

/******************************************************************************
 * only used for storage of data
 *
 *****************************************************************************/
struct __sat_transponder {
    fe_delivery_system_t modulation_system;
    uint32_t intermediate_frequency;
    fe_polarization_t polarization;
    uint32_t symbol_rate;
    fe_code_rate_t fec_inner;
    fe_rolloff_t rolloff;
    fe_modulation_t modulation_type;
};
#define SAT_TRANSPONDER_COUNT(x) (sizeof(x)/sizeof(struct __sat_transponder))

struct cSat {
    char const *short_name;
    int const id;
    char const *full_name;
    const struct __sat_transponder *items;
    int const item_count;
    fe_west_east_flag_t const west_east_flag;
    uint16_t const orbital_position;
    int rotor_position; // Note: *not* const
    char const *source_id; // VDR sources.conf
    int const skew;
};
#define SAT_COUNT(x) (sizeof(x)/sizeof(struct cSat))

extern struct cSat sat_list[];

#endif
