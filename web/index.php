<?php
include_once('config/settings.php');
include_once('inc/default_config.php');
require_once('inc/common.inc');
require_once('inc/theme.inc');
require_once('inc/menu.inc');
$error = db_check_connection_failed();
if ($error) {
  render_page('error','Database Connection Error',$error);
  die;
} else {
  check_auth();
  global $install_step;
  $path = ($install_step) ? $install_step : $_GET['q'];
  css_lib($base_url. 'theme/fluid_grid.css');
	css_lib($base_url. 'theme/style.css');
  if (!$path || ($path=='home')) {
    $user=current_user();
    $role = $user['role'];
    switch($role) {
      case 'sys_admin':
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
  }
  menu_dispatch($path);
  if ($path=="logout") {
    logout();
  }
}

