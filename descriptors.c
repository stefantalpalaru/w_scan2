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
 *
 * added    20090303 -wk-
 * extended 20120109 -wk-
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "extended_frontend.h"
#include "si_types.h"
#include "scan.h"
#include "descriptors.h"
#include "atsc_psip_section.h"
#include "char-coding.h"

#define hd(d)  hexdump(__FUNCTION__, d + 2, d[1])

/******************************************************************************
 * returns minimum repetition rates as specified in ETR211 4.4.1 and 4.4.2
 * and 13818-1 C.9 Bandwidth Utilization and Signal Acquisition Time
 *****************************************************************************/

int repetition_rate(scantype_t scan_type, enum table_id table) {
  switch(scan_type) {
     case SCAN_CABLE:
     case SCAN_SATELLITE:
        // ETR211 4.4.1 Satellite and cable delivery systems
        switch(table) {
           case TABLE_PAT:
           case TABLE_CAT:
           case TABLE_PMT:
           case TABLE_TSDT:
              // see 13818-1 C.9 Bandwidth Utilization and Signal Acquisition Time
              // FIXME: i did not understand fully
              // but i seems to be (1/1 .. [1/25] .. 1/100) sec
              // no hard spec.. :-(
              return 1;
           case TABLE_SDT_ACT:
           case TABLE_EIT_ACT:
           case TABLE_EIT_SCHEDULE_ACT_50 ... TABLE_EIT_SCHEDULE_ACT_5F:
              return 2;
           case TABLE_NIT_ACT:
           case TABLE_NIT_OTH:
           case TABLE_BAT:
           case TABLE_SDT_OTH:
           case TABLE_EIT_OTH:
              return 10;
           case TABLE_EIT_SCHEDULE_OTH_60 ... TABLE_EIT_SCHEDULE_OTH_60:
           case TABLE_TDT:
           case TABLE_TOT:
              return 30;
           default:
              debug("table id 0x%.02X no repetition rate defined.\n", table);
              return 30;
           }
        break;
     case SCAN_TERRESTRIAL:
        // ETR211 4.4.2 Terrestrial delivery systems
        switch(table) {
           case TABLE_PAT:
           case TABLE_CAT:
           case TABLE_PMT:
           case TABLE_TSDT:
              // see 13818-1 C.9 Bandwidth Utilization and Signal Acquisition Time
              // FIXME: i did not understand fully
              // but i seems to be (1/1 .. [1/25] .. 1/100) sec
              // no hard spec.. :-(
              return 1;
           case TABLE_NIT_ACT:
           case TABLE_NIT_OTH:
           case TABLE_BAT:
           case TABLE_SDT_OTH:
           case TABLE_EIT_SCHEDULE_ACT_50 ... TABLE_EIT_SCHEDULE_ACT_5F:
              return 12;
           case TABLE_SDT_ACT:
           case TABLE_EIT_ACT:
              return 2;
           case TABLE_EIT_OTH:
              return 20;
           case TABLE_EIT_SCHEDULE_OTH_60 ... TABLE_EIT_SCHEDULE_OTH_60:
              return 60;
           case TABLE_TDT:
           case TABLE_TOT:
              return 30;
           default:
              debug("table id 0x%.02X no repetition rate defined.\n", table);
              return 30;
           }
        break;
     case SCAN_TERRCABLE_ATSC:
        switch(table) {
           case TABLE_PAT:
           case TABLE_CAT:
           case TABLE_PMT:
           case TABLE_TSDT:
              // see 13818-1 C.9 Bandwidth Utilization and Signal Acquisition Time
              // FIXME: i did not understand fully
              // but i seems to be (1/1 .. [1/25] .. 1/100) sec
              // no hard spec.. :-(
              return 1;
           default:
              /* FIXME: i dont have *any* information about atsc
               * repetition rates. This should not break anything,
               * but may be we will loose performance or services..
               * these are the values mkrufky put in.
               * Probably the same values as above..?
               */
              debug("table id 0x%.02X no repetition rate defined.\n",
                      table);
              return 5;
           }
        return 5;
     default:
        fatal("undefined frontend type.\n");
     }
};

/******************************************************************************
 * 300468 v181 6.2.32 Service descriptor
 *****************************************************************************/

void parse_service_descriptor (const unsigned char *buf, struct service *s, unsigned user_charset_id) {
  unsigned char len;
  uint i, full_len, short_len, isUtf8;
  uint emphasis_on = 0;
  char * provider_name = NULL;
  char * provider_short_name = NULL;
  char * service_name = NULL;
  char * service_short_name = NULL;
  size_t inbytesleft, outbytesleft;
  char * inbuf = NULL;
  char * outbuf = NULL;

  hd(buf);
  s->type = buf[2];

  buf += 3;
  len = *buf;
  buf++;

  if (s->provider_name) {
     free (s->provider_name);
     s->provider_name = 0;
     }
  if (s->provider_short_name) {
     free (s->provider_short_name);
     s->provider_name = 0;
     }
  full_len = short_len = emphasis_on = 0;
  isUtf8 = (*buf == 0x15); 
  /* count length for short provider name
   * and long provider name
   */
  for(i=0; i < len; i++) {        
     switch(*(buf + i)) {
        case sb_cc_reserved_80 ... sb_cc_reserved_85:
        case sb_cc_reserved_88 ... sb_cc_reserved_89:
        case sb_cc_user_8B ... sb_cc_user_9F:
        // ETR211 4.6.1 Use of control codes in names
        case character_cr_lf:
           continue;
        case character_emphasis_on:
           emphasis_on = 1;
           continue;
        case character_emphasis_off:
           emphasis_on = 0;
           continue;
        case utf8_cc_start:
           if (isUtf8 && (i+1 < len)) {
              uint16_t utf8_cc;
              utf8_cc  = *(buf + i) << 8;
              utf8_cc += *(buf + i + 1);

              switch(utf8_cc) {
                 case utf8_character_emphasis_on:
                    emphasis_on = 1;
                    i++;
                    continue;
                 case utf8_character_emphasis_off:
                    emphasis_on = 0;
                    i++;
                    continue;
                 default:;
                 }
              }
        default:
           if (emphasis_on)
              short_len++;
           full_len++;
           continue;
        }
     }
  /* allocating memory and zero-terminating */
  provider_name = calloc (full_len + 1, 1);
  provider_short_name = calloc (short_len + 1, 1);
  full_len = short_len = emphasis_on = 0;
  /* copy data */
  for(i=0; i < len; i++) {
     switch(*(buf + i)) {
        case sb_cc_reserved_80 ... sb_cc_reserved_85:
        case sb_cc_reserved_88 ... sb_cc_reserved_89:
        case sb_cc_user_8B ... sb_cc_user_9F:
        // ETR211 4.6.1 Use of control codes in names
        case character_cr_lf:
           continue;
        case character_emphasis_on:
           emphasis_on = 1;
           continue;
        case character_emphasis_off:
           emphasis_on = 0;
           continue;
        case utf8_cc_start:
           if (isUtf8 && (i+1 < len)) {
              uint16_t utf8_cc;
              utf8_cc  = *(buf + i) << 8;
              utf8_cc += *(buf + i + 1);

              switch(utf8_cc) {
                 case utf8_character_emphasis_on:
                    emphasis_on = 1;
                    i++;
                    continue;
                 case utf8_character_emphasis_off:
                    emphasis_on = 0;
                    i++;
                    continue;
                 default:;
                 }
              } 
        default:
           if (emphasis_on)
              provider_short_name[short_len++] = *(buf + i);
           provider_name[full_len++] = *(buf + i);
           continue;
        }
     }
  if (provider_name[0]) {
     inbytesleft = full_len;
     outbytesleft = 4 * full_len + 1;
     s->provider_name = (char *) calloc(outbytesleft, 1);
     inbuf = provider_name;
     outbuf = s->provider_name;
     char_coding(&inbuf, &inbytesleft, &outbuf, &outbytesleft, user_charset_id);
     }

  free(provider_name);

  if (provider_short_name[0]) {
     inbytesleft = short_len;
     outbytesleft = 4 * short_len + 1;
     s->provider_short_name = (char *) calloc(outbytesleft, 1);
     inbuf = provider_short_name;
     outbuf = s->provider_short_name;
     char_coding(&inbuf, &inbytesleft, &outbuf, &outbytesleft, user_charset_id);
     }

  free(provider_short_name);

  buf += len;
  len = *buf;
  buf++;
  if (s->service_name)
     free (s->service_name);
  if (s->service_short_name)
     free (s->service_short_name);
  isUtf8 = (*buf == 0x15);
  /* count length for short service name
   * and long service name
   */
  full_len = short_len = emphasis_on = 0;
  for(i=0; i < len; i++) {
     switch(*(buf + i)) {
        case sb_cc_reserved_80 ... sb_cc_reserved_85:
        case sb_cc_reserved_88 ... sb_cc_reserved_89:
        case sb_cc_user_8B ... sb_cc_user_9F:
        // ETR211 4.6.1 Use of control codes in names
        case character_cr_lf:
           continue;
        case character_emphasis_on:
           emphasis_on = 1;
           continue;
        case character_emphasis_off:
           emphasis_on = 0;
           continue;
        case utf8_cc_start:
           if (isUtf8 && (i+1 < len)) {
              uint16_t utf8_cc;
              utf8_cc  = *(buf + i) << 8;
              utf8_cc += *(buf + i + 1);

              switch(utf8_cc) {
                 case utf8_character_emphasis_on:
                    emphasis_on = 1;
                    i++;
                    continue;
                 case utf8_character_emphasis_off:
                    emphasis_on = 0;
                    i++;
                    continue;
                 default:;
                 }
              }
        default:
                if (emphasis_on)
                   short_len++;
                full_len++;
                continue;
        }
     }
  /* allocating memory and zero-terminating */
  service_name = calloc (full_len + 1, 1);        
  service_short_name = calloc (short_len + 1, 1);
  full_len = short_len = emphasis_on = 0;
  /* copy data */
  for(i=0; i < len; i++) {
     switch(*(buf + i)) {
        case sb_cc_reserved_80 ... sb_cc_reserved_85:
        case sb_cc_reserved_88 ... sb_cc_reserved_89:
        case sb_cc_user_8B ... sb_cc_user_9F:
        // ETR211 4.6.1 Use of control codes in names
        case character_cr_lf:
           continue;
        case character_emphasis_on:
           emphasis_on = 1;
           continue;
        case character_emphasis_off:
           emphasis_on = 0;
           continue;
        case utf8_cc_start:
           if (isUtf8 && (i+1 < len)) {
              uint16_t utf8_cc;
              utf8_cc  = *(buf + i) << 8;
              utf8_cc += *(buf + i + 1);

              switch(utf8_cc) {
                 case utf8_character_emphasis_on:
                    emphasis_on = 1;
                    i++;
                    continue;
                 case utf8_character_emphasis_off:
                    emphasis_on = 0;
                    i++;
                    continue;
                 default:;
                 }
              }
        default:
           if (emphasis_on)
              service_short_name[short_len++] = *(buf + i);
           service_name[full_len++] = *(buf + i);
           continue;
        }
     }
  if (service_name[0]) {
     inbytesleft = full_len;
     outbytesleft = 4 * full_len + 1;
     s->service_name = (char *) calloc(outbytesleft, 1);
     inbuf = service_name;
     outbuf = s->service_name;
     char_coding(&inbuf, &inbytesleft, &outbuf, &outbytesleft, user_charset_id);
     }

  free(service_name);

  if (service_short_name[0]) {
     inbytesleft = short_len;
     outbytesleft = 4 * short_len + 1;
     s->service_short_name = (char *) calloc(outbytesleft, 1);
     inbuf = service_short_name;
     outbuf = s->service_short_name;
     char_coding(&inbuf, &inbytesleft, &outbuf, &outbytesleft, user_charset_id);
     }

  free(service_short_name);

  info("\tservice = %s (%s)\n", s->service_name, s->provider_name);
}

void parse_ca_identifier_descriptor (const unsigned char *buf, struct service *s) {
  unsigned char len = buf [1];
  unsigned int i;

   buf += 2;

   if (len > sizeof(s->ca_id)) {
      len = sizeof(s->ca_id);
      warning("too many CA system ids\n");
      }
   memcpy(s->ca_id, buf, len);
   s->ca_num=0;
   for(i = 0; i < len / sizeof(s->ca_id[0]); i++) {
      int id = ((s->ca_id[i] & 0x00FF) << 8) + ((s->ca_id[i] & 0xFF00) >> 8);
      s->ca_id[i] = id;
      moreverbose("\tCA ID\t: PID 0x%04x\n", s->ca_id[i]);
      s->ca_num++;
      }
}

void parse_ca_descriptor (const unsigned char *buf, struct service *s) {
  unsigned char descriptor_length = buf [1];
  int CA_system_ID;
  int found=0;
  int i;

  buf += 2;

  if (descriptor_length < 4)
     return;
  CA_system_ID = (buf[0] << 8) | buf[1];

  for(i=0; i<s->ca_num; i++)
     if (s->ca_id[i] == CA_system_ID)
        found++;

  if (!found) {
     if (s->ca_num + 1 >= CA_SYSTEM_ID_MAX)
        warning("TOO MANY CA SYSTEM IDs.\n");
     else {
        moreverbose("\tCA ID\t: PID 0x%04x\n", CA_system_ID);
        s->ca_id[s->ca_num]=CA_system_ID;
        s->ca_num++;
        }
     }
} 

void parse_iso639_language_descriptor (const unsigned char *buf, struct service *s) {
  unsigned int lang_count = buf[1] / 4;
  unsigned int i;
  buf += 2;
  if (s->audio_num < 1) return;
  for(i = 0; i < lang_count; i++) {
     // ISO_639_language_code 24 bslbf
     memcpy(s->audio_lang[s->audio_num-1], buf, 3);                
/*   switch(buf[3]) { // audio_type 8 bslbf, seems to be wrong all over the place
        case 1: // clean effects, program element has no language
                break;
        case 2: // hearing impaired, program element is prepared for the hearing impaired
                break;
        case 3: // visual_impaired_commentary, program element is prepared for the visually impaired viewer
                break;
        default:
                info("unhandled audio_type.\n");
        }*/
     buf += 4;
     }
}

void parse_subtitling_descriptor (const unsigned char *buf, struct service *s) {
  unsigned int N = buf[1] / 8; // descriptor_length divided by 8_bytes per subtitle
  unsigned int i;
  buf += 2;

  if (N > SUBTITLES_MAX)
     N = SUBTITLES_MAX;

  for(i = 0; i < N; i++) {
     memcpy(s->subtitling_lang[i], buf, 3);
     buf += 3;
     s->subtitling_type[i]     = buf[0];
     buf++;
     s->composition_page_id[i] = buf[0] << 8 | buf[1];
     buf += 2;
     s->ancillary_page_id[i]   = buf[0] << 8 | buf[1];
     buf += 2;
     }
}

void parse_network_name_descriptor (const unsigned char *buf, struct transponder *t) {
  unsigned char len = buf[1];
  //hd(buf);
  if (t == NULL) {
     info("%s: transponder == NULL\n", __FUNCTION__);
     return;
     }
  if (t->network_name)
     free (t->network_name);
  t->network_name = (char *) malloc(len + 1);
  memcpy(t->network_name, buf + 2, len);
  t->network_name[len] = '\0';
  if (!t->network_name[0]) {
     free (t->network_name);
     t->network_name = 0;
     }
}

static long bcd32_to_cpu (const int b0, const int b1, const int b2, const int b3) {
  return ((b0 >> 4) & 0x0f) * 10000000 + (b0 & 0x0f) * 1000000 +
         ((b1 >> 4) & 0x0f) * 100000   + (b1 & 0x0f) * 10000 +
         ((b2 >> 4) & 0x0f) * 1000     + (b2 & 0x0f) * 100 +
         ((b3 >> 4) & 0x0f) * 10       + (b3 & 0x0f);
}

time_t bcdtime(const unsigned char *t) {
  return ((t[0] >> 4)*10 + (t[0] & 0xF)) * 3600 +
         ((t[1] >> 4)*10 + (t[1] & 0xF)) * 60 +
         ((t[2] >> 4)*10 + (t[2] & 0xF));
}

__u32 get_u32(const unsigned char *p) {
  return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

__u32 get_u24(const unsigned char *p) {
  return (p[0] << 16) | (p[1] << 8) | p[2];
}

__u16 get_u16(const unsigned char *p) {
  return (p[0] << 8) | p[1];
}

void parse_S2_satellite_delivery_system_descriptor(const unsigned char *buf, void * dummy) {
  hd(buf);
  /* FIXME: finding that descriptor means that we're dealing with two
   * transponders on the same freq. I'm not shure now what to do this case.
   */
  //scrambling_sequence_selector 1 bslbf
  //scrambling_sequence_selector = (buf[2] & 0x80) >> 7;
  //multiple_input_stream_flag 1 bslbf
  //multiple_input_stream_flag = (buf[2] & 0x40) >> 6;
  //backwards_compatibility_indicator 1 bslbf
  //backwards_compatibility_indicator = (buf[2] & 0x20) >> 5;
  //reserved_future_use 5 bslbf
  //buf += 3;
  //if (scrambling_sequence_selector == 1) {
  //              Reserved 6 bslbf
  //              scrambling_sequence_index 18 uimsbf
  //              scrambling_sequence_index = (*buf++ & 0x03) << 16;
  //              scrambling_sequence_index |= *buf++ << 8;
  //              scrambling_sequence_index |= *buf++;
  //              }
  //if (multiple_input_stream_flag == 1) {
  //              input_stream_identifier 8 uimsbf
  //              input_stream_identifier = *buf++;
  //}
  verbose("S2_satellite_delivery_system_descriptor(skipped.)\n");
}

void parse_satellite_delivery_system_descriptor(const unsigned char *buf,
                        struct transponder *t, fe_spectral_inversion_t inversion) {
  if (t == NULL)
     return;
  hd(buf);
  t->type = SCAN_SATELLITE;
  t->source = 0x43;
  t->inversion = inversion;
  /* frequency is coded in GHz, where the decimal point occurs after the
   * third character (e.g. 011,75725 GHz).
   */
  t->frequency = 10 * bcd32_to_cpu (buf[2], buf[3], buf[4], buf[5]);
  //orbital_position 16 bslbf
  t->orbital_position = (buf[6] << 8) | buf[7];
  //west_east_flag 1 bslbf
  t->west_east_flag = (buf[8] & 0x80) >> 7;
  //polarization 2 bslbf
  switch((buf[8] & 0x60) >> 5) {
     case 0: t->polarization = POLARIZATION_HORIZONTAL;         break;
     case 1: t->polarization = POLARIZATION_VERTICAL;           break;
     case 2: t->polarization = POLARIZATION_CIRCULAR_LEFT;      break;
     case 3: t->polarization = POLARIZATION_CIRCULAR_RIGHT;     break;
     default:
        fatal("polarization decoding failed: %d\n", (buf[8] & 0x60) >> 5);
     }
  switch((buf[8] & 0x18) >> 3) {
     case 0: t->rolloff = ROLLOFF_35;   break;
     case 1: t->rolloff = ROLLOFF_25;   break;
     case 2: t->rolloff = ROLLOFF_20;   break;
     case 3:        
        warning("reserved rolloff value 3 found\n");
        t->rolloff = ROLLOFF_AUTO;
        break;
     default:
        fatal("rolloff decoding failed: %d\n", (buf[8] & 0x18) >> 3);
     }
  switch((buf[8] & 0x04) >> 2) {
     case 0: t->delsys = SYS_DVBS;   break;
     case 1: t->delsys = SYS_DVBS2;  break;
     default:
             t->delsys = SYS_DVBS;
     }
  //modulation_type 2 bslbf
  switch(buf[8] & 0x03) {
     case 1: t->modulation = QPSK;         break;
     case 2: t->modulation = PSK_8;        break;
     case 3: t->modulation = QAM_16;       break;
     default:
             t->modulation = QAM_AUTO;
     }
  if (t->modulation == PSK_8)
          t->delsys = SYS_DVBS2;
  //symbol_rate 28 bslbf
  t->symbolrate = 10 * bcd32_to_cpu(buf[9], buf[10], buf[11], buf[12] & 0xF0);
  //FEC_inner 4 bslbf
  switch (buf[12] & 0x0F) {
     case 1: t->coderate = FEC_1_2;    break;
     case 2: t->coderate = FEC_2_3;    break;
     case 3: t->coderate = FEC_3_4;    break;
     case 4: t->coderate = FEC_5_6;    break;
     case 5: t->coderate = FEC_7_8;    break;
     case 6: t->coderate = FEC_8_9;    break;
     case 7: t->coderate = FEC_3_5;    break;
     case 8: t->coderate = FEC_4_5;    break;
     case 9: t->coderate = FEC_9_10;   break;
     case 15:t->coderate = FEC_NONE;   break;
     default:
             verbose("\t%s: undefined inner fec %u\n",
             __FUNCTION__, buf[12] & 0x0F);
             t->coderate = FEC_AUTO;
     }
  /* some NIT's are broken. */
  if ((t->modulation == PSK_8) ||
     (t->rolloff == ROLLOFF_25) ||
     (t->rolloff == ROLLOFF_20) ||
     (t->coderate == FEC_9_10) ||
     (t->coderate == FEC_3_5)) {
         verbose("\t%s: fixing broken NIT, setting modulation_system to DVB-S2.\n",
             __FUNCTION__);
         t->delsys = SYS_DVBS2;
         }
}


#ifndef FEC_RS_204_208  //FIXME: as soon as defined in Linux DVB API, insert correct name here.
        #define FEC_RS_204_208  FEC_AUTO
#endif

void parse_cable_delivery_system_descriptor (const unsigned char * buf, struct transponder * t,
                                                fe_spectral_inversion_t inversion) {
  if (t == NULL)
     return;
  hd(buf);
  t->type = SCAN_CABLE;
  t->source = 0x44;
  t->delsys = SYS_DVBC_ANNEX_AC;
  t->inversion = inversion;
  /*frequency is coded in MHz, where the decimal occurs after the fourth
  character (e.g. 0312,0000 MHz).
  */
  t->frequency = 100 * bcd32_to_cpu (buf[2], buf[3], buf[4], buf[5]);
  //t->reserved_future_use = (buf[6] << 4) | ((buf[7] & 0xf0) >> 4); 
  //FEC_outer 4 bslbf -> not used by linuxtv dvb api. WHY?
  //   switch (buf[7] & 0x0f) {
  //      case 1: t->fec_outer = FEC_NONE;       break;
  //      case 2: t->fec_outer = FEC_RS_204_208; break;
  //      default:
  //         info("undefined outer fec\n");
  //         t->fec_outer = FEC_AUTO;
  //      }
  //modulation 8 bslbf
  switch (buf[8]) {
     case 1: t->modulation = QAM_16;  break;
     case 2: t->modulation = QAM_32;  break;
     case 3: t->modulation = QAM_64;  break;
     case 4: t->modulation = QAM_128; break;
     case 5: t->modulation = QAM_256; break;
     default:
        info("undefined modulation\n");
        t->modulation = QAM_AUTO;
     }
  //symbol_rate 28 bslbf
  t->symbolrate = 10 * bcd32_to_cpu(buf[9], buf[10], buf[11], buf[12] & 0xf0);
  //FEC_inner 4 bslbf
  switch (buf[12] & 0x0f) {
     case 1: t->coderate = FEC_1_2;   break;
     case 2: t->coderate = FEC_2_3;   break;
     case 3: t->coderate = FEC_3_4;   break;
     case 4: t->coderate = FEC_5_6;   break;
     case 5: t->coderate = FEC_7_8;   break;
     case 6: t->coderate = FEC_8_9;   break;
     case 7: t->coderate = FEC_3_5;   break;
     case 8: t->coderate = FEC_4_5;   break;
     case 9: t->coderate = FEC_9_10;  break;
     case 15: t->coderate = FEC_NONE; break;
     default:
        info("undefined inner fec\n");
        t->coderate = FEC_AUTO;
     }
}

/* DVB-C2: PRELIMINARY && UNTESTED CODE ONLY. I NEED SOMEBODY WITH ACCESS TO
 *         DVB-C2. IF YOU WANT TO HELP PLS CONTACT ME. --20111204, wirbel--
 */
// 300468 v011201_final_draft; 09/2011
void parse_C2_delivery_system_descriptor (const unsigned char *buf,
                        struct transponder *t, fe_spectral_inversion_t inversion) {
  __u8 descriptor_length;
//__u8 descriptor_tag_extension;
  unsigned char * bp;

  hd(buf);
  if (t == NULL) return;

  t->type = SCAN_CABLE;
  t->source = 0x0D;
  t->delsys = SYS_DVBC2;
  t->inversion = inversion;
                                                                                                     // descriptor_tag               8 uimsbf
  descriptor_length = buf[1];                                                                        // descriptor_length            8 uimsbf
//descriptor_tag_extension = buf[2];                                                                 // descriptor_tag_extension     8 uimsbf
  bp = (unsigned char *) &buf[3]; descriptor_length--;
  t->plp_id = *bp;                                                                                   // plp_id                       8 uimsbf; uniquely identifies a data PLP within the C2 System
  bp++; descriptor_length--;
  t->data_slice_id = *bp;                                                                            // data_slice_id                8 uimsbf; uniquely identifies a data slice within the C2 system
  bp++; descriptor_length--;
  t->frequency = get_u32(bp);                                                                        // C2_tuning_frequency          32 bslbf; see C2_tuning_frequency_type
  bp+=4; descriptor_length-=4;
  switch((*bp & 0xC0) >> 6) {                                                                        // C2_tuning_frequency_type     2 uimsbf
     case 0: t->C2_tuning_frequency_type = DATA_SLICE_TUNING_FREQUENCY; break;
     case 1: t->C2_tuning_frequency_type = C2_SYSTEM_CENTER_FREQUENCY; break;
     case 2: t->C2_tuning_frequency_type = INITIAL_TUNING_FOR_STATIC_DATA_SLICE; break;
   //case 3: reserved_for_future_use
     default:t->C2_tuning_frequency_type = DATA_SLICE_TUNING_FREQUENCY;                              // This is the default option for C2 systems
     }
  switch((*bp & 0x38) >> 3) {                                                                        // active_OFDM_symbol_duration  3 uimsbf
     case 0: t->active_OFDM_symbol_duration = FFT_4K_8MHZ; break;                                    // 448µsec    (4k FFT mode for 8MHz CATV systems)
     case 1: t->active_OFDM_symbol_duration = FFT_4K_6MHZ; break;                                    // 597,33µsec (4k FFT mode for 6MHz CATV systems)
   //case 2 ... 7: reserved_for_future_use                                                           //
     default:t->active_OFDM_symbol_duration = FFT_4K_8MHZ;                                           // defaulting to here to 8MHz CATV systems, as nothing better found so far.
     }
  switch(*bp & 0x07) {                                                                               // guard_interval               3 bslbf
     case 0: t->guard = GUARD_INTERVAL_1_128; break;                                                 //
     case 1: t->guard = GUARD_INTERVAL_1_64; break;                                                  // not defined in linux dvb api, see extended_frontend.h
   //case 2 ... 7: reserved_for_future_use                                                           //
     default:t->guard = GUARD_INTERVAL_1_128;                                                        // defaulting to here to 1/128, as nothing better found so far.
     }
  bp++; descriptor_length--;
}

/*
 *  20140626:
 *      - if center_frequency = 0 and other_frequency_flag not set -> set this flag explictly.
 */
void parse_terrestrial_delivery_system_descriptor(const unsigned char * buf,
                        struct transponder * t, fe_spectral_inversion_t inversion) {
  uint32_t center_frequency;
  struct frequency_item * p, * p1;
  bool known;

  hd(buf);
  if (t == NULL) return;

  t->type = SCAN_TERRESTRIAL;
  t->source = 0x5A;
  t->delsys = SYS_DVBT;
  t->inversion = inversion;

  center_frequency =  10 * get_u32(buf + 2);                                                         // center_frequency 32 bslbf, 10Hz steps
  switch(buf[6] >> 5) {                                                                              // bandwidth 3 bslbf
     case 0: t->bandwidth = 8000000; break;
     case 1: t->bandwidth = 7000000; break;
     case 2: t->bandwidth = 6000000; break;
     case 3: t->bandwidth = 5000000; break;
     default:
        info("undefined bandwidth value found.\n");
        t->bandwidth = 8000000;
     }
  t->priority = (buf[6] >> 4) & 0x1;                                                                 // priority 1 bslbf,               20140705: convert to bool.
  t->time_slicing = ((buf[6] >> 3) & 0x1) == 0;                                                      // Time_Slicing_indicator 1 bslbf  20140705: convert to bool.
  t->mpe_fec = ((buf[6] >> 2) & 0x1) == 0;                                                           // MPE-FEC_indicator 1 bslbf       20140705: convert to bool.
                                                                                                     // reserved_future_use 2 bslbf
  switch(buf[7] >> 6) {                                                                              // constellation 2 bslbf
     case 0: t->modulation = QPSK;   break;
     case 1: t->modulation = QAM_16; break;
     case 2: t->modulation = QAM_64; break;
     default:
        info("undefined modulation value found.\n");
        t->modulation = QAM_AUTO;
     }
  switch((buf[7] >> 3) & 0x7) {                                                                      // hierarchy_information 3 bslbf
     // what about alpha here?
     case 0: t->hierarchy = HIERARCHY_NONE; break; //non-hierarchical, native interleaver
     case 1: t->hierarchy = HIERARCHY_1;    break; //alpha = 1, native interleaver
     case 2: t->hierarchy = HIERARCHY_2;    break; //alpha = 2, native interleaver
     case 3: t->hierarchy = HIERARCHY_4;    break; //alpha = 4, native interleaver
     case 4: t->hierarchy = HIERARCHY_NONE; break; //non-hierarchical, in-depth interleaver
     case 5: t->hierarchy = HIERARCHY_1;    break; //alpha = 1, in-depth interleaver
     case 6: t->hierarchy = HIERARCHY_2;    break; //alpha = 2, in-depth interleaver
     case 7: t->hierarchy = HIERARCHY_4;    break; //alpha = 4, in-depth interleaver
     default:t->hierarchy = HIERARCHY_NONE;
     }        
  switch(buf[7] & 0x7) {                                                                             // code_rate-HP_stream 3 bslbf
     case 0: t->coderate = FEC_1_2; break;
     case 1: t->coderate = FEC_2_3; break;
     case 2: t->coderate = FEC_3_4; break;
     case 3: t->coderate = FEC_5_6; break;
     case 4: t->coderate = FEC_7_8; break;
     default:
             info("undefined coderate HP\n");
             t->coderate = FEC_AUTO;
     }
   switch((buf[8] >> 5) & 0x7) {                                                                     // code_rate-LP_stream 3 bslbf
      case 0: t->coderate_LP = FEC_1_2; break;
      case 1: t->coderate_LP = FEC_2_3; break;
      case 2: t->coderate_LP = FEC_3_4; break;
      case 3: t->coderate_LP = FEC_5_6; break;
      case 4: t->coderate_LP = FEC_7_8; break;
      default:
         info("undefined coderate LP\n");
         t->coderate_LP = FEC_AUTO;
      }
  if (t->hierarchy == HIERARCHY_NONE)
     t->coderate_LP = FEC_NONE;

  switch((buf[8] >> 3) & 0x3) {                                                                      // guard_interval 2 bslbf
     case 0: t->guard = GUARD_INTERVAL_1_32; break;
     case 1: t->guard = GUARD_INTERVAL_1_16; break;
     case 2: t->guard = GUARD_INTERVAL_1_8;  break;
     case 3: t->guard = GUARD_INTERVAL_1_4;  break;
     default:;
     }
  switch((buf[8] >> 1) & 0x3) {                                                                      // transmission_mode 2 bslbf
     case 0: t->transmission = TRANSMISSION_MODE_2K; break;
     case 1: t->transmission = TRANSMISSION_MODE_8K; break;
     case 2: t->transmission = TRANSMISSION_MODE_4K; break;
     default:
             info("undefined transmission mode\n");
             t->transmission = TRANSMISSION_MODE_AUTO;
     }
  t->other_frequency_flag = ((buf[8] & 0x01) != 0);                                                  // other_frequency_flag 1 bslbf
                                                                                                     // reserved_future_use 32 bslbf
  // ----------------------------------------------------------------------------
  if (center_frequency > 0) { // now: add center freq. 
     if (! t->other_frequency_flag)
        t->frequency = center_frequency;
     else {
        known = false;
        for(p = (t->frequencies)->first; p; p = p->next) {
           if (p->frequency == center_frequency) {
              known = true;
              break;
              }
           for(p1 = (p->transposers)->first; p1; p1 = p1->next) {
              if (p1->frequency == center_frequency) {
                 known = true;
                 break;
                 }
              }
           }           
        if (! known) {
           p = calloc(1, sizeof(*p));
           p->transposers = &(p->_transposers);
           NewList(p->transposers, "transposers");
           p->frequency = center_frequency;
           AddItem(t->frequencies, p);
           }
        } // end other_frequency_flag
     } // end if center_frequency > 0
  if ((t->frequency == 0) && (t->other_frequency_flag == 0)) {
     verbose("%s: center_freq = 0 && other_frequency_flag = 0 -> set other_frequency_flag = 1\n", __FUNCTION__);
     t->other_frequency_flag = 1;
     }

  verbose("          F%u B%u %s C%d D%d G%d T%d other_frequency=%d (%u)\n",
         freq_scale(t->frequency, 1e-3),
         freq_scale(t->bandwidth, 1e-6),
         (t->modulation == QPSK)?"QPSK":
         (t->modulation == QAM_16)?"M16":"M64",
         (t->coderate == FEC_1_2)?12:
         (t->coderate == FEC_2_3)?23:
         (t->coderate == FEC_3_4)?34:
         (t->coderate == FEC_5_6)?56:
         (t->coderate == FEC_7_8)?78:999,
         (t->coderate_LP == FEC_1_2)?12:
         (t->coderate_LP == FEC_2_3)?23:
         (t->coderate_LP == FEC_3_4)?34:
         (t->coderate_LP == FEC_5_6)?56:
         (t->coderate_LP == FEC_7_8)?78:999,
         (t->guard==GUARD_INTERVAL_1_32  )? 32:
         (t->guard==GUARD_INTERVAL_1_16  )? 16:
         (t->guard==GUARD_INTERVAL_1_8   )? 8:4,
         (t->transmission == TRANSMISSION_MODE_2K )?2:
         (t->transmission == TRANSMISSION_MODE_8K )?8:4,
         t->other_frequency_flag,t->other_frequency_flag?center_frequency:0
         );

  verbose("          %u frequencies\n", (t->frequencies)->count);
  for(p = (t->frequencies)->first; p; p = p->next) {
     verbose("             %u\n", p->frequency);
     for(p1 = (p->transposers)->first; p1; p1 = p1->next) {
        verbose("                transposer %u\n", p1->frequency);
        }
     }
} //end parse_terrestrial_delivery_system_descriptor


void parse_frequency_list_descriptor(const unsigned char * buf, struct transponder * t) {
  uint8_t i, coding_type = (buf[2] & 0x03);
  uint8_t num_frequencies = (buf[1] - 1) / 4;
  uint32_t f;
  bool known;
  struct frequency_item * p, * p1;

  if (t == NULL) return;
  hd(buf);
  buf += 3;

  for(i = 0; i < num_frequencies; i++) {
     switch(coding_type) {
        case 1:
           f = 10  * bcd32_to_cpu (buf[0], buf[1], buf[2], buf[3]);
           break;
        case 2:
           f = 100 * bcd32_to_cpu (buf[0], buf[1], buf[2], buf[3]);
           break;
        case 3:
           f = 10 * ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
           break;           
        default:
           f = 0;
        }
     buf += 4;
     if (f == 0) continue;

     known = false;
     for(p = (t->frequencies)->first; p; p = p->next) {
        if (p->frequency == f) {
           known = true;
           break;
           }
        for(p1 = (p->transposers)->first; p1; p1 = p1->next) {
           if (p1->frequency == f) {
              known = true;
              break;
              }
           }
        }           
     if (! known) {
        p = calloc(1, sizeof(*p));
        p->transposers = &(p->_transposers);
        NewList(p->transposers, "transposers");
        p->frequency = f;
        AddItem(t->frequencies, p);
        }
     } // end freq loop

  verbose("          %-.2u frequencies\n", (t->frequencies)->count);
  for(p = (t->frequencies)->first; p; p = p->next) {
     verbose("             %u\n", p->frequency);
     for(p1 = (p->transposers)->first; p1; p1 = p1->next) {
        verbose("                transposer %u\n", p1->frequency);
        }
     }
}


/* DVB-T2: PRELIMINARY && UNTESTED CODE ONLY. I NEED SOMEBODY WITH ACCESS TO
 *         DVB-T2. IF YOU WANT TO HELP PLS CONTACT ME. --20111204, wirbel--
 * 20140626:
 *      - if center_frequency = 0 and other_frequency_flag not set -> set this flag explictly.
 */
void parse_T2_delivery_system_descriptor(const unsigned char * buf,
                        struct transponder * t, fe_spectral_inversion_t inversion) {
  unsigned char * bp;
  __u8 descriptor_length;
  __u8 cell_id_extension;
  __u8 frequency_loop_length = 0;
  __u8 subcell_info_loop_length = 0;
  __u16 cell_id;
  __u32 center_frequency = 0, transposer_frequency;
  bool known = false;
  struct frequency_item * p, * p1;

  if (t == NULL) return;
  hd(buf);

  t->type = SCAN_TERRESTRIAL;
  t->source = 0x04;
  t->delsys = SYS_DVBT2;
  t->modulation = QAM_AUTO;
  t->inversion = inversion;
                                                                                                     // descriptor_tag               8 uimsbf
  descriptor_length = buf[1];                                                                        // descriptor_length            8 uimsbf
//descriptor_tag_extension = buf[2];                                                                 // descriptor_tag_extension     8 uimsbf
  t->plp_id = buf[3];                                                                                // plp_id                       8 uimsbf;  uniquely identifies the PLP carrying this TS within the T2 system.
  t->system_id = get_u16(buf + 4);                                                                   // T2_system_id                 16 uimsbf; uniquely identifies the T2 system within the network, two T2 systems with same T2_system_id && network_id ire identical, except that cell_id may differ..
  if ((t->extended_info = (descriptor_length > 4))) {                                                // has extension
     switch (buf[6] >> 6) {                                                                          // SISO/MISO 2 bslbf (Multiple-Input Single-Output)
        case 0: t->SISO_MISO = 0;       break;
        case 1: t->SISO_MISO = 1;       break;
        default:;
        }
     switch((buf[6] >> 2) & 0xF ) {                                                                  // bandwidth 4 bslbf
        case 0: t->bandwidth = 8000000;     break;
        case 1: t->bandwidth = 7000000;     break;
        case 2: t->bandwidth = 6000000;     break;
        case 3: t->bandwidth = 5000000;     break;
        case 4: t->bandwidth = 10000000;    break;
        case 5: t->bandwidth = 1712000;     break;
        default:t->bandwidth = 8000000;                                                              //       0110 to 1111 reserved for future use
        }
     //reserved_future_use = buf[6] & 0x3);                                                          // reserved_future_use 2 bslbf
     switch((buf[7] >> 5) & 0x7) {                                                                   // guard_interval 3 bslbf
        case 0: t->guard = GUARD_INTERVAL_1_32;   break;
        case 1: t->guard = GUARD_INTERVAL_1_16;   break;
        case 2: t->guard = GUARD_INTERVAL_1_8;    break;
        case 3: t->guard = GUARD_INTERVAL_1_4;    break;
        case 4: t->guard = GUARD_INTERVAL_1_128;  break;
        case 5: t->guard = GUARD_INTERVAL_19_128; break;
        case 6: t->guard = GUARD_INTERVAL_19_256; break;                                             //       111 reserved for future use
        default:t->guard = GUARD_INTERVAL_AUTO;                         
        }
     switch((buf[7] >> 2) & 0x7) {                                                                   // transmission_mode 3 bslbf
        case 0: t->transmission = TRANSMISSION_MODE_2K;  break;
        case 1: t->transmission = TRANSMISSION_MODE_8K;  break;
        case 2: t->transmission = TRANSMISSION_MODE_4K;  break;
        case 3: t->transmission = TRANSMISSION_MODE_1K;  break;
        case 4: t->transmission = TRANSMISSION_MODE_16K; break;
        case 5: t->transmission = TRANSMISSION_MODE_32K; break;
        default:t->transmission = TRANSMISSION_MODE_AUTO;                                            //       110 to 111 reserved for future use
        }
     t->other_frequency_flag = ((buf[7] >> 1) & 0x1) != 0;                                           // other_frequency_flag 1 bslbf; this TS is available on other frequencies.
     t->tfs_flag = (buf[7] & 0x1) != 0;                                                              // tfs_flag 1 bslbf, Bundling of more channels into a SuperMUX (called TFS)
     descriptor_length -= 6;                                                                         // so far, we read 6 bytes.
     bp = (unsigned char *) &buf[8];

     while(descriptor_length > 0) {                                                                  // for (i=0;i<N,i++) {
        cell_id = get_u16(bp); bp += 2; descriptor_length -= 2;                                      //      cell_id 16 uimsbf  
        if (t->tfs_flag) {                                                                           //      if (tfs_flag == 1) {
           // if tfs_flag (Time-Frequency Slicing) is set, we use 2..6 frequencies in parallel,      //
           // the TS is time interleaved && jumping from freq to freq. No idea, how this should      //
           // fit in future into linux dvb.                                                          //
           frequency_loop_length = *bp; bp++; descriptor_length--;                                   //          frequency_loop_length 8 uimsbf // 2 to 6 center freqs belonging to TFS arrangement
           while(frequency_loop_length > 3) {                                                        //          for (j=0;j<N;j++){
              bool known = false;
              struct frequency_item * p;
              center_frequency = 10 * get_u32(bp);                                                   //              centre_frequency 32 uimsbf     
              bp += 4; descriptor_length -= 4; frequency_loop_length -= 4;                           //

              for(p = (t->frequencies)->first; p; p = p->next) {
                 if (p->frequency == center_frequency) {
                    p->cell_id = cell_id;
                    known = true;
                    break;
                    }
                 }
              if (! known) {
                 p = calloc(1, sizeof(*p));
                 p->transposers = &(p->_transposers);
                 NewList(p->transposers, "transposers");
                 p->cell_id = cell_id;
                 p->frequency = center_frequency;
                 AddItem(t->frequencies, p);
                 }
              }                                                                                      //              }
           } // end tfs flag                                                                         //          }
        else {                                                                                       //      else { // no tfs_flag, just one center freq. the usual case.
           center_frequency = 10 * get_u32(bp);                                                      //          centre_frequency 32 uimsbf
           bp += 4; descriptor_length -= 4;                                                          //     

           if (center_frequency > 0) { // now: add center freq. 
              if (! t->other_frequency_flag)
                 t->frequency = center_frequency;
              else { // more than one center_freqs or transposers.
                 known = false;
                 for(p = (t->frequencies)->first; p; p = p->next) {
                    if (p->frequency == center_frequency) {
                       p->cell_id = cell_id;
                       known = true;
                       break;
                       }
                    for(p1 = (p->transposers)->first; p1; p1 = p1->next) {
                       if (p1->frequency == center_frequency) {
                          p->cell_id = cell_id;
                          known = true;
                          break;
                          }
                       }
                    }           
                 if (! known) {
                    p = calloc(1, sizeof(*p));
                    p->transposers = &(p->_transposers);
                    NewList(p->transposers, "transposers");
                    p->cell_id = cell_id;
                    p->frequency = center_frequency;
                    AddItem(t->frequencies, p);
                    }
                 }
              }
           }                                                                                         //          }
        subcell_info_loop_length = *bp; bp++; descriptor_length -= 1;                                //      subcell_info_loop_length 8 uimsbf

        for(p = (t->frequencies)->first; p; p = p->next) {
           if (cell_id == p->cell_id) break;
           }

        while(subcell_info_loop_length > 4) {                                                        //      for (k=0;k<N;k++){
           cell_id_extension = *bp;                                                                  //           cell_id_extension 8 uimsbf
           transposer_frequency = 10 * get_u32(bp + 1);                                              //           transposer_frequency 32 uimsbf
           bp += 5; descriptor_length -= 5; subcell_info_loop_length -= 5;                           //
           if (p == NULL) {
              p = calloc(1, sizeof(*p));
              p->transposers = &(p->_transposers);
              NewList(p->transposers, "transposers");
              p->cell_id = cell_id;
              p->frequency = center_frequency;
              AddItem(t->frequencies, p);              
              }
           known = false;
           for(p1 = (p->transposers)->first; p1; p1 = p1->next) {
              if (p1->frequency == transposer_frequency) {
                 p1->cell_id = cell_id_extension;
                 known = true;
                 break;
                 }
              }
           if (! known) {
              p1 = calloc(1, sizeof(*p1));
              p1->transposers = &(p1->_transposers);
              NewList(p1->transposers, "transposers");
              p1->cell_id = cell_id_extension;
              p1->frequency = transposer_frequency;
              AddItem(p->transposers, p1);
              }
           } // end subcell info loop                                                                //           }
        } // end while(descriptor_length > 0)                                                        //      }
     }
  if ((t->frequency == 0) && (t->other_frequency_flag == 0)) {
     verbose("%s: center_freq = 0 && other_frequency_flag = 0 -> set other_frequency_flag = 1\n", __FUNCTION__);
     t->other_frequency_flag = 1;
     }
  verbose("%s f%u system_id%u plp_id%u SISO/MISO=%s B%.1f G%d T%d other_frequency%d TFS%d\n",
         __FUNCTION__,
         freq_scale(t->frequency, 1e-3), t->system_id, t->plp_id,
         t->SISO_MISO?"MISO":"SISO",
         (t->bandwidth * 1e-6),
         (t->guard==GUARD_INTERVAL_1_32  )? 32:
         (t->guard==GUARD_INTERVAL_1_16  )? 16:
         (t->guard==GUARD_INTERVAL_1_8   )? 8:
         (t->guard==GUARD_INTERVAL_1_4   )? 4:
         (t->guard==GUARD_INTERVAL_1_128 )? 128:
         (t->guard==GUARD_INTERVAL_19_128)? 19128:19256,
         (t->transmission == TRANSMISSION_MODE_2K )?2:
         (t->transmission == TRANSMISSION_MODE_8K )?8:
         (t->transmission == TRANSMISSION_MODE_4K )?4:
         (t->transmission == TRANSMISSION_MODE_1K )?1:
         (t->transmission == TRANSMISSION_MODE_16K)?16:32,
         t->other_frequency_flag, t->tfs_flag);


  verbose("          %-.2u frequencies:\n", (t->frequencies)->count);
  for(p = (t->frequencies)->first; p; p = p->next) {
     verbose("             %u\n", p->frequency);
     for(p1 = (p->transposers)->first; p1; p1 = p1->next) {
        verbose("                transposer %u\n", p1->frequency);
        }
     }
}

void parse_logical_channel_descriptor(const unsigned char * buf, struct transponder * t) {
  if (t == NULL) return;
  hd(buf);

//uint8_t descriptor_tag    = buf[0];                              // descriptor_tag           8 uimsbf
  uint8_t descriptor_length = buf[1];                              // descriptor_length        8 uimsbf
  struct service * s;                                              //
  uint16_t service_id;                                             //
  int p = 2;                                                       //
                                                                   //
                                                                   //
  while(descriptor_length > 0) {                                   //
     service_id = (buf[p] << 8) | buf[p+1];                        // service_id              16 uimsbf
     s = find_service(t, service_id);                              //
     if (s == NULL)                                                //
        s = alloc_service(t, service_id);                          //
                                                                   //
     s->visible_service = (buf[p+2] & 0x80) > 0;                   // visible_service_flag     1 bslbf, reserved NorDig: 1bslbf: Australia: 5 bslbf
     s->logical_channel_number = (buf[p+2] & 0x3F) << 8 | buf[p+3];// logical_channel_number   NorDig: 14uimbsf; Australia: 10 uimsbf
     descriptor_length -= 4; p += 4;
     }
}


/* 300468 v011101 annex C, Conversion between time and date conventions
 * NOTE: These formulas are applicable between the inclusive dates 1900 March 1 to 2100 February 28.
 */

static  __u8 LeapYear(__u16 year) {
  if ((year % 400) == 0)
     return 1;
  else if ((year % 100) == 0)
     return 0;
  else if ((year % 4) == 0)
     return 1;
  return 0;
}

struct tm modified_julian_date_to_utc(__u32 MJD) {
  struct tm utc;
  __u8 mtab[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  int _Y = (int) (MJD - 15078.2) / 365.25;
  int _M = (int) (MJD - 14956.1 - (int) (_Y * 365.25)) / 30.6001;
  int K = (_M == 14) ? 1 : (_M == 15) ? 1 : 0;
       
  memset(&utc, 0, sizeof(struct tm));
  utc.tm_mday = MJD - 14956 - (int) (_Y * 365.25) - (int) (_M * 30.6001);
  utc.tm_year = _Y + K;
        
  if (LeapYear(utc.tm_year + 1900))
     mtab[1]=29;
        
  utc.tm_mon  = (_M - 1 - K * 12) - 1;
  //NOTE: 300468 = {mon=1..sun=7} => struct tm = {sun=0..sat=6}
  utc.tm_wday = ((MJD + 2) % 7 + 1) % 7;

  utc.tm_yday = utc.tm_mday;        
  for(K = 0; K < utc.tm_mon; K++)
     utc.tm_yday += mtab[K];
        
  return utc;
}

void parse_network_change_notify_descriptor(const unsigned char *buf, network_change_t *nc) {
  unsigned char * bp;
  int descriptor_length;
  int loop_length;
//__u8 descriptor_tag_extension;
  __u16 start_time_lsb;
  struct tm utc;
  time_t now;
  time_t utc_offset;
  changed_network_t * cn;
  network_change_loop_t * change;

  int v=verbosity; verbosity=5;
  hd(buf);
  verbosity=v;
  if (nc == NULL) return;

  /* calculate the time offset between local time and utc on this computer:
   * unfortunally there's no direct utc-time struct tm -> time_t conversion,
   * as mktime() works on local_time only.
   * I demand here, that any POSIX system handles negative time_t values correctly.
   * 20111211 --wirbel--
   */
  time(&now);
  utc_offset = difftime(mktime(localtime(&now)), mktime(gmtime(&now)));
                                                                            // descriptor_tag               8 uimsbf
  descriptor_length = buf[1];                                               // descriptor_length            8 uimsbf
//descriptor_tag_extension = buf[2];                                        // descriptor_tag_extension     8 uimsbf
  bp = (unsigned char *) &buf[3]; descriptor_length--;

  nc->num_networks = 0;
  nc->network = (changed_network_t *)
                calloc(descriptor_length / 15, sizeof(changed_network_t));

  while(descriptor_length > 0) {                                            //   for (i=0;i<N;i++) {
     cn = &nc->network[nc->num_networks];                                   //                              // next changed transponder
     nc->num_networks = nc->num_networks + 1;
     cn->ofdm_cell_id = get_u16(bp);                                        //       cell_id                16 uimsbf
     bp+=2; descriptor_length-=2;
     loop_length = *bp;                                                     //       loop_length            8 uimsbf
     bp++; descriptor_length--;
     cn->loop = (network_change_loop_t *)
                calloc(loop_length / 12, sizeof(network_change_loop_t));

     while(loop_length > 0) {                                               //       for (j=0;j<N;j++) {
        change = &cn->loop[cn->num_changes++];                              //           // next change on _this_ network
        change->network_change_id = *bp;                                    //           network_change_id          8 uimsbf
        bp++; descriptor_length--; loop_length--;
        change->network_change_version = *bp;                               //           network_change_version     8 uimsbf
        bp++; descriptor_length--; loop_length--;
        utc = modified_julian_date_to_utc(get_u16(bp));                     //           start_time_of_change       40 bslbf
        change->start_time_of_change = mktime(&utc) + utc_offset;           //
        bp+=2; descriptor_length-=2; loop_length-=2;
        start_time_lsb = bcdtime(bp);
        bp+=3; descriptor_length-=3; loop_length-=3;
        change->start_time_of_change += start_time_lsb;
        change->change_duration = bcdtime(bp);                              //           change_duration            24 uimsbf
        bp+=3; descriptor_length-=3; loop_length-=3;
        change->receiver_category = *bp >> 5;                               //           receiver_category          3 uimsbf
        change->invariant_ts.present = (*bp >> 4) & 0x1;                    //           invariant_ts_present       1 uimsbf
        change->change_type = *bp & 0xF;                                    //           change_type                4 uimsbf
        bp++; descriptor_length--; loop_length--;
        change->message_id = *bp;                                           //           message_id                 8 uimsbf
        bp++; descriptor_length--; loop_length--;
        if (change->invariant_ts.present == 1) {                            //           if (invariant_ts_present == ‘1’) {
           change->invariant_ts.tsid = get_u16(bp);                         //              invariant_ts_tsid       16 uimsbf
           bp+=2; descriptor_length-=2; loop_length-=2;
           change->invariant_ts.onid = get_u16(bp);                         //              invariant_ts_onid       16 uimsbf
           bp+=2; descriptor_length-=2; loop_length-=2;
           }                                                                //              }
        } // end: while (loop_length > 0) {                                 //           }
     } //end: while (descriptor_length > 0) {                               //   }
}


/* ATSC PSIP VCT */
void parse_atsc_service_location_descriptor(struct service *s,const unsigned char *buf) {
  struct ATSC_service_location_descriptor d = read_ATSC_service_location_descriptor(buf);
  int i;
  unsigned char *b = (unsigned char *) buf+5;

  s->pcr_pid = d.PCR_PID;
  for(i=0; i < d.number_elements; i++) {
     struct ATSC_service_location_element e = read_ATSC_service_location_element(b);
     switch(e.stream_type) {
        case iso_iec_13818_1_11172_2_video_stream:
           s->video_pid = e.elementary_PID;
           moreverbose("  VIDEO     : PID 0x%04x\n", e.elementary_PID);
           break;
        case atsc_a_52b_ac3:
           if (s->audio_num < AUDIO_CHAN_MAX) {
              s->audio_pid[s->audio_num] = e.elementary_PID;
              s->audio_lang[s->audio_num][0] = (e.ISO_639_language_code >> 16) & 0xff;
              s->audio_lang[s->audio_num][1] = (e.ISO_639_language_code >> 8)  & 0xff;
              s->audio_lang[s->audio_num][2] =  e.ISO_639_language_code        & 0xff;
              s->audio_num++;
              }
           moreverbose("\tAUDIO\t: PID 0x%04x lang: %s\n",e.elementary_PID,s->audio_lang[s->audio_num-1]);

           break;
        default:
           warning("unhandled stream_type: %X\n",e.stream_type);
           break;
        };
     b += 6;
  }
}

void parse_atsc_extended_channel_name_descriptor(struct service *s, const unsigned char *buf) {
  unsigned char *b = (unsigned char *) buf+2;
  int i,j;
  int num_str = b[0];

  #define uncompressed_string 0x00

  b++;
  for(i = 0; i < num_str; i++) {
     int num_seg = b[3];
     b += 4; /* skip lang code */
     for(j = 0; j < num_seg; j++) {
        int compression_type = b[0],/* mode = b[1],*/ num_bytes = b[2];

        switch (compression_type) {
           case uncompressed_string:
              if (s->service_name)
                 free(s->service_name);
              s->service_name = malloc(num_bytes * sizeof(char) + 1);
              memcpy(s->service_name,&b[3],num_bytes);
              s->service_name[num_bytes] = '\0';
              break;
           default:
              warning("compressed strings are not supported yet\n");
              break;
           }
        b += 3 + num_bytes;
        }
     }
}

const char * network_id_desc(uint16_t network_id) {
  switch(network_id) {
     case 0x0000           : return "reserved network id";
     case 0x0001 ... 0x2000: return "unique satellite network id"; 
     case 0x2001 ... 0x3000: return "unique terrestrial network id";
     case 0x3001 ... 0x3600: return "reusable terrestrial network id";
     case 0x3601 ... 0xA000: return "terrestrial network id (future use)";
     case 0xA001 ... 0xB000: return "reuseable cable network id";
     case 0xB001 ... 0xF000: return "cable network id (future use)";
     case 0xF001 ... 0xFEBF: return "unique cable network id";
     case 0xFEC0 ... 0xFF00: return "common interface betwork id";
     default:                return "private use network id";
     }
}




#ifdef DEVELOPER_VERSION

/* DVB-SH: radio for handhelds over DVB, expected to operate on frequencies around 2.2 GHz; consists of
 * satellite link with additional DVB-T link in regions without direct satellite view.
 * so far, do nothing with this code && try to understand
 * PRELIMINARY && UNTESTED CODE ONLY. --20111204, wirbel--
 */
typedef enum fe_sh_diversity_mode {
  DIVERSITY_OFF = 0,
  DIVERSITY_PATS = 8,
  DIVERSITY_PATS_FEC_LINK = 13,
  DIVERSITY_PATS_FEC_PHY = 14,
  DIVERSITY_PATS_FEC_PHY_LINK = 15,
  DIVERSITY_AUTO
} fe_sh_diversity_mode_t;

typedef enum fe_sh_modulation_type {
  MODULATION_TYPE_SH_B_TDM = 0, // SH-B, use TDM on the satellite link and OFDM on the terrestrial link
  MODULATION_TYPE_SH_A_OFDM = 1 // SH-A, use OFDM on both the satellite and terrestrial links
} fe_sh_modulation_type_t;

typedef enum fe_sh_interleaver {
  INTERLEAVER_OFF = 0,
  INTERLEAVER_ON = 1,
  INTERLEAVER_COMPLETE = 2,
  INTERLEAVER_SHORT = 3
} fe_sh_interleaver_t;

typedef enum fe_sh_tdm_symbolrate {
  TDM_34_5,
  TDM_32_5,
  TDM_29_5,
  TDM_62_9,
  TDM_56_9,
  TDM_52_9,
  TDM_116_17,
  TDM_108_17,
  TDM_100_17,
  TDM_224_33,
  TDM_208_33,
  TDM_64_11,
  TDM_119_20,
  TDM_28_5,
  TDM_203_40,
  TDM_217_36,
  TDM_49_9,
  TDM_91_18,
  TDM_203_34,
  TDM_189_34,
  TDM_175_34,
  TDM_196_33,
  TDM_182_33,
  TDM_56_11,
  TDM_51_10,
  TDM_24_5,
  TDM_87_20,
  TDM_31_6,
  TDM_14_3,
  TDM_13_3,
  TDM_87_17,
  TDM_81_17,
  TDM_75_17,
  TDM_52_11,
  TDM_48_11,
  TDM_17_4,
  TDM_4_1,
  TDM_29_8,
  TDM_155_36,
  TDM_35_9,
  TDM_65_18,
  TDM_145_34,
  TDM_135_34,
  TDM_125_34,
  TDM_140_33,
  TDM_130_33,
  TDM_40_11,
  TDM_34_25,
  TDM_32_25,
  TDM_29_25,
  TDM_62_45,
  TDM_56_45,
  TDM_52_45,
  TDM_116_85,
  TDM_108_85,
  TDM_20_17,
  TDM_224_165,
  TDM_208_165,
  TDM_64_55,
} fe_sh_tdm_symbolrate_t;

typedef struct {
  __u8                   modulation_type;
  fe_sh_interleaver_t    interleaver_presence;
  fe_sh_interleaver_t    interleaver_type;
  fe_polarization_t      polarization;
  fe_rolloff_t           roll_off;
  fe_modulation_t        modulation_mode;
  __u32                  code_rate;
  fe_sh_tdm_symbolrate_t symbol_rate;
  __u32                  bandwidth;
  __u8                   priority;
  __u8                   constellation_and_hierarchy;
  fe_hierarchy_t         hierarchy;
  fe_guard_interval_t    guard;
  fe_transmit_mode_t     transmission;
  __u8                   common_frequency;
} modulation_loop_t;

typedef enum fe_code_rate_ext {
   _FEC_NONE = FEC_NONE,
   _FEC_1_2  = FEC_1_2,
   _FEC_2_3  = FEC_2_3,
   _FEC_3_4  = FEC_3_4,
   _FEC_4_5  = FEC_4_5,
   _FEC_5_6  = FEC_5_6,
   _FEC_6_7  = FEC_6_7,
   _FEC_7_8  = FEC_7_8,
   _FEC_8_9  = FEC_8_9,
   _FEC_AUTO = FEC_AUTO,
   _FEC_3_5  = FEC_3_5,
   _FEC_9_10 = FEC_9_10,
  FEC_1_3,
  FEC_1_3_C,
  FEC_1_4,
  FEC_1_5,
  FEC_2_3_C,
  FEC_2_5,
  FEC_2_5_C,
  FEC_2_7,
  FEC_2_9,
} fe_code_rate_ext_t;

void parse_SH_delivery_system_descriptor (const unsigned char *buf,                
                  struct transponder *t, fe_spectral_inversion_t inversion) {
  unsigned char * bp;
  __u8 descriptor_length;
  __u8 descriptor_tag_extension;
  
  modulation_loop_t * modulation_loop;
  int n_modulations = 0;
  fe_sh_diversity_mode_t diversity_mode;
  
  hd(buf);
  if (t == NULL) return;

  // hm.., actually we're dealing with two different transponders && del_sys at the same time. what to do here?

  //t->type = SCAN_SATELLITE;                                                                        // FIXME
  t->source = 0x05;
  //q.delsys = SYS_DVBS;                                                                             // FIXME
  //t.inversion = inversion;

                                                                                                     // descriptor_tag 8 uimsbf
  descriptor_length = buf[1];                                                                        // descriptor_length 8 uimsbf
  descriptor_tag_extension = buf[2];                                                                 // descriptor_tag_extension 8 uimsbf
  switch(buf[3] >> 4) {                                                                              // diversity_mode 4 bslbf: paTS FEC_diversity FEC_@_phy FEC_@_link
     case 0:  diversity_mode = DIVERSITY_OFF; break;                                                 //       no  no  no  no
  // case 1..7:                                                                                      //       reserved for future use
     case 8:  diversity_mode = DIVERSITY_PATS; break;                                                //       yes no  no  no
  // case 9..12:                                                                                     //       reserved for future use
     case 13: diversity_mode = DIVERSITY_PATS_FEC_LINK; break;                                       //       yes yes no  yes
     case 14: diversity_mode = DIVERSITY_PATS_FEC_PHY; break;                                        //       yes yes yes no
     case 15: diversity_mode = DIVERSITY_PATS_FEC_PHY_LINK; break;                                   //       yes yes yes yes
     default: diversity_mode = DIVERSITY_AUTO;                                                       //
     }                                                                                               //
  //reserved = buf[3] & 0xF;                                                                         // reserved 4 bslbf

  descriptor_length -= 2; bp = (unsigned char *) &buf[4];
  modulation_loop = (modulation_loop_t *) calloc(1 + descriptor_length/4, sizeof(modulation_loop_t));
  while(descriptor_length > 0) {                                                                     //  for (i=0; i<N; i++){
     modulation_loop[n_modulations].modulation_type = (*bp >> 7) & 0x1;                              //      modulation_type 1 bslbf
     modulation_loop[n_modulations].interleaver_presence = (*bp >> 6) & 0x1;                         //      interleaver_presence 1 bslbf
     modulation_loop[n_modulations].interleaver_type = ((*bp >> 5) & 0x1) << 1;                      //      interleaver_type 1 bslbf
     bp++; descriptor_length--;                                                                      //      Reserved 5 bslbf
     if (modulation_loop[n_modulations].modulation_type == MODULATION_TYPE_SH_B_TDM) {               //      if (modulation_type == 0) {
        modulation_loop[n_modulations].polarization = (*bp >> 6) & 0x2;                              //         Polarization 2 bslbf
        switch((*bp >> 4) & 0x2) {                                                                   //         roll_off 2 bslbf
           case 0: modulation_loop[n_modulations].roll_off = ROLLOFF_35; break;                      //
           case 1: modulation_loop[n_modulations].roll_off = ROLLOFF_25; break;                      //
           case 2: modulation_loop[n_modulations].roll_off = ROLLOFF_15; break;                      //
         //case 3: reserved for future use                                                           //
           default:modulation_loop[n_modulations].roll_off = ROLLOFF_AUTO;                           //
           }                                                                                         //
        switch((*bp >> 2) & 0x2) {                                                                   //         modulation_mode 2 bslbf
           case 0: modulation_loop[n_modulations].modulation_mode = QPSK; break;                     //
           case 1: modulation_loop[n_modulations].modulation_mode = PSK_8; break;                    //
           case 2: modulation_loop[n_modulations].modulation_mode = APSK_16; break;                  //
         //case 3: reserved for future use                                                           //
           default:modulation_loop[n_modulations].modulation_mode = QPSK;                            //                     //NOTE: no valid default here.
           }                                                                                         //
        switch(((*bp & 0x2) << 2) | ((bp[1] >> 6) & 0x2)) {                                          //         code_rate 4 bslbf
           case 0: modulation_loop[n_modulations].code_rate = FEC_1_5;    break;                     //                     // 1/5 standard
           case 1: modulation_loop[n_modulations].code_rate = FEC_2_9;    break;                     //                     // 2/9 standard
           case 2: modulation_loop[n_modulations].code_rate = FEC_1_4;    break;                     //                     // 1/4 standard
           case 3: modulation_loop[n_modulations].code_rate = FEC_2_7;    break;                     //                     // 2/7 standard
           case 4: modulation_loop[n_modulations].code_rate = FEC_1_3;    break;                     //                     // 1/3 standard 
           case 5: modulation_loop[n_modulations].code_rate = FEC_1_3_C;  break;                     //                     // 1/3 complementary
           case 6: modulation_loop[n_modulations].code_rate = FEC_2_5;    break;                     //                     // 2/5 standard
           case 7: modulation_loop[n_modulations].code_rate = FEC_2_5_C;  break;                     //                     // 2/5 complementary 
           case 8: modulation_loop[n_modulations].code_rate = FEC_1_2;    break;                     //                     // 1/2 standard 
           case 9: modulation_loop[n_modulations].code_rate = FEC_1_3_C;  break;                     //                     // 1/3 complementary
           case 10: modulation_loop[n_modulations].code_rate = FEC_2_3;   break;                     //                     // 2/3 standard
           case 11: modulation_loop[n_modulations].code_rate = FEC_2_3_C; break;                     //                     // 2/3 complementary
         //case 12 ... 15: reserved for future use                                                   //
           default: modulation_loop[n_modulations].code_rate = FEC_AUTO;                             //        
           }                                                                                         //                                 
        bp++; descriptor_length--;                                                                   //  
        switch((*bp >> 1) & 0x1F) {                                                                  //         symbol_rate 5 bslbf
           case 0: modulation_loop[n_modulations].bandwidth = 8000000;                               //                     // 00000 8   1/4  34/5    32/5    29/5
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_4;                    //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_34_5; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_32_5; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_29_5; break;
                 }                                                                                   //
              break;                                                                                 //
           case 1: modulation_loop[n_modulations].bandwidth = 8000000;                               //                     // 00001 8   1/8  62/9    56/9    52/9
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_8;                    //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_62_9; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_56_9; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_52_9; break;
                 }                                                                                   //
              break;                                                                                 //
           case 2: modulation_loop[n_modulations].bandwidth = 8000000;                               //                     // 00010 8   1/16 116/17  108/17  100/17
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_16;                   //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_116_17; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_108_17; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_100_17; break;
                 }                                                                                   //
              break;                                                                                 //
           case 3: modulation_loop[n_modulations].bandwidth = 8000000;                               //                     // 00011 8   1/32 224/33  208/33  64/11
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_32;                   //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_224_33; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_208_33; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_64_11; break;
                 }                                                                                   //
              break;                                                                                 //        
           case 4: modulation_loop[n_modulations].bandwidth = 7000000;                               //                     // 00100 7   1/4  119/20  28/5    203/40
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_4;                    //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_119_20; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_28_5; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_203_40; break;
                 }                                                                                   //
              break;                                                                                 //
           case 5: modulation_loop[n_modulations].bandwidth = 7000000;                               //                     // 00101 7   1/8  217/36  49/9    91/18
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_8;                    //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_217_36; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_49_9; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_91_18; break;
                 }                                                                                   //
              break;                                                                                 //
           case 6: modulation_loop[n_modulations].bandwidth = 7000000;                               //                     // 00110 7   1/16 203/34  189/34  175/34
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_16;                   //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_203_34; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_189_34; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_175_34; break;
                 }                                                                                   //
              break;                                                                                 //
           case 7: modulation_loop[n_modulations].bandwidth = 7000000;                               //                     // 00111 7   1/32 196/33  182/33  56/11
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_32;                   //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_196_33; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_182_33; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_56_11; break;
                 }                                                                                   //
              break;                                                                                 //
           case 8: modulation_loop[n_modulations].bandwidth = 6000000;                               //                     // 01000 6   1/4  51/10   24/5    87/20
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_4;                    //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_51_10; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_24_5; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_87_20; break;
                 }                                                                                   // 
              break;                                                                                 //
           case 9: modulation_loop[n_modulations].bandwidth = 6000000;                               //                     // 01001 6   1/8  31/6    14/3    13/3
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_8;                    //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_31_6; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_14_3; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_13_3; break;
                 }                                                                                   //
              break;                                                                                 //
           case 10: modulation_loop[n_modulations].bandwidth = 6000000;                              //                     // 01010 6   1/16 87/17   81/17   75/17
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_16;                   //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_87_17; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_81_17; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_75_17; break;
                 }                                                                                   //
              break;                                                                                 //
           case 11: modulation_loop[n_modulations].bandwidth = 6000000;                              //                     // 01011 6   1/32 56/11   52/11   48/11
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_32;                   //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_56_11; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_52_11; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_48_11; break;
                 }                                                                                   //
              break;                                                                                 //
           case 12: modulation_loop[n_modulations].bandwidth = 5000000;                              //                     // 01100 5   1/4  17/4    4/1     29/8
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_4;                    //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_17_4; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_4_1; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_29_8; break;
                 }                                                                                   //
              break;                                                                                 //
           case 13: modulation_loop[n_modulations].bandwidth = 5000000;                              //                     // 01101 5   1/8  155/36  35/9    65/18
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_8;                    //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_155_36; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_35_9; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_65_18; break;
                 }                                                                                   //
              break;                                                                                 //
           case 14: modulation_loop[n_modulations].bandwidth = 5000000;                              //                     // 01110 5   1/16 145/34  135/34  125/34
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_16;                   //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_145_34; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_135_34; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_125_34; break;
                 }                                                                                   //
              break;                                                                                 //
           case 15: modulation_loop[n_modulations].bandwidth = 5000000;                              //                     // 01111 5   1/32 140/33  130/33  40/11
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_32;                   //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_140_33; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_130_33; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_40_11; break;
                 }                                                                                   //
              break;                                                                                 //
           case 16: modulation_loop[n_modulations].bandwidth = 1712000;                              //                     // 10000 1.7 1/4  34/25   32/25   29/25
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_4;                    //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_34_25; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_32_25; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_29_25; break;
                 }                                                                                   //
              break;                                                                                 //
           case 17: modulation_loop[n_modulations].bandwidth = 1712000;                              //                     // 10001 1.7 1/8  62/45   56/45   52/45
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_8;                    //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_62_45; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_56_45; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_52_45; break;
                 }                                                                                   //
              break;                                                                                 //
           case 18: modulation_loop[n_modulations].bandwidth = 1712000;                              //                     // 10010 1.7 1/16 116/85  108/85  20/17
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_16;                   //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_116_85; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_108_85; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_20_17; break;
                 }                                                                                   //
              break;                                                                                 //
           case 19: modulation_loop[n_modulations].bandwidth = 1712000;                              //                     // 10011 1.7 1/32 224/165 208/165 64/55
              modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_32;                   //
              switch(modulation_loop[n_modulations].roll_off) {                                      //
                 case ROLLOFF_15: modulation_loop[n_modulations].symbol_rate = TDM_224_165; break;
                 case ROLLOFF_25: modulation_loop[n_modulations].symbol_rate = TDM_208_165; break;
                 default:         modulation_loop[n_modulations].symbol_rate = TDM_64_55; break;
                 }                                                                                //
              break;                                                                              //
           default:;                                                                              //
           }                                                                                      //
      //Reserved = *buf & 0x1;                                                                    //         Reserved 1 bslbf
        bp++; descriptor_length--;                                                                //
        }                                                                                         //         }
     else {                                                                                       //      else {
        switch((*buf & 0xE0) >> 3) {                                                              //        bandwidth 3 bslbf
           case 0: modulation_loop[n_modulations].bandwidth = 8000000;                            //             // 000 8 MHz
           case 1: modulation_loop[n_modulations].bandwidth = 7000000;                            //             // 001 7 MHz
           case 2: modulation_loop[n_modulations].bandwidth = 6000000;                            //             // 010 6 MHz
           case 3: modulation_loop[n_modulations].bandwidth = 5000000;                            //             // 011 5 MHz
           case 4: modulation_loop[n_modulations].bandwidth = 1712000;                            //             // 100 1,7 MHz
           default:;                                                                              //
           }                                                                                      //
        modulation_loop[n_modulations].priority = (*buf & 0x10) >> 4;                             //        priority 1 bslbf
        switch((*buf & 0xE) >> 1) {                                                               //        constellation_and_hierarchy 3 bslbf
           case 0:                                                                                //
           case 1: modulation_loop[n_modulations].hierarchy = HIERACHY_NONE;                      //
              modulation_loop[n_modulations].priority = PRIORITY_NONE;                            //
              break;                                                                              //
           default:                                                                               //
              modulation_loop[n_modulations].hierachy = ((*buf & 0xE) >> 1) - 1;                  //
              modulation_loop[n_modulations].modulation = QAM16;                                  //
              if (modulation_loop[n_modulations].priority == 0) {                                 //
                 modulation_loop[n_modulations].priority = PRIORITY_LP;                           //
              else                                                                                //
                 modulation_loop[n_modulations].priority = PRIORITY_HP;                           //
           }                                                                                      //
        switch(((*bp & 0x1) << 3) | ((bp[1] >> 5) & 0x7)) {                                       //         code_rate 4 bslbf
           case 0: modulation_loop[n_modulations].code_rate = FEC_1_5;    break;                  //                     // 1/5 standard
           case 1: modulation_loop[n_modulations].code_rate = FEC_2_9;    break;                  //                     // 2/9 standard
           case 2: modulation_loop[n_modulations].code_rate = FEC_1_4;    break;                  //                     // 1/4 standard
           case 3: modulation_loop[n_modulations].code_rate = FEC_2_7;    break;                  //                     // 2/7 standard
           case 4: modulation_loop[n_modulations].code_rate = FEC_1_3;    break;                  //                     // 1/3 standard 
           case 5: modulation_loop[n_modulations].code_rate = FEC_1_3_C;  break;                  //                     // 1/3 complementary
           case 6: modulation_loop[n_modulations].code_rate = FEC_2_5;    break;                  //                     // 2/5 standard
           case 7: modulation_loop[n_modulations].code_rate = FEC_2_5_C;  break;                  //                     // 2/5 complementary 
           case 8: modulation_loop[n_modulations].code_rate = FEC_1_2;    break;                  //                     // 1/2 standard 
           case 9: modulation_loop[n_modulations].code_rate = FEC_1_3_C;  break;                  //                     // 1/3 complementary
           case 10: modulation_loop[n_modulations].code_rate = FEC_2_3;   break;                  //                     // 2/3 standard
           case 11: modulation_loop[n_modulations].code_rate = FEC_2_3_C; break;                  //                     // 2/3 complementary
         //case 12 ... 15: reserved for future use                                                //
           default: modulation_loop[n_modulations].code_rate = FEC_AUTO;                          //        
           }                                                                                      //
        bp++; descriptor_length--;                                                                //
        switch((*bp & 0x18) >> 3) {                                                               //        guard_interval 2 bslbf
           case 0: modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_32; break;             //                     // 00 1/32
           case 1: modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_16; break;             //                     // 01 1/16
           case 2: modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_8;  break;             //                     // 10 1/8
           case 3: modulation_loop[n_modulations].guard = GUARD_INTERVAL_1_4;  break;             //                     // 11 1/4
           default:;                                                                              //
           }                                                                                      // 
        switch((*bp & 0x6) >> 1) {                                                                //        transmission_mode 2 bslbf
           case 0: modulation_loop[n_modulations].transmission = TRANSMISSION_MODE_1K;            //                     // 00 1k
              break;                                                                              //
           case 1: modulation_loop[n_modulations].transmission = TRANSMISSION_MODE_2K;            //                     // 01 2k
              break;                                                                              //
           case 2: modulation_loop[n_modulations].transmission = TRANSMISSION_MODE_4K             //                     // 10 4k
              break;                                                                              //
           case 3: modulation_loop[n_modulations].transmission = TRANSMISSION_MODE_8K;            //                     // 11 8k
              break;                                                                              //
           default:;                                                                              //
           }                                                                                      // 
        modulation_loop[n_modulations].common_frequency = *bp & 0x1;                              //        common_frequency 1 bslbf
        bp++; descriptor_length--;                                                                //
        }                                                                                         //        }
     if (modulation_loop[n_modulations].interleaver_presence) {                                   //      if ((interleaver_presence == 1) {
        if (modulation_loop[n_modulations].interleaver_type == 0) {                               //         if (interleaver_type == 0) {
           modulation_loop[n_modulations].common_multiplier = (*bp >> 2);                         //            common_multiplier 6 uimsbf
           modulation_loop[n_modulations].nof_late_taps = ((*bp & 0x3) << 4) | (bp[1] >> 4);      //            nof_late_taps 6 uimsbf
           bp++; descriptor_length--;                                                             //
           modulation_loop[n_modulations].nof_slices = ((*bp & 0xF) << 2) | (bp[1] >> 6);         //            nof_slices 6 uimsbf
           bp++; descriptor_length--;                                                             //
           modulation_loop[n_modulations].slice_distance = ((*bp & 0x3F) << 2) | (bp[1] >> 6);    //            slice_distance 8 uimsbf
           bp++; descriptor_length--;                                                             //
           modulation_loop[n_modulations].non_late_increments = ((*bp & 0x3F);                    //            non_late_increments 6 uimsbf
           bp++; descriptor_length--;                                                             //
           }                                                                                      //            }
         else {                                                                                   //         else {
           modulation_loop[n_modulations].common_multiplier = (*bp >> 2);                         //            common_multiplier 6 uimsbf
         //reserved = (*bp & 0x3);                                                                //            reserved 2 uimsbf
           bp++; descriptor_length--;                                                             //
           }                                                                                      //            }
        }                                                                                         //         )
     }                                                                                            //      } // end loop
}
#else
//dummy.
void parse_SH_delivery_system_descriptor (const unsigned char *buf, 
         struct transponder *t, fe_spectral_inversion_t inversion){};
#endif

static __u32 crc_table[256];
static __u8  crc_initialized = 0;

int crc_check (const unsigned char * buf, __u16 len) {
  __u16 i, j;
  __u32 crc = 0xffffffff;
  __u32 transmitted_crc = buf[len-4] << 24 | buf[len-3] << 16 | buf[len-2] << 8 | buf[len-1];

  if (! crc_initialized) { // initialize crc lookup table before first use.
     __u32 accu;
     for(i = 0; i < 256; i++) {
        accu = ((__u32) i << 24);
        for(j = 0; j < 8; j++) {
           if (accu & 0x80000000L)
              accu = (accu << 1) ^ 0x04C11DB7L; // CRC32 Polynom
           else
              accu = (accu << 1);
           }
        crc_table[i] = accu;
        }
     crc_initialized = 1;
     }

  for(i = 0; i < len-4; i++)
     crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *buf++) & 0xFF];

  if (crc == transmitted_crc)
     return 1;
  else {
     warning("received garbage data: crc = 0x%08x; expected crc = 0x%08x\n", crc, transmitted_crc);
     return 0;
     }
}
