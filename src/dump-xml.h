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
#ifndef __DUMP_XML_H_
#define __DUMP_XML_H_

/* 20140628 --wk */
#include <stdio.h>
#include "tools.h"
#include "scan.h"

void xml_dump(FILE *dest, pList transponders, struct w_scan_flags *flags);

#endif
