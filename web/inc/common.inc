<?php

// Set the include path to include theme and inc directories. The test catches win32 development boxes which use semicolons for path separation.
if (strpos($_SERVER['SERVER_SOFTWARE'],'Win32') >0 ) {
  ini_set('include_path',ini_get('include_path').';inc;conf;lib;local;theme');
} else {
  ini_set('include_path',ini_get('include_path').':inc:conf:lib:local:theme');
}

require_once('db.inc');
session_start();
error_reporting(E_ALL ^ E_NOTICE);
// Trying the following to get around line ending problems.
ini_set('auto_detect_line_endings',true);
set_error_handler('custom_error_logger');

require_once('auth.inc');
session_name('pedaGoggle');
function custom_error_logger($level,$message,$file,$line) {
  global $errors;
  if ($level != E_NOTICE && ($level != E_STRICT)) {
    $title = 'PHP Error('.$level.') in  '. $file. ' line '. $line;
    $errors[] = $title;
    db_log($title,$message,'php');
  }
}
