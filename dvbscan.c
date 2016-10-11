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
 */

#include <strings.h>
#include "extended_frontend.h"
#include "scan.h"
#include "dvbscan.h"




#define STRUCT_COUNT(struct_list) (sizeof(struct_list)/sizeof(struct init_item))

/********************************************************************
 * dvbscan.c
 *
 * import / export of initial_tuning_data for dvbscan
 * see doc/README.file_formats
 *
 ********************************************************************/


/********************************************************************
 * DVB-T
 ********************************************************************/

struct init_item terr_bw_list[] = {
        {"8MHz"     , 8000000     },
        {"7MHz"     , 7000000     },
        {"6MHz"     , 6000000     },
        {"5MHz"     , 5000000     },
        {"10MHz"    , 10000000    },
        {"1.712MHz" , 1712000     },
        {"AUTO"     , 8000000     }
};                 
                   
struct init_item terr_fec_list[] = {
        {"NONE"     , FEC_NONE },
        {"1/2"      , FEC_1_2  },
        {"2/3"      , FEC_2_3  },
        {"3/4"      , FEC_3_4  },
        {"4/5"      , FEC_4_5  },
        {"5/6"      , FEC_5_6  },
        {"6/7"      , FEC_6_7  },
        {"7/8"      , FEC_7_8  },
        {"3/5"      , FEC_3_5  },
        {"4/5"      , FEC_4_5  },
        {"AUTO"     , FEC_AUTO }
};

struct init_item terr_mod_list[] = {
        {"QPSK"     , QPSK     },
        {"QAM16"    , QAM_16   },
        {"QAM64"    , QAM_64   },
        {"QAM256"   , QAM_256  },
        {"AUTO"     , QAM_AUTO }
};

struct init_item terr_transmission_list[] = {
        {"2k"   , TRANSMISSION_MODE_2K   },
        {"8k"   , TRANSMISSION_MODE_8K   },
        {"4k"   , TRANSMISSION_MODE_4K   },
        {"1k"   , TRANSMISSION_MODE_1K   },
        {"16k"  , TRANSMISSION_MODE_16K  },
        {"32k"  , TRANSMISSION_MODE_32K  },
        {"AUTO" , TRANSMISSION_MODE_AUTO }
};

struct init_item terr_guard_list[] = {
        {"1/32"   , GUARD_INTERVAL_1_32   },
        {"1/16"   , GUARD_INTERVAL_1_16   },
        {"1/8"    , GUARD_INTERVAL_1_8    },
        {"1/4"    , GUARD_INTERVAL_1_4    },
        {"1/128"  , GUARD_INTERVAL_1_128  },
        {"19/128" , GUARD_INTERVAL_19_128 },
        {"19/256" , GUARD_INTERVAL_19_256 },
        {"AUTO"   , GUARD_INTERVAL_AUTO   }
};

struct init_item terr_hierarchy_list[] = {
        {"NONE", HIERARCHY_NONE},
        {"1"   , HIERARCHY_1   },
        {"2"   , HIERARCHY_2   },
        {"4"   , HIERARCHY_4   },
        {"AUTO", HIERARCHY_AUTO}
};

/* convert text to identifiers */

int txt_to_terr_bw ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_bw_list); i++)
                if (! strcasecmp(txt, terr_bw_list[i].name))
                        return terr_bw_list[i].id;
        return 8000000; // fallback. should never happen.
}

int txt_to_terr_fec ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_fec_list); i++)
                if (! strcasecmp(txt, terr_fec_list[i].name))
                        return terr_fec_list[i].id;
        return FEC_AUTO; // fallback. should never happen.
}

int txt_to_terr_mod ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_mod_list); i++)
                if (! strcasecmp(txt, terr_mod_list[i].name))
                        return terr_mod_list[i].id;
        return QAM_AUTO; // fallback. should never happen.
}

int txt_to_terr_transmission ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_transmission_list); i++)
                if (! strcasecmp(txt, terr_transmission_list[i].name))
                        return terr_transmission_list[i].id;
        return TRANSMISSION_MODE_AUTO; // fallback. should never happen.
}

int txt_to_terr_guard ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_guard_list); i++)
                if (! strcasecmp(txt, terr_guard_list[i].name))
                        return terr_guard_list[i].id;
        return GUARD_INTERVAL_AUTO; // fallback. should never happen.
}

int txt_to_terr_hierarchy ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_hierarchy_list); i++)
                if (! strcasecmp(txt, terr_hierarchy_list[i].name))
                        return terr_hierarchy_list[i].id;
        return HIERARCHY_AUTO; // fallback. should never happen.
}

/*convert identifier to text */

const char * terr_bw_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_bw_list); i++)
                if (id == terr_bw_list[i].id)
                        return terr_bw_list[i].name;
        return "AUTO"; // fallback. should never happen.
}

const char * terr_fec_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_fec_list); i++)
                if (id == terr_fec_list[i].id)
                        return terr_fec_list[i].name;
        return "AUTO"; // fallback. should never happen.
}

const char * terr_mod_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_mod_list); i++)
                if (id == terr_mod_list[i].id)
                        return terr_mod_list[i].name;
        return "AUTO"; // fallback. should never happen.
}

const char * terr_transmission_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_transmission_list); i++)
                if (id == terr_transmission_list[i].id)
                        return terr_transmission_list[i].name;
        return "AUTO"; // fallback. should never happen.
}

const char * terr_guard_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_guard_list); i++)
                if (id == terr_guard_list[i].id)
                        return terr_guard_list[i].name;
        return "AUTO"; // fallback. should never happen.
}

const char * terr_hierarchy_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(terr_hierarchy_list); i++)
                if (id == terr_hierarchy_list[i].id)
                        return terr_hierarchy_list[i].name;
        return "AUTO"; // fallback. should never happen.
}


/********************************************************************
 * DVB-C
 ********************************************************************/

struct init_item cable_fec_list[] = {
        {"NONE" , FEC_NONE},
        {"1/2"  , FEC_1_2 },
        {"2/3"  , FEC_2_3 },
        {"3/4"  , FEC_3_4 },
        {"4/5"  , FEC_4_5 },
        {"5/6"  , FEC_5_6 },
        {"6/7"  , FEC_6_7 },
        {"7/8"  , FEC_7_8 },
        {"8/9"  , FEC_8_9 },
        {"3/5"  , FEC_3_5 },
        {"9/10" , FEC_9_10},
        {"AUTO" , FEC_AUTO}
};

struct init_item cable_mod_list[] = {
        {"QAM16"  , QAM_16  },
        {"QAM32"  , QAM_32  },
        {"QAM64"  , QAM_64  },
        {"QAM128" , QAM_128 },
        {"QAM256" , QAM_256 },
        #ifdef   SYS_DVBC2 //currently not supported by Linux DVB API
        {"QAM512" , QAM_512 },                    
        {"QAM1024", QAM_1024},
        {"QAM4096", QAM_4096},
        #endif
};

/* convert text to identifiers */

int txt_to_cable_fec ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(cable_fec_list); i++)
                if (! strcasecmp(txt, cable_fec_list[i].name))
                        return cable_fec_list[i].id;
        return FEC_AUTO; // fallback. should never happen.
}

int txt_to_cable_mod ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(cable_mod_list); i++)
                if (! strcasecmp(txt, cable_mod_list[i].name))
                        return cable_mod_list[i].id;
        return QAM_AUTO; // fallback. should never happen.
}

/*convert identifier to text */

const char * cable_fec_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(cable_fec_list); i++)
                if (id == cable_fec_list[i].id)
                        return cable_fec_list[i].name;
        return "AUTO"; // fallback. should never happen.
}

const char * cable_mod_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(cable_mod_list); i++)
                if (id == cable_mod_list[i].id)
                        return cable_mod_list[i].name;
        return "AUTO"; // fallback. should never happen.
}

/********************************************************************
 * ATSC
 ********************************************************************/


struct init_item atsc_mod_list[] = {
        {"QAM64" , QAM_64 },
        {"QAM256", QAM_256},
        {"8VSB"  , VSB_8  },
        {"16VSB" , VSB_16 },
};

/* convert text to identifiers */

int txt_to_atsc_mod ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(atsc_mod_list); i++)
                if (! strcasecmp(txt, atsc_mod_list[i].name))
                        return atsc_mod_list[i].id;
        return QAM_AUTO; // fallback. should never happen.
}

/*convert identifier to text */

const char * atsc_mod_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(atsc_mod_list); i++)
                if (id == atsc_mod_list[i].id)
                        return atsc_mod_list[i].name;
        return "AUTO"; // fallback. should never happen.
}

/********************************************************************
 * DVB-S
 ********************************************************************/

struct init_item sat_delivery_system_list[] = {
        {"S" , SYS_DVBS  },
        {"S1", SYS_DVBS  },
        {"S2", SYS_DVBS2 },
};

struct init_item sat_pol_list[] = {
        {"H", POLARIZATION_HORIZONTAL    },
        {"V", POLARIZATION_VERTICAL      },
        {"R", POLARIZATION_CIRCULAR_RIGHT},
        {"L", POLARIZATION_CIRCULAR_LEFT },
}; // NOTE: no AUTO used here.

struct init_item sat_fec_list[] = {
        {"NONE" , FEC_NONE},
        {"1/2"  , FEC_1_2 },
        {"2/3"  , FEC_2_3 },
        {"3/4"  , FEC_3_4 },
        {"4/5"  , FEC_4_5 },
        {"5/6"  , FEC_5_6 },
        {"6/7"  , FEC_6_7 },
        {"7/8"  , FEC_7_8 },
        {"8/9"  , FEC_8_9 },
        {"3/5"  , FEC_3_5 }, //S2
        {"9/10" , FEC_9_10}, //S2
        {"AUTO" , FEC_AUTO}
};

struct init_item sat_rolloff_list[] = {
        {"35"  , ROLLOFF_35},
        {"25"  , ROLLOFF_25},
        {"20"  , ROLLOFF_20},
        {"AUTO", ROLLOFF_AUTO},
}; // NOTE: "AUTO" == 0,35 in w_scan !

struct init_item sat_mod_list[] = {
        {"QPSK"  , QPSK   },
        {"8PSK"  , PSK_8  },
        {"16APSK", APSK_16},
        {"32APSK", APSK_32},
        {"AUTO"  , QPSK   },
}; // NOTE: "AUTO" == QPSK in w_scan !

/* convert text to identifiers */

int txt_to_sat_delivery_system ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(sat_delivery_system_list); i++)
                if (! strcasecmp(txt, sat_delivery_system_list[i].name))
                        return sat_delivery_system_list[i].id;
        return SYS_DVBS; // fallback. should never happen.
}

int txt_to_sat_pol ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(sat_pol_list); i++)
                if (! strcasecmp(txt, sat_pol_list[i].name))
                        return sat_pol_list[i].id;
        return POLARIZATION_HORIZONTAL; // fallback. should never happen.
}

int txt_to_sat_fec ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(sat_fec_list); i++)
                if (! strcasecmp(txt, sat_fec_list[i].name))
                        return sat_fec_list[i].id;
        return FEC_AUTO; // fallback. should never happen.
}

int txt_to_sat_rolloff ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(sat_rolloff_list); i++)
                if (! strcasecmp(txt, sat_rolloff_list[i].name))
                        return sat_rolloff_list[i].id;
        return ROLLOFF_35; // fallback. should never happen.
}

int txt_to_sat_mod ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(sat_mod_list); i++)
                if (! strcasecmp(txt, sat_mod_list[i].name))
                        return sat_mod_list[i].id;
        return QPSK; // fallback. should never happen.
}

/*convert identifier to text */

const char * sat_delivery_system_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(sat_delivery_system_list); i++)
                if (id == sat_delivery_system_list[i].id)
                        return sat_delivery_system_list[i].name;
        return "S"; // fallback. should never happen.
}

const char * sat_pol_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(sat_pol_list); i++)
                if (id == sat_pol_list[i].id)
                        return sat_pol_list[i].name;
        return "H"; // fallback. should never happen.
}

const char * sat_fec_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(sat_fec_list); i++)
                if (id == sat_fec_list[i].id)
                        return sat_fec_list[i].name;
        return "AUTO"; // fallback. should never happen.
}

const char * sat_rolloff_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(sat_rolloff_list); i++)
                if (id == sat_rolloff_list[i].id)
                        return sat_rolloff_list[i].name;
        return "35"; // fallback. should never happen.
}

const char * sat_mod_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(sat_mod_list); i++)
                if (id == sat_mod_list[i].id)
                        return sat_mod_list[i].name;
        return "QPSK"; // fallback. should never happen.
}

/********************************************************************
 * non-frontend specific part
 *
 ********************************************************************/

struct init_item scantype_list[] = {
        {"TERRCABLE_ATSC", SCAN_TERRCABLE_ATSC},
        {"CABLE",          SCAN_CABLE },
        {"TERRESTRIAL",    SCAN_TERRESTRIAL},
        {"SATELLITE",      SCAN_SATELLITE},
}; // NOTE: "AUTO" == SCAN_TERRESTRIAL in w_scan !

/* convert text to identifiers */

int txt_to_scantype ( const char * txt ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(scantype_list); i++)
                if (! strcasecmp(txt, scantype_list[i].name))
                        return scantype_list[i].id;
        return SCAN_TERRESTRIAL; // fallback. should never happen.
}

/*convert identifier to text */

const char * scantype_to_txt ( int id ) {
        unsigned int i;

        for (i = 0; i < STRUCT_COUNT(scantype_list); i++)
                if (id == scantype_list[i].id)
                        return scantype_list[i].name;
        return "TERRESTRIAL"; // fallback. should never happen.
}
