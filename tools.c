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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "scan.h"
#include "tools.h"

/*******************************************************************************
/* common typedefs && logging.                                                  
 ******************************************************************************/
int verbosity = 2;      // need signed -> use of fatal()



/*******************************************************************************
 * new implementation of double linked list since 20140118.
 *
 ******************************************************************************/
// #define LIST_DEBUG 1

#ifdef LIST_DEBUG
#define dbg(s...) info(s)

// debugging purposes only. do not use in distributed versions.
void report(pList list) {
  dbg("--------------------------------------------------------------\n");
  dbg("list '%s'@%p: count=%u; first=%p; last=%p\n",
       list->name, list, list->count, list->first, list->last);

  pItem p = list->first;
  while (p != NULL) {
     verbose("    item%.2u: prev = %p, ptr = %p: next = %p\n",
          p->index, p->prev, p, p->next);
     p = p->next;
     }
  dbg("--------------------------------------------------------------\n");
}
#else
#define dbg(s...)
#define report(s...)
#endif


#if 0
// an compare function for testing purposes only.
int alphabetically(void * a, void * b, int ascending) {
  pStringItem s_a, s_b;
  s_a = a;
  s_b = b;
  if (ascending != 0)
     return strcmp(s_a->buf, s_b->buf,255) > 0;
  else
     return strcmp(s_a->buf, s_b->buf,255) < 0;
}
#endif

// initializes a list before first use
void NewList(pList list, const char * name) {
  dbg("%s %d: list:'%s'\n", __FUNCTION__,__LINE__,name);
  list->first   = NULL;
  list->last    = NULL;
  list->count   = 0;
  list->name    = calloc(1,strlen(name) + 1);
  sprintf(list->name, "%s", name);
  report(list);
}

// returns true, if a pointer is part of list.
bool IsMember(pList list, void * item) {
  pItem p;
  for(p = list->first; p; p = p->next) {
     if (p == item) {
        return true;
        }
     }
  return false;
}

// remove all items from list && free allocated memory.
void ClearList(pList list) {
  while (list->lock);
  list->lock = true;
  dbg("%s %d: list:'%s'\n", __FUNCTION__,__LINE__,list->name);
  pItem p = list->last;

  while (p != NULL) {
     list->count--;
     list->last=p->prev;
     free(p);
     p=list->last;
     if (p != NULL) { 
        p->next=NULL;
        }
     }
  list->first=NULL;
  list->lock=false;
  report(list);
}

// returns item specified by zero-based index.
void * GetItem(pList list, uint32_t index) {
  dbg("%s %d: list:'%s'\n",
     __FUNCTION__,__LINE__,list->name);
  pItem p;  
  for(p = list->first; p; p = p->next) {
     dbg("    item%.2u: (prev=%p, p=%p, next=%p)\n", p->index,p->prev,p,p->next);
     if (p->index == index)
        return p;
     }
  return NULL;
}

// append item at end of list.
void AddItem(pList list, void * item) {
  pItem p = item;
  while (list->lock);
  list->lock = true;

  dbg("%s %d: list:'%s' add item: (prev=%p, p=%p, next=%p)\n",
     __FUNCTION__,__LINE__,list->name,p->prev,p,p->next);

  p->index = list->count;
  p->prev  = list->last;
  p->next  = NULL;

  if (list->count == 0) {
     list->first = p;
     }
  else {
     p = list->last;
     p->next=item;
     }

  list->last = item;
  list->count++;
  list->lock = false;
  report(list);
}

// insert item to list. index is zero-based pos of new item.
// if index greater as (list.count-1), item will be appended instead.
void InsertItem(pList list, void * item, uint32_t index) {
  while (list->lock);
  list->lock = true;

  dbg("%s %d: list:'%s' item=%p, index=%u\n",
      __FUNCTION__,__LINE__,list->name, item, index);

  pItem prev,next,p = item;
  p->index = 0;
  p->prev = NULL;
  p->next = NULL;

  if (index >= list->count) {
     dbg("insert at end of list.\n");
     AddItem(list,item);
     }
  else if (index == 0) {
     dbg("insert at begin of list.\n");
     p->prev = NULL;
     p->next = list->first;
     p->index = 0;
     next = list->first;
     next->prev = p;
     p = list->first;
     while (p != NULL) {
        p->index++;
        p = p->next;
        }
     list->first = item;
     list->count++;
     }
  else {
     dbg("insert somewhere in the middle.\n");
     next = GetItem(list, index);
     prev = next->prev;
     prev->next = p;
     next->prev = p;
     p->prev = prev;
     p->next = next;
     p->index = prev->index;
     list->count++;
     while (p != NULL) {
        p->index++;
        p = p->next;
        }
     }
  list->lock = false;
  report(list);
}

// remove item from list. free allocated memory if release_mem non-zero.
void UnlinkItem(pList list, void * item, bool freemem) {
  while (list->lock);
  list->lock = true;

  pItem prev,next,p = item;

  dbg("%s %d: list:'%s' item=%p, freemem = %d\n",
     __FUNCTION__, __LINE__, list->name, item, freemem);
  if (IsMember(list, item) == false) {
     warning("Cannot %s: item %p is not member of list %s.\n",
              freemem?"delete":"unlink", item, list->name);
     return;
     }
  else if (item == list->first) {
     dbg("delete at begin of list.\n");
     list->first = p->next;
     list->count--;
     if (freemem) {
        free(p);
        }
     p = list->first;
     if (p != NULL)
        p->prev = NULL;
     while (p != NULL) {
        p->index--;
        p = p->next;
        }
     }
  else if (item == list->last) {
     dbg("delete at end of list.\n");
     list->last = p->prev;
     list->count--;
     if (freemem) {
        free(p);
        }
     p = list->last;
     if (p != NULL) {
        p->next = NULL;
        }
     }
  else {
     dbg("delete somewhere in the middle.\n");
     prev = p->prev;
     next = p->next;
     prev->next = next;
     next->prev = prev;
     list->count--;
     if (freemem) {
        free(p);
        }
     p = next;
     while (p != NULL) {
        p->index--;
        p = p->next;
        }
     }
  list->lock = false;
}

// remove item from list and free allocated memory.
void DeleteItem(pList list, void * item) {
  dbg("%s %d: list:'%s' item=%p\n", __FUNCTION__, __LINE__, list->name, index);
  UnlinkItem(list, item, true);
}

// exchange two items in list.
void SwapItem(pList list, pItem a, pItem b) {
  while (list->lock);
  list->lock = true;

  dbg("%s %d: list:'%s' a:(prev=%p,p=%p,next=%p) <-> b:(prev=%p,p=%p,next=%p)\n",
     __FUNCTION__, __LINE__, list->name, a->prev,a,a->next, b->prev,b,b->next);
  uint32_t index_a, index_b;
  if (a == b) {
     list->lock = false;
     return;
     }
  
  index_a = a->index;
  index_b = b->index;

  if (index_a < index_b) {
     UnlinkItem(list,b,0);
     UnlinkItem(list,a,0);
     InsertItem(list,b,index_a);
     InsertItem(list,a,index_b);
     }
  else {
     UnlinkItem(list,a,0);
     UnlinkItem(list,b,0);
     InsertItem(list,a,index_b);
     InsertItem(list,b,index_a);
     }
  list->lock = false;
}

// sort the list. assign sort criteria function
// 'compare' to list before first use.
// warning: procedure is *slow* for large lists. 
void SortList(pList list, cmp_func compare) {
  dbg("%s %d: list:'%s'\n",__FUNCTION__, __LINE__, list->name);
  pItem c,d;
  if (compare == NULL) {
     warning("sort function not assigned.\n");
     return;
     }

  redo:
  c = list->first;
  while (c != NULL) {
     d = c;
     while (d != NULL) {
        if (d->next == NULL) {
           break;
           }
        if (compare(d, d->next) > 0) {
           SwapItem(list, d, d->next);
           goto redo;
           }
        d = d->next;
        }
     c = c->next;
     }
}

void * FindItem(pList list, void * prev, fnd_func criteria) {
  pItem p;  
  for(p = prev ? prev : list->first; p; p = p->next) {
     if (criteria(p))
        return p;
     }
  return NULL;
}


/*******************************************************************************
 * time related support functions.
 *
 * NOTE: clock_gettime needs linking against librt.
 *       Therefore librt is dependency for w_scan > 20140118.
 ******************************************************************************/

#ifdef CLOCK_MONOTONIC_COARSE
#define CLK_SPEC CLOCK_MONOTONIC_COARSE  /* faster, but only linux since kernel 2.6.32 */
#else
#define CLK_SPEC CLOCK_MONOTONIC
#endif


double elapsed(struct timespec * from, struct timespec * to) {
  double Result;
  int32_t nsec  = to->tv_nsec - from->tv_nsec;
  if (nsec < 0) {
     Result = -1.0 + to->tv_sec  - from->tv_sec;
     nsec += 1000000000;
     }
  else
     Result = to->tv_sec  - from->tv_sec;
  Result += (nsec / 1e9);
  return Result;
}

void get_time(struct timespec * dest) {
  clock_gettime(CLK_SPEC, dest);
}

void set_timeout(uint16_t msec, struct timespec * dest) {
  struct timespec t;
  uint32_t nsec;
  uint8_t  sec;

  clock_gettime(CLK_SPEC, &t);
  sec  = (t.tv_nsec + msec * 1000000U) / 1000000000U;
  nsec = (t.tv_nsec + msec * 1000000U) % 1000000000U;
  dest->tv_sec  = t.tv_sec + sec;
  dest->tv_nsec = nsec;
//dbg("now = %ld.%.9li timeout = %ld.%.9li\n", t.tv_sec, t.tv_nsec, dest->tv_sec, dest->tv_nsec);
}

int timeout_expired(struct timespec * src) {
  struct timespec t;
  int expired;
  clock_gettime(CLK_SPEC, &t);

  expired = (t.tv_sec > src->tv_sec) ||
            ((t.tv_sec == src->tv_sec) && (t.tv_nsec > src->tv_nsec));
//dbg("now = %ld.%.9li; expired=%d\n", t.tv_sec, t.tv_nsec, expired);
  return expired;
}

/*******************************************************************************
/* debug helpers.
 ******************************************************************************/
struct timespec starttime = { 0, 0 };

void run_time_init() {
  get_time(&starttime);
}

const char * run_time() {
  static char rtbuf[12];
  struct timespec now;
  double t;
  int sec, msec;
  get_time(&now);
  t = elapsed(&starttime,&now);

  sec = (int) t;
  msec = 1000.0 * (t - sec);
  sprintf(&rtbuf[0], "%.2d:%.2d.%.3d", sec / 60, sec % 60, msec);
  return &rtbuf[0];
}

void hexdump(const char * intro, const unsigned char * buf, int len) {
  int i, j;
  char sbuf[17];

  if (verbosity < 4)
     return;

  memset(&sbuf, 0, 17);

  info("\t===================== %s ", intro);
  for(i = strlen(intro) + 1; i < 50; i++)
     info("=");
  info("\n");
  info("\tlen = %d\n", len);
  for(i = 0; i < len; i++) {
     if ((i % 16) == 0) {
        info("%s0x%.2X: ",i?"\n\t":"\t",(i / 16) * 16);
        }
     info("%.2X ", (uint8_t) *(buf + i));
     sbuf[i % 16] = *(buf + i);
     if (((i + 1) % 16) == 0) {
        // remove non-printable chars
        for(j = 0; j < 16; j++)
           if (! ((sbuf[j] > 31)  && (sbuf[j] < 127)))
              sbuf[j] = ' ';
        
        info(": %s", sbuf);
        memset(&sbuf, 0, 17);
        }
     }
  if (len % 16) {
     for(i = 0; i < (len % 16); i++)
        if (! ((sbuf[i] > 31)  && (sbuf[i] < 127)))
           sbuf[i] = ' ';
     for(i = (len % 16); i < 16; i++)
        info("   ");
     info(": %s", sbuf);
     }
  info("\n");
  info("\t========================================================================\n");
}

const char * inversion_name(int inversion) {
  switch(inversion) {
     case INVERSION_OFF: return "INVERSION_OFF";
     case INVERSION_ON:  return "INVERSION_ON";
     default:            return "INVERSION_AUTO";
     }
}

const char * coderate_name(int coderate) {
  switch(coderate) {
     case FEC_NONE:  return "FEC_NONE";
     case FEC_2_5:   return "FEC_2_5";
     case FEC_1_2:   return "FEC_1_2";
     case FEC_3_5:   return "FEC_3_5";
     case FEC_2_3:   return "FEC_2_3";
     case FEC_3_4:   return "FEC_3_4";
     case FEC_4_5:   return "FEC_4_5";
     case FEC_5_6:   return "FEC_5_6";
     case FEC_6_7:   return "FEC_6_7";
     case FEC_7_8:   return "FEC_7_8";
     case FEC_8_9:   return "FEC_8_9";
     case FEC_9_10:  return "FEC_9_10";
     default:        return "FEC_AUTO";
     }
}

const char * modulation_name(int modulation) {
  switch(modulation) {
     case QPSK       : return "QPSK";
     case QAM_16     : return "QAM_16";
     case QAM_32     : return "QAM_32";
     case QAM_64     : return "QAM_64";
     case QAM_128    : return "QAM_128";
     case QAM_256    : return "QAM_256";
     case QAM_AUTO   : return "QAM_AUTO";
     case VSB_8      : return "VSB_8";
     case VSB_16     : return "VSB_16";
     case PSK_8      : return "PSK_8";
     case APSK_16    : return "APSK_16";
     case APSK_32    : return "APSK_32";
     case DQPSK      : return "DQPSK";
     case QAM_4_NR   : return "QAM_4_NR";
     default         : return "QAM_AUTO";
     }
}

const char * transmission_mode_name(int transmission_mode) {
  switch(transmission_mode) {
     case TRANSMISSION_MODE_1K    : return "TRANSMISSION_MODE_1K";
     case TRANSMISSION_MODE_2K    : return "TRANSMISSION_MODE_2K";
     case TRANSMISSION_MODE_4K    : return "TRANSMISSION_MODE_4K";
     case TRANSMISSION_MODE_8K    : return "TRANSMISSION_MODE_8K";
     case TRANSMISSION_MODE_16K   : return "TRANSMISSION_MODE_16K";
     case TRANSMISSION_MODE_32K   : return "TRANSMISSION_MODE_32K";
     case TRANSMISSION_MODE_C1    : return "TRANSMISSION_MODE_C1"; 
     case TRANSMISSION_MODE_C3780 : return "TRANSMISSION_MODE_C3780";
     default                      : return "TRANSMISSION_MODE_AUTO";
     }
}

const char * guard_interval_name(int guard_interval) {
  switch(guard_interval) {
     case GUARD_INTERVAL_1_32   : return "GUARD_INTERVAL_1_32";
     case GUARD_INTERVAL_1_16   : return "GUARD_INTERVAL_1_16";
     case GUARD_INTERVAL_1_8    : return "GUARD_INTERVAL_1_8";
     case GUARD_INTERVAL_1_4    : return "GUARD_INTERVAL_1_4";
     case GUARD_INTERVAL_1_128  : return "GUARD_INTERVAL_1_128";
     case GUARD_INTERVAL_19_128 : return "GUARD_INTERVAL_19_128";
     case GUARD_INTERVAL_19_256 : return "GUARD_INTERVAL_19_256";
     case GUARD_INTERVAL_PN420  : return "GUARD_INTERVAL_PN420";
     case GUARD_INTERVAL_PN595 : return "GUARD_INTERVAL_PN595";
     case GUARD_INTERVAL_PN945 : return "GUARD_INTERVAL_PN945";
     default                    : return "GUARD_INTERVAL_AUTO";
     }
}

const char * hierarchy_name(int hierarchy) {
  switch(hierarchy) {
     case HIERARCHY_NONE     : return "HIERARCHY_NONE";
     case HIERARCHY_1        : return "HIERARCHY_1";
     case HIERARCHY_2        : return "HIERARCHY_2";
     case HIERARCHY_4        : return "HIERARCHY_4";
     default                 : return "HIERARCHY_AUTO";
     }
}

const char * interleaving_name(int interleaving) {
  switch(interleaving) {
     case INTERLEAVING_NONE : return "INTERLEAVING_NONE";
     case INTERLEAVING_240  : return "INTERLEAVING_240";
     case INTERLEAVING_720  : return "INTERLEAVING_720";
     default:                 return "INTERLEAVING_AUTO";
     }
}

const char * delivery_system_name(int delsys) {
  switch(delsys) {
	  case SYS_DVBC_ANNEX_A  : return "SYS_DVBC_ANNEX_A";
	  case SYS_DVBC_ANNEX_B  : return "SYS_DVBC_ANNEX_B";
	  case SYS_DVBT          : return "SYS_DVBT";
	  case SYS_DSS           : return "SYS_DSS";
	  case SYS_DVBS          : return "SYS_DVBS";
	  case SYS_DVBS2         : return "SYS_DVBS2";
	  case SYS_DVBH          : return "SYS_DVBH";
	  case SYS_ISDBT         : return "SYS_ISDBT";
	  case SYS_ISDBS         : return "SYS_ISDBS";
	  case SYS_ISDBC         : return "SYS_ISDBC";
	  case SYS_ATSC          : return "SYS_ATSC";
	  case SYS_ATSCMH        : return "SYS_ATSCMH";
	  case SYS_DTMB          : return "SYS_DTMB";
	  case SYS_CMMB          : return "SYS_CMMB";
	  case SYS_DAB           : return "SYS_DAB";
	  case SYS_DVBT2         : return "SYS_DVBT2";
	  case SYS_TURBO         : return "SYS_TURBO";
	  case SYS_DVBC_ANNEX_C  : return "SYS_DVBC_ANNEX_C";
	  default:                 return "SYS_UNDEFINED";
     }
}

const char * bool_name(bool t) {
  if (t == false) return "false";
  return "true";
}

uint32_t freq_scale(uint32_t freq, double scale) {
  return (uint32_t) (0.5 + freq * scale);
}

const char * alpha_name(int alpha) {
  switch(alpha) {
     case ALPHA_1 : return "ALPHA_1";
     case ALPHA_2 : return "ALPHA_2";
     case ALPHA_4 : return "ALPHA_4";
     default:       return "ALPHA_AUTO";
     }
}

const char * interleaver_name(int i) {
  switch(i) {
     case INTERLEAVE_NATIVE   : return "INTERLEAVE_NATIVE";
     case INTERLEAVE_IN_DEPTH : return "INTERLEAVE_IN_DEPTH";
     default:                   return "INTERLEAVE_AUTO";
     }
}

/*******************************************************************************
/* fuzzy bit error recovery.
 ******************************************************************************/
#include "si_types.h"

typedef struct {
   void * prev;
   void * next;
   uint32_t index;
   uint8_t value;
   uint8_t count;
} byte_item;

static int sort_by_count(void * a, void * b) {
  return ((byte_item *) a)->count > ((byte_item *) b)->count; 
}

bool fuzzy_section(void * s) {
  struct section_buf * section = (struct section_buf *) s;
  cList current_byte;
  unsigned char * buf;
  unsigned i, j;  

  if (section->garbage == NULL)
     return false;

  buf = (unsigned char *) calloc(1, SECTION_BUF_SIZE);
  for(i = 0; i < SECTION_BUF_SIZE; i++) {
    byte_item * bi;
    NewList(&current_byte, "fuzzy_section: current_byte");
    
    for(j = 0; j < (section->garbage)->count; j++) {
       buf = GetItem(section->garbage,j) + sizeof(cList);
       for(bi = current_byte.first; bi; bi = bi->next) {
          if (bi->value == buf[i]) {
             bi->count++;
             break;
             }
          }
       if (bi == NULL) {
          bi = (byte_item *) calloc(1, sizeof(*bi));
          bi->value = buf[i];
          bi->count++;
          AddItem(&current_byte, bi);
          }
       }
    SortList(&current_byte, &sort_by_count);
    buf[i] = ((byte_item *) current_byte.first)->value;
    }
 
  hexdump(__FUNCTION__,buf,1024);
  return false; // fail.
}
