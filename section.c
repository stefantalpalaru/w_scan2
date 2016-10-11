/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006, 2007, 2008 Winfried Koehler 
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

#include "section.h"

/* shamelessly stolen from dvbsnoop, but modified */
u32 getBits (const u8 *buf, int startbit, int bitlen)
{
        const u8 *b;
        u32 mask,tmp_long;
        int bitHigh,i;

        b = &buf[startbit / 8];
        startbit %= 8;

        bitHigh = 8;
        tmp_long = b[0];
        for (i = 0; i < ((bitlen-1) >> 3); i++) {
                tmp_long <<= 8;
                tmp_long  |= b[i+1];
                bitHigh   += 8;
        }

        startbit = bitHigh - startbit - bitlen;
        tmp_long = tmp_long >> startbit;
        mask     = (1ULL << bitlen) - 1;
        return tmp_long & mask;
}
