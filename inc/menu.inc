<?php

function menu_dispatch($q) {
  // Separate page/extensions.
  // @TODO: Replace with filename parsing functons.
  list($filename, $ext) = explode('.', $q, 2);
  // Is there a report
  if ($_GET['debug']) {
    $flex_file = 'flex-debug/' . $filename. '.swf';
  }
  else {
    $flex_file = 'flex/' . $filename. '.swf';
  }
  $flex_exists = file_exists($flex_file);
  // Is there a file
  $report_file = 'reports/' . $filename .'.frx';
  $report_name = $filename;
  $flash_ver = $_COOKIE['PEDAGOGGLE_FLEX'];
  $report_exists = file_exists($report_file);
  // @TODO: Add IPAD browser detection
  $popup = @$_GET['popup']==TRUE;

  // Check if a flex app exists
  if ($flex_exists && !$ext && $flash_ver || ($flex_exists && !$report_exists))  {
	  if (flex_access($q)) {
	  	  $o = service_object($q);
	  	  $title = $o->title;
	  	  if (!$title) $title = $_GET['name'];
		    render_flex($q,$title, $popup);
    } else {
  	  render_page('error','Access Denied','Please see your system administrator if you believe this to be in error.');
    }
  }
  elseif ($report_exists) {
  	// Render report
  	require_once('forena/forena.standalone.inc');
  	$page_output = FrxReportGenerator::instance()->report($q,null, $fmt);
  	$title = FrxReportGenerator::instance()->title;

  	if ($ext && $ext != 'web') {
  		print $page_output;
  	}
  	else {

  		render_page('page', $title, $page_output);
  	}
  }
  else {
  	render_page('error',$q . ' Page Not Found','Please see your system administator if you believe this to be in error.');
  }
}

function service_object($class) {
  $class_file = 'services/'.$class.'.data.php';
  $data='';
  // Only load the file if the file name exists
  if (file_exists($class_file)) {
    include_once($class_file);
    $handler = new DataBroker();
    return $handler;
  }
}

function app_method($class,$method) {
	$class_file = 'data/'.$class.'.data.php';
	$data='';
  $handler = service_object($class);
	if ($handler && method_exists($handler,$method)) {
	    //header('Content-type: text/xml');
	    $data = call_user_func(array($handler,$method));
	}
	return $data;
}

function flex_access($class) {
	$class_file = 'services/'.$class.'.data.php';
	$method = 'auth';
	$data=true;
	// Only load the file if the file name exists
	if (file_exists($class_file)) {
	  include_once($class_file);
	  $handler = new DataBroker();
	  if (method_exists($handler,$method)) {
	    //header('Content-type: text/xml');
	    $data = call_user_func(array($handler,$method));
	     if (!$data)
	  	   db_log('Access Denied to ',"$class \n User: \n" . print_r(current_user(),1),'access');
	  }
	}
	return $data;
}


