#ifndef __EMULATE_H__
#define __EMULATE_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <linux/dvb/frontend.h>
#include "si_types.h"


void em_init(const char * log); 
void em_open(int * frontend_fd);
void em_info(struct dvb_frontend_info * fe_info);
void em_dvbapi(uint16_t * flags);
int  em_setproperty(struct dtv_properties * cmdseq);
int  em_getproperty(struct dtv_properties * cmdseq);
int  em_status(fe_status_t * status);
void em_lnb(bool high_band, uint32_t high_val, uint32_t low_val);
void em_polarization(uint8_t p);

//--------------------------------------------------
void em_addfilter(struct section_buf * s);
void em_readfilters(int * result);

#endif
