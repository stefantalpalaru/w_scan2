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
 * v 20110329
 */

#ifndef __COUNTRIES_H__
#define __COUNTRIES_H__

#include <stdint.h>
#ifdef __cplusplus
namespace COUNTRY {
#else
#include "extended_frontend.h"
#endif

enum channellist_t {
        ATSC_VSB                = 1,
        ATSC_QAM                = 2,
        DVBT_AU                 = 3,
        DVBT_DE                 = 4,
        DVBT_FR                 = 5,
        DVBT_GB                 = 6,
        DVBC_QAM                = 7,
        DVBC_FI                 = 8,
        DVBC_FR                 = 9,
        DVBC_BR                 = 10,
        ISDBT_6MHZ              = 11,
        USERLIST                = 999
};

#define SKIP_CHANNEL -1

enum offset_type_t {
        NO_OFFSET               = 0,
        POS_OFFSET              = 1,
        NEG_OFFSET              = 2,
        POS_OFFSET_1            = 3,
        POS_OFFSET_2            = 4,
        STOP_OFFSET_LOOP        = -1
};

typedef struct cCountry {
        const char *    short_name;
        int             id;
        const char*     full_name;
} _country;

#define COUNTRY_COUNT(x) (sizeof(x)/sizeof(struct cCountry))

extern struct cCountry country_list[];
int country_count(void);

void print_countries(void);

int choose_country (const char * country,
                    int * atsc,
                    int * dvb_cable,
                    uint16_t * scan_type,
                    int * channellist);

int txt_to_country (const char * id);

const char * country_to_short_name(int idx);
const char * country_to_full_name(int idx);


int base_offset(int channel, int channellist);
int freq_step  (int channel, int channellist);
int bandwidth  (int channel, int channellist);
int freq_offset(int channel, int channellist, int index);
int max_dvbc_srate(int bandwidth);

int dvbt_transmission_mode(int channel, int channellist);

int dvbc_qam_max(int channel, int channellist);
int dvbc_qam_min(int channel, int channellist);

int atsc_is_vsb(int atsc);
int atsc_is_qam(int atsc);

int get_user_country(void);

/* Every country has its own number here. if adding new one
 * simply append to enum.
 */
enum country_t {
    AF,
    AX,
    AL,
    DZ,
    AS,
    AD,
    AO,
    AI,
    AQ,
    AG,
    AR,
    AM,
    AW,
    AU,
    AT,
    AZ,
    BS,
    BH,
    BD,
    BB,
    BY,
    BE,
    BZ,
    BJ,
    BM,
    BT,
    BO,
    BQ,
    BA,
    BW,
    BV,
    BR,
    IO,
    BN,
    BG,
    BF,
    BI,
    KH,
    CM,
    CA,
    CV,
    KY,
    CF,
    TD,
    CL,
    CN,
    CX,
    CC,
    CO,
    KM,
    CG,
    CD,
    CK,
    CR,
    CI,
    HR,
    CU,
    CW,
    CY,
    CZ,
    DK,
    DJ,
    DM,
    DO,
    EC,
    EG,
    SV,
    GQ,
    ER,
    EE,
    ET,
    FK,
    FO,
    FJ,
    FI,
    FR,
    GF,
    PF,
    TF,
    GA,
    GM,
    GE,
    DE,
    GH,
    GI,
    GR,
    GL,
    GD,
    GP,
    GU,
    GT,
    GG,
    GN,
    GW,
    GY,
    HT,
    HM,
    VA,
    HN,
    HK,
    HU,
    IS,
    IN,
    ID,
    IR,
    IQ,
    IE,
    IM,
    IL,
    IT,
    JM,
    JP,
    JE,
    JO,
    KZ,
    KE,
    KI,
    KP,
    KR,
    KW,
    KG,
    LA,
    LV,
    LB,
    LS,
    LR,
    LY,
    LI,
    LT,
    LU,
    MO,
    MK,
    MG,
    MW,
    MY,
    MV,
    ML,
    MT,
    MH,
    MQ,
    MR,
    MU,
    YT,
    MX,
    FM,
    MD,
    MC,
    MN,
    ME,
    MS,
    MA,
    MZ,
    MM,
    NA,
    NR,
    NP,
    NL,
    NC,
    NZ,
    NI,
    NE,
    NG,
    NU,
    NF,
    MP,
    NO,
    OM,
    PK,
    PW,
    PS,
    PA,
    PG,
    PY,
    PE,
    PH,
    PN,
    PL,
    PT,
    PR,
    QA,
    RE,
    RO,
    RU,
    RW,
    BL,
    SH,
    KN,
    LC,
    MF,
    PM,
    VC,
    WS,
    SM,
    ST,
    SA,
    SN,
    RS,
    SC,
    SL,
    SX,
    SG,
    SK,
    SI,
    SB,
    SO,
    ZA,
    GS,
    ES,
    LK,
    SD,
    SR,
    SJ,
    SZ,
    SE,
    CH,
    SY,
    TW,
    TJ,
    TZ,
    TH,
    TL,
    TG,
    TK,
    TO,
    TT,
    TN,
    TR,
    TM,
    TC,
    TV,
    UG,
    UA,
    AE,
    GB,
    US,
    UM,
    UY,
    UZ,
    VU,
    VE,
    VN,
    VG,
    VI,
    WF,
    EH,
    YE,
    ZM,
    ZW
};

#ifdef __cplusplus
} //end of namespace
#endif

#endif
