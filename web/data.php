<?php
/**
* This is the data gateway for the flex php application
* It basically instantiates a class based on the "class" get parameter
* and then invokes the method based on the data
*/
require_once('config/settings.php');
require_once('inc/default_config.php');
require_once('inc/common.inc');
$error = db_check_connection_failed();

// Determine if the include file is in the path.
$class = $_REQUEST['service'];
$method = $_REQUEST['method'];
$class_file = 'services/'.$class.'.data.php';

// Only load the file if the file name exists
if (file_exists($class_file)) {
  include_once($class_file);
  $handler = new DataBroker();
  $auth = true;
  if (method_exists($handler,'auth')) {
  	$auth = call_user_func(array($handler,'auth'));
  }

  if ($auth) {
	  if (method_exists($handler,$method)) {
	    //header('Content-type: text/xml');
	    $data = call_user_func(array($handler,$method));
	    if ($data_debug==true && $class!='LogViewer') {
	    	db_log('Data '. $class . '.' . $method, htmlspecialchars(print_r($_REQUEST,1)). "\n" . htmlspecialchars($data),'dataService');
	    }
	    print $data;
	  }
	  else {
	    db_log('Method Not Found',print_r($_REQUEST,1),'dataService');
	    print "<pre>$method: Method Not Found</pre>";
	  }
  } else {
  	db_log('Data Service Access Denied',"Service: $class \n Method:$method \n User: \n" . print_r(current_user(),1),'access');
  	print "<pre> Access Denied</pre>";
  }
}
else {
    db_log('Data Service  Not Found',print_r($_REQUEST,1),'dataService');
	print "<pre>$class: Service Not Found</pre>";
}
