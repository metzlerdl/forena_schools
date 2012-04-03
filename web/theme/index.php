<?php
include_once('inc/default_config.php');
include_once('config/settings.php');
require_once('inc/common.inc');
require_once('forena/forena.common.inc');
require_once('inc/theme.inc');
require_once('inc/menu.inc');
$error = db_check_connection_failed();
if ($error) {
  render_page('error','Database Connection Error',$error);
  die;
} else {
  check_auth();
  $path = $_GET['q'];

  if (!$path || ($path=='home')) {
    $user=current_user();
    $role = $user['role'];
    switch($role) {
      case 'sys_admin':
        $path='AdminDashboard';
        break;
      case 'dist_admin':
          $path='DistrictDashboard';
          break;
      case 'bldg_admin':
        $path='BuildingDashboard';
        break;
      case 'data_entry':
        $path='DataEntryDashboard';
        $_GET['bldg_id'] = $user['bldg_id'];
        break;
      case 'teacher':
        $path='TeacherDashboard';
          break;
      case 'student':
        $path='StudentDashboard';
        break;
      default:
        render_page('error','Access Denied: '. $user['login']);
        die;
    }
    menu_dispatch($path);
  }
  if ($path=="logout") {
    logout();
  }
}

