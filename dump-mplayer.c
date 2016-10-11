/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Winfried Koehler 
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
#include "dump-mplayer.h"
#include "dump-xine.h"


void mplayer_dump_service_parameter_set (FILE * f, 
                                struct service * s,
                                struct transponder * t,
                                struct w_scan_flags * flags)
{
        int i;

        fprintf (f, "%s:", s->service_name);
        xine_dump_dvb_parameters (f, t, flags);
        fprintf (f, ":%i:", s->video_pid);

        // build '+' separated list of mpeg audio and ac3 audio pids
        if (s->audio_pid[0] || s->ac3_pid[0]) {

           // prefer ac3 audio.
           if (s->ac3_pid[0]) {
              fprintf(f, "%i", s->ac3_pid[0]);
              for (i = 1; i < s->ac3_num; i++) {
                  fprintf (f, "+%i", s->ac3_pid[i]);
                  }
              if (s->audio_pid[0])
                  fprintf (f, "%s", "+");
              }
           // standard audio pids follow.
           if (s->audio_pid[0]) {
              fprintf(f, "%i", s->audio_pid[0]);
              for (i = 1; i < s->audio_num; i++) {
                  fprintf (f, "+%i", s->audio_pid[i]);
                  }
              }
           }

        else
           // no audio or ac3 audio pids found.
           fprintf(f, "%i", 0);

        fprintf (f, ":%i\n", s->service_id);
}

