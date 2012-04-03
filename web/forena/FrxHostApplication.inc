<?php
global $forena_application_class;
$forena_application_class = 'FrxHostApplication';
class FrxHostApplication {
	/**
	 * @file HostApp.inc
	 * Defines all the interface points between the host application and Forena.
	 * Each of these methods must be specified in order for Forena to function properly.
	 * The base class is just here to let you know
	 */

	// This varialbe is a factory variable to make sure we only get one instance of this class
	static private $instance = '';
	public $repositories = Array();

	// A factory method that is used to generate the instance of the application.
	static public function instance() {
		if (self::$instance) {
			return new Application();
		}
		else
		  return self::$instance;
	}

	/**
	 * Configuration retrieval method
	 * Returns configuration varialbes used by forena.
	 * @param unknown_type $var_name
	 */
  public function configuration($var_name) {
  	 global $forena_conf;
     return $forena_conf[$var_name];
  }


  /**
   * Convert a relative link to appropriately rendered html
   * return html A properly formatted anchor tag
   */
  public function link($title, $path, $options=array()) {
    return '<a href="'. $path . '">' . $title . '</a>';
  }

  /**
   * Theme the output of a css
   * Enter description here ...
   * @param unknown_type $output
   * @param unknown_type $doc_type
   */
  public function theme($r, $doc_type, $title) {
    return $r->html;
  }

  /**
   * Add a css file for theming.
   * Enter description here ...
   * @param unknown_type $css_file
   */
  public function add_css($css_file) {

  }

  /**
   * Add a javascript file for theming css
   * Enter description here ...
   * @param unknown_type $js_file
   */
  public function add_js($js_file) {

  }

  /**
   * Convert the report name into a link to the report
   * Enter description here ...
   * @param unknown_type $report_name
   */
  public function report_link($report_name, $title) {
    return $this->link($title, $path );
  }

  /**
    * Error Reporting
    */
  public function error($short='', $long='') {
    print('Error: ' . $short . $long);
  }

  public function debug($short_message='', $log='') {
  	 print('Debug:' . $message . $log);
  }


	 /**
	 * Determines which css files need to be loaded.
	 *
	 * @param array $desc Report descriptor from forena_rport_desc
	 * @param string $form The report "form" to be used.  From the report
	 * @param string $format Document format that will be used for the report.
	 * @return array A list of css files that should be applied to the report.
	 */
	function report_css($desc, $form,  $format = '') {
	  $css_files = array();
	  // First check for the form file
	  $path = forena_report_path();
	  if (file_exists($path . '/' . $form . '.css')) {
	    $css_files[] = $path . '/' . $form . '.css';
	  }
	  if ($format && file_exists($path .'/'. $form .'-'. $format .'.css')) {
	    $css_files[] = $path . '/'. $form . '-' . $format .'.css';
	  }

	  // Now check for a report specific file
	  $base_file = $path . '/' . $desc['name'];

	  if ($format && file_exists($base_file .'-' . $format .'.css')) {
	    $css_files[] = $base_file .'-'. $format . '.css';
	  }
	  elseif (file_exists($base_file .'.css')) {

	    $css_files[] = $base_file .'.css';
	  }
	  //drupal_set_message("desc: <pre>". print_r($desc, 1) ."</pre>");
	  //drupal_set_message("css files: <pre>". print_r($css_files, 1) ."</pre>");
	  return $css_files;
	}

	public function forena_path() {
		return 'forena';
	}

	public function not_found($name) {

	}

	public function repositories() {
		global $_forena_repositories;
		return $_forena_repositories;
	}

	/**
	 * Accepts the name of a file
	 *
	 * Returns a xml object of the file.
	 *
	*/
	function load_report($report_name) {
	  $r=null;
	  if ($report_name) {
	    $report_path =  $this->configuration('report_repos');
	    $filename = 'reports/' . $report_name . '.frx';
	    if (file_exists($filename)) {
	      $r_text = file_get_contents($filename);
	    }
	    return $r_text;
	  }
	}


	/**
	 * Self register controls with forena.
	 *
	 */
	function controls() {
		$controls = array();
	  $controls[] = array('file' => 'plugins/FrxControls.inc',
	                      'class' => 'FrxControls',
	                     );
	  $controls[] = array('file' => 'plugins/FrxSource.inc',
	                      'class' => 'FrxSource');
	  $controls[] = array('file' => 'plugins/FrxXMLSource.inc',
	                      'class' => 'FrxXMLSource');
	  $controls[] = array('file' => 'plugins/FrxInclude.inc',
	                      'class' => 'FrxInclude');
	  return $controls;
	}


	function url($path, $options) {
		if ($options['query']) {
      $query = $options['query'];
      if (is_array($query)) {
      	$q='';
      	foreach ($query as $key => $value) {
      		$q .= '&' . urlencode($key) . '=' . urlencode($value);
      	}
      	$query = trim($q,'&');
      }
			if ($query) $path .= '?'. $query;
		}

		return $path;
	}

	function plugins() {
	  $plugins[] = array('file' => 'plugins/FrxPDO.inc',
	                     'class' => 'FrxPDO',
	                     );
	  $plugins[] = array('file' => 'plugins/FrxOracle.inc',
	                     'class' => 'FrxOracle',
	                     );
	  $plugins[] = array('file' => 'plugins/FrxFiles.inc',
	                     'class' => 'FrxFiles',
	                     );
	  $plugins[] = array('file' => 'plugins/FrxPostgres.inc',
	                     'class' => 'FrxPostgres',
	                     );
	  $plugins[] = array('file' => 'plugins/FrxMSSQL.inc',
	                     'class' => 'FrxMSSQL',
	                     );
	  return $plugins;
	}


	/**
	 * Gets the correct format function
	 * to render the document and returns an
	 * object of that class.
	 *
	 * If it fails it returns a 0;
	 */

	function get_doctypes($fkey) {
	  $controls = $this->controls();
	  foreach ($controls as $k => $r) {
	    $provider = $r;
	    if ($provider && method_exists($provider, 'doc_types'))  {
	      $f = $provider->doc_types();
	      if (isset($f[$fkey]) && method_exists($provider, $f[$fkey])) {
	        // We found an object with the advertised method return it
	        return $provider;
	      }
	    }
	  }
	  return 0;
	}


	/**
	 *
	 * @return returns an array of supported templates
	 *
	 */
	function supported_templates() {
	  $controls = $this->controls();
	  $supported_templates = array();
	  foreach ($controls as $k => $r) {
	    $provider = $r;
	    if ($provider && method_exists($provider, 'templates'))  {
	      $f = $provider->templates();
	      $supported_templates = array_merge($supported_templates, $f);
	    }
	  }
	  return $supported_templates;
	}


	/**
	 *
	 * @return returns an array of supported document format extensions
	 *
	 */
	function supported_doctypes() {
	  $controls = $this->controls();
	  $supported_doctypes = array();
	  foreach ($controls as $k => $r) {
	    $provider = $r;
	    if ($provider && method_exists($provider, 'doc_types'))  {
	      $f = $provider->doc_types();
	      $supported_doctypes = array_merge($supported_doctypes, $f);
	    }
	  }
	  $temp = array_keys($supported_doctypes);
	  $supported_doctypes = array_combine($temp, $temp);
	  return $supported_doctypes;
	}




}