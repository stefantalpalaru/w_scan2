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
 * satellite.c/h, added 20090425, version #20100116
 */



/* this file is shared between w_scan and the VDR plugin wirbelscan.
 * For details on both of them see http://wirbel.htpc-forum.de
 */
#define B(ID) static const struct __sat_transponder ID[] = {
#define E(ID) };

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "scan.h"
#include "satellites.h"
#include "extended_frontend.h"
#include "satellites.dat"


/******************************************************************************
 * convert position constant
 * to index number
 *
 *****************************************************************************/

int txt_to_satellite(const char * id) {
unsigned int i;
for (i = 0; i < SAT_COUNT(sat_list); i++)
   if (! strcasecmp(id,sat_list[i].short_name))
      return sat_list[i].id;
return -1;
}

/******************************************************************************
 * return numbers of satellites defined.
 *
 *
 *****************************************************************************/

int sat_count() {
      return SAT_COUNT(sat_list);
}

/******************************************************************************
 * convert index number
 * to position constant
 *
 *****************************************************************************/

const char * satellite_to_short_name(int idx) {
unsigned int i;
for (i = 0; i < SAT_COUNT(sat_list); i++)
   if (idx == sat_list[i].id)
      return sat_list[i].short_name;
return "??";
}

/******************************************************************************
 * convert index number
 * to satellite name
 *
 *****************************************************************************/

const char * satellite_to_full_name(int idx) {
unsigned int i;
for (i = 0; i < SAT_COUNT(sat_list); i++)
   if (idx == sat_list[i].id)
      return sat_list[i].full_name;
warning("SATELLITE CODE NOT DEFINED. PLEASE RE-CHECK WETHER YOU TYPED CORRECTLY.\n");
usleep(5000000);
return "??";
}

/******************************************************************************
 * return index number
 * from rotor position
 *
 *****************************************************************************/
int rotor_position_to_sat_list_index(int rotor_position) {
unsigned int i;
for (i = 0; i < SAT_COUNT(sat_list); i++)
   if (rotor_position == sat_list[i].rotor_position)
      return i;
return 0;
}

/******************************************************************************
 * print list of
 * all satellites
 *
 *****************************************************************************/

void print_satellites(void) {
unsigned int i;
for (i = 0; i < SAT_COUNT(sat_list); i++)
    info("\t%s\t\t%s\n", sat_list[i].short_name, sat_list[i].full_name);
}

/******************************************************************************
 * get transponder data
 *
 *****************************************************************************/
//int get_frontend_param(uint16_t satellite, uint16_t table_index, struct tuning_parameters * param) {
//unsigned int i;
//
//for (i = 0; i < SAT_COUNT(sat_list); i++)
//    if (satellite == sat_list[i].id) {
//        if (table_index >= sat_list[i].item_count)
//            return 0; //error
//        memset(param, 0, sizeof(struct tuning_parameters));
//        param->frequency = sat_list[i].items[table_index].intermediate_frequency;
//        param->inversion = INVERSION_AUTO;
//        param->sat.modulation_system = sat_list[i].items[table_index].modulation_system;
//        param->sat.polarization      = sat_list[i].items[table_index].polarization;
//        param->sat.symbol_rate       = sat_list[i].items[table_index].symbol_rate;
//        param->sat.fec_inner         = sat_list[i].items[table_index].fec_inner;
//        param->sat.rolloff           = sat_list[i].items[table_index].rolloff;
//        param->sat.modulation_type   = sat_list[i].items[table_index].modulation_type;    
//        return 1;
//        }
//return 0; // error
//}

int choose_satellite(const char * satellite, int * channellist) {
    int retval = 0;
    * channellist = txt_to_satellite(satellite);
    if (* channellist < 0) {
        * channellist = S19E2;
        warning("\n\nSATELLITE CODE IS NOT DEFINED. FALLING BACK TO \"S19E2\"\n\n");
        sleep(10);
        retval = -1;
        }
    info("using settings for %s\n", satellite_to_full_name(* channellist));
    return retval;
}
