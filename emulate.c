#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>
#include "emulate.h"

#include "dump-vdr.h"   // debugging transponder.
#include "dump-xine.h"  // debugging transponder.
#include "dvbscan.h"    // debugging transponder.
#include "tools.h"      // hexdump

#define Hz   1
#define kHz (1000 * Hz)
#define MHz (1000 * kHz)

//#define EM_INFO(msg...) info(msg)
#define EM_INFO(msg...)

/* this struct stores the contents (DVB SI data) of an section buffer,
 * alltogether with the current tuning state of the dvb device.
 */
typedef struct {
  /*----------------------------*/
  void * prev;
  void * next;
  uint32_t index;
  /*----------------------------*/
  uint16_t pid;
  uint16_t table_id;
  uint16_t table_id_ext;
  uint16_t original_network_id;
  uint16_t network_id;
  uint16_t transport_stream_id;
  uint16_t service_id;
  uint16_t len;
  struct transponder t;
  unsigned char buf[SECTION_BUF_SIZE];
} sidata_t;

 /*
  * list of DVB SI data and list of running demux filters.
  */
cList __em_buf1, * em_runningfilters = &__em_buf1;
cList __em_buf2, * em_sidata = &__em_buf2;

 /*
  * drivers DVB API.
  * NOTE:
  *   - API defaults to 5.3
  *   - if not overwritten from log -> bug.
  */
struct {
  unsigned major;
  unsigned minor;
} em_api = {5, 3};

 /*
  * The emulated DVB device.
  * NOTE:
  *   - state is set by em_setproperty and returned by em_getproperty
  *   - not to be exposed outside this file.
  */
static struct {
   fe_delivery_system_t     delsys;
   uint32_t                 frequency;
   uint32_t                 bandwidth_hz;
   uint32_t                 symbolrate;
   uint8_t                  stream_id;
   fe_spectral_inversion_t  inversion;
   fe_modulation_t          modulation;
   fe_code_rate_t           fec;
   fe_pilot_t               pilot;
   fe_rolloff_t             rolloff;
   fe_transmit_mode_t       transmission;
   fe_guard_interval_t      guard;
   fe_hierarchy_t           hierarchy;
   fe_polarization_t        polarization;
   struct dvb_frontend_info fe_info;
   fe_delivery_system_t     delsystems[32];
   uint8_t                  ndelsystems;
   scantype_t               scantype;
   bool                     T2_delsys_bug;
   bool                     highband;
   uint32_t                 lnb_low;
   uint32_t                 lnb_high;
} em_device;


/*
 * forward declarations.
 */
static int parse_logfile(const char * log);

// Declare parse_xyz in scan.h? Hmm..
extern void parse_pat     (const unsigned char * buf, uint16_t section_length, uint16_t transport_stream_id, uint32_t flags);
extern void parse_pmt     (const unsigned char * buf, uint16_t section_length, uint16_t service_id);
extern void parse_nit     (const unsigned char * buf, uint16_t section_length, uint8_t table_id, uint16_t network_id, uint32_t section_flags);
extern void parse_sdt     (const unsigned char * buf, uint16_t section_length, uint16_t transport_stream_id);
extern void parse_psip_vct(const unsigned char * buf, uint16_t section_length, uint8_t table_id, uint16_t transport_stream_id); 


 /*
  * initializes emulated dvb device and fills in data by parsing logfile.
  */
void em_init(const char * log) {
  NewList(em_runningfilters, "em_runningfilters");
  NewList(em_sidata, "em_sidata");
  memset(&em_device, 0, sizeof(em_device));
  parse_logfile(log);
}

 /*
  * let 'frontend_fd' be any non-negative int to pass validity checks.
  */
void em_open(int * frontend_fd) {
  *frontend_fd = 1;
}

 /*
  * replaces FE_GET_INFO ioctl.
  */
void em_info(struct dvb_frontend_info * fe_info) {
  memcpy(fe_info, &em_device.fe_info, sizeof(struct dvb_frontend_info));
}

 /*
  * replaces DTV_API_VERSION ioctl.
  */
void em_dvbapi(uint16_t * flags) {
  *flags = (em_api.major << 8) | em_api.minor;
}

 /*
  * replaces FE_SET_PROPERTY ioctl.
  */
int  em_setproperty(struct dtv_properties * cmdseq) {
  uint8_t i;
  for (i=0; i<cmdseq->num; i++) {
     switch(cmdseq->props[i].cmd) {
        case DTV_DELIVERY_SYSTEM:   em_device.delsys             = cmdseq->props[i].u.data; break;
        case DTV_FREQUENCY:         em_device.frequency          = cmdseq->props[i].u.data; break;
        case DTV_INVERSION:         em_device.inversion          = cmdseq->props[i].u.data; break;
        case DTV_MODULATION:        em_device.modulation         = cmdseq->props[i].u.data; break;
        case DTV_SYMBOL_RATE:       em_device.symbolrate         = cmdseq->props[i].u.data; break;
        case DTV_INNER_FEC:         em_device.fec                = cmdseq->props[i].u.data; break;
        case DTV_PILOT:             em_device.pilot              = cmdseq->props[i].u.data; break;
        case DTV_ROLLOFF:           em_device.rolloff            = cmdseq->props[i].u.data; break;
        case DTV_STREAM_ID:         em_device.stream_id          = cmdseq->props[i].u.data; break;
        case DTV_BANDWIDTH_HZ:      em_device.bandwidth_hz       = cmdseq->props[i].u.data; break;
        case DTV_CODE_RATE_HP:      em_device.fec                = cmdseq->props[i].u.data; break;
        case DTV_TRANSMISSION_MODE: em_device.transmission       = cmdseq->props[i].u.data; break;
        case DTV_GUARD_INTERVAL:    em_device.guard              = cmdseq->props[i].u.data; break;
        case DTV_HIERARCHY:         em_device.hierarchy          = cmdseq->props[i].u.data; break;
        case DTV_CLEAR:                                                                     break; // what to do with clear?
        default:;
        }
     }
  return 0; // no err.
}

 /*
  * replaces FE_GET_PROPERTY ioctl.
  */
int  em_getproperty(struct dtv_properties * cmdseq) {
  uint8_t i;
  int j,k;
  for (i=0; i<cmdseq->num; i++) {
     switch(cmdseq->props[i].cmd) {
        case DTV_DELIVERY_SYSTEM:   cmdseq->props[i].u.data = em_device.delsys           ; break;
        case DTV_FREQUENCY:         cmdseq->props[i].u.data = em_device.frequency        ; break;
        case DTV_INVERSION:         cmdseq->props[i].u.data = em_device.inversion        ; break;
        case DTV_MODULATION:        cmdseq->props[i].u.data = em_device.modulation       ; break;
        case DTV_SYMBOL_RATE:       cmdseq->props[i].u.data = em_device.symbolrate       ; break;
        case DTV_INNER_FEC:         cmdseq->props[i].u.data = em_device.fec              ; break;
        case DTV_PILOT:             cmdseq->props[i].u.data = em_device.pilot            ; break;
        case DTV_ROLLOFF:           cmdseq->props[i].u.data = em_device.rolloff          ; break;
        case DTV_STREAM_ID:         cmdseq->props[i].u.data = em_device.stream_id        ; break;
        case DTV_BANDWIDTH_HZ:      cmdseq->props[i].u.data = em_device.bandwidth_hz     ; break;
        case DTV_CODE_RATE_HP:      cmdseq->props[i].u.data = em_device.fec              ; break;
        case DTV_TRANSMISSION_MODE: cmdseq->props[i].u.data = em_device.transmission     ; break;
        case DTV_GUARD_INTERVAL:    cmdseq->props[i].u.data = em_device.guard            ; break;
        case DTV_HIERARCHY:         cmdseq->props[i].u.data = em_device.hierarchy        ; break;
        case DTV_ENUM_DELSYS:       cmdseq->props[i].u.buffer.len = em_device.ndelsystems;
                                    for(j = 0, k = em_device.ndelsystems - 1; j < em_device.ndelsystems; j++, k--)
                                       cmdseq->props[i].u.buffer.data[k] = em_device.delsystems[j];
                                    break;
        default:;
        }
     }
  return 0; // no err.
}


void em_lnb(int high_band, uint32_t high_val, uint32_t low_val) {
  em_device.highband = high_band;
  em_device.lnb_low = low_val;
  em_device.lnb_high = high_val;
}

void em_polarization(uint8_t p) {
  em_device.polarization = p & 0x3;
}

/*
 * INTERNAL USE ONLY. DONT EXPOSE THIS FUNC OUTSIDE THIS FILE.
 */
static int has_lock(fe_delivery_system_t em_delsys, struct transponder * t) {
  uint32_t tp_if = 0;

  switch(em_delsys) {
     case SYS_DVBT:
     case SYS_DVBT2:
        if (t->frequency != em_device.frequency) return 0;
        EM_INFO("%s: f=%u B%uC%dD0M%dT%dG%dY%dS%d\n"
                "          f=%u B%dC%dD0M%dT%dG%dY%dS%d\n",
             __FUNCTION__,
              freq_scale(em_device.frequency, 1e-3), freq_scale(em_device.bandwidth_hz, 1e-6), em_device.fec, em_device.modulation , em_device.transmission, em_device.guard, em_device.hierarchy, em_device.delsys==SYS_DVBT2, 
              freq_scale(t->frequency       , 1e-3), freq_scale(t->bandwidth          , 1e-6), t->coderate  , t->modulation        , t->transmission       , t->guard       , t->hierarchy       , t->delsys==SYS_DVBT2
              );
        if (t->bandwidth != em_device.bandwidth_hz)
           return 0;
        if ((t->coderate != em_device.fec) && (t->coderate != FEC_AUTO))
           return 0;
        if ((t->modulation != em_device.modulation) && (t->modulation != QAM_AUTO))
           return 0;
        if ((t->transmission != em_device.transmission) && (t->transmission != TRANSMISSION_MODE_AUTO))
           return 0;
        if ((t->guard != em_device.guard) && (t->guard != GUARD_INTERVAL_AUTO))
           return 0;
        if ((t->hierarchy != em_device.hierarchy) && (t->hierarchy != HIERARCHY_AUTO))
           return 0;
        if (! em_device.T2_delsys_bug) {
           // any dvb driver here, except CXD2820R
           if (t->delsys != em_device.delsys)
              return 0;
           }
        else {
           // CXD2820R only: driver silently changes delsys.
           em_device.delsys = t->delsys;
           }
        return 0x1F;
        break;
     case SYS_DVBC_ANNEX_A:
     case SYS_DVBC_ANNEX_C:
        if (t->frequency != em_device.frequency) return 0;
        if ((t->delsys != SYS_DVBC_ANNEX_A) && (t->delsys != SYS_DVBC_ANNEX_C))
           return 0;
        if ((t->modulation != em_device.modulation) && (em_device.modulation != QAM_AUTO))
           return 0;
        if (t->symbolrate != em_device.symbolrate)
           return 0;
        return 0x1F;
        break;
     case SYS_DVBS2:
        if ((t->rolloff != em_device.rolloff) && (em_device.rolloff != ROLLOFF_35) && (em_device.rolloff != ROLLOFF_AUTO)) {
           EM_INFO("wrong rolloff: %d != %d\n", t->rolloff, em_device.rolloff);
           return 0;
           }
        if ((t->pilot != em_device.pilot) && (em_device.pilot != PILOT_AUTO)) {
           EM_INFO("wrong pilot: %d != %d\n", t->pilot, em_device.pilot);
           return 0;
           }
        /* fall trough. */
     case SYS_DVBS:
        if (em_device.highband)
           tp_if = abs(t->frequency - em_device.lnb_high);
        else
           tp_if = abs(t->frequency - em_device.lnb_low);

        if (abs(tp_if - em_device.frequency) > 2000) {
           EM_INFO("t->frequency = %u: tp_if = %u <-> em_device.frequency = %u\n", t->frequency, tp_if, em_device.frequency);
           return 0;
           }
        if (t->delsys != em_device.delsys) {
           EM_INFO("wrong delsys\n");
           return 0;
           }
        if (t->polarization != em_device.polarization) {
           EM_INFO("wrong polarization\n");           
           return 0;
           }
        if ((t->coderate != em_device.fec) && (em_device.fec != FEC_AUTO)) {
           EM_INFO("wrong coderate\n");
           return 0;
           }
        if (t->symbolrate != em_device.symbolrate) {
           EM_INFO("wrong symbolrate\n");
           return 0;
           }
        if (t->modulation != em_device.modulation) {
           EM_INFO("wrong modulation\n");
           return 0;
           }
        return 0x1F;
        break;
     case SYS_ATSC:
        if (t->frequency != em_device.frequency) return 0;
        if (t->delsys != em_device.delsys)
           return 0;
        if (t->modulation != em_device.modulation)
           return 0;
        return 0x1F;
        break;
     default:
        fatal("unsupported del sys.\n");
     }
  return 0;
}

/*
 * replaces FE_READ_STATUS ioctl.
 */
int  em_status(fe_status_t * status) {
  sidata_t * s;

  *status = 0;
  for(s=em_sidata->first; s; s=s->next) {
     //char b[256];
     //print_transponder(b, &s->t);
     //info("%s: try %s (current: %d lo %d highband=%d)\n",__FUNCTION__,b, em_device.frequency, em_device.highband?em_device.lnb_high:em_device.lnb_low, em_device.highband);
     if (has_lock(em_device.delsys, &s->t)) {
        *status = 0x1F; // sync && lock.
        break;
        }
     }
  return 0;
}


/*----------------------------------------------------------------------------------------------------------------------
 * DEMUX related.
 *---------------------------------------------------------------------------------------------------------------------*/

static const char * table_name(uint16_t id) {
  switch(id) {
     case TABLE_PAT:       return "PAT";
     case TABLE_PMT:       return "PMT";
     case TABLE_NIT_ACT:   return "NIT(ACT)";
     case TABLE_NIT_OTH:   return "NIT(OTH)";
     case TABLE_SDT_ACT:   return "SDT(ACT)";
     case TABLE_SDT_OTH:   return "SDT(OTH)";
     case TABLE_VCT_TERR:  return "VCT(TERR)";
     case TABLE_VCT_CABLE: return "VCT(CABLE)";
     default:              return "???";
     }
}

void em_addfilter(struct section_buf * s) {
  EM_INFO("%s: %s pid=%d\n", __FUNCTION__, table_name(s->table_id), s->pid);
  AddItem(em_runningfilters, s);
}

void em_readfilters(int * result) {
  sidata_t * sidata;
  struct section_buf * filter;
  int maxiter = 10000;

  while ((filter = em_runningfilters->first)) {
     bool data_found = false;
     if (! maxiter--) fatal("%s: max iterations.\n", __FUNCTION__);
     for(sidata = em_sidata->first; sidata; sidata = sidata->next) {
        if (filter->table_id != sidata->table_id) {
        // info("(filter->table_id = %d != table_id = %d), table_id_ext = %d, pid = %d\n", filter->table_id, sidata->table_id, sidata->table_id_ext, sidata->pid);
           continue;
           }
        EM_INFO("f=%-6d: searching %-10s: table_id_ext = %d, pid = %d\n", freq_scale(em_device.frequency, 1e-3), table_name(filter->table_id), filter->table_id_ext, filter->pid);

        if (! has_lock(em_device.delsys, &sidata->t)) {
           EM_INFO(" -> no lock @ %d\n", freq_scale(sidata->t.frequency, 1e-3));
           continue;
           }

        if ((filter->pid != sidata->pid) && (filter->table_id == TABLE_PMT)) {
           EM_INFO(" -> wrong pid %d\n", sidata->pid);
           continue;
           }

        if ((filter->table_id_ext != sidata->table_id_ext) && (filter->table_id_ext > -1)){
           EM_INFO(" -> wrong table_id_ext = %d\n", sidata->table_id_ext);
           continue;
           }
        EM_INFO(" -> OK.\n");
        data_found = true;

        switch(filter->table_id) {
           case TABLE_PAT:       parse_pat(sidata->buf, sidata->len, sidata->transport_stream_id, filter->flags);
                                 break;
           case TABLE_NIT_ACT:
           case TABLE_NIT_OTH:   parse_nit(sidata->buf, sidata->len, filter->table_id, sidata->network_id, filter->flags);
                                 break;
           case TABLE_PMT:       verbose("PMT %d (0x%04x) for service %d (0x%04x)\n",
                                         sidata->pid, sidata->pid, sidata->service_id, sidata->service_id);
                                 parse_pmt(sidata->buf, sidata->len, sidata->service_id);
                                 break;
           case TABLE_SDT_ACT:   
           case TABLE_SDT_OTH:   verbose("SDT(%s TS, transport_stream_id %d (0x%04x) )\n",
                                         filter->table_id == 0x42 ? "actual":"other",
                                         sidata->transport_stream_id, sidata->transport_stream_id);
                                 parse_sdt(sidata->buf, sidata->len, sidata->transport_stream_id);
                                 break;
           case TABLE_VCT_TERR:  
           case TABLE_VCT_CABLE: verbose("ATSC VCT, table_id %d, table_id_ext %d\n",
                                         sidata->table_id, sidata->table_id_ext);
                                 parse_psip_vct(sidata->buf, sidata->len, filter->table_id, sidata->table_id_ext);
                                 break;           
           default:              fatal("%s %d: unhandled table_id %d\n", __FUNCTION__, __LINE__, filter->table_id);
           }
      //if (filter->run_once) /* probably i would have to check version nums here. */
      //   break;                 
        }

     if (! data_found) {
        const char * intro = "        Info: no data from ";
        // timeout waiting for data.
        switch(filter->table_id) {
           case TABLE_PAT:
           case TABLE_PMT:
           case TABLE_NIT_ACT:
           case TABLE_NIT_OTH:
           case TABLE_SDT_ACT:
           case TABLE_SDT_OTH:
           case TABLE_VCT_TERR:
           case TABLE_VCT_CABLE: info("%s%s after %lld seconds\n", intro, table_name(filter->table_id), (long long) filter->timeout);
                                 break;
           default:              info("%spid %u after %lld seconds\n", intro, filter->pid, (long long) filter->timeout);
           }
        *result = 0;
        }
     UnlinkItem(em_runningfilters, filter, filter->flags & SECTION_FLAG_FREE ? true:false);    
     }
  *result = 1;
  return;
}

static int parse_tp(const char * str) {
  uint32_t args[8];
  char mbuf[64], fecbuf[5], robuf[5];
  char c;
  char * p = strchr(str, '(');
  if (p && *p) *p=0; // cut '(ONID:NID:SID)'

  switch(em_device.scantype) {
     case SCAN_TERRESTRIAL:
        p = strchr(str, 'P');
        if (p) {
           em_device.delsys = SYS_DVBT2;
           sscanf(p + 1, "%u", &args[0]);
           em_device.stream_id = args[0];
           }
        else {
           em_device.delsys = SYS_DVBT;
           em_device.stream_id = 0;
           }
        sscanf(str, "%8s f = %6d kHz I%uB%uC%uD%uT%uG%uY%u",
               mbuf, &args[0], &args[1], &args[2], &args[3], &args[4], &args[5], &args[6], &args[7]);

        if      (strncmp(mbuf, "QPSK"    , 4) == 0) em_device.modulation = QPSK;
        else if (strncmp(mbuf, "QAM_16"  , 6) == 0) em_device.modulation = QAM_16;
        else if (strncmp(mbuf, "QAM_64"  , 6) == 0) em_device.modulation = QAM_64;
        else if (strncmp(mbuf, "QAM_256" , 7) == 0) em_device.modulation = QAM_256;
        else                                        em_device.modulation = QAM_AUTO;
        em_device.frequency = 1000 * args[0];
        switch(args[1]) {
           case 0:     em_device.inversion = INVERSION_OFF; break;
           case 1:     em_device.inversion = INVERSION_ON;  break;
           default:    em_device.inversion = INVERSION_AUTO;
           }
        switch(args[2]) {
           case 5 ... 10: em_device.bandwidth_hz = args[2] * MHz; break;
           case 1712:     em_device.bandwidth_hz = args[2] * kHz; break;
           default:       em_device.bandwidth_hz = 8 * MHz;
           }
        switch(args[3]) {
           case 0:     em_device.fec = FEC_NONE; break;
           case 12:    em_device.fec = FEC_1_2;  break;
           case 23:    em_device.fec = FEC_2_3;  break;
           case 34:    em_device.fec = FEC_3_4;  break;
           case 45:    em_device.fec = FEC_4_5;  break;
           case 56:    em_device.fec = FEC_5_6;  break;
           case 67:    em_device.fec = FEC_6_7;  break;
           case 78:    em_device.fec = FEC_7_8;  break;
           case 89:    em_device.fec = FEC_8_9;  break;
           case 35:    em_device.fec = FEC_3_5;  break;
           case 910:   em_device.fec = FEC_9_10; break;
           default:    em_device.fec = FEC_AUTO; break;
           }
        switch(args[5]) {                  
           case 2:     em_device.transmission = TRANSMISSION_MODE_2K;   break;
           case 8:     em_device.transmission = TRANSMISSION_MODE_8K;   break;
           case 4:     em_device.transmission = TRANSMISSION_MODE_4K;   break;
           case 1:     em_device.transmission = TRANSMISSION_MODE_1K;   break;
           case 16:    em_device.transmission = TRANSMISSION_MODE_16K;  break;
           case 32:    em_device.transmission = TRANSMISSION_MODE_32K;  break;
           default:    em_device.transmission = TRANSMISSION_MODE_AUTO; break;
           }
        switch(args[6]) {                  
           case 32:    em_device.guard = GUARD_INTERVAL_1_32;   break;
           case 16:    em_device.guard = GUARD_INTERVAL_1_16;   break;
           case 8:     em_device.guard = GUARD_INTERVAL_1_8;    break;
           case 4:     em_device.guard = GUARD_INTERVAL_1_4;    break;
           case 128:   em_device.guard = GUARD_INTERVAL_1_128;  break;
           case 19128: em_device.guard = GUARD_INTERVAL_19_128; break;
           case 19256: em_device.guard = GUARD_INTERVAL_19_256; break;
           default:    em_device.guard = GUARD_INTERVAL_AUTO;   break;
           }
        switch(args[7]) {
           case 0:     em_device.hierarchy = HIERARCHY_NONE; break;
           case 1:     em_device.hierarchy = HIERARCHY_1;    break;
           case 2:     em_device.hierarchy = HIERARCHY_2;    break;
           case 4:     em_device.hierarchy = HIERARCHY_4;    break;
           default:    em_device.hierarchy = HIERARCHY_AUTO; break;
           }
        if (em_device.delsys == SYS_DVBT2)
           sprintf(mbuf, "P%d", em_device.stream_id);
        else
           mbuf[0]=0;
      //EM_INFO("%s: '%-55s' -> %-8s f = %6d kHz I%sB%sC%sD%sT%sG%sY%s%s\n", \
      //    __FUNCTION__, str,                                               \
      //    xine_modulation_name(em_device.modulation),                      \
      //    freq_scale(em_device.frequency, 1e-3),                           \
      //    vdr_inversion_name(em_device.inversion),                         \
      //    vdr_bandwidth_name(em_device.bandwidth_hz),                      \
      //    vdr_fec_name(em_device.fec),                                     \
      //    vdr_fec_name(FEC_NONE),                                          \
      //    vdr_transmission_mode_name(em_device.transmission),              \
      //    vdr_guard_name(em_device.guard),                                 \
      //    vdr_hierarchy_name(em_device.hierarchy),                         \
      //    mbuf); 
        break;
     case SCAN_TERRCABLE_ATSC:
        sscanf(str, "%8s f=%u kHz ", mbuf, &args[0]);
        if      (strncmp(mbuf, "QAM64"   , 5) == 0) em_device.modulation = QAM_64;
        else if (strncmp(mbuf, "QAM256"  , 6) == 0) em_device.modulation = QAM_256;
        else if (strncmp(mbuf, "8VSB"    , 4) == 0) em_device.modulation = VSB_8;
        else if (strncmp(mbuf, "16VSB"   , 5) == 0) em_device.modulation = VSB_16;
        else                                        em_device.modulation = QAM_AUTO;
        em_device.frequency = 1000 * args[0];
      //EM_INFO("%s: '%-55s' -> %-8s f=%d kHz\n",  \
      //    __FUNCTION__, str,                     \
      //    atsc_mod_to_txt(em_device.modulation), \
      //    freq_scale(em_device.frequency, 1e-3));
        break;
     case SCAN_CABLE:
        sscanf(str, "%8s f = %u kHz S%uC%u  ", mbuf, &args[0], &args[1], &args[2]);
        if      (strncmp(mbuf, "QAM_16"  , 6) == 0) em_device.modulation = QAM_16;
        else if (strncmp(mbuf, "QAM_32"  , 6) == 0) em_device.modulation = QAM_32;
        else if (strncmp(mbuf, "QAM_64"  , 6) == 0) em_device.modulation = QAM_64;
        else if (strncmp(mbuf, "QAM_128" , 7) == 0) em_device.modulation = QAM_128;
        else if (strncmp(mbuf, "QAM_256" , 7) == 0) em_device.modulation = QAM_256;
        else                                        em_device.modulation = QAM_AUTO;
        em_device.frequency = 1000 * args[0];
        em_device.symbolrate = 1000 * args[1];
        em_device.fec = FEC_NONE;
      //EM_INFO("%s: '%-55s' -> %-8s f = %d kHz S%dC%s\n", \
      //    __FUNCTION__, str,                             \
      //    xine_modulation_name(em_device.modulation),    \
      //    freq_scale(em_device.frequency, 1e-3),         \
      //    freq_scale(em_device.symbol_rate, 1e-3),         \
      //    vdr_fec_name(FEC_NONE));
        break;
     case SCAN_SATELLITE:
        em_device.delsys = SYS_DVBS;
        em_device.rolloff = ROLLOFF_AUTO;
        em_device.pilot = PILOT_AUTO;
        if (strlen(str) > 2) 
           if (str[1] == '2') em_device.delsys = SYS_DVBS2;
        p = strchr(str, 'f');
        if (!p || !*p) return 0;
        sscanf(p, "f = %u kHz %c SR = %5d %4s 0,%s %5s ", &args[0], &c, &args[1], fecbuf, robuf, mbuf);
        em_device.frequency = 1000 * args[0];
        if      (c == 'V') em_device.polarization = POLARIZATION_VERTICAL;
        else if (c == 'R') em_device.polarization = POLARIZATION_CIRCULAR_RIGHT;
        else if (c == 'L') em_device.polarization = POLARIZATION_CIRCULAR_LEFT;
        else               em_device.polarization = POLARIZATION_HORIZONTAL;
        em_device.symbolrate = 1000 * args[1];
        if      (strncmp(fecbuf, "NONE" , 4) == 0)  em_device.fec = FEC_NONE;
        else if (strncmp(fecbuf, "1/2"  , 3) == 0)  em_device.fec = FEC_1_2;
        else if (strncmp(fecbuf, "2/3"  , 3) == 0)  em_device.fec = FEC_2_3;
        else if (strncmp(fecbuf, "3/4"  , 3) == 0)  em_device.fec = FEC_3_4;
        else if (strncmp(fecbuf, "4/5"  , 3) == 0)  em_device.fec = FEC_4_5;
        else if (strncmp(fecbuf, "5/6"  , 3) == 0)  em_device.fec = FEC_5_6;
        else if (strncmp(fecbuf, "6/7"  , 3) == 0)  em_device.fec = FEC_6_7;
        else if (strncmp(fecbuf, "7/8"  , 3) == 0)  em_device.fec = FEC_7_8;
        else if (strncmp(fecbuf, "8/9"  , 3) == 0)  em_device.fec = FEC_8_9;
        else if (strncmp(fecbuf, "3/5"  , 3) == 0)  em_device.fec = FEC_3_5;
        else if (strncmp(fecbuf, "9/10" , 4) == 0)  em_device.fec = FEC_9_10;
        else if (strncmp(fecbuf, "AUTO" , 4) == 0)  em_device.fec = FEC_AUTO;
        else                                        em_device.fec = FEC_AUTO;
        if      (strncmp(robuf, "35"   , 2) == 0)   em_device.rolloff = ROLLOFF_35;
        else if (strncmp(robuf, "25"   , 2) == 0)   em_device.rolloff = ROLLOFF_25;
        else if (strncmp(robuf, "20"   , 2) == 0)   em_device.rolloff = ROLLOFF_20;
        else if (strncmp(robuf, "AUTO" , 4) == 0)   em_device.rolloff = ROLLOFF_35;
        else                                        em_device.rolloff = ROLLOFF_35;
        if      (strncmp(mbuf, "QPSK"    , 4) == 0) em_device.modulation = QPSK;
        else if (strncmp(mbuf, "8PSK"    , 4) == 0) em_device.modulation = PSK_8;
        else if (strncmp(mbuf, "16APSK"  , 6) == 0) em_device.modulation = APSK_16;
        else if (strncmp(mbuf, "32APSK"  , 6) == 0) em_device.modulation = APSK_32;
        else                                        em_device.modulation = QPSK;
      EM_INFO("%s: '%-55s' -> %-2s f = %d kHz %s SR = %5d %4s 0,%s %5s\n", \
          __FUNCTION__, str,                                               \
          sat_delivery_system_to_txt(em_device.delsys),                    \
          freq_scale(em_device.frequency, 1e-3),                           \
          sat_pol_to_txt(em_device.polarization),                          \
          freq_scale(em_device.symbolrate, 1e-3),                          \
          sat_fec_to_txt(em_device.fec),                                   \
          sat_rolloff_to_txt(em_device.rolloff),                           \
          sat_mod_to_txt(em_device.modulation));
        break;
     default:
        fatal("could not find scan type for '%s'\n", str);
     }
  return 1; //success.
}

static void parse_intro(uint16_t table_id, const char * str, uint16_t * i1, uint16_t * i2) {
  unsigned tmp;
  switch(table_id) {
     case TABLE_PAT:     sscanf(str, "PAT (xxxx:xxxx:%hu)", i1); break;
     case TABLE_NIT_ACT: sscanf(str, "NIT(act): (xxxx:%hu:xxxx)", i1); break; 
     case TABLE_NIT_OTH: sscanf(str, "NIT(oth): (xxxx:%hu:xxxx)", i1); break; 
     case TABLE_SDT_ACT: sscanf(str, "SDT(actual TS, transport_stream_id %hu ", i1); break;
     case TABLE_SDT_OTH: sscanf(str, "SDT(other TS, transport_stream_id %hu ", i1); break;       
     case TABLE_PMT:     sscanf(str, "PMT %hu (0x%04x) for service %hu ", i1, &tmp, i2); break;
     default:;
     }
  //EM_INFO("table_id = %x -> %d %d\n", table_id, *i1, i2?*i2:-1);
}

static int parse_logfile(const char * log) {
  FILE * logfile = NULL;
  char * line = (char *) calloc(1, 256);
  char * p;
  int pid = 0, table_id = -1, len = 0, line_no = 0;
  uint16_t original_network_id = 0, network_id = 0, transport_stream_id = 0;
  uint16_t service_id = 0, pmt_pid = 0;
  int dev_props = 0;
  sidata_t * sidata = NULL;
  bool delsys_list = false;
  em_device.scantype = SCAN_UNDEFINED;


  if (!log || !*log) {
     free(line);
     error("could not open logfile: invalid file name.\n");
     return 0; // err
     }

  logfile = fopen(log, "r");
  if (!logfile) {
     free(line);
     fatal("cannot open '%s': error %d %s\n", log, errno, strerror(errno));
     return 0; // err
     }

  while (fgets(line, 256, logfile) != NULL) {
     bool is_tp=false;
     line_no++;

     // --- get scan type ----------------------------------------------------------------------------------------------
     if (em_device.scantype == SCAN_UNDEFINED) {
        if (strstr(line, "scan type")) {
           if      (strstr(line, "CABLE"))          em_device.scantype = SCAN_CABLE;
           else if (strstr(line, "SATELLITE"))      em_device.scantype = SCAN_SATELLITE;
           else if (strstr(line, "TERRCABLE_ATSC")) em_device.scantype = SCAN_TERRCABLE_ATSC;
           else if (strstr(line, "TERRESTRIAL"))    em_device.scantype = SCAN_TERRESTRIAL;
           else fatal("cannot read scan type from '%s'\n", line); 
           }
        continue; // anything else is meaningless until this is known.
        }
     // --- end scan type ----------------------------------------------------------------------------------------------


     // --- get the properties of dvb device ---------------------------------------------------------------------------
     if (dev_props < 2) {
        float fmin, fmax;
        if (strstr(line, "-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_")) { dev_props = 2; continue; }
        if (strstr(line, "-_-_-_-_ Getting frontend capabilities-_-_-_-_")) { dev_props = 1; continue; }
        if (dev_props != 1) continue;

        // used DVB API
        if (strstr(line, "Using DVB API ")) {
           sscanf(line, "Using DVB API %u.%u", &em_api.major, &em_api.minor);
           continue;
           }

        // capabilities: frontend name
        if ((p = strstr(line, "frontend ")) && strstr(line, " supports")) {
           char * pEnd;
           p += 10;
           pEnd = strchr(p, 39); *pEnd = 0;
           strncpy(&em_device.fe_info.name[0], p, 128);
           em_device.T2_delsys_bug = strstr(em_device.fe_info.name, "CXD2820R") != NULL;
           continue;
           }

        // all other capabilities.
        if (em_device.scantype == SCAN_TERRESTRIAL) {
           if (strstr(line, "DVB-T2")) em_device.fe_info.caps |= FE_CAN_2G_MODULATION;
           if (strstr(line, "INVERSION_AUTO")         && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_INVERSION_AUTO;
           if (strstr(line, "QAM_AUTO")               && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_QAM_AUTO;
           if (strstr(line, "TRANSMISSION_MODE_AUTO") && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_TRANSMISSION_MODE_AUTO;
           if (strstr(line, "GUARD_INTERVAL_AUTO")    && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_GUARD_INTERVAL_AUTO;
           if (strstr(line, "HIERARCHY_AUTO")         && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_HIERARCHY_AUTO;
           if (strstr(line, "FEC_AUTO")               && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_FEC_AUTO;
           if (sscanf(line, "FREQ (%fMHz ... %fMHz)", &fmin, &fmax) == 2) {
              em_device.fe_info.frequency_min=(0.5 + fmin * MHz);
              em_device.fe_info.frequency_max=(0.5 + fmax * MHz);
              }
           continue;
           }
        else if (em_device.scantype == SCAN_CABLE) {
           if (strstr(line, "DVB-C2")) em_device.fe_info.caps |= FE_CAN_2G_MODULATION;
           if (strstr(line, "INVERSION_AUTO") && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_INVERSION_AUTO;
           if (strstr(line, "QAM_AUTO")       && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_QAM_AUTO;
           if (strstr(line, "FEC_AUTO")       && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_FEC_AUTO;
           if (sscanf(line, "FREQ (%fMHz ... %fMHz)", &fmin, &fmax) == 2) {
              em_device.fe_info.frequency_min  =(0.5 + fmin * MHz);
              em_device.fe_info.frequency_max  =(0.5 + fmax * MHz);
              }
           if (sscanf(line, "SRATE (%fMSym/s ... %fMSym/s)", &fmin, &fmax) == 2) {
              em_device.fe_info.symbol_rate_min=(0.5 + fmin * MHz);
              em_device.fe_info.symbol_rate_max=(0.5 + fmax * MHz);
              }
           continue;
           }
        else if (em_device.scantype == SCAN_TERRCABLE_ATSC) {
           if (strstr(line, "INVERSION_AUTO") && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_INVERSION_AUTO;
           if (strstr(line, "8VSB"))    em_device.fe_info.caps |= FE_CAN_8VSB;
           if (strstr(line, "16VSB"))   em_device.fe_info.caps |= FE_CAN_16VSB;
           if (strstr(line, "QAM_64"))  em_device.fe_info.caps |= FE_CAN_QAM_64;
           if (strstr(line, "QAM_256")) em_device.fe_info.caps |= FE_CAN_QAM_256;
           if (sscanf(line, "FREQ (%fMHz ... %fMHz)", &fmin, &fmax) == 2) {
              em_device.fe_info.frequency_min=(0.5 + fmin * MHz);
              em_device.fe_info.frequency_max=(0.5 + fmax * MHz);
              }
           continue;
           }
        else if (em_device.scantype == SCAN_SATELLITE) {
           if (strstr(line, "QAM_AUTO") && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_QAM_AUTO;
           if (strstr(line, "DVB-S2")   && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_2G_MODULATION;
           if (strstr(line, "DVB-S")    && (strstr(line, "not supported") == NULL)) em_device.fe_info.caps |= FE_CAN_QPSK;
           if (sscanf(line, "FREQ (%fGHz ... %fGHz)", &fmin, &fmax) == 2) {
              em_device.fe_info.frequency_min = (0.5 + fmin * MHz);
              em_device.fe_info.frequency_max = (0.5 + fmax * MHz);
              }
           if (sscanf(line, "SRATE (%fMSym/s ... %fMSym/s)", &fmin, &fmax) == 2) {
              em_device.fe_info.symbol_rate_min=(0.5 + fmin * MHz);
              em_device.fe_info.symbol_rate_max=(0.5 + fmax * MHz);
              }
           continue;
           }
        else
           fatal("unsupported frontend type, cannot parse '%s'\n", line);  
        }

     if ((p = strstr(line, "   check ")) != NULL) {
        delsys_list=true;
        continue;
        }
     if (delsys_list) {
        const char * dname[]  = {
           "UNDEFINED", "DVB-C ann.A", "DVB-C ann.B", "DVB-T", "DSS", "DVB-S", "DVB-S2", "DVB-H", "ISDB-T", "ISDB-S",
           "ISDB-C", "ATSC", "ATSC/MH", "DTMB", "CMMB", "DAB", "DVB-T2", "TURBO-FEC", "DVB-C ann.C"};
        uint8_t i = 0;
        bool delsys = false;
        p = strchr(line, '\n');
        if (p) *p = 0;
        p = line;
        while(p && isspace(*p)) p++;


        for(i=0; i<sizeof(dname)/sizeof(dname[0]); i++) {
           //EM_INFO("'%s' = '%s'?\n",p,dname[i]);
           if (strncmp(p, dname[i], strlen(p)) == 0) {
              em_device.delsystems[em_device.ndelsystems++] = (fe_delivery_system_t) i;
              delsys = true;
              break;
              }
           }
        if (delsys)
           continue;
        else
           delsys_list = false;
        }

     if (dev_props != 2) continue; // continue only after frontend reading.
     // --- end of dvb device ------------------------------------------------------------------------------------------


     // --- we tuned to a new transponder. all si data belongs to this new tp ------------------------------------------
     if (strncmp(line, "        signal ok:	", 19) == 0) is_tp=true;  // '        signal ok:	<FOOBAR>'
     if (strncmp(line, "tune to: "            ,  9) == 0) is_tp=true;  // 'tune to: <FOOBAR>'
     if (is_tp) {
        char * p = strchr(line, ':'); p++;
        while(isspace(*p)) p++;
        parse_tp(p);
        table_id = -1;
        continue;
        }
     // --- end of tp reading ------------------------------------------------------------------------------------------


     // --- we got data from demux. first: intro with table_id_ext -----------------------------------------------------
     if (strstr(line, "no data from")) continue;
     if ((p = strstr(line, "PAT (xxxx:xxxx:")) != NULL)  { table_id = TABLE_PAT;     parse_intro(table_id, p, &transport_stream_id, 0); pid = PID_PAT;        continue; }
     if ((p = strstr(line, "NIT(act): (xxxx:")) != NULL) { table_id = TABLE_NIT_ACT; parse_intro(table_id, p, &network_id, 0);          pid = PID_NIT_ST;     continue; }
     if ((p = strstr(line, "NIT(oth): (xxxx:")) != NULL) { table_id = TABLE_NIT_OTH; parse_intro(table_id, p, &network_id, 0);          pid = PID_NIT_ST;     continue; }
     if ((p = strstr(line, "SDT(act")) != NULL)          { table_id = TABLE_SDT_ACT; parse_intro(table_id, p, &transport_stream_id,0);  pid = PID_SDT_BAT_ST; continue; }
     if ((p = strstr(line, "SDT(oth")) != NULL)          { table_id = TABLE_SDT_OTH; parse_intro(table_id, p, &transport_stream_id,0);  pid = PID_SDT_BAT_ST; continue; }
     if ((p = strstr(line, "PMT ")) != NULL)             { table_id = TABLE_PMT;     parse_intro(table_id, p, &pmt_pid, &service_id);   pid = pmt_pid;        continue; }

     if (strstr(line, "========================================================================")) {
        table_id = -1;
        continue;
        }
     if (table_id < 0) continue; // we found something, which we cannot assign to any table.

     if (strstr(line, "	len = ")) {
        sscanf(line, "	len = %d", &len);
        if (len > 0) {          
           sidata = (sidata_t *) calloc(1, sizeof(sidata_t));
           sidata->t.frequency = em_device.frequency;
           sidata->t.inversion = em_device.inversion;
           switch(em_device.delsys) {
              case SYS_DVBT:
              case SYS_DVBT2:
                 sidata->t.type                              = SCAN_TERRESTRIAL;
                 sidata->t.bandwidth                         = em_device.bandwidth_hz;
                 sidata->t.coderate                          = em_device.fec;
                 sidata->t.coderate_LP                       = FEC_AUTO;
                 sidata->t.modulation                        = em_device.modulation;
                 sidata->t.transmission                      = em_device.transmission;
                 sidata->t.guard                             = em_device.guard;
                 sidata->t.hierarchy                         = em_device.hierarchy;
                 sidata->t.delsys                            = em_device.delsys;
                 break;
              case SYS_DVBC_ANNEX_A:
              case SYS_DVBC_ANNEX_C:
                 sidata->t.type                              = SCAN_CABLE;
                 sidata->t.delsys                            = em_device.delsys;
                 sidata->t.modulation                        = em_device.modulation;
                 sidata->t.symbolrate                        = em_device.symbolrate;
                 break;
              case SYS_DVBS:
              case SYS_DVBS2:
                 sidata->t.type                              = SCAN_SATELLITE;
                 sidata->t.rolloff                           = em_device.rolloff;
                 sidata->t.pilot                             = em_device.pilot;
                 sidata->t.delsys                            = em_device.delsys;
                 sidata->t.polarization                      = em_device.polarization;
                 sidata->t.coderate                          = em_device.fec;
                 sidata->t.symbolrate                        = em_device.symbolrate;
                 sidata->t.modulation                        = em_device.modulation;
                 break;
              case SYS_ATSC:
                 sidata->t.type                              = SCAN_TERRCABLE_ATSC;
              default:
                 fatal("unsupported del sys.\n");
              }
           sidata->pid = pid;
           sidata->table_id = table_id;
           sidata->original_network_id = original_network_id;
           sidata->network_id = network_id;
           sidata->transport_stream_id = transport_stream_id;
           sidata->service_id = service_id;
           switch (table_id) {
              case TABLE_PAT:      sidata->table_id_ext = transport_stream_id;
                                   break;
              case TABLE_PMT:      sidata->table_id_ext = service_id;
                                   sidata->pid = pmt_pid;
                                   break;
              case TABLE_NIT_ACT:
              case TABLE_NIT_OTH:  sidata->table_id_ext = network_id;
                                   break;
              case TABLE_SDT_ACT:
              case TABLE_SDT_OTH:  sidata->table_id_ext = transport_stream_id;
                                   break;
              default: info("%s: no table id.\n", __FUNCTION__);
              }
         //EM_INFO("line %-5d: new sidata: table_id = %-8s, table_id_ext = %u, pid = %u (%u:%u:%u)\n", \
         //     line_no,                                                                               \
         //     sidata->table_id == TABLE_PAT    ?"PAT":                                               \
         //     sidata->table_id == TABLE_NIT_ACT?"NIT(act)":                                          \
         //     sidata->table_id == TABLE_NIT_OTH?"NIT(oth)":                                          \
         //     sidata->table_id == TABLE_SDT_ACT?"SDT(act)":                                          \
         //     sidata->table_id == TABLE_SDT_OTH?"SDT(oth)":                                          \
         //     sidata->table_id == TABLE_PMT    ?"PMT":"---ERROR---" ,                                \
         //     sidata->table_id_ext, sidata->pid,                                                     \
         //     sidata->original_network_id, sidata->network_id, sidata->transport_stream_id);
           }
        continue;
        }
     if (strstr(line, "=== parse")) continue;

     if (!sidata) fatal("%d: sidata invalid for table_id 0x%02x\n", line_no, table_id);
     if (strlen(line) > 8) {
        if ((p = strstr(line, "	0x")) != NULL) {
           unsigned tmp, args[16];
           int nitems = 0, i;
           if (len > 0) {
              p = strchr(line, '\n');
              if (p) *p = 0;

              nitems = sscanf(line, "	0x%X: %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X :",
                               &tmp,
                               &args[0], &args[1], &args[2], &args[3], &args[4], &args[5], &args[6], &args[7],
                               &args[8], &args[9], &args[10], &args[11], &args[12], &args[13], &args[14], &args[15]);
              len-=(nitems-1);
              //EM_INFO("%d: '%-80s' (%d bytes left)\n", line_no, line, len);


              for(i=0; i < nitems-1; i++)
                 sidata->buf[sidata->len++] = args[i];
              if (len < 1) {
               //int v = verbosity;
               //verbosity = 5;
               //hexdump(table_name(sidata->table_id, &sidata->buf[0], sidata->len);
               //verbosity = v;
               //char b[256];
               //print_transponder(b, &sidata->t);
               //EM_INFO("add: %6d %-10s pid=%-3d, table_id_ext=%-5d, ONID=%-5d, NID=%-5d, TSID=%-5d, SID=%-5d (%s)\n", \
               //     freq_scale(em_device.frequency, 1e-3),                                                       \
               //     table_name(sidata->table_id),                                                                \
               //     sidata->pid,                                                                                 \
               //     sidata->table_id_ext,                                                                        \
               //     sidata->original_network_id,                                                                 \
               //     sidata->network_id,                                                                          \
               //     sidata->transport_stream_id,                                                                 \
               //     sidata->service_id, b);
                 AddItem(em_sidata, sidata);
                 sidata = NULL;
                 }

              }
           }
        }
     } 
  fclose(logfile);
  free(line);
  return 1;
}
