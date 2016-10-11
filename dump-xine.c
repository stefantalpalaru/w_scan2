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
 *
 * The author can be reached at: w_scan AT gmx-topmail DOT de
 *
 * The project's page is http://wirbel.htpc-forum.de/w_scan/index2.html
 */

/* 20110702 --wk */

#include <stdio.h>
#include <stdlib.h>
#include "extended_frontend.h"
#include "scan.h"
#include "dump-xine.h"
#include "tools.h"


const char * xine_bandwidth_name (int bandwidth) {
  switch(bandwidth) {                  
     case 8000000    : return "BANDWIDTH_8_MHZ";
     case 7000000    : return "BANDWIDTH_7_MHZ";
     case 6000000    : return "BANDWIDTH_6_MHZ";
     case 5000000    : return "BANDWIDTH_5_MHZ";
     case 10000000   : return "BANDWIDTH_10_MHZ";
     case 1712000    : return "BANDWIDTH_1_712_MHZ";
     default         : return "BANDWIDTH_AUTO";
     }                         
}

void xine_dump_dvb_parameters (FILE * f, struct transponder * t, struct w_scan_flags * flags)
{

        switch (flags->scantype) {
        case SCAN_TERRCABLE_ATSC:
                fprintf (f, "%i:", t->frequency);
                fprintf (f, "%s",  modulation_name(t->modulation));
                break;
        case SCAN_CABLE:
                fprintf (f, "%i:", t->frequency);
                fprintf (f, "%s:", inversion_name(t->inversion));
                fprintf (f, "%i:", t->symbolrate);
                fprintf (f, "%s:", coderate_name(t->coderate));
                fprintf (f, "%s",  modulation_name(t->modulation));
                break;
        case SCAN_TERRESTRIAL:
                fprintf (f, "%i:", t->frequency);
                fprintf (f, "%s:", inversion_name(t->inversion));
                fprintf (f, "%s:", xine_bandwidth_name(t->bandwidth));
                fprintf (f, "%s:", coderate_name(t->coderate));
                fprintf (f, "%s:", coderate_name(t->coderate_LP));
                fprintf (f, "%s:", modulation_name(t->modulation));
                fprintf (f, "%s:", transmission_mode_name(t->transmission));
                fprintf (f, "%s:", guard_interval_name(t->guard));
                fprintf (f, "%s",  hierarchy_name(t->hierarchy));
                break;
        case SCAN_SATELLITE:
                fprintf (f, "%i:", t->frequency / 1000);
                switch (t->polarization) {
                        case POLARIZATION_HORIZONTAL:
                                fprintf (f, "h:");
                                break;
                        case POLARIZATION_VERTICAL:
                                fprintf (f, "v:");
                                break;
                        case POLARIZATION_CIRCULAR_LEFT:
                                fprintf (f, "l:");
                                break;
                        case POLARIZATION_CIRCULAR_RIGHT:
                                fprintf (f, "r:");
                                break;
                        default:
                                fatal("Unknown Polarization %d\n", t->polarization);
                        }

                if (flags->rotor_position > 0)
                        fprintf (f, "%i:", flags->rotor_position);
                else
                        fprintf (f, "0:");

                fprintf (f, "%i", t->symbolrate / 1000);
                break;
        default:
                fatal("Unknown scantype %d\n", flags->scantype);
        };
}

void xine_dump_service_parameter_set (FILE * f, 
                                struct service * s,
                                struct transponder * t,
                                struct w_scan_flags * flags)
{
        if (s->video_pid || s->audio_pid[0]) {
                if (s->provider_name)
                        fprintf (f, "%s(%s):", s->service_name, s->provider_name);
                else
                        fprintf (f, "%s:", s->service_name);
                xine_dump_dvb_parameters (f, t, flags);
                fprintf (f, ":%i:%i:%i", s->video_pid, s->ac3_pid[0]?s->ac3_pid[0]:s->audio_pid[0], s->service_id);
                /* what about AC3 audio here && multiple audio pids? see also: dump_mplayer.c/h */
                fprintf (f, "\n");
                }
}
