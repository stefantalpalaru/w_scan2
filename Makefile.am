AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip
bin_PROGRAMS = w_scan2
w_scan2_SOURCES	= atsc_psip_section.c atsc_psip_section.h \
				  countries.c countries.h \
				  descriptors.c descriptors.h \
				  diseqc.c diseqc.h \
				  dump-dvbscan.c dump-dvbscan.h \
				  dump-vdr.c dump-vdr.h \
				  dump-xine.c dump-xine.h \
				  dump-mplayer.c dump-mplayer.h \
				  dump-vlc-m3u.c dump-vlc-m3u.h \
				  dvbscan.c dvbscan.h \
				  lnb.c lnb.h \
				  parse-dvbscan.c parse-dvbscan.h \
				  satellites.c satellites.h satellites.dat \
				  scan.c scan.h \
				  section.c section.h \
				  tools.c tools.h \
				  emulate.c emulate.h \
				  dump-xml.c dump-xml.h \
				  iconv_codes.c iconv_codes.h \
				  char-coding.c char-coding.h \
				  extended_frontend.h \
				  si_types.h

dist_man_MANS	= doc/w_scan2.1
EXTRA_DIST	= doc \
			  .ycm_extra_conf.py \
			  autogen.sh \
			  README.md
AM_LDFLAGS = -lrt
AM_CFLAGS = -Wall -Wextra -Wno-comment -Wswitch-default -Wno-unused-parameter

