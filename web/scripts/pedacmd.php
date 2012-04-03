<?php

ini_set('include_path',ini_get('include_path').':../inc:../config:../lib:../local:../theme');
$_SESSION = array('login'=>'metzlerd');
error_reporting(E_ALL ^ E_NOTICE);
require_once('settings.php');
require_once('db.inc');
require_once('auth.inc');
require_once('csvimport.inc');
//require_once('import_preprocessors.php');
// Facke a login
$error = db_check_connection_failed();
if ($error) {
  print "no database connection.\n";
} else {
	print("Starting import Processing\n");
	$defaults = array();
	while (!feof(STDIN)) {
	    $buffer = fgets(STDIN);
	    if ($buffer) {

	      // = Implies Variables
	      if(strpos(trim($buffer),'#')===0) {
	      	// do nothing its a comment
	      }
	      elseif (strpos($buffer,"=")>0)  {
	      	list($var,$value) = explode("=",$buffer,2);
	      	$var = trim($var);
	      	$value= trim($value);
	      	if ($var=='import_directory') {
	      	  $import_dir= rtrim($value,'/').'/';
	      	  print "Import directory is now $import_dir \n";
	      	} else {
	      	  $defaults[$var]=$value;
	      	}
	      }
	      // Space delimited implies table import
	      elseif (strpos($buffer,'>') >0) {
	        list($filename,$dest) = explode('>',$buffer,2);
	        list($table_name,$option) = explode(' ',$dest,2);
	        $table_name = trim($table_name);
	        $option = trim($option);
	      	$filename = trim($filename);
	      	if (strtolower($option=='nodelete')) {
	      		$delete = false ;
	      		$del_msg = ' no delete specified';
	      	} else {
	      		$delete = true;
	      		$del_msg = '';
	      	}
	      	print('Importing '.$table_name .' from '.$filename. $del_msg."\n");
            $filepath = $import_dir.$filename;
	      	print table_from_csv($table_name,$filepath,$delete,$defaults) . "\n";
	      } elseif (strpos($buffer,'(')>0) {
	      	print "Executing $buffer....\n";
	      	print db_call($buffer,$defaults)."\n";
	      }
	    }
	}

}
print ("\nProcessing Complete\n");

