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

/* 20110702 --wk */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scan.h"
#include "extended_frontend.h"
#include "dump-vdr.h"
#include "satellites.h"

/******************************************************************************
 * an orbital position occurs in two spellings: "S19.2E" is what VDR
 * writes, since cSource::ToString() always prints the fraction, and
 * "S19E2" is what w_scan2 uses internally.  Older w_scan2 versions
 * wrote a bare "S13E" for integral positions, so that is read too.
 *
 *****************************************************************************/

static bool
sat_direction_valid(char direction)
{
    return (direction == 'E') || (direction == 'W');
}

static bool
sat_position_valid(unsigned int degrees, unsigned int tenths)
{
    // half the equator either way, and 180 degrees exactly at the ends
    return (degrees < 180) || ((degrees == 180) && (tenths == 0));
}

static bool
parse_sat_position(char const *satname, unsigned int *degrees, unsigned int *tenths, char *direction)
{
    int end;

    if (satname == NULL)
        return false;

    // the whole name has to be consumed: "S19.2EX" is not a position
    end = 0;
    *tenths = 0;
    if ((sscanf(satname, "S%u.%1u%c%n", degrees, tenths, direction, &end) == 3) && (satname[end] == '\0') &&
        sat_direction_valid(*direction) && sat_position_valid(*degrees, *tenths))
        return true;

    end = 0;
    *tenths = 0;
    if ((sscanf(satname, "S%u%c%1u%n", degrees, direction, tenths, &end) == 3) && (satname[end] == '\0') &&
        sat_direction_valid(*direction) && sat_position_valid(*degrees, *tenths))
        return true;

    end = 0;
    *tenths = 0;
    return (sscanf(satname, "S%u%c%n", degrees, direction, &end) == 2) && (satname[end] == '\0') &&
        sat_direction_valid(*direction) && sat_position_valid(*degrees, *tenths);
}

/******************************************************************************
 * translate w_scan2 satellite short names into VDR source identifiers.
 *
 *****************************************************************************/

static char const *
short_name_to_vdr_name(char const *satname)
{
    static char buf[16];
    unsigned int degrees, tenths;
    char direction;

    if (!parse_sat_position(satname, &degrees, &tenths, &direction))
        return satname; // fallback.
    snprintf(buf, sizeof(buf), "S%u.%u%c", degrees, tenths, direction);
    return buf;
}

/******************************************************************************
 * translate VDR satellite identifiers into short names used by w_scan2.
 *
 *****************************************************************************/

char const *
vdr_name_to_short_name(char const *satname)
{
    static char buf[16];
    unsigned int degrees, tenths;
    char direction;

    if (!parse_sat_position(satname, &degrees, &tenths, &direction))
        return "unknown satellite"; // fallback.
    snprintf(buf, sizeof(buf), "S%u%c%u", degrees, direction, tenths);
    return buf;
}

/******************************************************************************
 * translate linuxtv inversions to VDR inversion identifiers.
 *
 *****************************************************************************/

char const *
vdr_inversion_name(int inversion)
{
    switch (inversion) {
    case INVERSION_OFF:
        return "0";
    case INVERSION_ON:
        return "1";
    default:
        return "999";
    }
}

/******************************************************************************
 * translate linuxtv forw err corr to VDR fec identifiers.
 *
 *****************************************************************************/

char const *
vdr_fec_name(int fec)
{
    switch (fec) {
    case FEC_NONE:
        return "0";
    case FEC_1_2:
        return "12";
    case FEC_2_3:
        return "23";
    case FEC_3_4:
        return "34";
    case FEC_4_5:
        return "45";
    case FEC_5_6:
        return "56";
    case FEC_6_7:
        return "67";
    case FEC_7_8:
        return "78";
    case FEC_8_9:
        return "89";
    case FEC_3_5:
        return "35";
    case FEC_9_10:
        return "910";
    default:
        return "999";
    }
}

/******************************************************************************
 * translate linuxtv modulation types to VDR > 1.7.0 identifiers.
 *
 *****************************************************************************/

char const *
vdr_modulation_name(int modulation)
{
    switch (modulation) {
    case QAM_16:
        return "16";
    case QAM_32:
        return "32";
    case QAM_64:
        return "64";
    case QAM_128:
        return "128";
    case QAM_256:
        return "256";
    case QAM_512:
        return "512";
    case QAM_1024:
        return "1024";
    case QAM_4096:
        return "4096";
    case QAM_AUTO:
        return "998";
    case QPSK:
        return "2";
    case PSK_8:
        return "5";
    case APSK_16:
        return "6";
    case APSK_32:
        return "7";
    case VSB_8:
        return "10";
    case VSB_16:
        return "11";
    case DQPSK:
        return "12";
    default:
        return "999";
    }
}

/******************************************************************************
 * translate linuxtv bandwidth values to VDR identifiers.
 *
 *****************************************************************************/

char const *
vdr_bandwidth_name(int bandwidth)
{
    switch (bandwidth) {
    case 8000000:
        return "8";
    case 7000000:
        return "7";
    case 6000000:
        return "6";
    case 5000000:
        return "5";
    case 10000000:
        return "10";
    case 1712000:
        return "1712";
    default:
        return "999";
    }
}

/******************************************************************************
 * translate linuxtv tm values to VDR identifiers.
 *
 *****************************************************************************/

char const *
vdr_transmission_mode_name(int transmission_mode)
{
    switch (transmission_mode) {
    case TRANSMISSION_MODE_2K:
        return "2";
    case TRANSMISSION_MODE_8K:
        return "8";
    case TRANSMISSION_MODE_4K:
        return "4";
    case TRANSMISSION_MODE_1K:
        return "1";
    case TRANSMISSION_MODE_16K:
        return "16";
    case TRANSMISSION_MODE_32K:
        return "32";
    default:
        return "999";
    }
}

/******************************************************************************
 * translate linuxtv guard values to VDR identifiers.
 *
 *****************************************************************************/

char const *
vdr_guard_name(int guard_interval)
{
    switch (guard_interval) {
    case GUARD_INTERVAL_1_32:
        return "32";
    case GUARD_INTERVAL_1_16:
        return "16";
    case GUARD_INTERVAL_1_8:
        return "8";
    case GUARD_INTERVAL_1_4:
        return "4";
    case GUARD_INTERVAL_1_128:
        return "128";
    case GUARD_INTERVAL_19_128:
        return "19128";
    case GUARD_INTERVAL_19_256:
        return "19256";
    default:
        return "999";
    }
}

/******************************************************************************
 * translate linuxtv hierarchy values to VDR identifiers.
 * what about alpha? hm..
 *****************************************************************************/

char const *
vdr_hierarchy_name(int hierarchy)
{
    switch (hierarchy) {
    case HIERARCHY_NONE:
        return "0";
    case HIERARCHY_1:
        return "1";
    case HIERARCHY_2:
        return "2";
    case HIERARCHY_4:
        return "4";
    default:
        return "999";
    }
}

/******************************************************************************
 * translate linuxtv rolloffs values to VDR identifiers.
 *
 *****************************************************************************/

char const *
vdr_rolloff_name(int rolloff)
{
    switch (rolloff) {
    case ROLLOFF_20:
        return "20";
    case ROLLOFF_25:
        return "25";
    default:
        return "35";
    }
}

/******************************************************************************
 * translate linuxtv delivery_system values to VDR identifiers.
 *
 *****************************************************************************/

char const *
vdr_delsys_name(int delsys)
{
    switch (delsys) {
    case SYS_DVBT:
    case SYS_DVBS:
        return "0";
    case SYS_DVBS2:
    case SYS_DVBT2:
        return "1";
    default:
        return "0";
    }
}

/******************************************************************************
 * print "frequency:<params>:symbolrate:" to 'f' in vdr >= 1.7.4 format
 * NOTE: 1.7.0 .. 1.7.3 not supported here.
 *****************************************************************************/
#define vdrprint(fd, Param, Default, ID, Value) if (Param != Default) fprintf(fd,"%s%s", ID, Value)

void
dump_param_vdr(FILE *f, struct transponder *t, struct w_scan_flags *flags)
{

    switch (flags->scantype) {
    case SCAN_TERRCABLE_ATSC:
        fprintf(f, ":%i:", t->frequency / 1000);
        fprintf(f, "M%s:A:", vdr_modulation_name(t->modulation));
        fprintf(f, "%i:", t->symbolrate / 1000);
        break;

    case SCAN_CABLE:
        fprintf(f, ":%i:", t->frequency / 1000);
        fprintf(f, "M%s:C:", vdr_modulation_name(t->modulation));
        fprintf(f, "%i:", t->symbolrate / 1000);
        break;

    case SCAN_TERRESTRIAL:
        fprintf(f, ":%i:", t->frequency / 1000);
        vdrprint(f, t->bandwidth, 0, "B", vdr_bandwidth_name(t->bandwidth));
        vdrprint(f, t->coderate, FEC_AUTO, "C", vdr_fec_name(t->coderate));
        vdrprint(f, t->coderate_LP, FEC_AUTO, "D", vdr_fec_name(t->coderate_LP));
        vdrprint(f, t->guard, GUARD_INTERVAL_AUTO, "G", vdr_guard_name(t->guard));
        vdrprint(f, t->inversion, INVERSION_AUTO, "I", vdr_inversion_name(t->inversion));
        vdrprint(f, t->modulation, QAM_AUTO, "M", vdr_modulation_name(t->modulation));
        vdrprint(f, t->delsys, SYS_DVBT, "S", vdr_delsys_name(t->delsys));
        vdrprint(f, t->transmission, TRANSMISSION_MODE_AUTO, "T", vdr_transmission_mode_name(t->transmission));
        vdrprint(f, t->hierarchy, HIERARCHY_AUTO, "Y", vdr_hierarchy_name(t->hierarchy));
        if (t->delsys == SYS_DVBT2)
            fprintf(f, "P%u", t->plp_id);
        fprintf(f, ":T:27500:");
        break;

    case SCAN_SATELLITE:
        fprintf(f, ":%i:", t->frequency / 1000);
        switch (t->polarization) {
        case POLARIZATION_HORIZONTAL:
            fprintf(f, "h");
            break;
        case POLARIZATION_VERTICAL:
            fprintf(f, "v");
            break;
        case POLARIZATION_CIRCULAR_LEFT:
            fprintf(f, "l");
            break;
        case POLARIZATION_CIRCULAR_RIGHT:
            fprintf(f, "r");
            break;
        default:
            fatal("Unknown Polarization %d\n", t->polarization);
        }
        fprintf(f, "C%s", vdr_fec_name(t->coderate));
        switch (t->delsys) {
        case SYS_DVBS2:
            fprintf(f, "M%sO%sS1:", vdr_modulation_name(t->modulation), vdr_rolloff_name(t->rolloff));
            break;
        default:
            /* DVB-S always r = 0.35 according to specs
             * but vdr specifies 'O0' here (should be O35),
             * modulation is fix QPSK = 'M2'.
             */
            fprintf(f, "M2O0S0:");
        }

        fprintf(f, "%s:", short_name_to_vdr_name(satellite_to_short_name(flags->list_id)));
        fprintf(f, "%i:", t->symbolrate / 1000);
        break;

    default:;
    };
}

/******************************************************************************
 * print complete vdr channels.conf line from service params.
 *
 *****************************************************************************/

void
vdr_dump_service_parameter_set(FILE *f, struct service *s, struct transponder *t, struct w_scan_flags *flags)
{
    int i;

    if (!flags->ca_select && s->scrambled)
        return;
    fprintf(f, "%s", s->service_name);

    if (flags->dump_provider)
        fprintf(f, ";%s", s->provider_name);

    dump_param_vdr(f, t, flags);

    fprintf(f, "%i", s->video_pid);

    if (s->video_pid && (s->pcr_pid != s->video_pid))
        fprintf(f, "+%i", s->pcr_pid);
    if (s->video_stream_type)
        fprintf(f, "=%u", s->video_stream_type);

    fprintf(f, ":");

    fprintf(f, "%i", s->audio_pid[0]);
    if (s->audio_lang[0][0])
        fprintf(f, "=%.4s", s->audio_lang[0]);
    if (s->audio_stream_type[0])
        fprintf(f, "@%u", s->audio_stream_type[0]);
    for (i = 1; i < s->audio_num; i++) {
        fprintf(f, ",%i", s->audio_pid[i]);
        if (s->audio_lang[i][0])
            fprintf(f, "=%.4s", s->audio_lang[i]);
        if (s->audio_stream_type[i])
            fprintf(f, "@%u", s->audio_stream_type[i]);
    }

    if (s->ac3_num) {
        fprintf(f, "%s", ";");
        for (i = 0; i < s->ac3_num; i++) {
            if (i > 0)
                fprintf(f, "%s", ",");
            fprintf(f, "%i", s->ac3_pid[i]);
            if (s->ac3_lang[i][0])
                fprintf(f, "=%.4s", s->ac3_lang[i]);
        }
    }

    fprintf(f, ":%d", s->teletext_pid);

    // add subtitling here
    if (s->subtitling_num) {
        fprintf(f, "%s", ";");
        for (i = 0; i < s->subtitling_num; i++) {
            if (i > 0)
                fprintf(f, "%s", ",");
            fprintf(f, "%i", s->subtitling_pid[i]);
            if (s->subtitling_lang[i][0])
                fprintf(f, "=%.4s", s->subtitling_lang[i]);
        }
    }

    fprintf(f, ":%X", s->ca_id[0]);
    for (i = 1; i < s->ca_num; i++) {
        if (s->ca_id[i] == 0)
            continue;
        fprintf(f, ",%X", s->ca_id[i]);
    }

    fprintf(f, ":%d:%d:%d:0", s->service_id, (t->transport_stream_id > 0) ? t->original_network_id : 0, t->transport_stream_id);

    if (flags->print_pmt) {
        fprintf(f, ":%d", s->pmt_pid);
    }

    fprintf(f, "%s", "\n");
}
