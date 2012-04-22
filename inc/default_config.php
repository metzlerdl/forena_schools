<?php
GLOBAL $install_home, $db_connection_string, $_forena_repositories;
$_forena_repositories['menus'] = array(
  'path' => $install_home. '/data/menus',
  'title' => 'Menus'
);

$_forena_repositories['ims'] = array(
  'path' => $install_home. '/data_blocks/ims',
  'title' => 'Testing pedagoggle IMS',
  'uri' => $db_connection_string,
  'user callback' => 'current_login',
  'search path' => 'public,import',
  'debug' => @$_GET['fdebug'],
);