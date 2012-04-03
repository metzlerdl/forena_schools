<?php
require_once('FrxHostApplication.inc');
global $forena_application_class;
$forena_application_class = 'FrxDrupalApplication';
class FrxDrupalApplication extends FrxHostApplication {
  /**
   * @file HostApp.inc
   * Defines all the interface points between the host application and Forena.
   * Each of these methods must be specified in order for Forena to function properly.
   * The base class here is drupal, so those
   */

	private $default_config = array();
	public function __construct() {
	  $this->default_config['report_repos'] = drupal_get_path('forena', 'module') . '/repos/reports';
	  $this->default_config['default_form'] = 'letter';
	  $this->default_config['doc_defaults'] = array();
	  $this->default_config['library_path'] = 'sites/all/libraries';
	  $this->default_config['doc_formats'] = array();
	}

	/**
	 * Configuration retrieval method
	 * Returns configuration varialbes used by forena.
	 * @param unknown_type $var_name
	 */
	public function configuration($var_name) {
	  $v = variable_get('forena_' . $var_name, @$this->default_config[$var_name]);
	  return $v;
	}


	/**
	 * Convert a relative link to appropriately rendered html
	 * return html A properly formatted anchor tag
	*/
	public function link($title, $path, $options=array()) {
    return l($title, $path, $options);
  }

  /**
   * Theme the output of a css
   * Enter description here ...
   * @param unknown_type $output
   * @param unknown_type $doc_type
   */
  public function theme($r, $title, $doc_type) {
  	$output = $r->html;
    if (!$doc_type || $doc_type='embed') {
      // Set the title and return the output
      $f = drupal_get_form('forena_parameter_form', $r);
      if ($f) $output = drupal_render($f) . $output;
      drupal_set_title(filter_xss($title));
      $output = '<div class="forena-report">' . $output . '</div>';
    }
    else {
      //Build the doucment and theme the output
      print($output);
    }
    return $output;
  }

  /**
   * Add a css file for theming.
   * Enter description here ...
   * @param unknown_type $css_file
   */
  public function add_css($css_file) {
     drupal_add_css($css_file, 'module');
  }

  /**
   * Add a javascript file for theming css
   * Enter description here ...
   * @param unknown_type $js_file
   */
  public function add_js($js_file) {
     drupal_add_js($js_file, 'module');
  }

  /**
   * Convert the report name into a link to the report
   * Enter description here ...
   * @param unknown_type $report_name
   */
  public function report_link($report_name, $title) {

  }

  /**
   * What to do if we don't find a report
   * Enter description here ...
   */
  public function not_found($name) {
   require_once('forena.admin.inc');
   forena_delete_report($name);
   return 'Report Not Found';
  }

  public function forena_path() {
     return drupal_get_path('module', 'forena');
  }


/**
 * Accepts the name of a file
 *
 * Returns a xml object of the file.
 *
*/
function load_report($report_name) {
  $r=null;
  global $language;
  if ($report_name) {
  	$i_report_name = $report_name;
    $report_path =  $this->configuration('report_repos');
    $int_filename = $report_path . '/' . $language->language . '/' . $report_name . '.frx';
    $filename = $report_path . '/' . $report_name . '.frx';
    if (file_exists($int_filename) && @$_GET['language']!= 'en') {
    	$i_report_name = $language->language . '/' . $report_name;
      $r_text = file_get_contents($int_filename);
      $modified = filemtime($int_filename);
      $filename = $int_filename;
    }
    elseif (file_exists($filename)) {
      $r_text = file_get_contents($filename);
      $modified = filemtime($filename);
    }
    $result = db_query("SELECT * FROM {forena_reports} WHERE report_name=:report_name AND language=:language",
      array(':report_name' => $report_name, ':language' => $language->language));
    $save = FALSE;
    if ($rpt = $result->fetchObject()) {
		    // If the file modification time has changed then save.
        if ($modified && $rpt->modified != $modified) {
          $save = TRUE;
        }
        // If the report has been altered by a user then use that.
	      if ($rpt->altered == -1) {
	        if ($rpt->src) {
	          $r_text = $rpt->src;
	        }
	      }
    }
    else if ($r_text) {
        $save = TRUE;
    }

    if ($save) {
      require_once('forena.admin.inc');
      forena_save_report($i_report_name, $r_text, FALSE);
    }
    return $r_text;
  }
}

/**
 * General wrapper procedure for reporting erros
 *
 * @param string $short_message Message that will be displayed to the users
 * @param string $log Message that will be recorded in the logs.
 */
function error($short_message='', $log='') {
  if ($short_message) {
    drupal_set_message(check_markup($short_message), 'error');
  }
  if ($log) {
    watchdog('forena', $log, NULL, WATCHDOG_ERROR);
  }
}

/**
 * Debug handler
 * Enter description here ...
 * @param unknown_type $short_message
 * @param unknown_type $log
 */
function debug($short_message='', $log='') {
  if ($log) {
    watchdog('forena debug', $log, NULL);
  }
  if ($short_message) {
    drupal_set_message(check_markup($short_message));
  }
}

/**
 * Invokes the hooks required for forena plugin registration.
 * Each module returns an array structure that defines multiple plugins
 *
 * class - Indicates the name of the class that will be used to define the plugins behaviors.
 *
 */
function plugins() {
  static $plugins = '';
  if (!$plugins) {
    $plugins = array();
    foreach (module_list() as $module) {
      $function = $module . '_forena_plugins';
      if (function_exists($function)) {
        $returned_plugins = $function();
        if ($returned_plugins) foreach ((array)$returned_plugins as $p) {
          $p['module'] = $module;
          $p['file'] = drupal_get_path('module' , $p['module']) . '/' . $p['file'];
          $plugins[]=$p;
        }
      }
    }
  }
  return $plugins;
}



/**
 * Builds a global array of available controls
 * and returns the array.
 */
function controls() {
  static $controls = '';
  if (!$controls) {
    $controls = array();
    foreach (module_list() as $module) {
      $function = $module . '_forena_controls';
      if (function_exists($function)) {
        $returned_controls = $function();
        if ($returned_controls) foreach ((array)$returned_controls as $c) {
          $c['module'] = $module;
          $c['file'] = drupal_get_path('module' , $c['module']) . '/' . trim($c['file'], '/');
          $controls[]=$c;
        }
      }
    }
  }
  return $controls;
}

/**
 * Return the repositories configured for this applicaiton.
 */
public function repositories() {
  global $_forena_repositories;

  $repos = array();
  // Load the repository list from the global settings.php file.
  if ($_forena_repositories) {
    $repos = $_forena_repositories;
  }
  $path = $this->forena_path();
  // Overide difinitions of the sample and drupal repositories.
  $repos['forena_help'] = array(
    'path' => $path .'/repos/forena_help',
    'title' => 'Forena Help Reports',
  );
  $repos['drupal'] = array(
    'path' => $path . '/repos/drupal',
    'title' => 'Drupal Reports',
  );
  $repos['sampledb'] = array(
    'path' => $path . '/repos/sample',
    'title' => 'Sample DB Repository'
  );
  // Retrieve the repositories defined in the database;
  $results = db_query('SELECT * FROM {forena_repositories}');
  foreach ($results as $r) {
  	if ($r->config) {
  		$new_r = unserialize($r->config);
  	}
  	else {
  		$new_r = array();
  	}
  	$r_name = $r->repository;
  	if (is_array(@$repos[$r_name])) {
  		$new_r = array_merge($new_r, $repos[$r_name]);
  	}
  	else {
  		$new_r['source'] = 'user';
  	}
  	$new_r ['title'] = $r->title;
  	$new_r ['enabled'] = $r->enabled;

  	$repos[$r_name] = $new_r;
  }


  module_invoke_all('forena_repos_alter', $repos);
  return $repos;
}

}