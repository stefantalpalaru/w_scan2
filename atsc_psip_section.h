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
 * 20080815: file added by mkrufky
 */


#ifndef __ATSC_PSIP_SECTION_H_
#define __ATSC_PSIP_SECTION_H_

#include "section.h"

enum _atsc_service_type {
   atsc_analog_television          = 0x01,
   atsc_digital_television         = 0x02,
   atsc_radio                      = 0x03,
   atsc_data                       = 0x04,
};

struct ATSC_extended_channel_name_descriptor {
      u8  descriptor_tag            : 8;
      u8  descriptor_length         : 8;
      u8  TODO                      : 1;
} PACKED;
struct ATSC_extended_channel_name_descriptor read_ATSC_extended_channel_name_descriptor(const u8 *);

struct ATSC_service_location_descriptor {
      u8  descriptor_tag            : 8;
      u8  descriptor_length         : 8;
      u8  reserved                  : 3;
      u16 PCR_PID                   :13;
      u8  number_elements           : 8;
} PACKED;
struct ATSC_service_location_descriptor read_ATSC_service_location_descriptor(const u8 *);

struct ATSC_service_location_element {
      u8  stream_type               : 8;
      u8  reserved                  : 3;
      u16 elementary_PID            :13;
      u32 ISO_639_language_code     :24;
} PACKED;
struct ATSC_service_location_element read_ATSC_service_location_element(const u8 *);

struct tvct_channel {
      u16 short_name0               :16;
      u16 short_name1               :16;
      u16 short_name2               :16;
      u16 short_name3               :16;
      u16 short_name4               :16;
      u16 short_name5               :16;
      u16 short_name6               :16;
      u8  reserved0                 : 4;
      u16 major_channel_number      :10;
      u16 minor_channel_number      :10;
      u8  modulation_mode           : 8;
      u32 carrier_frequency         :32;
      u16 channel_TSID              :16;
      u16 program_number            :16;
      u8  ETM_location              : 2;
      u8  access_controlled         : 1;
      u8  hidden                    : 1;
      u8  reserved1                 : 2;
      u8  hide_guide                : 1;
      u8  reserved2_hi              : 1;
      u8  reserved2_mid             : 1;
      u8  reserved2_lo              : 1;
      u8  service_type              : 6;
      u16 source_id                 :16;
      u8  reserved3                 : 6;
      u16 descriptors_length        :10;
} PACKED;
struct tvct_channel read_tvct_channel(const u8 *);

#endif
