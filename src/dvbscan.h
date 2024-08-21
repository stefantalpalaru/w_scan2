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

#ifndef __HEADER_DVBSCAN_H__
#define __HEADER_DVBSCAN_H__

#include "extended_frontend.h"

typedef struct init_item {
    char const *name;
    int id;
} _init_item;

/********************************************************************
 * DVB-T
 *
 ********************************************************************/

/* convert text to identifiers */
int txt_to_terr_bw(char const *txt);
int txt_to_terr_fec(char const *txt);
int txt_to_terr_mod(char const *txt);
int txt_to_terr_transmission(char const *txt);
int txt_to_terr_guard(char const *txt);
int txt_to_terr_hierarchy(char const *txt);

/*convert identifier to text */
char const *terr_bw_to_txt(int id);
char const *terr_fec_to_txt(int id);
char const *terr_mod_to_txt(int id);
char const *terr_mod_to_txt_v5(int id);
char const *terr_transmission_to_txt(int id);
char const *terr_guard_to_txt(int id);
char const *terr_hierarchy_to_txt(int id);

/********************************************************************
 * DVB-C
 *
 ********************************************************************/

/* convert text to identifiers */
int txt_to_cable_fec(char const *txt);
int txt_to_cable_mod(char const *txt);

/*convert identifier to text */
char const *cable_fec_to_txt(int id);
char const *cable_mod_to_txt(int id);
char const *cable_mod_to_txt_v5(int id);

/********************************************************************
 * ATSC
 *
 ********************************************************************/

/* convert text to identifiers */
int txt_to_atsc_mod(char const *txt);

/*convert identifier to text */
char const *atsc_mod_to_txt(int id);
char const *atsc_mod_to_txt_v5(int id);

/********************************************************************
 * DVB-S
 *
 ********************************************************************/

/* convert text to identifiers */
int txt_to_sat_delivery_system(char const *txt);
int txt_to_sat_pol(char const *txt);
int txt_to_sat_fec(char const *txt);
int txt_to_sat_rolloff(char const *txt);
int txt_to_sat_mod(char const *txt);

/*convert identifier to text */
char const *sat_delivery_system_to_txt(int id);
char const *sat_pol_to_txt(int id);
char const *sat_pol_to_txt_v5(int id);
char const *sat_fec_to_txt(int id);
char const *sat_rolloff_to_txt(int id);
char const *sat_mod_to_txt(int id);
char const *sat_mod_to_txt_v5(int id);

/********************************************************************
 * non-frontend specific part
 *
 ********************************************************************/

/* convert text to identifiers */
int txt_to_scantype(char const *txt);

/*convert identifier to text */
char const *scantype_to_txt(int id);

#endif
