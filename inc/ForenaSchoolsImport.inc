<?php
require_once 'ForenaSchools.inc';
require_once 'ForenaSchoolsCSV.inc';
class ForenaSchoolsImport extends ForenaSchools {

	static function instance() {
		static $o = '';
		if (!$o) {
			$o = new ForenaSchoolsImport();
		}
		return $o;
	}

	public function preProcessImport($script, &$context) {
  	$lines = explode( "\n", $script);
		$variables = $context['sandbox']['variables'] = array();
		$context['sandbox']['max_lines'] = count($lines);
		$context['sandbox']['line'] = 0;
		$context['sandbox']['lines'] = $lines;
	}

	public function processImport($script, &$context) {
		  $output ='';
		  set_time_limit(3600);
		  if (empty($context['sandbox'])) {
		  	$this->preProcessImport($script, $context);
		  }
		  $l = $context['sandbox']['line'];
		  $defaults = $context['sandbox']['variables'];
      $context['sandbox']['line'] = $l + 1;
		  $line = $context['sandbox']['lines'][$l];
	  	$buffer = trim($line);
      if ($buffer) {
	      // = Implies Variables
	      if(strpos(trim($buffer),'#')===0) {
	      	$output = trim($buffer, '#');
	      	sleep(1);
	      }
	      elseif (strpos($buffer,"=")>0)  {
	      	list($var,$value) = explode("=",$buffer,2);
	      	$var = trim($var);
	      	$value= trim($value);
	      	if ($var=='import_directory') {
	      	  $value= rtrim($value,'/').'/';
	      	}

	      	$defaults[$var]=$value;
	      	$context['sandbox']['variables'] = $defaults;
	      }
	      // Space delimited implies table import
	      elseif (strpos($buffer,'>') >0) {
	      	$csv = new ForenaSchoolsCSV();
	        list($filename,$dest) = explode('>',$buffer,2);
	        list($table_name,$option) = explode(' ',$dest,2);
	        $table_name = trim($table_name);
	        $option = trim($option);
	        $import_dir = $defaults['import_directory'];
	      	$filename = trim($filename);
	      	if (strtolower($option=='nodelete')) {
	      		$delete = false ;
	      		$del_msg = ' no delete specified';
	      	} else {
	      		$delete = true;
	      		$del_msg = '';
	      	}
          $filepath = $import_dir. $filename;

	      	$output = 'Imported '.$table_name .' from '.$filename . ' '. $del_msg . ' '
	      	  . $csv->table_from_csv($table_name,$filepath,$delete,$defaults);
	      	sleep(1);
	      } elseif (strpos($buffer,'(')>0) {
	      	$output = "Completed $buffer....\n";
	      	$output .=  $this->db->call($buffer,$defaults)."\n";
	      	sleep(1);
	      }
	    }
		  $context['finished'] = ($l + 1) / $context['sandbox']['max_lines'];
	    if ($output) {
	      $context['message'] = $output;
	    }
	  }



}