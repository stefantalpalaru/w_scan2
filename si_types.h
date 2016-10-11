/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006 - 2014 Winfried Koehler 
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

#ifndef __SI_TYPES_H
#define __SI_TYPES_H

#include <stdint.h>
#include "descriptors.h"
#include "tools.h"

/*******************************************************************************
/* section buffer
 ******************************************************************************/

#define SECTION_FLAG_DEFAULT  (1U) << 0
#define SECTION_FLAG_INITIAL  (1U) << 1
#define SECTION_FLAG_FREE     (1U) << 2
#define SECTION_BUF_SIZE      4096     

typedef struct section_buf {
  /*----------------------------*/
  void * prev;
  void * next;
  uint32_t index;
  /*----------------------------*/
  const char * dmx_devname;
  unsigned int run_once   : 1;
  unsigned int segmented  : 1;          // segmented by table_id_ext
  int fd;
  int pid;
  int table_id;
  int table_id_ext;
  int section_version_number;
  uint8_t section_done[32];
  int sectionfilter_done;
  unsigned char buf[SECTION_BUF_SIZE];
  uint32_t flags;
  time_t timeout;
  time_t start_time;
  time_t running_time;
  struct section_buf * next_seg;        // this is used to handle segmented tables (like NIT-other)
  pList  garbage;
} section_t, * p_section_t;

/*******************************************************************************
/* service type.
 ******************************************************************************/

#define AUDIO_CHAN_MAX    (32)
#define AC3_CHAN_MAX      (32)
#define CA_SYSTEM_ID_MAX  (16)
#define SUBTITLES_MAX     (32)

struct transponder;
struct service {
  /*----------------------------*/
  void *   prev;
  void *   next;
  uint32_t index;
  /*----------------------------*/
  struct transponder * transponder;
  uint16_t transport_stream_id;
  uint16_t service_id;
  char   * provider_name;
  char   * provider_short_name;
  char   * service_name;
  char   * service_short_name;
  uint16_t pmt_pid;
  uint16_t pcr_pid;
  uint16_t video_pid;
  uint8_t  video_stream_type;
  uint16_t audio_pid[AUDIO_CHAN_MAX];
  uint8_t  audio_stream_type[AUDIO_CHAN_MAX];
  char     audio_lang[AUDIO_CHAN_MAX][4];
  int      audio_num;
  uint16_t ca_id[CA_SYSTEM_ID_MAX];
  int      ca_num;
  uint16_t teletext_pid;
  uint16_t subtitling_pid[SUBTITLES_MAX];
  char     subtitling_lang[SUBTITLES_MAX][4];
  uint8_t  subtitling_type[SUBTITLES_MAX];
  uint16_t composition_page_id[SUBTITLES_MAX];
  uint16_t ancillary_page_id[SUBTITLES_MAX];
  int      subtitling_num;
  uint16_t ac3_pid[AC3_CHAN_MAX];
  uint8_t  ac3_stream_type[AC3_CHAN_MAX];
  char     ac3_lang[AC3_CHAN_MAX][4];
  int      ac3_num;
  unsigned int type : 8;
  bool     scrambled;
  bool     visible_service;
  uint32_t logical_channel_number;
  uint8_t  running;
  void   * priv;
} service_t, * p_service_t;

/*******************************************************************************
/* transponder type.
 ******************************************************************************/

struct frequency_item {
  /*----------------------------*/
  void *   prev;
  void *   next;
  uint32_t index;
  /*----------------------------*/
  pList transposers;
  cList _transposers;
  uint16_t cell_id;
  uint32_t frequency;
};

struct transponder {
  /*----------------------------*/
  void *   prev;                 
  void *   next;                 
  uint32_t index;                 
  pList  services;                 
  cList _services;
  pList frequencies;  /* DVB-T/T2 */
  cList _frequencies;                 
  /*----------------------------- starting from here copied by 'copy_fe_params' ------------------------------------------*/
  /* NOTE: 'frequency' needs to be first item - dont touch!                                                               */
  uint32_t frequency;                                        /* unit Hz, except satellite: kHz                      1..4  */
  fe_spectral_inversion_t    inversion                   :8; /*                                                     5     */
  uint32_t                   symbolrate;                     /* symbols per second                                  6..9  */
  uint32_t                   bandwidth;                      /* Hz                                                  10..13*/
  uint16_t                   orbital_position;               /* 0x0000-0x1800                                       14..15*/
  uint8_t                    input_stream_identifier;        /* ISI 8bit.                                           16    */
  fe_delivery_system_t       delsys                      :6; /*                               0..18 -> 5 -> 6 (63)  17    */
  fe_polarization_t          polarization                :2; /* H,V,L,R                       0..3  -> 2 -> 2 (3)   17    */
  fe_modulation_t            modulation                  :6; /*                               0..13 -> 4 -> 6 (63)  18    */
  fe_pilot_t                 pilot                       :2; /* pilot, on, off, auto          0..2  -> 2 -> 2 (3)   18    */
  fe_code_rate_t             coderate                    :7; /* code rate                     0..12 -> 4 -> 7 (127) 19    */
  fe_code_rate_t             coderate_LP                 :7; /* code rate low priority stream 0..12 -> 4 -> 7 (127) 19..20*/
  fe_guard_interval_t        guard                       :7; /*                               0..10 -> 4 -> 7 (127) 20..21*/
  fe_rolloff_t               rolloff                     :3; /* 0.35, 0.25, 0.20, auto        0..3  -> 2 -> 3 (7)   21    */
  fe_transmit_mode_t         transmission                :7; /*                               0..8  -> 4 -> 7 (127) 22    */
  fe_west_east_flag_t        west_east_flag              :1; /* east, west                    0..1  -> 1 -> 1 (1)   22    */
  fe_hierarchy_t             hierarchy                   :4; /*                               0..4  -> 3 -> 4 (15)  23    */
  unsigned                   time_slicing                :2; /* only defined in w_scan.       0..2  -> 2 -> 2 (3)   23    */
  unsigned                   scrambling_sequence_index   :18;/* not yet in use.                                     23..25*/
  unsigned                   scrambling_sequence_selector:1; /* on/off                        0..1  -> 1            26    */
  unsigned                   multiple_input_stream_flag  :1; /* on/off                        0..1  -> 1            26    */
  fe_frequency_type_t        C2_tuning_frequency_type    :2; /* 0..2                          0..2  -> 2 -> 2 (3)   26    */
  fe_ofdm_symbol_duration_t  active_OFDM_symbol_duration :2; /*                                                     26    */
  fe_alpha_t                 alpha                       :2; /* only defined in w_scan                              26    */
  fe_interleave_t            terr_interleaver            :2; /* only defined in w_scan                              27    */
  unsigned                   priority                    :1; /* only defined in w_scan                              27    */
  unsigned                   mpe_fec                     :1; /*                                                     27    */
  unsigned                   extended_info               :1; /*                                                     27    */
  unsigned                   SISO_MISO                   :1; /* 1 = multiple input single output                    27    */
  unsigned                   locks_with_params           :1; /* do we get lock with current tp params?              27    */
  unsigned                   reserved_byte27             :1; /* align to byte.                                      27    */
  unsigned                   tfs_flag                    :1; /* 1 if Time-Frequency-Slicing                         28    */
  unsigned                   other_frequency_flag        :1; /* DVB-T/T2                                            28    */
  unsigned                   last_tuning_failed          :1; /*                                                     28    */
  unsigned                   type                        :5; /* 3bit + 2bit byte_align                              28    */
  uint32_t                   source;                         /*                                                     29..32*/
  uint16_t                   system_id;                      /* DVB-C2, DVB-T2 system_id                            33..34*/
  uint8_t                    plp_id;                         /* DVB-C2, DVB-T2                                      35    */
  uint8_t                    data_slice_id;                  /* DVB-C2                                              36    */
  /*---------------------------- below is not copied by 'copy_fe_params' -------------------------------------------------*/
  uint8_t private_from_here; 
  /*----------------------------*/
  uint16_t network_PID;                   // which PID contains NIT ? (0x0010..0x1FFE)
  uint16_t network_id;
  uint16_t original_network_id;
  uint16_t transport_stream_id;
  /*----------------------------*/
  char * network_name;
  network_change_t network_change;
} __attribute__((packed))  transponder_t, * p_transponder_t;

/*******************************************************************************
/* satellite channel routing type.
 ******************************************************************************/

struct scr {
  uint16_t user_frequency;                // 20140101: DVB-S/S2, satellite channel routing
  uint8_t  slot;                          // 50494: 0..7  , 50607: 0..31
  uint8_t  pos;                           // 50494: 0..1  , 50607: 0..0xCC
  uint16_t pin;                           // 50494: 0..255, 50607: 0..255
  int8_t   offset;                        // 50494: -2..+2, 50607: 0
  uint8_t  norm;                          // 50494: 1     , 50607: 2
};

#endif
