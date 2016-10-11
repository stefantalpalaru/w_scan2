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

#ifndef __HEADER_DVBSCAN_H__
#define __HEADER_DVBSCAN_H__

#include "extended_frontend.h"


typedef struct init_item {
        const char *    name;
        int             id;
} _init_item;



/********************************************************************
 * DVB-T
 *
 ********************************************************************/

/* convert text to identifiers */
int txt_to_terr_bw ( const char * txt );
int txt_to_terr_fec ( const char * txt );
int txt_to_terr_mod ( const char * txt );
int txt_to_terr_transmission ( const char * txt );
int txt_to_terr_guard ( const char * txt );
int txt_to_terr_hierarchy ( const char * txt );

/*convert identifier to text */
const char * terr_bw_to_txt ( int id );
const char * terr_fec_to_txt ( int id );
const char * terr_mod_to_txt ( int id );
const char * terr_transmission_to_txt ( int id );
const char * terr_guard_to_txt ( int id );
const char * terr_hierarchy_to_txt ( int id );

/********************************************************************
 * DVB-C
 *
 ********************************************************************/

/* convert text to identifiers */
int txt_to_cable_fec ( const char * txt );
int txt_to_cable_mod ( const char * txt );

/*convert identifier to text */
const char * cable_fec_to_txt ( int id );
const char * cable_mod_to_txt ( int id );

/********************************************************************
 * ATSC
 *
 ********************************************************************/

/* convert text to identifiers */
int txt_to_atsc_mod ( const char * txt );

/*convert identifier to text */
const char * atsc_mod_to_txt ( int id );

/********************************************************************
 * DVB-S
 *
 ********************************************************************/

/* convert text to identifiers */
int txt_to_sat_delivery_system ( const char * txt );
int txt_to_sat_pol ( const char * txt );
int txt_to_sat_fec ( const char * txt );
int txt_to_sat_rolloff ( const char * txt );
int txt_to_sat_mod ( const char * txt );

/*convert identifier to text */
const char * sat_delivery_system_to_txt ( int id );
const char * sat_pol_to_txt ( int id );
const char * sat_fec_to_txt ( int id );
const char * sat_rolloff_to_txt ( int id );
const char * sat_mod_to_txt ( int id );

/********************************************************************
 * non-frontend specific part
 *
 ********************************************************************/

/* convert text to identifiers */
int txt_to_scantype ( const char * txt );

/*convert identifier to text */
const char * scantype_to_txt ( int id );


#endif
