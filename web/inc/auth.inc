<?php

/**
* @file auth.inc.php
*/
define('CAS_NO_VERIFY', 'none');
define('CAS_VERIFY', 'verify');
define('CAS_CA_VERIFY', 'ca_verify');


function check_auth() {
	GLOBAL $cas_config;
	GLOBAL $default_user;
	$user = current_user();
	// Login via cas if appropriately configured.
	if ($cas_config['server'] && !$user) {
	  $login = cas_auth();
    $user = valid_user($login);
	}

	if (!$cas_config['server'] ) {
	  $login = $default_user;  # default user is used to back door if necessary.
	  // Make sure we don't used cached info for permisssions.
	  if (isset($_SESSION['forena_access'])) unset($_SESSION['forena_access']);
    $user = valid_user($login);
	}
	//Determine the users role.
	return $user;
}

function valid_user($login, $bldg_id='') {
	    GLOBAL $default_user;
	    // Check to see if the username is in the database
	    $parms = array('login' => $login, 'bldg_id' => $bldg_id);
      $user = db_first_row(db_query('SELECT p.*,s.role,s.bldg_id FROM p_people p JOIN p_staff s
         ON p.person_id=s.person_id
         where login=:login',$parms));
       if ($user) {
         // Set up session for currently logged in user.
         $_SESSION['pedagoggle_user'] = $user;
         if (!isset($default_user)) db_log($user['login']. " Logged in succesfully",print_r($user,1),'info');
       } else {
         unset($_SESSION['pedagoggle_user']);
       }
	   return $user;
}

function current_user() {
  $user =  $_SESSION['pedagoggle_user'];
	return $user;
}

function current_login() {
	$user = current_user();
	return $user['login'];
}


function cas_auth() {
	GLOBAL $cas_config;
    require_once ('CAS/CAS.php');
  //     phpCAS::setDebug();
    $server_version    = $cas_config['version'];
    $server_cas_server = $cas_config['server'];
    $server_port       = $cas_config['port'];  #  '443');
    $server_uri        = $cas_config['uri']='';
    $cas_cert_verify   =$cas_config['cert_verify']; # CAS_NO_VERIFY);
    $cas_cert          = $cas_cofnig['cert'];

    $start_session = TRUE;

    phpCAS::client($server_version, $server_cas_server, $server_port, $server_uri, $start_session);
    // force CAS authentication
    // Determine if CA option is set.  This is only avialable in version 0.6 or greater, so we need to test
    // to make sure its callable.
    if (is_callable(array(phpCAS, 'setNoCasServerValidation'))) {
      switch ($cas_cert_verify) {

        case CAS_VERIFY:
          phpCAS::setCasServerCert($cas_cert);
          break;

        case CAS_CA_VERIFY:
          phpCAS::setCasServerCACert($cas_cert);
          break;

        case CAS_NO_VERIFY:
        default:
          phpCAS::setNoCasServerValidation();
          break;

      }
    }
    phpCAS::forceAuthentication();
    $cas_name = phpCAS::getUser();
    return $cas_name;
}

function logout() {
	global $cas_config;
	$server = $cas_config['server'];
	$port = $cas_config['port'];
	$uri = $cas_config['uri'];
	session_destroy();
	if ($cas_config['server']) {
				 // Begin constructing logout destination
		  $logout_destination = 'https://'. $server;

		  // Add abnormal port
		  if ($port != '443') {
		    $logout_destination .= ':'. $port;
		  }

		  // Add logout
		    $logout_destination .= $uri .'/logout';
		header('Location: '.$logout_destination);
	}
}


function access_building($building) {
	$user = current_user();
    $parms['bldg_id'] = $building;
    $parms['user_id'] = $user['user_id'];
     $result = db_call('usr_access_building({user_id},{bldg_id})',$parms);
    return $result;
	}

function access_collection($coll_id,$modify=false) {
	$user = current_user();
	$parms['stu_coll_id'] = $coll_id;
	$parms['user_id'] = $user['user_id'];
	$parms['modify'] = $modify;

	$result = db_call('usr_access_collection({user_id},{stu_coll_id},{modify})');
	return result;
}

function access_course($course_id) {
    $parms['course_id'] = $course_id;
    $result = db_call('usr_access_course({user_id},{course_id})',$parms);
    return $result;
}
function access_faculty($faculty_id) {
    $parms['faculty_id'] = $faculty_id;
     $result = db_call('usr_access_faculty({user_id},{faculty_id})',$parms);
    return $result;
}

/**
 * Check access levels
 *
 * @param string $role
 * @return boolean
 */
function access_level($role) {
	global $install_step;
	if ($install_step) RETURN true;
	$user = current_user();
	$ur = $user['role'];

	switch($role) {
		case 'sys_admin':
			$result = ($ur=='sys_admin');
			break;
		case 'dist_admin':
			$result = (($ur=='dist_admin') || ($ur=='sys_admin'));
			break;
		case 'bldg_admin':
			$result = (($ur=='dist_admin') || ($ur=='bldg_admin') || ($ur=='sys_admin'));
			break;
		case 'teacher':
            $result = (($ur=='dist_admin') || ($ur=='bldg_admin') || ($ur=='sys_admin') || ($ur=='teacher'));
            break;
		case 'data_entry':
           $result = (($ur=='dist_admin') || ($ur=='bldg_admin')|| ($ur=='sys_admin') || ($ur=='teacher') || 'data_entry');
            break;
		default:
			$result = false;
	  }
	  return $result;
}