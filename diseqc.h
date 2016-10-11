/* This file was taken from scan-s2, and changed to use with w_scan.
 */

#ifndef __DISEQC_H__
#define __DISEQC_H__

#include <stdint.h>
#include "extended_frontend.h"
#include "si_types.h"
#include "lnb.h"

struct diseqc_cmd {
        struct dvb_diseqc_master_cmd cmd;
        uint32_t wait;
};

extern int diseqc_send_msg (int fd, fe_sec_voltage_t v, struct diseqc_cmd **cmd,
                                                        fe_sec_tone_mode_t t, fe_sec_mini_cmd_t b);


/*
*   set up the switch to position/voltage/tone
*/
int setup_switch (int frontend_fd, int switch_pos, int voltage_18, int freq, int uncommitted_switch_pos);
int rotate_rotor (int frontend_fd, int * from, int to, uint8_t voltage_18, uint8_t hiband);

int setup_scr(int frontend_fd, struct transponder * t, struct lnb_types_st * lnb, struct scr * config);

#endif

