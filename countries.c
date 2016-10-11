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
 * country.c/h, added 20090101
 * last update 20110329
 */

#include "countries.h"
#include "scan.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>

#ifdef VDRVERSNUM
#include <linux/dvb/frontend.h>
namespace COUNTRY {
#else
#include "extended_frontend.h"
#endif

/**************************************************************************************************
 * FREQUENCY CALCULATION SCHEME:
 * each frequency used by w_scan is calculated as follows:
 * 
 * frequency(channellist, channel, frequency_offset_index) = 
 *      base_offset(channel, channellist) +
 *      channel * freq_step(channel, channellist) +
 *      freq_offset(channel, channellist, frequency_offset_index);
 *
 *
 * - channellist is choosen by user, if not choosen defaults are used.
 * - channel = 0..133 (might be extended if needed)
 * - frequency_offset_index = 0 (no offset) -> 1 (pos offset) -> 2 (neg offset)
 *
 * if base_offset(channel, channellist) returns -1 this channel will be skipped.
 * if freq_offset(channel, channellist, frequency_offset_index) returns -1 this offset will be skipped.
 *
 * example:
 * channellist = 4; channel = 12
 *
 * base_offset(12, 4) = 142500000
 * freq_step(12, 4) = 7000000
 * freq_offset(12, 4, 0) = 0
 * freq_offset(12, 4, 1) = -1
 * freq_offset(12, 4, 2) = -1
 * 
 * frequency = 142500000 + 12 * 7000000 = 226500000
 * since freq_offset returns -1 for frequency_offset_index = (1,2) no frequency offset is applied.
 *
 * 20090101 -wk
 **************************************************************************************************/



/*
 * User selects a country specific channellist.
 * therefore we know
 *      - frontend type DVB or ATSC
 *      - used frequency list (base_offsets, freq_step)
 *      - other country-specific things (transmission mode, frequency offsets from center..)
 * use two letter uppercase for 'country', as defined by ISO 3166-1
 */
int choose_country (const char * country,
                    int * atsc,
                    int * dvb,
                    uint16_t * scan_type,
                    int * channellist) {
        if (*channellist == USERLIST) return 0;

        if (strcasecmp(country_to_short_name(txt_to_country(country)), country)) {
                warning("\n\nCOUNTRY CODE IS NOT DEFINED. FALLING BACK TO \"DE\"\n\n");
                sleep(10);
                }
        info("using settings for %s\n", country_to_full_name(txt_to_country(country)));

        /*
         * choose DVB or ATSC frontend type
         */
        switch(txt_to_country(country)) {

                case    AT:     //      AUSTRIA
                case    BE:     //      BELGIUM
                case    CH:     //      SWITZERLAND
                case    CZ:     //      CZECH REPUBLIC
                case    DE:     //      GERMANY
                case    DK:     //      DENMARK
                case    ES:     //      SPAIN
                case    FR:     //      FRANCE
                case    FI:     //      FINLAND
                case    GB:     //      UNITED KINGDOM
                case    GR:     //      GREECE
                case    HR:     //      CROATIA 
                case    HK:     //      HONG KONG
                case    IL:     //      ISRAEL
                case    IS:     //      ICELAND
                case    IT:     //      ITALY
                case    LU:     //      LUXEMBOURG
                case    LV:     //      LATVIA
                case    NL:     //      NETHERLANDS
                case    NO:     //      NORWAY
                case    NZ:     //      NEW ZEALAND
                case    PL:     //      POLAND
                case    SE:     //      SWEDEN
                case    SK:     //      SLOVAKIA
                case    TW:     //      TAIWAN, DVB-T w. ATSC freq list (thanks for freqlist to mkrufky)
                case    AU:     //      AUSTRALIA, DVB-T w. 7MHz step
                        switch(*dvb) {    
                                case SCAN_CABLE:
                                        *scan_type = SCAN_CABLE;
                                        info("DVB cable\n");
                                        break;
                                default:
                                        *scan_type = SCAN_TERRESTRIAL;
                                        info("DVB aerial\n");
                                        break;
                                }
                        break;
                
                case BR:        //      Brazil
                        switch(*dvb) {    
                                case SCAN_CABLE:
                                        *scan_type = SCAN_CABLE;
                                        info("DVB cable\n");
                                        break;
                                default:
                                        *scan_type = SCAN_TERRESTRIAL;
                                        info("ISDB-T (SBTVD)\n");
                                        break;
                                }
                        break;

                case    US:     //      UNITED STATES
                case    CA:     //      CANADA
                        *scan_type = SCAN_TERRCABLE_ATSC;
                        info("ATSC\n");
                        break;
                        
                default:
                        info("Country identifier %s not defined. Using defaults.\n", country);
                        return -1;
                        break;
                }


        /*
         * choose channellist name
         */
        switch(txt_to_country(country)) {
                //**********DVB freq lists*******************************************//
                case    AT:     //      AUSTRIA
                case    BE:     //      BELGIUM
                case    CH:     //      SWITZERLAND
                case    DE:     //      GERMANY
                case    DK:     //      DENMARK
                case    ES:     //      SPAIN
                case    GR:     //      GREECE
                case    HR:     //      CROATIA
                case    HK:     //      HONG KONG
                case    IL:     //      ISRAEL
                case    IS:     //      ICELAND
                case    IT:     //      ITALY
                case    LU:     //      LUXEMBOURG
                case    LV:     //      LATVIA
                case    NL:     //      NETHERLANDS
                case    NO:     //      NORWAY
                case    NZ:     //      NEW ZEALAND
                case    PL:     //      POLAND
                case    SE:     //      SWEDEN
                case    SK:     //      SLOVAKIA    
                        switch(*dvb) {    
                                case SCAN_CABLE:
                                        *channellist = DVBC_QAM;
                                        info("DVB-C\n");
                                        break;
                                default:
                                        *channellist = DVBT_DE;
                                        info("DVB-T Europe\n");
                                        break;               
                                }
                        break;
                case    CZ:     //      CZECH REPUBLIC
                case    FI:     //      FINLAND
                        switch(*dvb) {    
                                case SCAN_CABLE:
                                        *channellist = DVBC_FI;
                                        info("DVB-C FI\n");
                                        break;
                                default:
                                        *channellist = DVBT_DE;
                                        info("DVB-T Europe\n");
                                        break;               
                                }
                        break;
                case    FR:     //      FRANCE
                        switch(*dvb) {    
                                case SCAN_CABLE:
                                        *channellist = DVBC_FR;
                                        info("DVB-C FR\n");
                                        break;
                                default:
                                        *channellist = DVBT_FR;
                                        info("DVB-T FR\n");
                                        break;               
                                }
                        break;
                case    GB:     //      UNITED KINGDOM
                        switch(*dvb) {    
                                case SCAN_CABLE:
                                        *channellist = DVBC_QAM;
                                        info("DVB-C\n");
                                        break;
                                default:
                                        *channellist = DVBT_GB;
                                        info("DVB-T GB\n");
                                        break;               
                                }
                        break;
                case    AU:     //      AUSTRALIA
                        switch(*dvb) {    
                                case SCAN_CABLE:
                                        info("cable australia not yet defined.\n");
                                        break;
                                default:
                                        *channellist = DVBT_AU;
                                        info("DVB-T AU\n");
                                        break;               
                                }
                        break;
                //**********ATSC freq lists******************************************//    
                case    US:     //      UNITED STATES
                case    CA:     //      CANADA
                case    TW:     //      TAIWAN, DVB-T w. ATSC freq list    
                        if (atsc_is_vsb(*atsc)) {
                                *channellist = ATSC_VSB;
                                info("VSB US/CA, DVB-T TW\n");
                                }
                        if (atsc_is_qam(*atsc)) {
                                *channellist = ATSC_QAM;
                                info("QAM US/CA\n");    
                                }
                        break;
                case    BR:     //      BRAZIL, DVB-C/ISDB-T w. ATSC freq list
                        switch(*dvb) {    
                                case SCAN_CABLE:
                                        *channellist = DVBC_BR;
                                        info("DVB-C BR\n");
                                        break;
                                default:
                                        *channellist = ISDBT_6MHZ;
                                        info("ISDB-T, BR\n");
                                        break;               
                                }
                        break;
                //******************************************************************//    
                default:
                        info("Country identifier %s not defined. Using default freq lists.\n", country);
                        return -1;
                        break;
                }
        return 0; // success        
}

/*
 * return the base offsets for specified channellist and channel.
 */
int base_offset(int channel, int channellist) {
switch (channellist) {
        case ATSC_QAM: //ATSC cable, US EIA/NCTA Std Cable center freqs + IRC list
        case DVBC_BR:  //BRAZIL - same range as ATSC IRC
                switch (channel) {
                        case  2 ...  4:   return   45000000;
                        case  5 ...  6:   return   49000000;
                        case  7 ... 13:   return  135000000;
                        case 14 ... 22:   return   39000000;
                        case 23 ... 94:   return   81000000;
                        case 95 ... 99:   return -477000000;
                        case 100 ... 133: return   51000000;
                        default:          return SKIP_CHANNEL; 
                        }
        case ATSC_VSB: //ATSC terrestrial, US NTSC center freqs
                switch (channel) {
                        case  2 ...  4: return   45000000;
                        case  5 ...  6: return   49000000;
                        case  7 ... 13: return  135000000;
                        case 14 ... 69: return  389000000;
                        default:        return  SKIP_CHANNEL;
                        }
        case ISDBT_6MHZ: // ISDB-T, 6 MHz central frequencies
                switch (channel) {
                       // Channels 7-13 are reserved but aren't used yet
                      //case  7 ... 13: return  135000000;
                        case 14 ... 69: return  389000000;
                        default:        return  SKIP_CHANNEL;
                        }
        case DVBT_AU:  //AUSTRALIA, 7MHz step list
                switch (channel) {
                        case  5 ... 12: return  142500000;
                        case 21 ... 69: return  333500000;
                        default:        return  SKIP_CHANNEL;
                        }
        case DVBT_DE:  //GERMANY
        case DVBT_FR:  //FRANCE, +/- offset 166kHz & +offset 332kHz & +offset 498kHz
        case DVBT_GB:  //UNITED KINGDOM, +/- offset
                switch (channel) {
                        case  5 ... 12: return  142500000; // VHF unused in FRANCE, skip those in offset loop
                        case 21 ... 69: return  306000000;
                        default:        return  SKIP_CHANNEL;
                        }

        case DVBC_QAM: //EUROPE
                switch (channel) {
                        case  0 ... 1:  
                        case  5 ... 12: return   73000000;
                        case 22 ... 90: return  138000000;
                        default:        return  SKIP_CHANNEL;
                        }
        case DVBC_FI:  //FINLAND, QAM128
                switch (channel) {
                        case  1 ... 90: return  138000000;
                        default:        return  SKIP_CHANNEL;
                        }
        case DVBC_FR:  //FRANCE, needs user response.
                switch (channel) {
                        case  1 ... 39: return  107000000;
                        case 40 ... 89: return  138000000;
                        default:        return  SKIP_CHANNEL;
                        }
        default:
                fatal("%s: undefined channellist %d\n", __FUNCTION__, channellist);
                return SKIP_CHANNEL;
        }
}

/*
 * return the freq step size for specified channellist
 */
int freq_step(int channel, int channellist) {
switch (channellist) {
        case ATSC_QAM:
        case ATSC_VSB:
        case DVBC_BR:
        case ISDBT_6MHZ:
                       return  6000000; // atsc, 6MHz step
        case DVBT_AU:  return  7000000; // dvb-t australia, 7MHz step
        case DVBT_DE:
        case DVBT_FR:
        case DVBT_GB:  switch (channel) { // dvb-t europe, 7MHz VHF ch5..12, all other 8MHz
                              case  5 ... 12:    return 7000000;
                              case 21 ... 69:    return 8000000;
                              default:           return 8000000; // should be never reached.
                              }
        case DVBC_QAM:
        case DVBC_FI:
        case DVBC_FR:  return  8000000; // dvb-c, 8MHz step
        default:
             fatal("%s: undefined channellist %d\n", __FUNCTION__, channellist);
             return SKIP_CHANNEL;
        }
}


// EN300429 v1.2.1, chapter 9 Modulation p.16:
// "I and Q signals shall be square-root raised cosine filtered. The roll-off factor shall be 0,15"
#define DVBC_ROLLOFF 0.15
#define DVBC_SYMBOL_LEN (1.0 + DVBC_ROLLOFF)
 
int max_dvbc_srate  (int bandwidth) {
switch(bandwidth)  {
       case 0 ... 6:
             fatal("%s %d, using old style DVB API", __FUNCTION__, __LINE__);
             return SKIP_CHANNEL;

       case 8000000:
       case 7000000:
       case 6000000:
       case 5000000:
             return (int) (0.5 + ((double) bandwidth) / DVBC_SYMBOL_LEN);
       default:
             fatal("%s: unknown channel bandwidth %d\n", __FUNCTION__, bandwidth);
             return SKIP_CHANNEL;
       }
}

int bandwidth(int channel, int channellist) {
    return freq_step(channel, channellist);
}

/*
 * some countries use constant offsets around center frequency.
 * define them here.
 */
int freq_offset(int channel, int channellist, int index) {
switch (channellist) {
        case USERLIST:
                return 0;
        case ATSC_QAM:
                switch (channel) {
                        case 14 ... 16:
                        case 25 ... 53:
                        case 98 ... 99:
                            switch (index) {
                                case NO_OFFSET:     return 0;       //Incrementally Related Carriers (IRC)
                                case POS_OFFSET:    return 12500;   //US EIA/NCTA Standard Cable center frequencies
                                default:            return STOP_OFFSET_LOOP;
                                }
                        default: // IRC = standard cable center
                            switch (index) {
                                case NO_OFFSET:     return 0;       //center freq
                                default:            return STOP_OFFSET_LOOP;
                                }
                        }
        case DVBT_FR:
                switch (channel) {
                        // see http://tvignaud.pagesperso-orange.fr/tv/canaux.htm
                        case  5 ... 12: //VHF channels not used in FR
                            return STOP_OFFSET_LOOP;
                        default: //UHF channels. - 0,166 MHz /+ 0,166 MHz /+ 0,332 MHz /+ 0,498 MHz
                            switch (index) {
                                case NO_OFFSET:     return 0;       //center freq
                                case POS_OFFSET:    return +166000; //center+offset 166kHz
                                case NEG_OFFSET:    return -166000; //center-offset 166kHz
                                case POS_OFFSET_1:  return +332000; //center+offset 332kHz
                                case POS_OFFSET_2:  return +498000; //center+offset 498kHz
                                default:            return STOP_OFFSET_LOOP;
                                }
                        }
        case DVBT_GB:
                switch (channel) {
                        case  5 ... 12: //VHF channels
                            switch (index) {
                                case NO_OFFSET:     return 0;       //no offset
                                default:            return STOP_OFFSET_LOOP;
                                }
                        default: //UHF channels
                            switch (index) {
                                case NO_OFFSET:     return 0;       //center freq
                                case POS_OFFSET:    return +167000; //center+offset
                                case NEG_OFFSET:    return -167000; //center-offset
                                default:            return STOP_OFFSET_LOOP;
                                }
                        }
        case DVBT_AU    :
                switch (index) {
                        case NO_OFFSET:             return 0;       //center freq
                        case POS_OFFSET:            return +125000; //center+offset
                        default:                    return STOP_OFFSET_LOOP;
                        }
        case DVBC_FR:
                switch (channel) {
                        case 1 ... 39:
                            switch (index) {
                                case NO_OFFSET:     return 0;       //center freq
                                case POS_OFFSET:    return +125000; //center+offset
                                default:            return STOP_OFFSET_LOOP;
                                }
                        case 40 ... 89:
                        default:
                            switch (index) {
                                case NO_OFFSET:     return 0;
                                default:            return STOP_OFFSET_LOOP;
                                }
                        }
        case ISDBT_6MHZ: // ISDB-T, 6 MHz central frequencies
                switch (channel) {
                        // Channels 7-13 are reserved but aren't used yet
                        case  7 ... 13:
                        case 14 ... 69:
                            switch (index) {
                                case NO_OFFSET:     return SKIP_CHANNEL; //center freq
                                case POS_OFFSET:    return +142857;      //center+offset
                                default:            return STOP_OFFSET_LOOP;
                                }
                        default:        return  STOP_OFFSET_LOOP;
                        }                
        default:
                switch (index) {
                        case NO_OFFSET: return 0;
                        default:        return STOP_OFFSET_LOOP;
                        }
        }
}


/*
 * DVB-T: default value if transmission mode AUTO
 * not supported
 */
int dvbt_transmission_mode(int channel, int channellist) {
switch (channellist) {
        // GB seems to use 8k since 12/2009
        default:        return TRANSMISSION_MODE_8K;  
        }
}


/*
 * start/stop values for dvbc qam loop
 * 0 == QAM_64, 1 == QAM_256, 2 == QAM_128
 */
int dvbc_qam_max(int channel, int channellist) {
switch(channellist) {
       case DVBC_FI:    return 2; //QAM128
       case DVBC_BR:
       case DVBC_FR:
       case DVBC_QAM:   return 1; //QAM256
       default:         return 0; //no qam loop
       }
}

    
int dvbc_qam_min(int channel, int channellist) {
switch(channellist) {
       case DVBC_FI:
       case DVBC_BR:
       case DVBC_FR:
       case DVBC_QAM:   return 0; //QAM64
       default:         return 0; //no qam loop
       }
}

int atsc_is_vsb(int atsc) {
    return (atsc & ATSC_VSB);
}

int atsc_is_qam(int atsc) {
    return (atsc & ATSC_QAM);
}

/* 
 * two letters constants from ISO 3166-1.
 * sorted alphabetically by long country name
 */

struct cCountry country_list[] = {
/*- ISO 3166-1 - unique id - long country name                 alpha-3 numeric */
       {"AF", AF, "AFGHANISTAN"},                                  /*AFG, 4  },*/
       {"AX", AX, "ÅLAND ISLANDS"},                                /*ALA, 248},*/
       {"AL", AL, "ALBANIA"},                                      /*ALB, 8  },*/
       {"DZ", DZ, "ALGERIA"},                                      /*DZA, 12 },*/
       {"AS", AS, "AMERICAN SAMOA"},                               /*ASM, 16 },*/
       {"AD", AD, "ANDORRA"},                                      /*AND, 20 },*/
       {"AO", AO, "ANGOLA"},                                       /*AGO, 24 },*/
       {"AI", AI, "ANGUILLA"},                                     /*AIA, 660},*/
       {"AQ", AQ, "ANTARCTICA"},                                   /*ATA, 10 },*/
       {"AG", AG, "ANTIGUA AND BARBUDA"},                          /*ATG, 28 },*/
       {"AR", AR, "ARGENTINA"},                                    /*ARG, 32 },*/
       {"AM", AM, "ARMENIA"},                                      /*ARM, 51 },*/
       {"AW", AW, "ARUBA"},                                        /*ABW, 533},*/
       {"AU", AU, "AUSTRALIA"},                                    /*AUS, 36 },*/
       {"AT", AT, "AUSTRIA"},                                      /*AUT, 40 },*/
       {"AZ", AZ, "AZERBAIJAN"},                                   /*AZE, 31 },*/
       {"BS", BS, "BAHAMAS"},                                      /*BHS, 44 },*/
       {"BH", BH, "BAHRAIN"},                                      /*BHR, 48 },*/
       {"BD", BD, "BANGLADESH"},                                   /*BGD, 50 },*/
       {"BB", BB, "BARBADOS"},                                     /*BRB, 52 },*/
       {"BY", BY, "BELARUS"},                                      /*BLR, 112},*/
       {"BE", BE, "BELGIUM"},                                      /*BEL, 56 },*/
       {"BZ", BZ, "BELIZE"},                                       /*BLZ, 84 },*/
       {"BJ", BJ, "BENIN"},                                        /*BEN, 204},*/
       {"BM", BM, "BERMUDA"},                                      /*BMU, 60 },*/
       {"BT", BT, "BHUTAN"},                                       /*BTN, 64 },*/
       {"BO", BO, "BOLIVIA"},                                      /*BOL, 68 },*/
       {"BQ", BQ, "BONAIRE"},                                      /*BES, 535},*/
       {"BA", BA, "BOSNIA AND HERZEGOVINA"},                       /*BIH, 70 },*/
       {"BW", BW, "BOTSWANA"},                                     /*BWA, 72 },*/
       {"BV", BV, "BOUVET ISLAND"},                                /*BVT, 74 },*/
       {"BR", BR, "BRAZIL"},                                       /*BRA, 76 },*/
       {"IO", IO, "BRITISH INDIAN OCEAN TERRITORY"},               /*IOT, 86 },*/
       {"BN", BN, "BRUNEI DARUSSALAM"},                            /*BRN, 96 },*/
       {"BG", BG, "BULGARIA"},                                     /*BGR, 100},*/
       {"BF", BF, "BURKINA FASO"},                                 /*BFA, 854},*/
       {"BI", BI, "BURUNDI"},                                      /*BDI, 108},*/
       {"KH", KH, "CAMBODIA"},                                     /*KHM, 116},*/
       {"CM", CM, "CAMEROON"},                                     /*CMR, 120},*/
       {"CA", CA, "CANADA"},                                       /*CAN, 124},*/
       {"CV", CV, "CAPE VERDE"},                                   /*CPV, 132},*/
       {"KY", KY, "CAYMAN ISLANDS"},                               /*CYM, 136},*/
       {"CF", CF, "CENTRAL AFRICAN REPUBLIC"},                     /*CAF, 140},*/
       {"TD", TD, "CHAD"},                                         /*TCD, 148},*/
       {"CL", CL, "CHILE"},                                        /*CHL, 152},*/
       {"CN", CN, "CHINA"},                                        /*CHN, 156},*/
       {"CX", CX, "CHRISTMAS ISLAND"},                             /*CXR, 162},*/
       {"CC", CC, "COCOS (KEELING) ISLANDS"},                      /*CCK, 166},*/
       {"CO", CO, "COLOMBIA"},                                     /*COL, 170},*/
       {"KM", KM, "COMOROS"},                                      /*COM, 174},*/
       {"CG", CG, "CONGO"},                                        /*COG, 178},*/
       {"CD", CD, "CONGO, THE DEMOCRATIC REPUBLIC OF THE"},        /*COD, 180},*/
       {"CK", CK, "COOK ISLANDS"},                                 /*COK, 184},*/
       {"CR", CR, "COSTA RICA"},                                   /*CRI, 188},*/
       {"CI", CI, "CÔTE D'IVOIRE"},                                /*CIV, 384},*/
       {"HR", HR, "CROATIA"},                                      /*HRV, 191},*/
       {"CU", CU, "CUBA"},                                         /*CUB, 192},*/
       {"CW", CW, "CURAÇAO"},                                      /*CUW, 531},*/
       {"CY", CY, "CYPRUS"},                                       /*CYP, 196},*/
       {"CZ", CZ, "CZECH REPUBLIC"},                               /*CZE, 203},*/
       {"DK", DK, "DENMARK"},                                      /*DNK, 208},*/
       {"DJ", DJ, "DJIBOUTI"},                                     /*DJI, 262},*/
       {"DM", DM, "DOMINICA"},                                     /*DMA, 212},*/
       {"DO", DO, "DOMINICAN REPUBLIC"},                           /*DOM, 214},*/
       {"EC", EC, "ECUADOR"},                                      /*ECU, 218},*/
       {"EG", EG, "EGYPT"},                                        /*EGY, 818},*/
       {"SV", SV, "EL SALVADOR"},                                  /*SLV, 222},*/
       {"GQ", GQ, "EQUATORIAL GUINEA"},                            /*GNQ, 226},*/
       {"ER", ER, "ERITREA"},                                      /*ERI, 232},*/
       {"EE", EE, "ESTONIA"},                                      /*EST, 233},*/
       {"ET", ET, "ETHIOPIA"},                                     /*ETH, 231},*/
       {"FK", FK, "FALKLAND ISLANDS (MALVINAS)"},                  /*FLK, 238},*/
       {"FO", FO, "FAROE ISLANDS"},                                /*FRO, 234},*/
       {"FJ", FJ, "FIJI"},                                         /*FJI, 242},*/
       {"FI", FI, "FINLAND"},                                      /*FIN, 246},*/
       {"FR", FR, "FRANCE"},                                       /*FRA, 250},*/
       {"GF", GF, "FRENCH GUIANA"},                                /*GUF, 254},*/
       {"PF", PF, "FRENCH POLYNESIA"},                             /*PYF, 258},*/
       {"TF", TF, "FRENCH SOUTHERN TERRITORIES"},                  /*ATF, 260},*/
       {"GA", GA, "GABON"},                                        /*GAB, 266},*/
       {"GM", GM, "GAMBIA"},                                       /*GMB, 270},*/
       {"GE", GE, "GEORGIA"},                                      /*GEO, 268},*/
       {"DE", DE, "GERMANY"},                                      /*DEU, 276},*/
       {"GH", GH, "GHANA"},                                        /*GHA, 288},*/
       {"GI", GI, "GIBRALTAR"},                                    /*GIB, 292},*/
       {"GR", GR, "GREECE"},                                       /*GRC, 300},*/
       {"GL", GL, "GREENLAND"},                                    /*GRL, 304},*/
       {"GD", GD, "GRENADA"},                                      /*GRD, 308},*/
       {"GP", GP, "GUADELOUPE"},                                   /*GLP, 312},*/
       {"GU", GU, "GUAM"},                                         /*GUM, 316},*/
       {"GT", GT, "GUATEMALA"},                                    /*GTM, 320},*/
       {"GG", GG, "GUERNSEY"},                                     /*GGY, 831},*/
       {"GN", GN, "GUINEA"},                                       /*GIN, 324},*/
       {"GW", GW, "GUINEA-BISSAU"},                                /*GNB, 624},*/
       {"GY", GY, "GUYANA"},                                       /*GUY, 328},*/
       {"HT", HT, "HAITI"},                                        /*HTI, 332},*/
       {"HM", HM, "HEARD ISLAND AND MCDONALD ISLANDS"},            /*HMD, 334},*/
       {"VA", VA, "HOLY SEE (VATICAN CITY STATE)"},                /*VAT, 336},*/
       {"HN", HN, "HONDURAS"},                                     /*HND, 340},*/
       {"HK", HK, "HONG KONG"},                                    /*HKG, 344},*/
       {"HU", HU, "HUNGARY"},                                      /*HUN, 348},*/
       {"IS", IS, "ICELAND"},                                      /*ISL, 352},*/
       {"IN", IN, "INDIA"},                                        /*IND, 356},*/
       {"ID", ID, "INDONESIA"},                                    /*IDN, 360},*/
       {"IR", IR, "IRAN, ISLAMIC REPUBLIC OF"},                    /*IRN, 364},*/
       {"IQ", IQ, "IRAQ"},                                         /*IRQ, 368},*/
       {"IE", IE, "IRELAND"},                                      /*IRL, 372},*/
       {"IM", IM, "ISLE OF MAN"},                                  /*IMN, 833},*/
       {"IL", IL, "ISRAEL"},                                       /*ISR, 376},*/
       {"IT", IT, "ITALY"},                                        /*ITA, 380},*/
       {"JM", JM, "JAMAICA"},                                      /*JAM, 388},*/
       {"JP", JP, "JAPAN"},                                        /*JPN, 392},*/
       {"JE", JE, "JERSEY"},                                       /*JEY, 832},*/
       {"JO", JO, "JORDAN"},                                       /*JOR, 400},*/
       {"KZ", KZ, "KAZAKHSTAN"},                                   /*KAZ, 398},*/
       {"KE", KE, "KENYA"},                                        /*KEN, 404},*/
       {"KI", KI, "KIRIBATI"},                                     /*KIR, 296},*/
       {"KP", KP, "KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF"},       /*PRK, 408},*/
       {"KR", KR, "KOREA, REPUBLIC OF"},                           /*KOR, 410},*/
       {"KW", KW, "KUWAIT"},                                       /*KWT, 414},*/
       {"KG", KG, "KYRGYZSTAN"},                                   /*KGZ, 417},*/
       {"LA", LA, "LAO PEOPLE'S DEMOCRATIC REPUBLIC"},             /*LAO, 418},*/
       {"LV", LV, "LATVIA"},                                       /*LVA, 428},*/
       {"LB", LB, "LEBANON"},                                      /*LBN, 422},*/
       {"LS", LS, "LESOTHO"},                                      /*LSO, 426},*/
       {"LR", LR, "LIBERIA"},                                      /*LBR, 430},*/
       {"LY", LY, "LIBYAN ARAB JAMAHIRIYA"},                       /*LBY, 434},*/
       {"LI", LI, "LIECHTENSTEIN"},                                /*LIE, 438},*/
       {"LT", LT, "LITHUANIA"},                                    /*LTU, 440},*/
       {"LU", LU, "LUXEMBOURG"},                                   /*LUX, 442},*/
       {"MO", MO, "MACAO"},                                        /*MAC, 446},*/
       {"MK", MK, "MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF"},   /*MKD, 807},*/
       {"MG", MG, "MADAGASCAR"},                                   /*MDG, 450},*/
       {"MW", MW, "MALAWI"},                                       /*MWI, 454},*/
       {"MY", MY, "MALAYSIA"},                                     /*MYS, 458},*/
       {"MV", MV, "MALDIVES"},                                     /*MDV, 462},*/
       {"ML", ML, "MALI"},                                         /*MLI, 466},*/
       {"MT", MT, "MALTA"},                                        /*MLT, 470},*/
       {"MH", MH, "MARSHALL ISLANDS"},                             /*MHL, 584},*/
       {"MQ", MQ, "MARTINIQUE"},                                   /*MTQ, 474},*/
       {"MR", MR, "MAURITANIA"},                                   /*MRT, 478},*/
       {"MU", MU, "MAURITIUS"},                                    /*MUS, 480},*/
       {"YT", YT, "MAYOTTE"},                                      /*MYT, 175},*/
       {"MX", MX, "MEXICO"},                                       /*MEX, 484},*/
       {"FM", FM, "MICRONESIA, FEDERATED STATES OF"},              /*FSM, 583},*/
       {"MD", MD, "MOLDOVA"},                                      /*MDA, 498},*/
       {"MC", MC, "MONACO"},                                       /*MCO, 492},*/
       {"MN", MN, "MONGOLIA"},                                     /*MNG, 496},*/
       {"ME", ME, "MONTENEGRO"},                                   /*MNE, 499},*/
       {"MS", MS, "MONTSERRAT"},                                   /*MSR, 500},*/
       {"MA", MA, "MOROCCO"},                                      /*MAR, 504},*/
       {"MZ", MZ, "MOZAMBIQUE"},                                   /*MOZ, 508},*/
       {"MM", MM, "MYANMAR"},                                      /*MMR, 104},*/
       {"NA", NA, "NAMIBIA"},                                      /*NAM, 516},*/
       {"NR", NR, "NAURU"},                                        /*NRU, 520},*/
       {"NP", NP, "NEPAL"},                                        /*NPL, 524},*/
       {"NL", NL, "NETHERLANDS"},                                  /*NLD, 528},*/
       {"NC", NC, "NEW CALEDONIA"},                                /*NCL, 540},*/
       {"NZ", NZ, "NEW ZEALAND"},                                  /*NZL, 554},*/
       {"NI", NI, "NICARAGUA"},                                    /*NIC, 558},*/
       {"NE", NE, "NIGER"},                                        /*NER, 562},*/
       {"NG", NG, "NIGERIA"},                                      /*NGA, 566},*/
       {"NU", NU, "NIUE"},                                         /*NIU, 570},*/
       {"NF", NF, "NORFOLK ISLAND"},                               /*NFK, 574},*/
       {"MP", MP, "NORTHERN MARIANA ISLANDS"},                     /*MNP, 580},*/
       {"NO", NO, "NORWAY"},                                       /*NOR, 578},*/
       {"OM", OM, "OMAN"},                                         /*OMN, 512},*/
       {"PK", PK, "PAKISTAN"},                                     /*PAK, 586},*/
       {"PW", PW, "PALAU"},                                        /*PLW, 585},*/
       {"PS", PS, "PALESTINIAN TERRITORY, OCCUPIED"},              /*PSE, 275},*/
       {"PA", PA, "PANAMA"},                                       /*PAN, 591},*/
       {"PG", PG, "PAPUA NEW GUINEA"},                             /*PNG, 598},*/
       {"PY", PY, "PARAGUAY"},                                     /*PRY, 600},*/
       {"PE", PE, "PERU"},                                         /*PER, 604},*/
       {"PH", PH, "PHILIPPINES"},                                  /*PHL, 608},*/
       {"PN", PN, "PITCAIRN"},                                     /*PCN, 612},*/
       {"PL", PL, "POLAND"},                                       /*POL, 616},*/
       {"PT", PT, "PORTUGAL"},                                     /*PRT, 620},*/
       {"PR", PR, "PUERTO RICO"},                                  /*PRI, 630},*/
       {"QA", QA, "QATA"},                                         /*QAT, 634},*/
       {"RE", RE, "RÉUNION"},                                      /*REU, 638},*/
       {"RO", RO, "ROMANIA"},                                      /*ROU, 642},*/
       {"RU", RU, "RUSSIAN FEDERATION"},                           /*RUS, 643},*/
       {"RW", RW, "RWANDA"},                                       /*RWA, 646},*/
       {"BL", BL, "SAINT BARTHÉLEMY"},                             /*BLM, 652},*/
       {"SH", SH, "SAINT HELENA"},                                 /*SHN, 654},*/
       {"KN", KN, "SAINT KITTS AND NEVIS"},                        /*KNA, 659},*/
       {"LC", LC, "SAINT LUCIA"},                                  /*LCA, 662},*/
       {"MF", MF, "SAINT MARTIN"},                                 /*MAF, 663},*/
       {"PM", PM, "SAINT PIERRE AND MIQUELON"},                    /*SPM, 666},*/
       {"VC", VC, "SAINT VINCENT AND THE GRENADINES"},             /*VCT, 670},*/
       {"WS", WS, "SAMOA"},                                        /*WSM, 882},*/
       {"SM", SM, "SAN MARINO"},                                   /*SMR, 674},*/
       {"ST", ST, "SAO TOME AND PRINCIPE"},                        /*STP, 678},*/
       {"SA", SA, "SAUDI ARABIA"},                                 /*SAU, 682},*/
       {"SN", SN, "SENEGAL"},                                      /*SEN, 686},*/
       {"RS", RS, "SERBIA"},                                       /*SRB, 688},*/
       {"SC", SC, "SEYCHELLES"},                                   /*SYC, 690},*/
       {"SL", SL, "SIERRA LEONE"},                                 /*SLE, 694},*/
       {"SX", SX, "SINT MAARTEN"},                                 /*SXM, 534},*/
       {"SG", SG, "SINGAPORE"},                                    /*SGP, 702},*/
       {"SK", SK, "SLOVAKIA"},                                     /*SVK, 703},*/
       {"SI", SI, "SLOVENIA"},                                     /*SVN, 705},*/
       {"SB", SB, "SOLOMON ISLANDS"},                              /*SLB, 90 },*/
       {"SO", SO, "SOMALIA"},                                      /*SOM, 706},*/
       {"ZA", ZA, "SOUTH AFRICA"},                                 /*ZAF, 710},*/
       {"GS", GS, "SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS"}, /*SGS, 239},*/
       {"ES", ES, "SPAIN"},                                        /*ESP, 724},*/
       {"LK", LK, "SRI LANKA"},                                    /*LKA, 144},*/
       {"SD", SD, "SUDAN"},                                        /*SDN, 736},*/
       {"SR", SR, "SURINAME"},                                     /*SUR, 740},*/
       {"SJ", SJ, "SVALBARD AND JAN MAYEN"},                       /*SJM, 744},*/
       {"SZ", SZ, "SWAZILAND"},                                    /*SWZ, 748},*/
       {"SE", SE, "SWEDEN"},                                       /*SWE, 752},*/
       {"CH", CH, "SWITZERLAND"},                                  /*CHE, 756},*/
       {"SY", SY, "SYRIAN ARAB REPUBLIC"},                         /*SYR, 760},*/
       {"TW", TW, "TAIWAN"},                                       /*TWN, 158},*/
       {"TJ", TJ, "TAJIKISTAN"},                                   /*TJK, 762},*/
       {"TZ", TZ, "TANZANIA, UNITED REPUBLIC OF"},                 /*TZA, 834},*/
       {"TH", TH, "THAILAND"},                                     /*THA, 764},*/
       {"TL", TL, "TIMOR-LESTE"},                                  /*TLS, 626},*/
       {"TG", TG, "TOGO"},                                         /*TGO, 768},*/
       {"TK", TK, "TOKELAU"},                                      /*TKL, 772},*/
       {"TO", TO, "TONGA"},                                        /*TON, 776},*/
       {"TT", TT, "TRINIDAD AND TOBAGO"},                          /*TTO, 780},*/
       {"TN", TN, "TUNISIA"},                                      /*TUN, 788},*/
       {"TR", TR, "TURKEY"},                                       /*TUR, 792},*/
       {"TM", TM, "TURKMENISTAN"},                                 /*TKM, 795},*/
       {"TC", TC, "TURKS AND CAICOS ISLANDS"},                     /*TCA, 796},*/
       {"TV", TV, "TUVALU"},                                       /*TUV, 798},*/
       {"UG", UG, "UGANDA"},                                       /*UGA, 800},*/
       {"UA", UA, "UKRAINE"},                                      /*UKR, 804},*/
       {"AE", AE, "UNITED ARAB EMIRATES"},                         /*ARE, 784},*/
       {"GB", GB, "UNITED KINGDOM"},                               /*GBR, 826},*/
       {"US", US, "UNITED STATES"},                                /*USA, 840},*/
       {"UM", UM, "UNITED STATES MINOR OUTLYING ISLANDS"},         /*UMI, 581},*/
       {"UY", UY, "URUGUAY"},                                      /*URY, 858},*/
       {"UZ", UZ, "UZBEKISTAN"},                                   /*UZB, 860},*/
       {"VU", VU, "VANUATU"},                                      /*VUT, 548},*/
       {"VE", VE, "VENEZUELA"},                                    /*VEN, 862},*/
       {"VN", VN, "VIET NAM"},                                     /*VNM, 704},*/
       {"VG", VG, "VIRGIN ISLANDS, BRITISH"},                      /*VGB, 92 },*/
       {"VI", VI, "VIRGIN ISLANDS, U.S."},                         /*VIR, 850},*/
       {"WF", WF, "WALLIS AND FUTUNA"},                            /*WLF, 876},*/
       {"EH", EH, "WESTERN SAHARA"},                               /*ESH, 732},*/
       {"YE", YE, "YEMEN"},                                        /*YEM, 887},*/
       {"ZM", ZM, "ZAMBIA"},                                       /*ZMB, 894},*/
       {"ZW", ZW, "ZIMBABWE"}                                      /*ZWE, 716},*/
};



/* convert ISO 3166-1 two-letter constant
 * to index number
 */
int txt_to_country(const char * id) {
unsigned int i;
for (i = 0; i < COUNTRY_COUNT(country_list); i++)
   if (! strcasecmp(id,country_list[i].short_name))
      return country_list[i].id;
return DE; // w_scan defaults to DVB-t de_DE
}


/* convert index number
 * to ISO 3166-1 two-letter constant
 */
const char * country_to_short_name(int idx) {
unsigned int i;
for (i = 0; i < COUNTRY_COUNT(country_list); i++)
   if (idx == country_list[i].id)
      return country_list[i].short_name;
return "DE"; // w_scan defaults to DVB-t de_DE
}

/* convert index number
 * to country name
 */
const char * country_to_full_name(int idx) {
unsigned int i;
for (i = 0; i < COUNTRY_COUNT(country_list); i++)
   if (idx == country_list[i].id)
      return country_list[i].full_name;
warning("COUNTRY CODE NOT DEFINED. PLEASE RE-CHECK WETHER YOU TYPED CORRECTLY.\n");
usleep(5000000);
return "GERMANY"; // w_scan defaults to DVB-t de_DE
}

void print_countries(void) {
unsigned int i;
for (i = 0; i < COUNTRY_COUNT(country_list); i++)
    info("\t%s\t\t%s\n", country_list[i].short_name, country_list[i].full_name);
}

int country_count() {
  return COUNTRY_COUNT(country_list);
}

#define MIN(X,Y) (X < Y ? X : Y)

int get_user_country(void) {
  const int cats[] = { LC_CTYPE, LC_COLLATE, LC_MESSAGES };
  char * buf, * pch, * pbuf;
  unsigned cat, idx = 0xffff;

  for (cat = 0; cat < sizeof(cats)/sizeof(cats[0]); cat++) {

    // Note: program's locale is not changed here, since locale isn't given.
    //       the returned char * should be "C", "POSIX" or something valid.
    //       If valid, we can only *guess* which format is returned.
    //       Assume here something like "de_DE.iso8859-1@euro" or "de_DE.utf-8" 
    if (! (buf = setlocale(cats[cat], "")) || strlen(buf) < 2)
       continue;

    buf = strdup(buf);
    pbuf= buf;
       
    if (!  strncmp(buf, "POSIX", MIN(strlen(buf), 5)) ||
        ! (strncmp(buf, "en",    MIN(strlen(buf), 2)) && !isalpha(buf[2])) )
       continue;

    // assuming 'language_country.encoding@variant'

    // country after '_', if given
    if ((pch = strchr(buf, '_')))
       pbuf = pch + 1;

    // remove all after '@', including '@'
    if ((pch = strchr(pbuf, '@')))
       *pch = 0;

    // remove all after '.', including '.'
    if ((pch = strchr(pbuf, '.')))
       *pch = 0;

    if (strlen(pbuf) == 2) {
       unsigned int i;
       verbose("%s: lang id = %s\n", __FUNCTION__, pbuf);
       for (i = 0; i < COUNTRY_COUNT(country_list); i++)
           if (! strcasecmp(pbuf,country_list[i].short_name)) {
              idx = country_list[i].id;
              break;
              }
       }
    free(buf);
    if (idx != 0xffff)
       return idx;
    }

  warning("could not guess your country. Falling back to 'DE'\n");
  return DE;
}

#ifdef VDRVERSNUM
} // end namespace COUNTRY
#endif
