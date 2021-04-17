<?
/* generate_satellites_dat.php // (c) 2021 Jean-Marc Wislez
 * This script generates a satellites.dat file, based on kingofsat.net or fastsatfinder.com ini-files
 * It is written in php, just because that's the language I'm most fluent in, and anyway this script only needs to be run every few years
 * Instructions:
 * - with a download application (e.g. "Simple mass downloader" on Firefox), download all .ini files from https://en.kingofsat.net/satellites.php or https://www.fastsatfinder.com/transponders.html#transponders-sorted
 * - put this script in the same directory as the .ini files
 * - run "php -n generate_satellites_dat.php > satellites.dat"
 * - move the autogenerated satellites.dat file to w_scan2/src
 * - compile w_scan2
 * Note: if a file satellite_names.csv is available (with columns "ini filename","satellite name", no header line), the correct satellite names will be used
 */

print ("/******************************************************************************\n");
print (" * This file provides autoscan initial data for w_scan2.\n");
print (" * This file is autogenerated using 'generate_satellites_dat.php'\n");
print (" * based on ini-files downloaded from the net.\n");
print (" *\n");
print (" * Transponder data is encoded as a dump of the __sat_transponder struct.\n");
print (" * as defined in satellites.h.\n");
print (" * This file was autogenerated on ".date(DATE_RFC2822).".\n");
print (" *****************************************************************************/\n\n");

$error = false;
$in_dvb = false;
$in_sattype = false;
$sat_id = "";
$sat_list = "";
$enum_satellite = "";
$flag = "";
$satellite_name = array();

if(file_exists("satellite_names.csv")) {
	$namefile = fopen("satellite_names.csv", "r");
	while ($line = fgets($namefile)) {
		$field = explode(",", $line);
		$ini_filename = $field[0];
		$satellite_name[$ini_filename] = $field[1];
	}
	fclose($namefile);
}

$files = scandir(".");
foreach($files as $file) {
	if(strstr($file, ".ini")) {
		$inifile = fopen($file, "r");
		$line_number = 0;
		$update_date = "";
		$update_source = "";
		$source_info = "";
		$in_update = false;
		$in_sattype = false;
		$in_dvb = false;
		while ($line = fgets($inifile)) {
			$line_number++;
			if ($line_number == 1) {
				$source_info = substr($line, strpos($line, "["), -2);
			}
			else if (trim($line) == "[UPDATE]") {
				$in_update = true;
				$in_sattype = false;
				$in_dvb = false;
			}
			else if (trim($line) == "[SATTYPE]") {
				$in_update = false;
				$in_sattype = true;
				$in_dvb = false;
			}
			else if (trim($line) == "[DVB]") {
				$in_update = false;
				$in_sattype = false;
				$in_dvb = true;
			}
			else if ($in_update) {
				if (substr($line, 0, 2) == "0=") {
					$update_date = substr(trim($line), 2);
				}
				if (substr($line, 0, 2) == "2=") {
					$update_source = substr(trim($line), 2);
					$source_info = "[ $update_source (c) $update_date ]";
				}
				
			}
			else if ($in_sattype) {
				if (substr($line,0,2) == "1=") {
					if ($sat_id) {
						print ("E(__$sat_id)\n\n");
					}
					$sat_pos=intval(substr($line,2,5));
					if ($sat_pos > 1800) {
						$sat_id = "S".floor((3600 - $sat_pos) / 10)."W".((3600 - $sat_pos) % 10);  
						$sat_pos = 3600 - $sat_pos;
						$flag = "WEST_FLAG";
					}
					else {
						$sat_id = "S".floor($sat_pos / 10)."E".($sat_pos % 10);  
						$flag = "EAST_FLAG";
					}
					$enum_satellite .= "$sat_id,\n";
					if (array_key_exists($file, $satellite_name)) {
						$sat_name = trim($satellite_name[$file])." (".($sat_pos/10)." ".substr($flag, 0, 4).")";
					}
					else {
						$sat_name = "Unnamed satellite (".($sat_pos/10)." ".substr($flag, 0, 4).")";
					}
					$sat_list .= "{ \"$sat_id\", $sat_id, \"$sat_name\", __$sat_id, SAT_TRANSPONDER_COUNT(__$sat_id), $flag, 0x$sat_pos, -1, \"S".($sat_pos/10).substr($flag,0,1)."\", 0 },\n";
					print ("/******************************************************************************\n");
					print (" * $sat_id ($file) - $sat_name\n");
					print (" * $source_info\n");  
					print (" ******************************************************************************/\n");
					print ("B(__$sat_id)\n");
				}
			}
			else if ($in_dvb) {
				if (substr($line, 0, 2) != "0=" and strpos($line, "=")) {
					$field = explode(",", str_replace(";", ",", substr($line, strpos($line, "=") + 1)));
					$frequency = trim($field[0]);
					switch(trim($field[1])) {
					case "H": $polarization = 0; break;
					case "V": $polarization = 1; break;
					case "R": $polarization = 2; break;
					case "L": $polarization = 3; break;
					default: print ("/* NOTE: polarization '".trim($field[1])."' not supported, using horizontal */\n"); $polarization = 0; break;
					}
					$symbolrate = trim($field[2]);
					switch(trim($field[3])) { // defined in linux kernel dvb/frontend.h
					case "None": $fec = 0; break;
					case "12": $fec = 1; break;
					case "23": $fec = 2; break;
					case "34": $fec = 3; break;
					case "45": $fec = 4; break;
					case "56": $fec = 5; break;
					case "67": $fec = 6; break;
					case "78": $fec = 7; break;
					case "89": $fec = 8; break;
					case "Auto": $fec = 9; break;
					case "35": $fec = 10; break;
					case "91":
					case "910": $fec = 11; break;
					case "25": $fec = 12; break;
					default: print ("/* NOTE: FEC '".trim($field[3])."' not supported, using auto */\n"); $fec = 9; break;
					}
					if (array_key_exists(4, $field)) { // defined in linux kernel dvb/frontend.h
						switch(trim($field[4])) {
						case "DCII":
						case "DSS": $delsys = 4; break;
						case "DVB-S": $delsys = 5; break;
						case "S2": $delsys = 6; break;
						case "ISDB": $delsys = 9; break;
						case "8PSK": // hack
								$delsys = 5;
								$modulation = 9;
								break;
						default: print ("/* NOTE: Delivery system '".trim($field[4])."' not supported, using DVB-S */\n"); $delsys = 5; break;
						}
					}
					else {
						$delsys = 5;
					}
					if (array_key_exists(5, $field)) {
					switch(trim($field[5])) {
						case "QPSK": $modulation = 0; break;
						case "8PSK ACM/VCM":
						case "8PSK": $modulation = 9; break;
						case "16APSK": $modulation = 10; break;
						case "32APSK": $modulation = 11; break;
						default: print ("/* NOTE: Modulation '".trim($field[5])."' not supported, using QPSK */\n"); $modulation = 0; break;
						}
					}
					else {
						$modulation = 0;
					}
					print ("{ $delsys, $frequency, $polarization, $symbolrate, $fec, 0, $modulation },\n"); // NOTE: rolloff is always set to AUTO
				}
			}
		}
		fclose($inifile);
	}
}
if ($sat_id) {
	print ("E(__$sat_id)\n\n");
}
		
print ("/******************************************************************************\n");
print (" * Every satellite has its own number here.\n");
print (" * COPY FROM TABLE BELOW - DO NOT EDIT BY HAND.\n");
print (" *****************************************************************************/\n");
print ("enum __satellite {\n$enum_satellite};\n\n");

print ("/******************************************************************************\n");
print (" * position constants, east/west flag as separator, 22.0 west => S22W0\n");
print (" * .X if two or more sats on nearly same position. <- deprecated.\n");
print (" * satellites sorted by position\n");
print (" *****************************************************************************/\n");

print ("struct cSat sat_list[] = {\n");
print ("/** pos *** id *** long satellite name ***************************** items ************ item_count ********** we_flag * orbit * rotor * vdrid * skew */\n");
print ($sat_list);
print ("};\n");

?>
