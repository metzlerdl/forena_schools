<?php
/**
* Global settings for the application
*/
// Database conenction stirng
//$db_connection_string = 'host=mypostgreshost dbname=pedaprod user=webdev password=changeme';
// The url to the app_home directory within the site.
$base_url = 'http://ims.example.com';
// Installation location of pedagoggle
$install_home = '/dirtoapacheroot/imssubfolder';
// Set this to true to enable logging of every query sent to the database
$db_debug=TRUE;
$data_debug=TRUE;
// This should only be set for the back door when cas is not enabled.   It lets you be a user without authentication.  Use with care!
//$default_user ='atest';
// Cas configuration
// Uncomment the following line to enable cas integration with the server indicated.
$cas_config['server']='cas.example.com';
$cas_config['port'] = 443;
$cas_config['version']='2.0';
global $_forena_repositories;


