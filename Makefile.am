AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip
bin_PROGRAMS = w_scan2
w_scan2_SOURCES	= src/atsc_psip_section.c src/atsc_psip_section.h \
				  src/countries.c src/countries.h \
				  src/descriptors.c src/descriptors.h \
				  src/diseqc.c src/diseqc.h \
				  src/dump-dvbscan.c src/dump-dvbscan.h \
				  src/dump-vdr.c src/dump-vdr.h \
				  src/dump-xine.c src/dump-xine.h \
				  src/dump-mplayer.c src/dump-mplayer.h \
				  src/dump-vlc-m3u.c src/dump-vlc-m3u.h \
				  src/dvbscan.c src/dvbscan.h \
				  src/lnb.c src/lnb.h \
				  src/parse-dvbscan.c src/parse-dvbscan.h \
				  src/satellites.c src/satellites.h src/satellites.dat \
				  src/scan.c src/scan.h \
				  src/section.c src/section.h \
				  src/tools.c src/tools.h \
				  src/emulate.c src/emulate.h \
				  src/dump-xml.c src/dump-xml.h \
				  src/iconv_codes.c src/iconv_codes.h \
				  src/char-coding.c src/char-coding.h \
				  src/extended_frontend.h \
				  src/si_types.h

dist_man_MANS = doc/w_scan2.1
EXTRA_DIST = doc \
			 .ycm_extra_conf.py \
			 autogen.sh \
			 README.md
AM_LDFLAGS = -lrt
AM_CFLAGS = -Wall -Wextra -Wno-comment -Wswitch-default -Wno-unused-parameter

