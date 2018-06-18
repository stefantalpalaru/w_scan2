## description

*w\_scan2* is a small channel scan tool which generates ATSC, DVB-C, DVB-S/S2 and DVB-T channels.conf files.

It's based on the old "scan" tool from linuxtv-dvb-apps-1.1.0. The differences are:
- no initial tuning data needed, because scanning without this data is exactly
  what a scan tool like this should do
- it detects automatically which DVB/ATSC card to use
- much more output formats, interfacing to other dtv software.

*w\_scan2* is a fork of the original *w\_scan* from http://wirbel.htpc-forum.de/w_scan/index2.html

#### main changes from **w\_scan** to **w\_scan2**

- keep duplicate transponders by default because a stronger transponder with
  the same ID might have a higher frequency and be discarded simply because
  it's scanned later. Also don't replace the current transponder with an
  advertised one by default. The latter may have a lower signal strength.
  More details here:
  https://stefantalpalaru.wordpress.com/2016/02/04/scan-all-the-things/ .
  The old behaviour can be enabled with -d (--delete-duplicate-transponders).

## requirements

- up-to-date(!) DVB kernel headers with DVB API 5.3 support

## build

```sh
./autogen.sh
./configure
make
```

## distro support

#### Gentoo

```sh
layman -a stefantalpalaru
emerge w_scan2
```

## basic usage

NOTE: Newer versions of *w\_scan2* need the '-c' option for specifying the
country (ATSC, DVB-C and DVB-T) or '-s' for satellite (DVB-S).


#### DVB-C (using Germany as country, option -c)

```sh
./w_scan2 -fc -c DE >> channels.conf
```

#### DVB-T

```sh
./w_scan2 -c DE >> channels.conf
```

#### DVB-C and DVB-T

```sh
./w_scan2 -c DE >> channels.conf && ./w_scan2 -fc -c DE >> channels.conf
```

#### ATSC (terrestrial, using United States as country)

```sh
./w_scan2 -fa -c US >> channels.conf
```

#### US Digital Cable (QAM Annex-B)

```sh
./w_scan2 -A2 -c US >> channels.conf
```

#### ATSC, both terrestrial and digital cable

```sh
./w_scan2 -A3 -c US >> channels.conf
```

#### DVB-S, here: Astra 19.2 east

```sh
./w_scan2 -fs -s S19E2
```

NOTE: see ```./w_scan2 -s?``` for list of satellites.

#### generate (dvb)scan initial-tuning-data

```sh
./w_scan2 -c DE -x > initial_tuning_data.txt
```

#### generate kaffeine-0.8.5 channels.dvb

```sh
./w_scan2 -c DE -k > channels.dvb
```

For more sophisticated scan options see ```./w_scan2 -h``` and ```./w_scan2 -H```.

## credits

- "wirbel" Winfried Koehler <w_scan AT gmx-topmail DOT de> - the original author
- "e9hack" Hartmut Birr for onid-patch (2006-09-01)
- "seaman" giving his his Airstar2 for testing purposes to me
- "Wicky" for testing with Airstar2/Zarlink MT352 DVB-T
- "kilroy" for testing with Airstar2/Zarlink MT352 DVB-T and Avermedia
- "Fabrizio" for testing with Airstar2/Zarlink MT352 DVB-T
- Arturo Martinez <martinez@embl.de> for a huge bunch of tests on DVB-S/DVB-S2
- Rolf Ahrenberg for doing DVB-T/T2 tests and suggestions to improve *w\_scan*
- Ștefan Talpalaru <stefantalpalaru@yahoo.com> - the *w\_scan2* fork

## license

*w\_scan2* is licensed under GPLv2. See the included file COPYING for details.

## homepage

https://github.com/stefantalpalaru/w_scan2

