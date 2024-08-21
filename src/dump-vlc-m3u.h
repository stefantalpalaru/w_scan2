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

#ifndef __DUMP_VLC_M3U_H__
#define __DUMP_VLC_M3U_H__

/* 20110702 --wk */

#include <stdint.h>
#include "extended_frontend.h"
#include "si_types.h"
#include "scan.h"
#include "lnb.h"

void vlc_xspf_prolog(FILE *f, uint16_t adapter, uint16_t frontend, struct w_scan_flags *flags, struct lnb_types_st *lnbp);

void vlc_dump_service_parameter_set_as_xspf(
    FILE *f,
    struct service *s,
    struct transponder *t,
    struct w_scan_flags *flags,
    struct lnb_types_st *lnbp);

void vlc_xspf_epilog(FILE *f);

#endif
