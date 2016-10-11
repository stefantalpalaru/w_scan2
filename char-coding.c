/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Winfried Koehler 
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
 * v 20110402
 */


/**********************************************************************************************************************
 * char conversation is based on a patch from Mauro Carvalho Chehab for (dvb)scan and adapted for
 * w_scan (used v1 of patch).
 * see http://www.mail-archive.com/linux-media@vger.kernel.org/msg29914.html
 *********************************************************************************************************************/

/*-------------------------------- 120 chars max width, no <TAB> ----------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <iconv.h>
#include <errno.h>
#include "scan.h"
#include "char-coding.h"
#include "iconv_codes.h"

#define MIN(X,Y) (X < Y ? X : Y)
#define IsCharacterCodingCode(C) (C < 0x20)

/*
 * handle character set correctly (via glib iconv),
 * ISO/EN 300 468 annex A 
 */
void char_coding(char **inbuf, size_t * inbytesleft, char **outbuf, size_t * outbytesleft, unsigned user_charset_id) {
  unsigned dvb_charset_id = 9999;
  const char * psrc = *inbuf;
  char * pdest = *outbuf;
  size_t nsrc  = *inbytesleft;
  size_t ndest = *outbytesleft;
  int err = 0;

  uint8_t first_byte_value;

  if (! *inbytesleft || ! *inbuf)
     return;

  if (*outbytesleft < (4 * (*inbytesleft) + 1)) {
     warning("%s %d: inbytesleft = %zu, outbytesleft = %zu\n", __FUNCTION__, __LINE__, *inbytesleft, *outbytesleft);
     warning("output buffer needs to be bigger than 3 * input length, skipping iconv.\n");
     return;
     }

  first_byte_value = **inbuf; 
  
  if (IsCharacterCodingCode(first_byte_value)) {
     // ISO/EN 300 468 v011101p, Annex A.2 Selection of character table
     *inbuf += 1; *inbytesleft -= 1; // skip over coding byte

     #define DVBCHARSET(S) dvb_charset_id = get_codepage_index(S);

     switch (first_byte_value) {
            case 0x1:   DVBCHARSET("ISO88595");      break; // ISO/IEC 8859-5  Latin/Cyrillic alphabet A.2
            case 0x2:   DVBCHARSET("ISO88596");      break; // ISO/IEC 8859-6  Latin/Arabic alphabet A.3
            case 0x3:   DVBCHARSET("ISO88597");      break; // ISO/IEC 8859-7  Latin/Greek alphabet A.4
            case 0x4:   DVBCHARSET("ISO88598");      break; // ISO/IEC 8859-8  Latin/Hebrew alphabet A.5
            case 0x5:   DVBCHARSET("ISO88599");      break; // ISO/IEC 8859-9  Latin alphabet No. 5 A.6
            case 0x6:   DVBCHARSET("ISO885910");     break; // ISO/IEC 8859-10 Latin alphabet No. 6 A.7
            case 0x7:   DVBCHARSET("ISO885911");     break; // ISO/IEC 8859-11 Latin/Thai (draft only) A.8
            case 0x8:                                break; //                 reserved for future use (see note)
            case 0x9:   DVBCHARSET("ISO885913");     break; // ISO/IEC 8859-13 Latin alphabet No. 7 A.9
            case 0xA:   DVBCHARSET("ISO885914");     break; // ISO/IEC 8859-14 Latin alphabet No. 8 (Celtic) A.10
            case 0xB:   DVBCHARSET("ISO885915");     break; // ISO/IEC 8859-15 Latin alphabet No. 9 A.11
            case 0xC ... 0xF:                        break; //                 reserved for future use
            case 0x10:  {                                   // ISO/IEC 8859    See table A.4
                        uint8_t second_byte_value;
                        uint8_t third_byte_value;

                        // the following two bytes carry a 16-bit value (uimsbf) to
                        // indicate that the remaining data of the text field is coded
                        // using the character code table specified in table A.4.
                        second_byte_value = **inbuf; *inbuf += 1; *inbytesleft -= 1;
                        third_byte_value  = **inbuf; *inbuf += 1; *inbytesleft -= 1;

                        switch(second_byte_value) {
                              case 0x0:
                                  switch(third_byte_value) {
                                        case 0x0:                          break; // reserved for future use
                                        case 0x1: DVBCHARSET("ISO88591");  break; // ISO/IEC 8859-1 W European
                                        case 0x2: DVBCHARSET("ISO88592");  break; // ISO/IEC 8859-2 E European
                                        case 0x3: DVBCHARSET("ISO88593");  break; // ISO/IEC 8859-3 S European
                                        case 0x4: DVBCHARSET("ISO88594");  break; // ISO/IEC 8859-4 N/NE European
                                        case 0x5: DVBCHARSET("ISO88595");  break; // ISO/IEC 8859-5 Lat./Cyrill. A.2
                                        case 0x6: DVBCHARSET("ISO88596");  break; // ISO/IEC 8859-6 Lat./Arabic A.3
                                        case 0x7: DVBCHARSET("ISO88597");  break; // ISO/IEC 8859-7 Lat./Greek A.4
                                        case 0x8: DVBCHARSET("ISO88598");  break; // ISO/IEC 8859-8 Lat./Hebrew A.5
                                        case 0x9: DVBCHARSET("ISO88599");  break; // ISO/IEC 8859-9 WE & Turk A.6
                                        case 0xA: DVBCHARSET("ISO885910"); break; // ISO/IEC 8859-10 N European A.7
                                        case 0xB: DVBCHARSET("ISO885911"); break; // ISO/IEC 8859-11 Thai A.8
                                        case 0xC:                          break; // Reserved for future use
                                        case 0xD: DVBCHARSET("ISO885913"); break; // ISO/IEC 8859-13 Baltic A.9
                                        case 0xE: DVBCHARSET("ISO885914"); break; // ISO/IEC 8859-14 Celtic A.10
                                        case 0xF: DVBCHARSET("ISO885915"); break; // ISO/IEC 8859-15 W European A.11
                                        default:
                                               warning("%s %d: unknown third byte value 0x%X\n",
                                                               __FUNCTION__, __LINE__, third_byte_value);
                                        }
                                  break;
                              default:
                                    warning("%s %d: unknown second byte value 0x%X\n",
                                                    __FUNCTION__, __LINE__, second_byte_value);
                              }
                        break;
                        }                   
            case 0x11:  DVBCHARSET("ISO-10646");       break; // ISO/IEC 10646   Basic Multilingual Plane
            case 0x12:  DVBCHARSET("ISO-2022-KR");     break; // KSX1001-2004    Korean Character Set
            case 0x13:  DVBCHARSET("GB2312");          break; // GB-2312-1980    Simplified Chinese Character
            case 0x14:  DVBCHARSET("BIG5");            break; // Big5 subset of ISO/IEC 10646 Traditional Chinese
            case 0x15:  DVBCHARSET("ISO-10646/UTF-8"); break; // UTF-8 encoding of ISO/IEC 10646 Basic Multil. Plane
            case 0x16 ... 0x1E:                        break; // reserved for future use
            case 0x1F:  {                                     // the following byte carries an 8-bit value (uimsbf)
                        uint8_t encoding_dvb_charset_id;      // containing the encoding_dvb_charset_id

                        encoding_dvb_charset_id = **inbuf; *inbuf += 1; *inbytesleft -= 1;

                        switch(encoding_dvb_charset_id) { // TS 101 162 V1.2.1 (2009-07), 5.10 Encoding_dvb_charset_id
                              case 0x0:           break; // Reserved
                              case 0x1 ... 0xEF:  break; // Reserved for general registration through the DVB Project
                              case 0xF0 ... 0xFF: break; // Reserved for future use
                              default:;
                              }
                        break;
                        }
            default:
                   warning("%s %d: unknown first byte value 0x%X\n",
                                   __FUNCTION__, __LINE__, first_byte_value);
            }
     }
  if (dvb_charset_id > iconv_codes_count()) {
     // no special character coding applied: use iso6937-2 w. euro add-on
     char * pEuro;
     DVBCHARSET("ISO69372");

     while (**inbuf && (pEuro = strchr(*inbuf, 0xA4))) {
           // handle the euro add-on
           char * ebuf = calloc(3,1);
           char * pi = ebuf;
           char * po = *outbuf;
           size_t inbytes = pEuro-*inbuf;

           *(pi++) = 0x0B;
           *(pi++) = 0xA4;

           verbose("\t\t%s: euro char in iso-6937-2\n", __FUNCTION__);
           if (inbytes) {
              char * ibuf = calloc(inbytes + 1, 1);
              strncpy(ibuf, *inbuf, inbytes);

              // translate *inbuf up to euro sign
              pi = ibuf; *inbuf += inbytes; *inbytesleft -= inbytes;
              char_coding(&pi, &inbytes, &po, outbytesleft, user_charset_id);
              *outbuf = *outbuf + strlen(*outbuf);
              free(ibuf);
              }

           // skip over euro sign in *inbuf
           *inbuf += 1; *inbytesleft -= 1;

           // translate euro sign to users charset and add it to *outbuf
           inbytes = 2; pi = ebuf; po = *outbuf;
           char_coding(&pi, &inbytes, &po, outbytesleft, user_charset_id);
           *outbuf = *outbuf + strlen(*outbuf);
           free(ebuf);
           }
     }

  if (! *inbytesleft || ! **inbuf)
     return;

  if (user_charset_id < iconv_codes_count()) {
     char * usr = calloc(strlen(iconv_codes[user_charset_id]) +strlen("//IGNORE") + 1, 1);
     iconv_t conversion_descriptor;
    
     strcpy(usr, iconv_codes[user_charset_id]);
     strcpy(&usr[strlen(iconv_codes[user_charset_id])], "//IGNORE");

   //debug("\t\t%s: converting '%s' from '%s' to '%s'\n", __FUNCTION__, *inbuf, iconv_codes[dvb_charset_id], usr);
     conversion_descriptor = iconv_open((const char *) usr, iconv_codes[dvb_charset_id]);
     free(usr);

     if (conversion_descriptor == (iconv_t)(-1)) {
        warning("\t\t%s %d: iconv_open failed.\n", __FUNCTION__, __LINE__);
        switch(errno) {
              case EINVAL:
                   info("\t\tThe conversion from '%s' to '%s' is not supported.\n",
                       iconv_codes[dvb_charset_id], iconv_codes[user_charset_id]);
                   break;
              default:
                   info("\t\t%s\n", strerror(errno));
              }
        err++;
        }
     else {
	size_t result = iconv(conversion_descriptor, inbuf, inbytesleft, outbuf, outbytesleft);

        if (result == (size_t)(-1)) {
           warning("\t\t%s %d: iconv failed.\n", __FUNCTION__, __LINE__);
           info("\t\t%s\n", strerror(errno));
           switch (errno) {
                  case EILSEQ:
                  case EINVAL:
                       hexdump("*inbuf",  (unsigned char *) psrc,  nsrc);
                       hexdump("*outbuf", (unsigned char *) pdest, ndest);
                       break;
                  default:;
                  }
           err++;
           }

        if (iconv_close(conversion_descriptor) == -1) {
           warning("\t\t%s %d: iconv_close failed.\n", __FUNCTION__, __LINE__);
           info("\t\t%s\n", strerror(errno));
           err++;
           }

        **outbuf = 0;
        return;
        }
     }

  if (err) {
     // Fallback method: copy all printable chars from *inbuf to *outbuf.
     size_t i;
     size_t pos = 0;

     for (i = 0; i < nsrc; i++) {  
         switch((uint8_t) *(psrc + i)) {
               //case 0x20 ... 0x7E:
               //case 0xA0 ... 0xFF:
                    // printable chars ISO-6937-2
                    // Figure A.1: Character code table 00 - Latin alphabet
               case 0x01 ... 0xFF: // 20121202: don't touch anything; leave it as it is.
                    *(pdest + pos++) = *(psrc + i);
               default:;
               }
         }
     *(pdest + pos++) = 0;
     }

}

// clean_str: enshure upper case and remove all { '-', '_', ' ' }
static inline void clean_str(const char * in, char * outbuf) {
  unsigned i, pos = 0;

  for (i = 0; i < strlen(in); i++) {
      if ((in[i] == '-') ||
          (in[i] == '_') ||
          (in[i] == ' '))
         continue;
      outbuf[pos++] = toupper(in[i]);
      }
  outbuf[pos++] = 0;
}

int get_codepage_index(const char * codepage) {
  unsigned i;
  int idx = -1;
  char * buf = strdup(codepage);

  clean_str(codepage, buf);

  for (i = 0; i < iconv_codes_count(); i++) {
      char * ibuf = strdup(iconv_codes[i]);

      clean_str(iconv_codes[i], ibuf);
      if (strlen(buf) == strlen(ibuf))
         if (! strncmp(buf, ibuf, strlen(buf))) {
            idx = i;
            free(ibuf);
            break;
            }
      free(ibuf);
      }

  free(buf);
  if (idx < 0) {
     warning("unknown codepage '%s', using default 'UTF-8'\n", codepage);
     idx = get_codepage_index("UTF-8");
     }
  return idx;
}

int get_user_codepage(void) {

  const int cats[] = { LC_CTYPE, LC_COLLATE, LC_MESSAGES };
  char * buf, * pch, * pbuf;
  unsigned cat, idx = 0;

  for (cat = 0; cat < sizeof(cats)/sizeof(cats[0]); cat++) {

    // Note: program's locale is not changed here, since locale isn't given.
    //       the returned char * should be "C", "POSIX" or something valid.
    //       If valid, we can only *guess* which format is returned.
    //       Assume here something like "de_DE.iso8859-1@euro" or "de_DE.utf-8" 
    if (! (buf = setlocale(cats[cat], "")) || strlen(buf) < 2)
       continue;

    buf = strdup(buf);
    pbuf= buf;
       
    if (!  strncmp(buf, "POSIX", MIN(strlen(buf), 5)) ||
        ! (strncmp(buf, "en",    MIN(strlen(buf), 2)) && !isalpha(buf[2])) )
       continue;

    // assuming 'language_country.encoding@variant'

    // codeset after '.', if given
    if ((pch = strchr(buf, '.')))
       pbuf = pch + 1;

    // remove all after '@', including '@'
    if ((pch = strchr(pbuf, '@')))
       *pch = 0;

    idx = get_codepage_index(pbuf);
    free(buf);
    return idx;
    }

  warning("could not guess your codepage. Falling back to 'UTF-8'\n");
  return get_codepage_index("UTF-8");
}
