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
 *
 * added 20090303 -wk-
 */

#ifndef __DESCRIPTORS_H__
#define __DESCRIPTORS_H__

#include "extended_frontend.h"

/******************************************************************************
 * descriptor ids as defined by standards.
 *
 *****************************************************************************/
enum _descriptors {
        MHP_application_descriptor              = 0x00,
        MHP_application_name_desriptor          = 0x01,
        MHP_transport_protocol_descriptor       = 0x02,
        dvb_j_application_descriptor            = 0x03,
        dvb_j_application_location_descriptor   = 0x04,
        atsc_ac3_registration_descriptor        = 0x05, // see a_52b.pdf A3.3
        ca_descriptor                           = 0x09,
        iso_639_language_descriptor             = 0x0A,
        application_icons_descriptor            = 0x0B,
        carousel_identifier_descriptor          = 0x13,
        network_name_descriptor                 = 0x40,
        service_list_descriptor                 = 0x41,
        stuffing_descriptor                     = 0x42,
        satellite_delivery_system_descriptor    = 0x43,
        cable_delivery_system_descriptor        = 0x44,
        vbi_data_descriptor                     = 0x45,
        vbi_teletext_descriptor                 = 0x46,
        bouquet_name_descriptor                 = 0x47,
        service_descriptor                      = 0x48,
        country_availability_descriptor         = 0x49,
        linkage_descriptor                      = 0x4A,
        nvod_reference_descriptor               = 0x4B,
        time_shifted_service_descriptor         = 0x4C,
        short_event_descriptor                  = 0x4D,
        extended_event_descriptor               = 0x4E,
        time_shifted_event_descriptor           = 0x4F,
        component_descriptor                    = 0x50,
        mosaic_descriptor                       = 0x51,
        stream_identifier_descriptor            = 0x52,
        ca_identifier_descriptor                = 0x53,
        content_descriptor                      = 0x54,
        parental_rating_descriptor              = 0x55,
        teletext_descriptor                     = 0x56,
        telephone_descriptor                    = 0x57,
        local_time_offset_descriptor            = 0x58,
        subtitling_descriptor                   = 0x59,
        terrestrial_delivery_system_descriptor  = 0x5A,
        multilingual_network_name_descriptor    = 0x5B,
        multilingual_bouquet_name_descriptor    = 0x5C,
        multilingual_service_name_descriptor    = 0x5D,
        multilingual_component_descriptor       = 0x5E,
        private_data_specifier_descriptor       = 0x5F,
        service_move_descriptor                 = 0x60,
        short_smoothing_buffer_descriptor       = 0x61,
        frequency_list_descriptor               = 0x62,
        partial_transport_stream_descriptor     = 0x63,
        data_broadcast_descriptor               = 0x64,
        scrambling_descriptor                   = 0x65,
        data_broadcast_id_descriptor            = 0x66,
        transport_stream_descriptor             = 0x67,
        dsng_descriptor                         = 0x68,
        pdc_descriptor                          = 0x69,
        ac3_descriptor                          = 0x6A, // a_52b.pdf 4.1, PES stream_id 0xBD (private_stream_1); descriptor ETSI TS 102366 chapter A3;
        ancillary_data_descriptor               = 0x6B,
        cell_list_descriptor                    = 0x6C,
        cell_frequency_link_descriptor          = 0x6D,
        announcement_support_descriptor         = 0x6E,
        application_signalling_descriptor       = 0x6F,
        service_identifier_descriptor           = 0x71,
        service_availability_descriptor         = 0x72,
        default_authority_descriptor            = 0x73,
        related_content_descriptor              = 0x74,
        tva_id_descriptor                       = 0x75,
        content_identifier_descriptor           = 0x76,
        time_slice_fec_identifier_descriptor    = 0x77,
        ecm_repetition_rate_descriptor          = 0x78,
        s2_satellite_delivery_system_descriptor = 0x79,
        enhanced_ac3_descriptor                 = 0x7A,
        dts_descriptor                          = 0x7B,
        aac_descriptor                          = 0x7C,
        extension_descriptor                    = 0x7F,
        atsc_ac3_audio_stream_descriptor        = 0x81, // a_52b.pdf 3.2 PES stream_id 0xBD (private_stream_1); descriptor ETSI TS 102366 chapter A2;
        logical_channel_descriptor              = 0x83, // http://mumudvb.braice.net/mumudrupal/sites/default/files/Specifiche_LCN.pdf
        preferred_name_list_descriptor          = 0x84,
        preferred_name_identifier_descriptor    = 0x85,
        EICTA_stream_identifier_descriptor      = 0x86,
        atsc_A_53B_enhanced_AC3_audio_stream    = 0x87,
        atsc_extended_channel_name_descriptor   = 0xA0,
        atsc_service_location_descriptor        = 0xA1,
};

// 300468v011101p, 6.3 Extended descriptor identification and location
// NOTE: Only found in Partial Transport Streams.
enum _extended_descriptors {
        image_icon_descriptor                   = 0x00,
        cpcm_delivery_signalling_descriptor     = 0x01,
        CP_descriptor                           = 0x02,
        CP_identifier_descriptor                = 0x03,
        T2_delivery_system_descriptor           = 0x04,
        SH_delivery_system_descriptor           = 0x05,
        supplementary_audio_descriptor          = 0x06,
        network_change_notify_descriptor        = 0x07,
        message_descriptor                      = 0x08,
        target_region_descriptor                = 0x09,
        target_region_name_descriptor           = 0x0A,
        service_relocated_descriptor            = 0x0B,
        XAIT_PID_descriptor_descriptor          = 0x0C,
        C2_delivery_system_descriptor           = 0x0D,
//      reserved_for_future_use                 = 0x0E ... 0x0F,
        video_depth_range_descriptor            = 0x10,
        T2MI_descriptor                         = 0x11,
//      reserved_for_future_use                 = 0x12 ... 0x7F,
//      user_defined                            = 0x80 ... 0xFF,
};

/******************************************************************************
 * stream types as defined by standards.
 *
 *****************************************************************************/
enum _stream_type {
        iso_iec_11172_video_stream              = 0x01,
        iso_iec_13818_1_11172_2_video_stream    = 0x02,
        iso_iec_11172_audio_stream              = 0x03,        
        iso_iec_13818_3_audio_stream            = 0x04,
        iso_iec_13818_1_private_sections        = 0x05,
        iso_iec_13818_1_private_data            = 0x06, // ac-3 @ dvb
        iso_iec_13522_MHEG                      = 0x07,
        iso_iec_13818_1_Annex_A_DSM_CC          = 0x08,
        iso_iec_13818_1_11172_1_auxiliary       = 0x09,
        iso_iec_13818_6_type_a_multiproto_encaps= 0x0A,
        iso_iec_13818_6_type_b                  = 0x0B,
        iso_iec_13818_6_type_c                  = 0x0C,
        iso_iec_13818_6_type_d                  = 0x0D,
        iso_iec_13818_1_auxiliary               = 0x0E,
        iso_iec_13818_7_audio_w_ADTS_transp     = 0x0F,
        iso_iec_14496_2_visual                  = 0x10,
        iso_iec_14496_3_audio_w_LATM_transp     = 0x11,
        iso_iec_14496_1_packet_stream_in_PES    = 0x12,
        iso_iec_14496_1_packet_stream_in_14996  = 0x13,
        iso_iec_13818_6_synced_download_protocol= 0x14,
        metadata_in_PES                         = 0x15,
        metadata_in_metadata_sections           = 0x16,
        metadata_in_iso_iec_13818_6_data_carous = 0x17,
        metadata_in_iso_iec_13818_6_obj_carous  = 0x18,
        metadata_in_iso_iec_13818_6_synced_dl   = 0x19,
        iso_iec_13818_11_IPMP_stream            = 0x1A,
        iso_iec_14496_10_AVC_video_stream       = 0x1B,
        iso_iec_23008_2_H265_video_hevc_stream  = 0x24,
        atsc_a_52b_ac3                          = 0x81, // ac-3 @ atsc
};

/******************************************************************************
 * table ids as defined by standards.
 *
 *****************************************************************************/
enum table_id {
        TABLE_PAT                               = 0x00, // program_association_section
        TABLE_CAT                               = 0x01, // conditional_access_section
        TABLE_PMT                               = 0x02, // program_map_section
        TABLE_TSDT                              = 0x03, // transport_stream_description_section
        TABLE_NIT_ACT                           = 0x40, // network_information_section - actual_network
        TABLE_NIT_OTH                           = 0x41, // network_information_section - other_network
        TABLE_SDT_ACT                           = 0x42, // service_description_section - actual_transport_stream
        TABLE_SDT_OTH                           = 0x46, // service_description_section - other_transport_stream
        TABLE_BAT                               = 0x4A, // bouquet_association_section
        TABLE_EIT_ACT                           = 0x4E, // event_information_section - actual_transport_stream, present/following
        TABLE_EIT_OTH                           = 0x4F, // event_information_section - other_transport_stream, present/following
        TABLE_EIT_SCHEDULE_ACT_50               = 0x50, // 0x50 to 0x5F event_information_section - actual_transport_stream, schedule
        TABLE_EIT_SCHEDULE_ACT_5F               = 0x5F, //
        TABLE_EIT_SCHEDULE_OTH_60               = 0x60, // 0x60 to 0x6F event_information_section - other_transport_stream, schedule
        TABLE_EIT_SCHEDULE_OTH_6F               = 0x6F, //
        TABLE_TDT                               = 0x70, // time_date_section
        TABLE_RST                               = 0x71, // running_status_section
        TABLE_STUFFING                          = 0x72, // stuffing_section
        TABLE_TOT                               = 0x73, // time_offset_section
        TABLE_AIT                               = 0x74, // application information section (TS 102 812 [17])
        TABLE_CST                               = 0x75, // container section (TS 102 323 [15])
        TABLE_RCT                               = 0x76, // related content section (TS 102 323 [15])
        TABLE_CIT                               = 0x77, // content identifier section (TS 102 323 [15])
        TABLE_MPE_FEC                           = 0x78, //
        TABLE_RNS                               = 0x79, // resolution notification section (TS 102 323 [15])
        TABLE_DIT                               = 0x7E, // discontinuity_information_section
        TABLE_SIT                               = 0x7F, // selection_information_section
        TABLE_PREMIERE_CIT                      = 0xA0, // premiere content information section
        TABLE_VCT_TERR                          = 0xC8, // ATSC VCT VSB (terr)
        TABLE_VCT_CABLE                         = 0xC9, // ATSC VCT QAM (cable)
};

/******************************************************************************
 * PIDs as defined for accessing tables.
 *
 *****************************************************************************/
enum pid_type {
        PID_PAT                                 = 0x0000,
        PID_CAT                                 = 0x0001,
        PID_TSDT                                = 0x0002,
        PID_NIT_ST                              = 0x0010,
        PID_SDT_BAT_ST                          = 0x0011,
        PID_EIT_ST_CIT                          = 0x0012,
        PID_RST_ST                              = 0x0013,
        PID_TDT_TOT_ST                          = 0x0014,
        PID_RNT                                 = 0x0016,
        PID_DIT                                 = 0x001E,
        PID_SIT                                 = 0x001F,
        PID_VCT                                 = 0x1FFB,
};

/******************************************************************************
 * 300468 v181 5.2.3 Service Description Table (SDT) table 6
 *
 *****************************************************************************/
enum running_mode {
        rm_undefined                            = 0x00,
        rm_not_running                          = 0x01,
        rm_starts_soon                          = 0x02,
        rm_pausing                              = 0x03,
        rm_running                              = 0x04,
        rm_service_off_air                      = 0x05,
        rm_reserved_06                          = 0x06,
        rm_reserved_07                          = 0x07,
};

/******************************************************************************
 * 300468 A.1 Control codes (character control functions)
 *
 *****************************************************************************/
enum __single_byte_control_codes {
        sb_cc_reserved_80                       = 0x80,
        sb_cc_reserved_81                       = 0x81,
        sb_cc_reserved_82                       = 0x82,
        sb_cc_reserved_83                       = 0x83,
        sb_cc_reserved_84                       = 0x84,
        sb_cc_reserved_85                       = 0x85,
        character_emphasis_on                   = 0x86,
        character_emphasis_off                  = 0x87,
        sb_cc_reserved_88                       = 0x88,
        sb_cc_reserved_89                       = 0x89,
        character_cr_lf                         = 0x8A,
        sb_cc_user_8B                           = 0x8B,
        sb_cc_user_9F                           = 0x9F,
};

enum __utf8_control_codes {
        utf8_cc_start                           = 0xC2,
        utf8_cc_reserved_C280                   = 0xC280,
        utf8_cc_reserved_C281                   = 0xC281,
        utf8_cc_reserved_C282                   = 0xC282,
        utf8_cc_reserved_C283                   = 0xC283,
        utf8_cc_reserved_C284                   = 0xC284,
        utf8_cc_reserved_C285                   = 0xC285,
        utf8_character_emphasis_on              = 0xC286,
        utf8_character_emphasis_off             = 0xC287,
        utf8_cc_reserved_C288                   = 0xC288,
        utf8_cc_reserved_C289                   = 0xC289,
        utf8_character_cr_lf                    = 0xC28A,
        utf8_cc_user_C28B                       = 0xC28B,
        utf8_cc_user_C29F                       = 0xC29F,
};

/******************************************************************************
 * returns minimum repetition rates as specified in ETR211 4.4.1 and 4.4.2
 * and 13818-1 C.9 Bandwidth Utilization and Signal Acquisition Time
 * return value is in seconds.
 *****************************************************************************/


struct service;
struct transponder;

typedef struct {
        __u8 cell_id_extension;
        __u32 transposer_frequency;
} subcell_t;

typedef struct {
        __u8 network_change_id;
        __u8 network_change_version;
        time_t start_time_of_change;
        time_t change_duration;
        __u8 receiver_category;
        __u8 change_type;
        __u8 message_id;
        struct {
           __u8 present;
           __u16 tsid;
           __u16 onid;
           } invariant_ts;
} network_change_loop_t;

typedef struct {
        __u16 ofdm_cell_id;
        __u8 num_changes;
        network_change_loop_t * loop;
} changed_network_t;

typedef struct {
        __u16 num_networks;
        changed_network_t * network;
} network_change_t;

int repetition_rate(scantype_t scan_type, enum table_id table);
void parse_service_descriptor (const unsigned char *buf, struct service *s, unsigned user_charset_id);
void parse_ca_identifier_descriptor (const unsigned char *buf, struct service *s);
void parse_ca_descriptor (const unsigned char *buf, struct service *s);
void parse_iso639_language_descriptor (const unsigned char *buf, struct service *s);
void parse_subtitling_descriptor (const unsigned char *buf, struct service *s);
void parse_network_name_descriptor (const unsigned char *buf, struct transponder *t);
void parse_frequency_list_descriptor (const unsigned char *buf, struct transponder *t);
void parse_cable_delivery_system_descriptor (const unsigned char *buf,
                        struct transponder *t, fe_spectral_inversion_t inversion);
void parse_C2_delivery_system_descriptor (const unsigned char *buf,
                        struct transponder *t, fe_spectral_inversion_t inversion);
void parse_S2_satellite_delivery_system_descriptor(const unsigned char *buf, void * dummy);
void parse_satellite_delivery_system_descriptor(const unsigned char *buf,
                        struct transponder *t, fe_spectral_inversion_t inversion);
void parse_terrestrial_delivery_system_descriptor (const unsigned char *buf,
                        struct transponder *t, fe_spectral_inversion_t inversion);
void parse_T2_delivery_system_descriptor (const unsigned char *buf,
                        struct transponder *t, fe_spectral_inversion_t inversion);
void parse_SH_delivery_system_descriptor (const unsigned char *buf,                
                        struct transponder *t, fe_spectral_inversion_t inversion);
void parse_network_change_notify_descriptor(const unsigned char *buf, network_change_t *nc);
void parse_logical_channel_descriptor(const unsigned char * buf, struct transponder * t);
void parse_atsc_service_location_descriptor(struct service *s, const unsigned char *buf);
void parse_atsc_extended_channel_name_descriptor(struct service *s, const unsigned char *buf);

const char * network_id_desc(uint16_t network_id);

int crc_check(const unsigned char * buf, __u16 len);

#endif
