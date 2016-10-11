/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006 - 2014 Winfried Koehler 
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

#ifndef _TOOLS_H
#define _TOOLS_H

#include <stdint.h>
#include <time.h>     // link with -lrt

/*******************************************************************************
/* common typedefs && logging.
 ******************************************************************************/
#ifndef bool
  typedef int bool;
  #define false 0
  #define true  !(false)
#endif

#define min(a,b)  (b<a?b:a)
#define max(a,b)  (b>a?b:a)
#define diff(a,b) (a>b?(a-b):(b-a))

#define range(x,low,high) ((x>=low) && (x<=high))

extern int verbosity;

#define dprintf(level, fmt...)   \
   do {                          \
      if (level <= verbosity) {  \
         fprintf(stderr, fmt); } \
   } while (0)

#define dpprintf(level, fmt, args...) \
        dprintf(level, "%s:%d: " fmt, __FUNCTION__, __LINE__ , ##args)

#define fatal(fmt, args...)  do { dpprintf(-1, "FATAL: " fmt , ##args); exit(1); } while(0)
#define error(msg...)         dprintf(0, "\nERROR: " msg)
#define errorn(msg)           dprintf(0, "%s:%d: ERROR: " msg ": %d %s\n", __FUNCTION__, __LINE__, errno, strerror(errno))
#define warning(msg...)       dprintf(1, "WARNING: " msg)
#define info(msg...)          dprintf(2, msg)
#define verbose(msg...)       dprintf(3, msg)
#define moreverbose(msg...)   dprintf(4, msg)
#define debug(msg...)        dpprintf(5, msg)
#define verbosedebug(msg...) dpprintf(6, msg)

/*******************************************************************************
/* time functions.
 ******************************************************************************/

double elapsed (struct timespec * from, struct timespec * to);
void   get_time(struct timespec * dest);
void   set_timeout(uint16_t msec, struct timespec * dest);
int    timeout_expired(struct timespec * src);

/*******************************************************************************
/* debug helpers.
 ******************************************************************************/
void  run_time_init();
const char * run_time();
void  hexdump(const char * intro, const unsigned char * buf, int len);

const char * inversion_name(int inversion);
const char * coderate_name(int coderate);
const char * modulation_name(int modulation);
const char * transmission_mode_name(int transmission_mode);
const char * guard_interval_name(int guard_interval);
const char * hierarchy_name(int hierarchy);
const char * interleaving_name(int interleaving);
const char * delivery_system_name(int delsys);
const char * bool_name(bool t);
uint32_t freq_scale(uint32_t freq, double scale);

const char * alpha_name(int alpha);   /* somehow missing. */
const char * interleaver_name(int i); /* somehow missing. */

/*******************************************************************************
/* double linked list.
 ******************************************************************************/

typedef int  (*cmp_func) (void * a, void * b);
typedef bool (*fnd_func) (void * a);

typedef struct {
   void * first;
   void * last;
   uint32_t count;
   char * name;
   bool lock;
   } cList, * pList;

typedef struct {
   void * prev;
   void * next;
   uint32_t index;
   } cItem, * pItem;

void   NewList(pList list, const char * name);
void   ClearList(pList list);
void   SortList(pList list, cmp_func compare);
void   AddItem(pList list, void * item);
void   DeleteItem(pList list, void * item);
void   SwapItem(pList list, pItem a, pItem b);
void   UnlinkItem(pList list, void * item, bool freemem);
void   InsertItem(pList list, void * item, uint32_t index);
void * GetItem(pList list, uint32_t index);
bool   IsMember(pList list, void * item);

/*******************************************************************************
/* fuzzy bit error recovery.
 ******************************************************************************/

bool fuzzy_section(void * s);

#endif
