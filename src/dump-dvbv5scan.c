/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006-2014 Winfried Koehler
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "extended_frontend.h"
#include "si_types.h"
#include "scan.h"
#include "dump-dvbv5scan.h"
#include "dvbscan.h"
#include "countries.h"
#include "satellites.h"

/******************************************************************************
 * print initial tuning data for dvbv5scan.
 *****************************************************************************/

void
dvbv5scan_dump_tuningdata(FILE *f, struct transponder *t, uint16_t index, struct w_scan_flags *flags)
{
    char const *network_name = t->network_name;
    if (index == 0) {
        struct tm *ti;
        time_t rawtime;
        time(&rawtime);
        ti = localtime(&rawtime);
        fprintf(f, "#------------------------------------------------------------------------------\n");
        fprintf(f, "# file automatically generated by %s\n", PACKAGE_NAME);
        fprintf(f, "# (https://github.com/stefantalpalaru/w_scan2)\n");
        fprintf(
            f,
            "#! <w_scan> %s %u %u %s %s </w_scan>\n",
            flags->version,
            flags->tuning_timeout,
            flags->filter_timeout,
            scantype_to_txt(flags->scantype),
            flags->scantype == SCAN_SATELLITE ? satellite_to_short_name(flags->list_id) :
                                                country_to_short_name(flags->list_id));
        fprintf(f, "#------------------------------------------------------------------------------\n");

        if (flags->scantype == SCAN_SATELLITE)
            fprintf(f, "# satellite            : %s\n", satellite_to_short_name(flags->list_id));
        else
            fprintf(f, "# location and provider: <add description here>\n");
        fprintf(f, "# date (yyyy-mm-dd)    : %.04d-%.02d-%.02d\n", ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday);
        fprintf(f, "# provided by (opt)    : <your name or email here>\n");
        fprintf(f, "#------------------------------------------------------------------------------\n");
    } /* end if index == 0 */
    fprintf(f, "\n");
    if (network_name != NULL)
        fprintf(f, "# %s\n", network_name);
    fprintf(f, "[CHANNEL]\n");
    switch (flags->scantype) {
    case SCAN_TERRCABLE_ATSC:
        fprintf(f, "\tDELIVERY_SYSTEM = ATSC\n");
        fprintf(f, "\tFREQUENCY = %u\n", t->frequency);
        fprintf(f, "\tMODULATION = %s\n", atsc_mod_to_txt_v5(t->modulation));
        break;
    case SCAN_CABLE:
        if (t->delsys == SYS_DVBC2) {
            fprintf(f, "\tDELIVERY_SYSTEM = DVBC2\n");
            fprintf(f, "\tPLP_ID = %u\n", t->plp_id);
            fprintf(f, "\tDATA_SLICE_ID = %u\n", t->data_slice_id);
            fprintf(f, "\tSYSTEM_ID = %u\n", t->system_id);
        } else {
            fprintf(f, "\tDELIVERY_SYSTEM = DVBC/ANNEX_A\n");
        }
        fprintf(f, "\tFREQUENCY = %u\n", t->frequency);
        fprintf(f, "\tSYMBOL_RATE = %u\n", t->symbolrate);
        fprintf(f, "\tINNER_FEC = %s\n", sat_fec_to_txt(t->mpe_fec));
        fprintf(f, "\tMODULATION = %s\n", cable_mod_to_txt_v5(t->modulation));
        break;
    case SCAN_TERRESTRIAL:
        fprintf(f, "\tDELIVERY_SYSTEM = %s\n", t->delsys == SYS_DVBT2 ? "DVBT2" : "DVBT");
        fprintf(f, "\tFREQUENCY = %u\n", t->frequency);
        fprintf(f, "\tBANDWIDTH_HZ = %u\n", t->bandwidth);
        fprintf(f, "\tMODULATION = %s\n", terr_mod_to_txt_v5(t->modulation));
        fprintf(f, "\tTRANSMISSION_MODE = %s\n", terr_transmission_to_txt(t->transmission));
        fprintf(f, "\tGUARD_INTERVAL = %s\n", terr_guard_to_txt(t->guard));
        fprintf(f, "\tHIERARCHY = %s\n", terr_hierarchy_to_txt(t->hierarchy));
        if (t->plp_id)
            fprintf(f, "\tPLP_ID = %u\n", t->plp_id);
        break;
    case SCAN_SATELLITE:
        fprintf(f, "\tDELIVERY_SYSTEM = %s\n", t->delsys == SYS_DVBS2 ? "DVBS2" : "DVBS");
        fprintf(f, "\tFREQUENCY = %u\n", t->frequency);
        fprintf(f, "\tPOLARIZATION = %s\n", sat_pol_to_txt_v5(t->polarization));
        fprintf(f, "\tSYMBOL_RATE = %u\n", t->symbolrate);
        fprintf(f, "\tINNER_FEC = %s\n", sat_fec_to_txt(t->mpe_fec));
        if (t->delsys != SYS_DVBS) {
            fprintf(f, "\tROLLOFF = %s\n", sat_rolloff_to_txt(t->rolloff));
            fprintf(f, "\tMODULATION = %s\n", sat_mod_to_txt_v5(t->modulation));
        }
        break;
    default:;
    };
}
