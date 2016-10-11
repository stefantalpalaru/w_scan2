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

#include "atsc_psip_section.h"

struct ATSC_extended_channel_name_descriptor read_ATSC_extended_channel_name_descriptor(const u8 *b)
{
   struct ATSC_extended_channel_name_descriptor v;
   v.descriptor_tag            = getBits(b,  0, 8);
   v.descriptor_length         = getBits(b,  8, 8);
   v.TODO                      = getBits(b, 16, 1);
   return v;
}

struct ATSC_service_location_descriptor read_ATSC_service_location_descriptor(const u8 *b)
{
   struct ATSC_service_location_descriptor v;
   v.descriptor_tag            = getBits(b,  0, 8);
   v.descriptor_length         = getBits(b,  8, 8);
   v.reserved                  = getBits(b, 16, 3);
   v.PCR_PID                   = getBits(b, 19,13);
   v.number_elements           = getBits(b, 32, 8);
   return v;
}

struct ATSC_service_location_element read_ATSC_service_location_element(const u8 *b)
{
   struct ATSC_service_location_element v;
   v.stream_type               = getBits(b,  0, 8);
   v.reserved                  = getBits(b,  8, 3);
   v.elementary_PID            = getBits(b, 11,13);
   v.ISO_639_language_code     = getBits(b, 24,24);
   return v;
}

/* a_65_2009.pdf, Table 6.4 Bit Stream Syntax for the Terrestrial Virtual Channel Table */
struct tvct_channel read_tvct_channel(const u8 *b)
{
   struct tvct_channel v;
   v.short_name0               = getBits(b,  0,16);
   v.short_name1               = getBits(b, 16,16);
   v.short_name2               = getBits(b, 32,16);
   v.short_name3               = getBits(b, 48,16);
   v.short_name4               = getBits(b, 64,16);
   v.short_name5               = getBits(b, 80,16);
   v.short_name6               = getBits(b, 96,16);
   v.reserved0                 = getBits(b,112, 4);
   v.major_channel_number      = getBits(b,116,10);
   v.minor_channel_number      = getBits(b,126,10);
   v.modulation_mode           = getBits(b,136, 8);
   v.carrier_frequency         = getBits(b,144,32);
   v.channel_TSID              = getBits(b,176,16);
   v.program_number            = getBits(b,192,16);
   v.ETM_location              = getBits(b,208, 2);
   v.access_controlled         = getBits(b,210, 1);
   v.hidden                    = getBits(b,211, 1);
   v.reserved1                 = getBits(b,212, 2);
   v.hide_guide                = getBits(b,214, 1);
   v.reserved2_hi              = getBits(b,215, 1);
   v.reserved2_mid             = getBits(b,216, 1);
   v.reserved2_lo              = getBits(b,217, 1);
   v.service_type              = getBits(b,218, 6);
   v.source_id                 = getBits(b,224,16);
   v.reserved3                 = getBits(b,240, 6);
   v.descriptors_length        = getBits(b,246,10);
   return v;
}
