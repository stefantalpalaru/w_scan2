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
#include "scan.h"
#include "extended_frontend.h"
#include "dump-vdr.h"
#include "satellites.h"

struct cTr {
    char const *sat_name;
    char const *vdr_sat_name;
};

static struct cTr translations[] = {
    { "S180E0", "S180E" }, // S180E   Intelsat 18                     //
    { "S172E0", "S172E" }, // S172E   Eutelsat 172A                   //
    { "S169E0", "S169E" }, // S169E   Intelsat 8                      //
    { "S166E0", "S166E" }, // S166E   Intelsat 19                     //
                           // S164E   Optus B3                        //
    { "S162E0", "S162E" }, // S162E   Superbird B2                    //
    { "S160E0", "S160E" }, // S160E   Optus D1                        //
    { "S156E0", "S156E" }, // S156E   Optus C1/D3                     //
    { "S154E0", "S154E" }, // S154E   JCSAT 2A                        //
    { "S152E0", "S152E" }, // S152E   Optus D2                        //
                           // S150E   JCSAT 1B                        //
    { "S144E0", "S144E" }, // S144E   Superbird C2                    //
    { "S140E0", "S140E" }, // S140E   Express AM3                     //
    { "S138E0", "S138E" }, // S138E   Telstar 18                      //
    { "S134E0", "S134E" }, // S134E   Apstar 6                        //
    { "S132E0", "S132E" }, // S132E   Vinasat 1 & JCSAT 5A            //
    { "S128E0", "S128E" }, // S128E   JCSAT 3A                        //
    { "S125E0", "S125E" }, // S125E   ChinaSat 6A                     //
    { "S124E0", "S124E" }, // S124E   JCSAT 4B                        //
    { "S122E2", "S122.2E" }, // S122.2E Asiasat 4                       //
                             // S119.5E Thaicom 4                       //
    { "S118E0", "S118E" }, // S118E   Telkom 2                        //
    { "S116E0", "S116E" }, // S116E   ABS 7 & Koreasat 6              //
    { "S115E5", "S115.5E" }, // S115.5E ChinaSat 6B                     //
    { "S113E0", "S113E" }, // S113E   Palapa D & Koreasat 5           //
    { "S110E5", "S110.5E" }, // S110.5E ChinaSat 10                     //
    { "S110E0", "S110E" }, // S110E   N-Sat 110 & BSAT 3A/3C          //
    { "S108E2", "S108.2E" }, // S108.2E Telkom 1 & NSS 11 & SES 7       //
    { "S105E5", "S105.5E" }, // S105.5E Asiasat 3S                      //
    { "S103E0", "S103E" }, // S103E   Express A2                      //
    { "S100E5", "S100.5E" }, // S100.5E Asiasat 5                       //
    { "S96E5", "S96.5E" }, // S96.5E  Express AM33                    //
    { "S95E0", "S95E" }, // S95E    NSS 6                           //
    { "S93E5", "S93.5E" }, // S93.5E  Insat 3A/4B                     //
                           // S92.2E  ChinaSat 9                      //
    { "S91E5", "S91.5E" }, // S91.5E  Measat 3/3A                     //
    { "S90E0", "S90E" }, // S90E    Yamal 201/300K                  //
    { "S88E0", "S88E" }, // S88E    ST 2                            //
    { "S87E5", "S87.5E" }, // S87.5E  ChinaSat 12                     //
    { "S86E5", "S86.5E" }, // S86.5E                                  //
    { "S85E0", "S85.2E" }, // S85.2E  Intelsat 15 & Horizons 2        //
    { "S83E0", "S83E" }, // S83E    Insat 4A                        //
                         // S80E    Express AM2                     //
    { "S78E5", "S78.5E" }, // S78.5E  Thaicom 5/6A                    //
    { "S76E5", "S76.5E" }, // S76.5E  Apstar 7                        //
    { "S75E0", "S75E" }, // S75E    ABS 1A                          //
                         // S74E    Insat 3C/4CR                    //
                         // S72E    Intelsat 22                     //
    { "S70E5", "S70.5E" }, // S70.5E  Eutelsat 70B                    //
    { "S68E5", "S68.5E" }, // S68.5E  Intelsat 7/10                   //
    { "S66E0", "S66E" }, // S66E    Intelsat 17                     //
    { "S64E2", "S64E" }, // S64E    Intelsat 906                    //
    { "S62E0", "S62E" }, // S62E    Intelsat 902                    //
    { "S60E0", "S60E" }, // S60E    Intelsat 904                    //
    { "S57E0", "S57E" }, // S57E    NSS 12                          //
    { "S56E0", "S56E" }, // S56E    DirecTV 1R                      //
    { "S53E0", "S53E" }, // S53E    Express AM22                    //
    { "S52E5", "S52.5E" }, // S52.5E  Yahsat 1A                       //
    { "S49E0", "S49E" }, // S49E    Yamal 202                       //
                         // S47.5E  Intelsat 10                     //
                         // S46E    Azerspace-1                     //
    { "S45E0", "S45E" }, // S45E    Intelsat 12                     //
    { "S42E0", "S42E" }, // S42E    Turksat 2A/3A                   //
    { "S40E0", "S40E" }, // S40E    Express AM1                     //
    { "S39E0", "S39E" }, // S39E    Hellas Sat 2                    //
    { "S38E0", "S38E" }, // S38E    Paksat 1R                       //
    { "S36E0", "S36E" }, // S36E    Eutelsat 36A/36B                //
    { "S33E0", "S33E" }, // S33E    Eutelsat 33A & Intelsat 28      //
    { "S31E5", "S31.5E" }, // S31.5E  Astra 1G                        //
    { "S30E5", "S30.5E" }, // S30.5E  Arabsat 5A                      //
    { "S28E2", "S28.2E" }, // S28.2E  Astra 1N/2A/2F                  //
    { "S26E0", "S26E" }, // S26E    Badr C/3/4/5/6                  //
    { "S25E5", "S25.5E" }, // S25.5E  Eutelsat 25B                    //
    { "S23E5", "S23.5E" }, // S23.5E  Astra 3B                        //
    { "S21E6", "S21.6E" }, // S21.6E  Eutelsat 21B                    //
    { "S20E0", "S20E" }, // S20E    Arabsat 5C                      //
    { "S19E2", "S19.2E" }, // S19.2E  Astra 1KR/1L/1M/2C              //
                           // S17E    Amos 5                          //
    { "S16E0", "S16E" }, // S16E    Eutelsat 16A/16B                //
    { "S13E0", "S13E" }, // S13E    Eutelsat Hot Bird 13B/13C/13D   //
    { "S10E0", "S10E" }, // S10E    Eutelsat 10A                    //
    { "S9E0", "S9E" }, // S9E     Eutelsat 9A/Ka-Sat 9A           //
    { "S7E0", "S7E" }, // S7E     Eutelsat 7A                     //
    { "S4E8", "S4.8E" }, // S4.8E   Astra 4A & SES 5                //
                         // S4E     Eutelsat 4B                     //
    { "S3E0", "S3E" }, // S3E     Eutelsat 3A/3D & Rascom 1R      //
    { "S0W8", "S1W" }, // S1W     Thor 3/5 & Intelsat 10-02       //
    { "S4W0", "S4W" }, // S4W     Amos 1/2/3                      //
    { "S5W0", "S5W" }, // S5W     Eutelsat 5 West A               //
    { "S7W0", "S7W" }, // S7W     Nilesat 101/201 & Eutelsat 7W A //
    { "S8W0", "S8W" }, // S8W     Eutelsat 8 West A/C             //
    { "S11W0", "S11W" }, // S11W    Express AM44                    //
    { "S12W5", "S12.5W" }, // S12.5W  Eutelsat 12 West A              //
    { "S14W0", "S14W" }, // S14W    Express A4                      //
    { "S15W0", "S15W" }, // S15W    Telstar 12                      //
    { "S18W0", "S18W" }, // S18W    Intelsat 901                    //
    { "S20W0", "S20W" }, // S20W    NSS 7                           //
    { "S22W0", "S22W" }, // S22W    SES 4                           //
    { "S24W5", "S24.5W" }, // S24.5W  Intelsat 905                    //
    { "S27W5", "S27.5W" }, // S27.5W  Intelsat 907                    //
    { "S30W0", "S30W" }, // S30W    Hispasat 1D/1E                  //
    { "S31W5", "S31.5W" }, // S31.5W  Intelsat 25                     //
    { "S34W5", "S34.5W" }, // S34.5W  Intelsat 903                    //
    { "S37W5", "S37.5W" }, // S37.5W  NSS 10 & Telstar 11N            //
    { "S40W5", "S40.5W" }, // S40.5W  SES 6                           //
    { "S43W0", "S43W" }, // S43W    Intelsat 11                     //
    { "S45W0", "S45W" }, // S45W    Intelsat 14                     //
    { "S50W0", "S50W" }, // S50W    Intelsat 1R                     //
    { "S53W0", "S53W" }, // S53W    Intelsat 23                     //
    { "S55W5", "S55.5W" }, // S55.5W  Intelsat 805                    //
    { "S58W0", "S58W" }, // S58W    Intelsat 21                     //
                         // S61W    Amazonas 2/3                    //
                         // S61.5W  Echostar 16                     //
    { "S63W0", "S63W" }, // S63W    Telstar 14R                     //
    { "S65W0", "S65W" }, // S65W    Star One C1                     //
                         // S67W    AMC 4                           //
    { "S70W0", "S70W" }, // S70W    Star One C2                     //
    { "S72W0", "S72W" }, // S72W    AMC 6                           //
                         // S72.7W  Nimiq 5                         //
                         // S75W    Star One C3                     //
                         // S77W    QuetzSat 1                      //
    { "S78W0", "S78W" }, // S78W                                    //
                         // S82W    Nimiq 4                         //
    { "S83W0", "S83W" }, // S83W    AMC 9                           //
    { "S84W0", "S84W" }, // S84W    Brasilsat B4                    //
    { "S85W0", "S85W" }, // S85W    AMC 16                          //
                         // S85.1W  XM 3                            //
    { "S87W0", "S87W" }, // S87W    SES 2                           //
    { "S89W0", "S89W" }, // S89W    Galaxy 28                       //
    { "S91W0", "S91W" }, // S91W    Galaxy 17 & Nimiq 6             //
    { "S93W1", "S93.1W" }, // S93.1W  Galaxy 25                       //
    { "S95W0", "S95W" }, // S95W    Galaxy 3C                       //
    { "S97W0", "S97W" }, // S97W    Galaxy 19                       //
    { "S99W2", "S99W2" }, // S99.2W  Galaxy 16                       //
    { "S101W0", "S101W" }, // S101W   DirecTV 4S/8 & SES 1            //
    { "S103W0", "S103W" }, // S103W   AMC 1                           //
    { "S105W0", "S105W" }, // S105W   AMC 15/18                       //
    { "S107W3", "S107.3W" }, // S107.3W Anik F1R/G1                     //
                             // S110W   DirecTV 5 & Echostar 10/11      //
    { "S111W1", "S111.1W" }, // S111.1W Anik F2                         //
    { "S113W0", "S113W" }, // S113W   SatMex 6                        //
                           // S114.9W SatMex 5                        //
    { "S116W8", "S116.8W" }, // S116.8W SatMex 8                        //
    { "S119W0", "S118.8W" }, // S118.8W Anik F3                         //
                             // S119W   Echostar 14 & DirecTV 7S        //
    { "S121W0", "S121W" }, // S121W   Echostar 9/Galaxy 23            //
    { "S123W0", "S123W" }, // S123W   Galaxy 18                       //
    { "S125W0", "S125W" }, // S125W   Galaxy 14 & AMC 21              //
    { "S127W0", "S127W" }, // S127W   Galaxy 13/Horizons 1            //
                           // S129W   Ciel 2                          //
    { "S131W0", "S131W" }, // S131W   AMC 11                          //
    { "S133W0", "S133W" }, // S133W   Galaxy 15                       //
    { "S135W0", "S135W" }, // S135W   AMC 10                          //
    { "S137W0", "S137W" }, // S137W   AMC 7                           //
    { "S139W0", "S139W" }, // S139W   AMC 8                           //
    { "S177W0", "S177W" }, // S177W   NSS 9                           //
};

#define TR_COUNT(x) (sizeof(x)/sizeof(struct cTr))

/******************************************************************************
 * translate short names used by w_scan2 into VDR satellite identifiers.
 *
 *****************************************************************************/

static char const *
short_name_to_vdr_name(char const *satname)
{
    unsigned int i;
    for (i = 0; i < TR_COUNT(translations); i++)
        if (!strcmp(satname, translations[i].sat_name))
            return translations[i].vdr_sat_name;
    return satname; // fallback.
}

/******************************************************************************
 * translate VDR satellite identifiers into short names used by w_scan2.
 *
 *****************************************************************************/

char const *
vdr_name_to_short_name(char const *satname)
{
    unsigned int i;
    for (i = 0; i < TR_COUNT(translations); i++)
        if (!strcmp(satname, translations[i].vdr_sat_name))
            return translations[i].sat_name;
    return "unknown satellite"; // fallback.
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
    if (s->audio_lang && s->audio_lang[0][0])
        fprintf(f, "=%.4s", s->audio_lang[0]);
    if (s->audio_stream_type[0])
        fprintf(f, "@%u", s->audio_stream_type[0]);
    for (i = 1; i < s->audio_num; i++) {
        fprintf(f, ",%i", s->audio_pid[i]);
        if (s->audio_lang && s->audio_lang[i][0])
            fprintf(f, "=%.4s", s->audio_lang[i]);
        if (flags->vdr_version > 7)
            if (s->audio_stream_type[i])
                fprintf(f, "@%u", s->audio_stream_type[i]);
    }

    if (s->ac3_num) {
        fprintf(f, "%s", ";");
        for (i = 0; i < s->ac3_num; i++) {
            if (i > 0)
                fprintf(f, "%s", ",");
            fprintf(f, "%i", s->ac3_pid[i]);
            if (flags->vdr_version > 7)
                if (s->ac3_lang && s->ac3_lang[i][0])
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
            if (s->subtitling_lang && s->subtitling_lang[i][0])
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
