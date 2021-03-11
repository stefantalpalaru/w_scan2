/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006-2014 Winfried Koehler
 * Copyright (C) 2021 Jean-Marc Wislez
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

 // Possible improvements or things to look at:
 // XML DTD example shows delivery_system as transponder parameter, while DTD specifies delsys
 // struct transponder has no members named 'input_stream_id', 'interleave'

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "si_types.h"
#include "dump-xml.h"

#define STRING_BUFFER_SIZE 255

typedef struct {
	char name[32];
	uint8_t delsys[32];
	uint16_t xml_default;
} xml_param;

#define NO_AUTO (1<<15)

const xml_param xml_params[] = {
	{"modulation", {SYS_DVBC_ANNEX_A, SYS_DVBC_ANNEX_B, SYS_DVBC_ANNEX_C,
			SYS_DVBT, SYS_DVBT2,
			SYS_DVBS, SYS_DVBS2, SYS_DSS, SYS_TURBO,
			SYS_DVBH,
			SYS_ISDBT, SYS_ISDBS, SYS_ISDBC,
			SYS_ATSC, SYS_ATSCMH,
			SYS_DTMB, SYS_CMMB, SYS_DAB, 0}, QAM_AUTO},
	{"coderate", {SYS_DVBT, SYS_DVBT2,
		SYS_DVBS, SYS_DVBS2, SYS_DSS, SYS_TURBO,
		SYS_DVBC_ANNEX_A, SYS_DVBC_ANNEX_B, SYS_DVBC_ANNEX_C,
		SYS_DTMB, 0}, FEC_AUTO},
	{"inversion", {SYS_DVBT, SYS_DVBT2,
		SYS_DVBS, SYS_DVBS2, SYS_DSS, SYS_TURBO,
		SYS_DVBC_ANNEX_A, SYS_DVBC_ANNEX_B, SYS_DVBC_ANNEX_C,
		SYS_ISDBT, SYS_DTMB, 0}, INVERSION_AUTO},
	{"bandwidth",
		{SYS_DVBT, SYS_DVBT2, SYS_ISDBT, SYS_ATSC, SYS_ATSCMH, SYS_DTMB, 0},
		NO_AUTO},
	{"priority", {SYS_DVBT, SYS_DVBT2, 0}, true},
	{"time_slicing", {SYS_DVBT, 0}, false},	// EN 301 192
	{"mpe_fec", {SYS_DVBT, SYS_DVBT2, 0}, false},
	{"hierarchy", {SYS_DVBT, SYS_DVBT2, 0}, HIERARCHY_NONE},
	{"alpha", {SYS_DVBT, SYS_DVBT2, 0}, NO_AUTO},	//Table 46: Signalling format for the avalues and the used interleaver
	{"interleaver", {SYS_DVBT, SYS_DVBT2, SYS_DTMB, 0}, INTERLEAVE_AUTO},	//Table 46: Signalling format for the avalues and the used interleaver
	{"terr_interleaver", {SYS_DVBT, SYS_DVBT2, 0}, INTERLEAVE_AUTO},	//Table 46: Signalling format for the avalues and the used interleaver
	{"coderate_LP", {SYS_DVBT, SYS_DVBT2, 0}, FEC_NONE},
	{"guard", {SYS_DVBT, SYS_DVBT2, SYS_ISDBT, SYS_DTMB, 0},
		GUARD_INTERVAL_AUTO},
	{"transmission", {SYS_DVBT, SYS_DVBT2, SYS_ISDBT, SYS_DTMB, 0},
		TRANSMISSION_MODE_AUTO},
	{"other_frequency_flag", {SYS_DVBT, SYS_DVBT2, 0}, 0},
	{"plp_id", {SYS_DVBT2, SYS_DVBC2, 0}, 0},
	{"system_id", {SYS_DVBT2, SYS_DVBC2, 0}, 0},
	{"extended_info", {SYS_DVBT2, 0}, 1},	// T2 delivery system descriptor: 'if (descriptor_length > 4){'
	{"SISO_MISO", {SYS_DVBT2, 0}, 0},
	{"tfs_flag", {SYS_DVBT2, 0}, 0},
	{"orbital_position", {SYS_DVBS, SYS_DVBS2, SYS_DSS, SYS_TURBO, 0},
		0x0192},
	{"west_east_flag", {SYS_DVBS, SYS_DVBS2, SYS_DSS, SYS_TURBO, 0}, 'E'},
	{"polarization", {SYS_DVBS, SYS_DVBS2, SYS_DSS, SYS_TURBO, 0}, NO_AUTO},
	{"rolloff", {SYS_DVBS2, 0}, ROLLOFF_AUTO},
	{"symbolrate", {SYS_DVBS, SYS_DVBS2, SYS_DSS, SYS_TURBO,
			SYS_DVBC_ANNEX_A, SYS_DVBC_ANNEX_B, SYS_DVBC_ANNEX_C,
			0}, NO_AUTO},
	{"multiple_input_stream_flag", {SYS_DVBS2, 0}, 0},
	{"scrambling_sequence_selector", {SYS_DVBS2, 0}, 0},
	{"scrambling_sequence_index", {SYS_DVBS2, 0}, 0},
	{"input_stream_id", {SYS_DVBT2, SYS_DVBS2, 0}, NO_AUTO},
	{"pilot", {SYS_DVBS2, 0}, PILOT_AUTO},
	{"data_slice_id", {SYS_DVBC2, 0}, 0},
	{"C2_tuning_frequency_type", {SYS_DVBC2, 0},
		C2_SYSTEM_CENTER_FREQUENCY},
	{"active_OFDM_symbol_duration", {SYS_DVBC2, 0}, FFT_4K_8MHZ},
	{"", {0}, NO_AUTO},
};

#define NUM_XML_PARAMS (sizeof(xml_params) / sizeof(xml_params[0]))

char *CaName(int id)
{
// function taken from w_scan_cpp (https://www.gen2vdr.de/wirbel/w_scan_cpp/index2.html)
// int minor = id & 0xFF;
	int major = id >> 8;

	if (major == 0x01)
		return "Seca Mediaguard";
	else if (major == 0x05)
		return "Viaccess";
	else if (major == 0x06)
		return "Irdeto";
	else if (major == 0x07)
		return "DigiCipher-2";
	else if (major == 0x09)
		return "NDS/Cisco Videoguard";
	else if (major == 0x0B)
		return "Conax";
	else if (major == 0x0D)
		return "Cryptoworks";
	else if (major == 0x0E)
		return "PowerVu";
	else if (major == 0x10)
		return "RAS (Remote Authorization System)";
	else if (major == 0x17)
		return "Betaresearch (or Tunnel)";
	else if (major == 0x18)
		return "Nagravision";
	else if (major == 0x22)
		return "Codicrypt";
	else if (major == 0x26)
		return "BISS";
	else if (id == 0x0464)
		return "EuroDec";
	else if (id == 0x2719)
		return "InCrypt Cas";
	else if (id == 0x4347)
		return "Crypton";
	else if (id == 0x4800)
		return "Accessgate";
	else if (id == 0x4900)
		return "China Crypt";
	else if (id == 0x4A02)
		return "Tongfang";
	else if (id == 0x4A10)
		return "EasyCas";
	else if (id == 0x4A20)
		return "AlphaCrypt";
	else if (id == 0x4A60)
		return "Skycrypt / Neotioncrypt / Neotion SHL";
	else if (id == 0x4A61)
		return "Skycrypt / Neotioncrypt / Neotion SHL";
	else if (id == 0x4A63)
		return "Skycrypt / Neotioncrypt / Neotion SHL";
	else if (id == 0x4A70)
		return "Dreamcrypt";
	else if (id == 0x4A80)
		return "ThalesCrypt";
	else if (id == 0x4AA1)
		return "KeyFly";
	else if (id == 0x4ABF)
		return "CTI-CAS";
	else if (id == 0x4AD0)
		return "X-Crypt";
	else if (id == 0x4AD1)
		return "X-Crypt";
	else if (id == 0x4AD4)
		return "OmniCrypt";
	else if (id == 0x4AE0)
		return "DRE-Crypt";
	else if (id == 0x4AE1)
		return "DRE-Crypt";
	else if (id == 0x4AE4)
		return "CoreCrypt";
	else if (id == 0x4AEA)
		return "Cryptoguard";
	else if (id == 0x4AEB)
		return "Abel Quintic";
	else if (id == 0x4AF0)
		return "ABV CAS";
	else if (id == 0x4AFC)
		return "Panaccess";
	else if (id == 0x4B19)
		return "RCAS or RIDSYS cas";
	else if (id == 0x4B30)
		return "Vicas";
	else if (id == 0x4B31)
		return "Vicas";
	else if (id == 0x5448)
		return "Gospell VisionCrypt";
	else if (id == 0x5501)
		return "Greif";
	else if (id == 0x5581)
		return "Bulcrypt";
	else if (id == 0x7BE1)
		return "DRE-Crypt";
	else if (id == 0xA101)
		return "RosCrypt-M";
	else if (id == 0xEAD0)
		return "InCrypt Cas";
	return "";
}

char *StreamName(int id)
{
	// list taken from https://github.com/stefantalpalaru/w_scan2/issues/18
	switch (id) {
	case 1:
		return "ISO/IEC 11172-2 Video | MPEG 1 - H.261";
	case 2:
		return "ISO/IEC 13818-2 Video | MPEG-2 - H.262";
	case 16:
		return "ISO/IEC 14496-2 Visual | MPEG-4 - H.263";
	case 27:
		return "ISO/IEC 14496-10 Video | H.264";
	case 36:
		return "ISO/IEC 23008-2 Video | H.265";
	case 3:
		return "ISO/IEC 11172 Audio | MPEG-1";
	case 4:
		return "ISO/IEC 13818-3 Audio | MPEG-2";
	case 15:
		return "ISO/IEC 13818-7 Audio | ADTS AAC";
	case 17:
		return "ISO/IEC 14496-3 Audio (LATM) | AAC LC";
	case 6:
		return "ISO/IEC 13818-1 PES private data";
	case 5:
		return "ISO/IEC 13818-1 | HBBTV";
	case 7:
		return "ISO/IEC 13522 (MHEG)";
	case 8:
		return "ITU-T Rec. H.222 and ISO/IEC 13818-1 DSM CC";
	case 9:
		return "ITU-T Rec. H.222 and ISO/IEC 13818-1/11172-1 auxiliary data";
	case 10:
		return "ISO/IEC 13818-6 DSM CC multiprotocol encapsulation";
	case 11:
		return "ISO/IEC 13818-6 DSM CC U-N messages";
	case 12:
		return "ISO/IEC 13818-6 DSM CC stream descriptors";
	case 13:
		return "ISO/IEC 13818-6 DSM CC tabled data";
	case 14:
		return "ISO/IEC 13818-1 auxiliary data";
	case 20:
		return "ISO/IEC 13818-6 DSM CC synchronized download protocol";
	case 21:
		return "Packetized metadata";
	case 22:
		return "Sectioned metadata";
	case 23:
		return "ISO/IEC 13818-6 DSM CC Data Carousel metadata";
	case 24:
		return "ISO/IEC 13818-6 DSM CC Object Carousel metadata";
	case 25:
		return "ISO/IEC 13818-6 Synchronized Download Protocol metadata";
	case 26:
		return "ISO/IEC 13818-11 IPMP";
	case 28:
		return "ISO/IEC 14496-3 | AAC";
	case 29:
		return "ISO/IEC 14496-17 (MPEG-4 text)";
	case 30:
		return "ISO/IEC 23002-3 (MPEG-4 auxiliary video)";
	case 31:
		return "ISO/IEC 14496-10 SVC (MPEG-4 AVC sub-bitstream)";
	case 32:
		return "ISO/IEC 14496-10 MVC (MPEG-4 AVC sub-bitstream)";
	case 66:
		return "Chinese Video Standard";
	case 127:
		return "ISO/IEC 13818-11 IPMP (DRM)";
	case 135:
		return "Dolby Digital Plus (enhanced AC-3)";
	case 145:
		return "ATSC DSM CC Network Resources table";
	case 193:
		return "Dolby Digital (AC-3) up to six channel audio with AES-128-CBC data encryption";
	case 194:
		return "ATSC DSM CC synchronous data or Dolby Digital Plus audio with AES-128-CBC data encryption";
	case 207:
		return "ISO/IEC 13818-7 ADTS AAC with AES-128-CBC frame encryption";
	case 209:
		return "BBC Dirac (Ultra HD video)";
	case 219:
		return "ITU-T Rec. H.264 and ISO/IEC 14496-10 with AES-128-CBC slice encryption";
	default:
		return "UNKNOWN";
	}
}

char *safe_xml(const char *input, char *output)
{

	if (input == NULL) {
		output[0] = '\0';
		return (output);
	}

	unsigned char *p = (unsigned char *)input, shift = 0;
	int i = 0;

	while (*p != '\0' && i < STRING_BUFFER_SIZE) {
		if (*p < 32) {
			output[i++] = '?';
		} else if (*p < 127) {
			shift = 0;
			if (*p == '&' && i < STRING_BUFFER_SIZE - 5) {
				output[i++] = '&';
				output[i++] = 'a';
				output[i++] = 'm';
				output[i++] = 'p';
				output[i++] = ';';
			} else {
				output[i++] = *p;
			}
		} else if (*p < 192) {
			// key to be shifted
			switch (shift) {
			case 0xc3:
				if (*p >= 0x80 && *p <= 0x85) {
					output[i++] = 'A';
					break;
				}
				if (*p == 0x86) {
					output[i++] = 'A';
					output[i++] = 'E';
					break;
				}
				if (*p == 0x87) {
					output[i++] = 'C';
					break;
				}
				if (*p >= 0x88 && *p <= 0x8b) {
					output[i++] = 'E';
					break;
				}
				if (*p >= 0x8c && *p <= 0x8f) {
					output[i++] = 'I';
					break;
				}
				if (*p == 0x91) {
					output[i++] = 'N';
					break;
				}
				if ((*p >= 0x92 && *p <= 0x96) || *p == 0x98) {
					output[i++] = 'O';
					break;
				}
				if (*p >= 0x99 && *p <= 0x9c) {
					output[i++] = 'U';
					break;
				}
				if (*p == 0x9d) {
					output[i++] = 'Y';
					break;
				}
				if (*p == 0x9f) {
					output[i++] = 's';
					output[i++] = 's';
					break;
				}
				if (*p >= 0xa0 && *p <= 0xa5) {
					output[i++] = 'a';
					break;
				}
				if (*p == 0xa6) {
					output[i++] = 'a';
					output[i++] = 'e';
					break;
				}
				if (*p == 0xa7) {
					output[i++] = 'c';
					break;
				}
				if (*p >= 0xa8 && *p <= 0xab) {
					output[i++] = 'e';
					break;
				}
				if (*p >= 0xac && *p <= 0xaf) {
					output[i++] = 'i';
					break;
				}
				if (*p == 0xb1) {
					output[i++] = 'n';
					break;
				}
				if ((*p >= 0xb2 && *p <= 0xb6) || *p == 0xb8) {
					output[i++] = 'o';
					break;
				}
				if (*p >= 0xb9 && *p <= 0xbc) {
					output[i++] = 'u';
					break;
				}
				if (*p == 0xbd || *p == 0xbf) {
					output[i++] = 'y';
					break;
				}
				output[i++] = '?';
				break;
			case 0xc5:
				if (*p == 0x92) {
					output[i++] = 'O';
					output[i++] = 'E';
					break;
				}
				if (*p == 0x93) {
					output[i++] = 'o';
					output[i++] = 'e';
					break;
				}
				output[i++] = '?';
				break;
			default:	// unknown shift
				output[i++] = '?';
			}
			shift = 0;
		} else {
			// UTF-8 shift
			shift = *p;
		}
		p++;
	}
	output[i] = '\0';
	return (output);
}

static bool want_to_print(const char *param, uint8_t delsys, uint16_t value)
{
	uint16_t i;
	uint8_t j;
	bool found_param = false;

	for (i = 0; i < NUM_XML_PARAMS - 1; i++) {
		if (strcmp(xml_params[i].name, param))
			continue;
		found_param = true;
		for (j = 0; j < 32; j++) {
			if (xml_params[i].delsys[j] == 0)
				break;
			if (xml_params[i].delsys[j] == delsys) {
				if (value != xml_params[i].xml_default)
					return true;
				else
					return false;
			}
		}
	}

	if (!found_param)
		warning("could not find \"%s\" in list of xml_params\n", param);
	return false;
}

#define needs_param(p) (want_to_print(#p, t->delsys, t->p))

static const char *get_indent(int indent)
{
	static char buf[256];
	memset(&buf[0], ' ', 255);
	buf[255] = 0;
	buf[3 * indent] = 0;
	return &buf[0];
}

void xml_dump(FILE * dest, pList transponders, struct w_scan_flags *flags)
{
	struct transponder *t;
	struct service *s;
	static char buffer[STRING_BUFFER_SIZE];
	int indent = 0;
	int i = 0;

	fprintf(dest, "<?xml version=\"1.0\" ?>\n");
	fprintf(dest,
		"<!DOCTYPE service_list SYSTEM \"https://raw.githubusercontent.com/stefantalpalaru/w_scan2/master/doc/service_list.dtd\">\n");
	fprintf(dest, "\n");
	fprintf(dest, "<!-- NOTE:\n");
	fprintf(dest, "     if reading or writing w_scan2 XML file format:\n");
	fprintf(dest, "        - please validate XML against DTD above.\n");
	fprintf(dest, "        - indent each XML element\n");
	fprintf(dest,
		"        - indent using three spaces, dont use <TAB> char to indent.\n");
	fprintf(dest,
		"        - conform to requirements mentionend in DTD file.\n");
	fprintf(dest, " -->\n\n");
	fprintf(dest, "%s<service_list>\n", get_indent(indent));

	// TRANSPONDERS
	indent++;
	fprintf(dest, "%s<transponders>\n", get_indent(indent));
	for (t = transponders->first; t; t = t->next) {
		indent++;
		fprintf(dest,
			"%s<transponder ONID=\"%u\" NID=\"%u\" TSID=\"%u\">\n",
			get_indent(indent),
			t->original_network_id,
			t->network_id, t->transport_stream_id);
		indent++;
		fprintf(dest,
			"%s<params delsys=\"%s\" center_frequency=\"%.3f\">\n",
			get_indent(indent),
			delivery_system_name(t->delsys),
			(double)t->frequency / 1e6);
		indent++;

		if needs_param(modulation)
			fprintf(dest,
				"%s<param modulation=\"%s\"/>\n",
				get_indent(indent),
				modulation_name(t->modulation));
		if needs_param(bandwidth)
			fprintf(dest,
				"%s<param bandwidth=\"%.3f\"/>\n",
				get_indent(indent),
				(double)t->bandwidth / 1e6);
		if needs_param(coderate)
			fprintf(dest,
				"%s<param coderate=\"%s\"/>\n",
				get_indent(indent),
				coderate_name(t->coderate));
		if needs_param(transmission)
			fprintf(dest,
				"%s<param transmission=\"%s\"/>\n",
				get_indent(indent),
				transmission_mode_name(t->transmission));
		if needs_param(guard)
			fprintf(dest,
				"%s<param guard=\"%s\"/>\n",
				get_indent(indent),
				guard_interval_name(t->guard));
		if (t->hierarchy != HIERARCHY_NONE) {
			// print those only if hierarchy is used.
			if needs_param(hierarchy)
				fprintf(dest,
					"%s<param hierarchy=\"%s\"/>\n",
					get_indent(indent),
					hierarchy_name(t->hierarchy));
			if needs_param(alpha)
				fprintf(dest,
					"%s<param alpha=\"%s\"/>\n",
					get_indent(indent),
					alpha_name(t->alpha));
			if needs_param(terr_interleaver)
				fprintf(dest,
					"%s<param terr_interleaver=\"%s\"/>\n",
					get_indent(indent),
					interleaver_name(t->terr_interleaver));
			if needs_param(coderate_LP)
				fprintf(dest,
					"%s<param coderate_LP=\"%s\"/>\n",
					get_indent(indent),
					coderate_name(t->coderate_LP));
			if needs_param(priority)
				fprintf(dest,
					"%s<param priority=\"%s\"/>\n",
					get_indent(indent),
					bool_name(t->priority));
		}
		if needs_param(mpe_fec)
			fprintf(dest,
				"%s<param mpe_fec=\"%s\"/>\n",
				get_indent(indent), bool_name(t->mpe_fec));
		if needs_param(time_slicing)
			fprintf(dest,
				"%s<param time_slicing=\"%s\"/>\n",
				get_indent(indent),
				bool_name(t->time_slicing));
		if needs_param(plp_id)
			fprintf(dest,
				"%s<param plp_id=\"%u\"/>\n",
				get_indent(indent), t->plp_id);
		if needs_param(system_id)
			fprintf(dest,
				"%s<param system_id=\"%d\"/>\n",
				get_indent(indent), t->system_id);
		if needs_param(extended_info)
			fprintf(dest,
				"%s<param extended_info=\"%s\"/>\n",
				get_indent(indent),
				bool_name(t->extended_info));
		if needs_param(SISO_MISO)
			fprintf(dest,
				"%s<param SISO_MISO=\"%s\"/>\n",
				get_indent(indent),
				bool_name(t->SISO_MISO));
		if ((t->other_frequency_flag != false)
			&& ((t->cells)->count > 0)) {
			struct cell *f;
			if needs_param(other_frequency_flag) {
				fprintf(dest,
					"%s<param other_frequency_flag=\"%s\"/>\n",
					get_indent(indent), bool_name(true));
				fprintf(dest, "%s<frequency_list>\n",
					get_indent(indent));
				indent++;
				for (f = t->cells->first; f; f = f->next) {
					if (t->tfs_flag) {
						fprintf(dest,
							"%s<tfs_center>\n",
							get_indent(indent));
					}
				}
				indent--;
				fprintf(dest,
					"%s</frequency_list>\n",
					get_indent(indent));
				}
		}
		if needs_param(polarization)
			fprintf(dest,
				"%s<param polarization=\"%s\"/>\n",
				get_indent(indent),
				polarization_name(t->polarization));
		if (needs_param(orbital_position) && needs_param(west_east_flag)) {
			if (t->orbital_position && t->west_east_flag) {
				fprintf(dest,
					"%s<param orbital_position=\"0x%04x\"/>\n",
					get_indent(indent),
					t->orbital_position);
				fprintf(dest,
					"%s<param west_east_flag=\"%s\"/>\n",
					get_indent(indent),
					west_east_flag_name(t->west_east_flag));
			}
		}
		if needs_param(symbolrate)
			fprintf(dest,
				"%s<param symbolrate=\"%.3f\"/>\n",
				get_indent(indent),
				((double)t->symbolrate / 1e6));
//    if needs_param                            // not yet part of the transponder struct
//          (input_stream_id)
//              fprintf(dest,
//                "%s<param inputs_stream_id=\"%u\"/>\n", get_indent(indent),
//                t->input_stream_id);
		if needs_param(pilot)
			fprintf(dest,
				"%s<param pilot=\"%s\"/>\n",
				get_indent(indent), pilot_name(t->pilot));
		if needs_param(rolloff)
			fprintf(dest,
				"%s<param rolloff=\"%s\"/>\n",
				get_indent(indent),
				rolloff_name(t->rolloff));
//    if needs_param                            // not yet part of the transponder struct
//          (interleave)
//              fprintf(dest,
//                "%s<param interleave=\"%s\"/>\n", get_indent(indent),
//                interleaver_name(t->interleave));
		if needs_param(multiple_input_stream_flag)
			fprintf(dest,
				"%s<param multiple_input_stream_flag=\"%s\"/>\n",
				get_indent(indent),
				bool_name(t->multiple_input_stream_flag));
		if needs_param(scrambling_sequence_selector)
			fprintf(dest,
				"%s<param scrambling_sequence_selector=\"%s\"/>\n",
				get_indent(indent),
				bool_name(t->scrambling_sequence_selector));
		if needs_param(scrambling_sequence_index)
			fprintf(dest,
				"%s<param scrambling_sequence_index=\"%u\"/>\n",
				get_indent(indent),
				t->scrambling_sequence_index);
		if needs_param(data_slice_id)
			fprintf(dest,
				"%s<param data_slice_id=\"%u\"/>\n",
				get_indent(indent), t->data_slice_id);
		if needs_param(C2_tuning_frequency_type)
			fprintf(dest,
				"%s<param C2_System_tuning_frequency_type=\"%s\"/>\n",
				get_indent(indent),
				frequency_type_name(t->C2_tuning_frequency_type));
		if needs_param(active_OFDM_symbol_duration)
			fprintf(dest,
				"%s<param active_OFDM_symbol_duration=\"%s\"/>\n",
				get_indent(indent),
				ofdm_symbol_duration_name(t->active_OFDM_symbol_duration));
		indent--;
		fprintf(dest, "%s</params>\n", get_indent(indent));
		indent--;
		fprintf(dest, "%s</transponder>\n", get_indent(indent));
		indent--;
	}
	fprintf(dest, "%s</transponders>\n", get_indent(indent));

	// SERVICES
	fprintf(dest, "%s<services>\n", get_indent(indent));
	for (t = transponders->first; t; t = t->next) {
		for (s = (t->services)->first; s; s = s->next) {
			if ((s->video_pid || s->audio_pid[0])
				&& (flags->ca_select || !s->scrambled)) {
				indent++;
				fprintf(dest,
					"%s<service ONID=\"%u\" TSID=\"%u\" SID=\"%u\">\n",
					get_indent(indent),
					t->original_network_id,
					t->transport_stream_id, s->service_id);
				indent++;

				fprintf(dest, "%s<name char256=\"%s\"/>\n",
					get_indent(indent),
					safe_xml(s->service_name,
						(char *)&buffer));
				fprintf(dest, "%s<provider char256=\"%s\"/>\n",
					get_indent(indent),
					safe_xml(s->provider_name,
						(char *)&buffer));
				fprintf(dest, "%s<pcr pid=\"%u\"/>\n",
					get_indent(indent), s->pcr_pid);
				fprintf(dest, "%s<streams>\n",
					get_indent(indent));
				indent++;
				// video stream characteristics
				if (s->video_pid) {
					fprintf(dest, "%s<stream",
						get_indent(indent));
					fprintf(dest, " type=\"%u\"",
						s->video_stream_type);
					fprintf(dest, " pid=\"%u\"",
						s->video_pid);
					fprintf(dest, " description=\"%s\"",
						StreamName(s->video_stream_type));
					fprintf(dest, "/>\n");
				}
				// audio streams characteristics
				for (i = 0; i < s->audio_num; i++) {
					fprintf(dest, "%s<stream",
						get_indent(indent));
					if (s->audio_stream_type[i])
						fprintf(dest, " type=\"%u\"",
							s->
							audio_stream_type[i]);
					fprintf(dest, " pid=\"%u\"",
						s->audio_pid[i]);
					fprintf(dest, " description=\"%s\"",
						StreamName(s->audio_stream_type[i]));
					if (s->audio_lang && s->audio_lang[i][0])
						fprintf(dest,
							" language_code=\"%s\"",
							safe_xml(s->audio_lang[i],
								(char *) &buffer));
					fprintf(dest, "/>\n");
				}
				if (s->ac3_num) {
					for (i = 0; i < s->ac3_num; i++) {
						fprintf(dest, "%s<stream",
							get_indent(indent));
						fprintf(dest, " type=\"%u\"",
							s->ac3_stream_type[i]);
						fprintf(dest, " pid=\"%u\"",
							s->ac3_pid[i]);
						fprintf(dest,
							" description=\"AC3 audio\"");
						if (s->ac3_lang && s->ac3_lang[i][0])
							fprintf(dest,
								" language_code=\"%s\"",
								safe_xml(s->ac3_lang[i],
									(char *) &buffer));
						fprintf(dest, "/>\n");
					}
				}
				// teletext stream chacteristics
				if (s->teletext_pid) {
					fprintf(dest,
						"%s<stream type=\"6\" pid=\"%u\" description=\"teletext\"/>\n",
						get_indent(indent),
						s->teletext_pid);
				}
				// subtitling streams chacteristics
				if (s->subtitling_num) {
					for (i = 0; i < s->subtitling_num; i++) {
						fprintf(dest, "%s<stream",
							get_indent(indent));
						fprintf(dest, " type=\"6\"");
						fprintf(dest, " pid=\"%u\"",
							s->subtitling_pid[i]);
						fprintf(dest,
							" description=\"subtitling\"");
						if (s->subtitling_lang && s->subtitling_lang[i][0])
							fprintf(dest,
								" language_code=\"%s\"",
								safe_xml(s->subtitling_lang[i],
									(char *) &buffer));
						fprintf(dest, "/>\n");
					}
				}
				indent--;
				fprintf(dest, "%s</streams>\n",
					get_indent(indent));

				if (s->scrambled) {
					fprintf(dest, "%s<CA_systems>\n",
						get_indent(indent));
					indent++;
					for (i = 0; i < s->ca_num; i++) {
						fprintf(dest, "%s<CA_system",
							get_indent(indent));
						fprintf(dest, " name=\"%s\"",
							CaName(s->ca_id[i]));
						fprintf(dest, " ca_id=\"%u\"",
							s->ca_id[i]);
						fprintf(dest, "/>\n");
					}
					indent--;
					fprintf(dest, "%s</CA_systems>\n",
						get_indent(indent));
				}
				indent--;

				fprintf(dest, "%s</service>\n",
					get_indent(indent));
				indent--;
			}
		}
	}
	fprintf(dest, "%s</services>\n", get_indent(indent));
	indent--;

	fprintf(dest, "%s</service_list>\n", get_indent(indent));
}
