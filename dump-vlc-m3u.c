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
 * The project's page is http://wirbel.htpc-forum.de/w_scan/idx2.html
 */

/* 20120525 --wk */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extended_frontend.h"
#include "scan.h"
#include "dump-vlc-m3u.h"
#include "lnb.h"

/******************************************************************************
 * NOTE: VLC-2.x.x seems to support DVB-S2 now, whereas VLC-1.x missed DVB-S2
 *       support.
 *
 * TODO:
 *       1) IMPLEMENT DVB-S2 SYNTAX FOR VLC.                        <- done.
 *       2) check all values for system, modulation, fec, ..        <- done.
 *       3) enshure UTF-8 compliance of service names (should be the easiest task) <- wrong. It's the hardest task. Names are converted by iconv to UTF8 and still probs..
 *****************************************************************************/
static int idx = 1;

#define T1 "\t"
#define T2 "\t\t"
#define T3 "\t\t\t"
#define T4 "\t\t\t\t"
#define T5 "\t\t\t\t\t"
#define fprintf_tab0(v) fprintf(f, "%s", v)
#define fprintf_tab1(v) fprintf(f, "%s%s", T1, v)
#define fprintf_tab2(v) fprintf(f, "%s%s", T2, v)
#define fprintf_tab3(v) fprintf(f, "%s%s", T3, v)
#define fprintf_tab4(v) fprintf(f, "%s%s", T4, v)
#define fprintf_tab5(v) fprintf(f, "%s%s", T5, v)
#define fprintf_pair(p,v) fprintf(f, "%s=%c%s%c", p,34,v,34)

int vlc_inversion(int inversion) {
  switch(inversion) {
     case INVERSION_OFF:             return 0;
     case INVERSION_ON:              return 1;
     default:                        return 2;
     }
}

static const char * vlc_fec(int fec) {
  static const char * const code_rate_vlc[] = { "0", "1/2", "2/3", "3/4" ,"4/5", "5/6", "6/7", "7/8", "8/9", "", "3/5", "9/10", "2/5" }; /*"1/4", "1/3",*/
  if (fec > FEC_2_5) return ""; 
  return code_rate_vlc[fec];
}

static const char * vlc_modulation(int modulation) { 
  static const char * const mod_vlc[] = {"QPSK", "16QAM", "32QAM", "64QAM", "128QAM", "256QAM", "QAM", "8VSB", "16VSB", "8PSK", "16APSK", "32APSK", "DQPSK"};
  return  mod_vlc[modulation];
}

static const char * vlc_delsys (int guard_interval) {
  switch(guard_interval) {          
     case SYS_DVBC_ANNEX_A:    return "dvb-c";
     case SYS_DVBC_ANNEX_B:    return "dvb-c";
     case SYS_DVBT        :    return "dvb-t";
     case SYS_DVBS        :    return "dvb-s";
     case SYS_DVBS2       :    return "dvb-s2";
     case SYS_ISDBT       :    return "isdb-t";
     case SYS_ISDBS       :    return "isdb-s";
     case SYS_ISDBC       :    return "isdb-t";
     case SYS_ATSC        :    return "atsc";
     case SYS_DVBT2       :    return "dvb-t2";
     case SYS_DVBC_ANNEX_C:    return "dvb-c";
     default:                  return "unknown";
     }                        
}

int vlc_bandwidth (int bandwidth) {
  switch(bandwidth) {                  
     case 8000000:                   return 8;
     case 7000000:                   return 7;
     case 6000000:                   return 6;
     case 5000000:                   return 5;
     case 10000000:                  return 10;
     case 1712000:                   return 2;     // wrong in VLC. It's 1.712, not 2.
     default:                        return 0;
     }                         
}
                                       
int vlc_transmission (int transmission) {
  switch(transmission) {         
     case TRANSMISSION_MODE_2K:      return 2;
     case TRANSMISSION_MODE_8K:      return 8;
     case TRANSMISSION_MODE_4K:      return 4;
     case TRANSMISSION_MODE_1K:      return 1;
     case TRANSMISSION_MODE_16K:     return 16;
     case TRANSMISSION_MODE_32K:     return 32;
     default:                        return 0;
     }                         
}  

static const char * vlc_guard (int guard_interval) {
  switch(guard_interval) {          
     case GUARD_INTERVAL_1_32:       return "1/32";
     case GUARD_INTERVAL_1_16:       return "1/16";
     case GUARD_INTERVAL_1_8:        return "1/8";
     case GUARD_INTERVAL_1_4:        return "1/4";
     case GUARD_INTERVAL_1_128:      return "1/128";
     case GUARD_INTERVAL_19_128:     return "19/128";
     case GUARD_INTERVAL_19_256:     return "19/256";
     default:                        return "";
     }                        
}  

int vlc_hierarchy (int hierarchy) {
  switch(hierarchy) {                  
     case HIERARCHY_NONE:            return 0;
     case HIERARCHY_1:               return 1;
     case HIERARCHY_2:               return 2;
     case HIERARCHY_4:               return 4;
     default:                        return 0;
     }                         
}

int vlc_rolloff (int rolloff) {
  switch(rolloff) {                  
     case ROLLOFF_35:                return 35;
     case ROLLOFF_20:                return 20;
     case ROLLOFF_25:                return 25;
     default:                        return 35;
     }                         
}

extern uint version;

void vlc_xspf_prolog(FILE * f, uint16_t adapter, uint16_t frontend, struct w_scan_flags * flags, struct lnb_types_st * lnbp) {
  fprintf_tab0("<?");
  fprintf_pair("xml version", "1.0");
  fprintf_tab0(" ");
  fprintf_pair("encoding", "UTF-8");
  fprintf_tab0("?>\n");
  fprintf_tab0("<playlist ");
  fprintf_pair("xmlns", "http://xspf.org/ns/0/");
  fprintf_tab0(" ");
  fprintf_pair("xmlns:vlc", "http://www.videolan.org/vlc/playlist/ns/0/");
  fprintf_tab0(" ");
  fprintf_pair("version", "1");
  fprintf_tab0(">\n");
  fprintf_tab1("<title>DVB Playlist</title>\n");
  fprintf (f,"%s<creator>w_scan-%i</creator>\n", T1, version);        
  fprintf_tab1("<info>http://wirbel.htpc-forum.de</info>\n");
  fprintf_tab1("<trackList>\n");

  idx = 1;
}

void vlc_xspf_epilog(FILE *f) {
  fprintf_tab1("</trackList>\n");
  fprintf_tab0("</playlist>\n");
}


/* PROBLEM: The xspf channel syntax VLC a DVB channel list saves, is INVALID xspf, see
 * http://validator.xspf.org/ :(
 *
 * So, i try to save data in a VALID xspf syntax, but still READABLE BY VLC.
 */

void vlc_dump_dvb_parameters_as_xspf (FILE * f, struct transponder * t, struct w_scan_flags * flags, struct lnb_types_st * lnbp) {
  fprintf (f, "%s%s",             T3, "<location>");
  switch (flags->scantype) {
     case SCAN_TERRCABLE_ATSC:
        fprintf (f, "%s", "atsc://");
        fprintf (f, "frequency=%i",             t->frequency);
        fprintf (f, "%s\n", "</location>");

        fprintf_tab3("<extension ");
        fprintf_pair("application", "http://www.videolan.org/vlc/playlist/0");
        fprintf_tab0(">\n");
        if (t->modulation != QAM_AUTO)
           fprintf(f,"%s<vlc:option>dvb-modulation=%s</vlc:option>\n",       T4, vlc_modulation(t->modulation));
        break;

     case SCAN_CABLE://<location>dvb-c:frequency=522000000:modulation=64QAM:srate=6900000</location>
        fprintf (f, "%s://",                    vlc_delsys(t->delsys));
        fprintf (f, "frequency=%i",             t->frequency);
        fprintf (f, "%s\n", "</location>");

        fprintf_tab3("<extension ");
        fprintf_pair("application", "http://www.videolan.org/vlc/playlist/0");
        fprintf_tab0(">\n");

        fprintf(f, "%s<vlc:option>dvb-srate=%i</vlc:option>\n",              T4, t->symbolrate);
        fprintf(f,"%s<vlc:option>dvb-ts-id=%i</vlc:option>\n",               T4, t->transport_stream_id);

        if (t->modulation != QAM_AUTO)
           fprintf(f,"%s<vlc:option>dvb-modulation=%s</vlc:option>\n",       T4, vlc_modulation(t->modulation));
        if (t->inversion != INVERSION_AUTO)
           fprintf(f,"%s<vlc:option>dvb-inversion=%i</vlc:option>\n",        T4, vlc_inversion(t->inversion));
        break;

     case SCAN_TERRESTRIAL:
        fprintf (f, "%s://",                    vlc_delsys(t->delsys));
        fprintf (f, "frequency=%i",             t->frequency);
        fprintf (f, "%s\n", "</location>");

        fprintf_tab3("<extension ");
        fprintf_pair("application", "http://www.videolan.org/vlc/playlist/0");
        fprintf_tab0(">\n");

        fprintf(f,"%s<vlc:option>dvb-bandwidth=%i</vlc:option>\n", T4, vlc_bandwidth(t->bandwidth));
        fprintf(f,"%s<vlc:option>dvb-ts-id=%i</vlc:option>\n",     T4, t->transport_stream_id);

        if (t->plp_id != 0)
           fprintf(f,"%s<vlc:option>dvb-plp-id=%i</vlc:option>\n",           T4, t->plp_id);
        if (t->inversion != INVERSION_AUTO)
           fprintf(f,"%s<vlc:option>dvb-inversion=%i</vlc:option>\n",        T4, vlc_inversion(t->inversion));
        if (t->coderate != FEC_AUTO)
           fprintf(f,"%s<vlc:option>dvb-code-rate-hp=%s</vlc:option>\n",     T4, vlc_fec(t->coderate));
        if ((t->coderate_LP != FEC_AUTO) && (t->coderate_LP != FEC_NONE))
           fprintf(f,"%s<vlc:option>dvb-code-rate-lp=%s</vlc:option>\n",     T4, vlc_fec(t->coderate_LP));
        if (t->modulation != QAM_AUTO)
           fprintf(f,"%s<vlc:option>dvb-modulation=%s</vlc:option>\n",       T4, vlc_modulation(t->modulation));
        if (t->transmission != TRANSMISSION_MODE_AUTO)
           fprintf(f,"%s<vlc:option>dvb-transmission=%i</vlc:option>\n",     T4, vlc_transmission(t->transmission));
        if (t->guard != GUARD_INTERVAL_AUTO)
           fprintf(f,"%s<vlc:option>dvb-guard=%s</vlc:option>\n",            T4, vlc_guard(t->guard));
        if ((t->hierarchy != HIERARCHY_AUTO) && (t->hierarchy != HIERARCHY_NONE))
           fprintf(f,"%s<vlc:option>dvb-hierarchy=%i</vlc:option>\n",        T4, vlc_hierarchy(t->hierarchy));
        break;

     case SCAN_SATELLITE:
        /* NOTE: VLC         20140102) more than 3 years later.. 
         *       now supports DVB API v5 and S2 - good. But still seems to miss Rotor/positioner, SCR and DISEQC support.
         *       But at least they broke this xspf file format successfully several times in this time.
         *       - Comma vs. Semicolon change
         *       - VLC still dies if '&' occurs in channel name. << understood, xml prob but shouldnt die.
         *       - obsoleting options
         *       - still NO FILE DOCUMENTATION for this dvb xspf format. :-(
         */
        fprintf (f, "%s://",                    vlc_delsys(t->delsys));
        fprintf (f, "frequency=%i",             t->frequency);
        fprintf (f, "%s\n", "</location>");

        fprintf_tab3("<extension ");
        fprintf_pair("application", "http://www.videolan.org/vlc/playlist/0");
        fprintf_tab0(">\n");

        switch (t->polarization) {
           case POLARIZATION_HORIZONTAL:
           case POLARIZATION_CIRCULAR_LEFT:
              fprintf(f,"%s<vlc:option>dvb-polarization=H</vlc:option>\n", T4);
              break;
           default:
              fprintf(f,"%s<vlc:option>dvb-polarization=V</vlc:option>\n", T4);
              break;                                
           }

        fprintf(f, "%s<vlc:option>dvb-srate=%i</vlc:option>\n",           T4, t->symbolrate);
        fprintf(f,"%s<vlc:option>dvb-ts-id=%i</vlc:option>\n",            T4, t->transport_stream_id);

        if (t->delsys != SYS_DVBS) {
           fprintf(f,"%s<vlc:option>dvb-modulation=%s</vlc:option>\n",    T4, vlc_modulation(t->modulation));
           if (t->rolloff != ROLLOFF_AUTO)
              fprintf(f,"%s<vlc:option>dvb-rolloff=%i</vlc:option>\n",    T4, vlc_rolloff(t->rolloff));
           }
        
        if (t->inversion != INVERSION_AUTO)
           fprintf(f,"%s<vlc:option>dvb-inversion=%i</vlc:option>\n",     T4, vlc_inversion(t->inversion));
        if (t->coderate != FEC_AUTO)
           fprintf(f,"%s<vlc:option>dvb-fec=%s</vlc:option>\n",           T4, vlc_fec(t->coderate));

        fprintf(f,"%s<vlc:option>dvb-lnb-low=%lu</vlc:option>\n",         T4, lnbp->low_val);
        fprintf(f,"%s<vlc:option>dvb-lnb-high=%lu</vlc:option>\n",        T4, lnbp->high_val);
        fprintf(f,"%s<vlc:option>dvb-lnb-switch=%lu</vlc:option>\n",      T4, lnbp->switch_val);

        if ((flags->sw_pos & 0xF) < 0xF)
           fprintf(f,"%s<vlc:option>dvb-satno=%i</vlc:option>\n",         T4, flags->sw_pos & 0xF);
        break;

     default:
        fatal("Unknown scantype %d\n", flags->scantype);
     };
}



/* TODO: THIS IS TEMPORAL && UGLY SOLUTION ONLY.
 *
 *       - REWORK LOGIC BEHIND SERVICE NAME;
 *       - MOVE TO GENERAL XML FUNC IN CHAR_CODING.C/.H && DEFINE ENTITIES;
 *       - ADD CONVERSION TO 8859-1.
 *       IN GENERAL, THERE WILL BE NEVER A COMPLETE SOLUTION POSSIBLE,
 *       SINCE INPUT CHAR CODING IS VERY OFTEN WRONG CODED BY SERVICE
 *       PROVIDERS. :(
 */
void vlc_dump_service_parameter_set_as_xspf (FILE * f, struct service * s,
                                struct transponder * t, struct w_scan_flags * flags,
                                struct lnb_types_st *lnbp) {
  char buf[256];
  int i,j,len = s->service_name? strlen(s->service_name):0;
  for(i=0,j=0; i<len; i++) {
     uint8_t u = (unsigned) s->service_name[i];
     if (u < 0x7F) {
        if (range(u,0x09,0x09)) {
           buf[j++]=' ';
           }
        else if (range(u,0x20,0x21) ||
                 range(u,0x23,0x25) ||
                 range(u,0x28,0x3B) ||
                 range(u,0x3D,0x3D) ||
                 range(u,0x3F,0x7E)) {
           buf[j++] = (char) u;
           }
        else if (range(u,0x22,0x22)) { // double quotation mark
           buf[j++]='&';
           buf[j++]='q';
           buf[j++]='u';
           buf[j++]='o';
           buf[j++]='t';
           buf[j++]=';';
           }
        else if (range(u,0x26,0x26)) { // double quotation mark
           buf[j++]='&';
           buf[j++]='a';
           buf[j++]='m';
           buf[j++]='p';
           buf[j++]=';';
           }
        else if (range(u,0x27,0x27)) { // apostrophe
           buf[j++]='&';
           buf[j++]='a';
           buf[j++]='p';
           buf[j++]='o';
           buf[j++]='s';
           buf[j++]=';';
           }
        else if (range(u,0x3C,0x3C)) { // less-than sign 
           buf[j++]='&';
           buf[j++]='l';
           buf[j++]='t';
           buf[j++]=';';
           }
        else if (range(u,0x3E,0x3E)) { // greater-than sign
           buf[j++]='&';
           buf[j++]='g';
           buf[j++]='t';
           buf[j++]=';';
           }
        }
     else {
        // threat them as ISO8859-1: SOME HACK ANYWAY AS INPUT CHAR SET MAY BE WRONG.
        if (u >= 0xA1) { // skip unused and NBSP: 0x7F .. 0xA0
           // numeric character reference "&#xhhhh;"
           uint8_t ca = u >> 4;  // A .. F
           uint8_t cb = u & 0xF; // 1 .. F
           buf[j++]='&';
           buf[j++]='#';
           buf[j++]='x';
           buf[j++]='0';
           buf[j++]='0';
           buf[j++]= ca < 10 ? '0' + ca : 'A' + (ca - 10);
           buf[j++]= cb < 10 ? '0' + cb : 'A' + (cb - 10);
           buf[j++]=';';
           }              
        }
     }
  buf[j++]=0;

  fprintf_tab2("<track>\n");
  fprintf (f, "%s%s%.4d. %s%s\n", T3, "<title>", idx++, buf, "</title>");

  vlc_dump_dvb_parameters_as_xspf(f, t, flags, lnbp);

  fprintf (f, "%s%s%d%s\n", T4, "<vlc:id>", idx, "</vlc:id>");
  fprintf (f, "%s%s%d%s\n", T4, "<vlc:option>program=", s->service_id, "</vlc:option>");
  fprintf_tab3("</extension>\n");
  fprintf_tab2("</track>\n");
}
