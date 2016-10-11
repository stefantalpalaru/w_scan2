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
 */



/* this file is shared between w_scan and the VDR plugin wirbelscan.
 * For details on both of them see http://wirbel.htpc-forum.de
 */


#ifndef _EXTENDED_DVBFRONTEND_H_
#define _EXTENDED_DVBFRONTEND_H_

#include <linux/dvb/frontend.h>
#include <linux/dvb/version.h>

#define DVB_API ( 10 * DVB_API_VERSION + DVB_API_VERSION_MINOR )

/******************************************************************************
 * definitions which are missing in <linux/dvb/frontend.h>
 *
 *****************************************************************************/

#ifndef fe_polarization // 300468 v181  6.2.13.2 Satellite delivery system descriptor
typedef enum fe_polarization {
        POLARIZATION_HORIZONTAL,
        POLARIZATION_VERTICAL,
        POLARIZATION_CIRCULAR_LEFT,
        POLARIZATION_CIRCULAR_RIGHT,
} fe_polarization_t;
#endif

#ifndef fe_west_east_flag       // 300468 v181  6.2.13.2 Satellite delivery system descriptor
typedef enum fe_west_east_flag {
        EAST_FLAG,
        WEST_FLAG,
} fe_west_east_flag_t;
#endif

#ifndef fe_interleave   // 300468 v181  6.2.13.4 Terrestrial delivery system descriptor
typedef enum fe_interleaver {
        INTERLEAVE_NATIVE,
        INTERLEAVE_IN_DEPTH,
        INTERLEAVE_AUTO,
} fe_interleave_t;
#endif

#ifndef fe_alpha        // 300468 v181  6.2.13.4 Terrestrial delivery system descriptor
typedef enum fe_alpha {
        ALPHA_1,
        ALPHA_2,
        ALPHA_4,
        ALPHA_AUTO,
} fe_alpha_t;
#endif

typedef enum fe_siso_miso {
        SISO,
        MISO,
        SISO_MISO_RESERVED1,
        SISO_MISO_RESERVED2,
} fe_siso_miso_t;

#ifndef SYS_DVBC2                              //// \BEGIN   FIX_ME: _REALLY_ DIRTY HACK. JUST FOR TESTING THIS CODE. REMOVE LATER!!!!!
#define SYS_DVBC2 (63U)  /* max value in use: 18U */
#ifndef QAM_512
#define QAM_512   (61U)  /* max value in use: 13U */
#endif
#ifndef QAM_1024
#define QAM_1024  (62U)  /* max value in use: 13U */
#endif
#ifndef QAM_4096
#define QAM_4096  (63U)  /* max value in use: 13U */
#endif
#ifndef GUARD_INTERVAL_1_64
#define GUARD_INTERVAL_1_64 (127U) /* max value in use: 10U */
#endif
#endif                                         //// \END     REMOVE UP TO HERE!!!

#ifndef SYS_TURBO  // remove later.
#define SYS_TURBO (SYS_DVBT2 + 1) /* correct in any case. */
#endif

#ifndef DTV_ENUM_DELSYS // remove later.
#define DTV_ENUM_DELSYS 44
#endif

#if !defined DTV_STREAM_ID && defined DTV_DVBT2_PLP_ID
#define DTV_STREAM_ID DTV_DVBT2_PLP_ID
#endif

typedef enum {
        DATA_SLICE_TUNING_FREQUENCY,
        C2_SYSTEM_CENTER_FREQUENCY,
        INITIAL_TUNING_FOR_STATIC_DATA_SLICE,
} fe_frequency_type_t;

typedef enum {
        FFT_4K_8MHZ,
        FFT_4K_6MHZ,
} fe_ofdm_symbol_duration_t;

/* don't use FE_OFDM, FE_QAM, FE_QPSK, FE_ATSC any longer.
 *
 * With evolving delivery systems, cable uses also ofdm (DVB-C2), satellites doesnt
 * use only qpsk (DVB-S2), 2nd gen terrestrial and cable use physical layer pipes.
 * Use of delivery_system doesnt fit for me either, since i dont want to distinguish
 * between DVB-S <-> DVB-S2 or DVB-C <-> DVB-C2 or DVB-T <-> DVB-T2.
 * Therefore, name it on physical path for now.
 * 20120107 wirbel
 */
typedef enum {
        SCAN_UNDEFINED,
        SCAN_SATELLITE,
        SCAN_CABLE,
        SCAN_TERRESTRIAL,
        SCAN_TERRCABLE_ATSC,   /* I dislike this mixture of terr and cable. fix later, as it leads to problems now. */
} scantype_t;

#endif
