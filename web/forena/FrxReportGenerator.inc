<?php
// $Id$
/**
 * @file
 * Common functions used throughout the project but loaded in this
 * file to keep the module file lean.
 */
// Include Report renderer.
require_once('FrxReport.inc');
require_once('FrxDataProvider.inc');

class FrxReportGenerator {

  public $app;
  private $repositories = array();
  static $instance='';
  public $title;
  public $content;

  static public function instance() {
    global $forena_application_class;
    if (!self::$instance) {
      if (!$forena_application_class) {
        $forena_application_class = 'FrxDrupalApplication';
      }

      self::$instance = new FrxReportGenerator($forena_application_class);
    }
    return self::$instance;
  }


  public function __construct($application_class) {

    $app = $this->app = new $application_class();
  }

  /**
   * Get name from argument 1 or alterntaively from a file name
   *
   * @param unknown_type $name
   */
  public function report_desc($name='') {
    $app = $this->app;
    // Get defined document types
    $doctypes = $this->supported_doctypes();
    $parts = explode('.', $name);
    $name ='';

    if ($parts) foreach ($parts as $part) {
      if (!isset($doctypes[$part])) {
        $name .= '/' . $part;
      }
      else {
        $desc['format'] = $part;
      }
    }
    $desc['name'] = trim($name, '/');
    $report_path = $app->configuration('report_repos');
    @list ($dir, $name_part) = explode('/', $name_part, -2);
    if (!$name_part) $dir = '';
    $desc['directory'] = $report_path . '/' . $dir;
    $desc['filename'] = $report_path . '/' . trim($name, ' /') . '.frx';
    $desc['link'] = 'reports/' . trim(str_replace('/' , '.', $name), '.');

    if (file_exists($desc['filename'])) {
      $desc['exists'] = TRUE;
    }

    return $desc;
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
    $path = $this->app->configuration('report_repos');
    if (file_exists($path . '/' . $form . '.css')) {
      $css_files[] = $path . '/' . $form . '.css';
    }
    if ($format && file_exists($path . '/' . $form . '-' . $format . '.css')) {
      $css_files[] = $path . '/' . $form . '-' . $format . '.css';
    }

    // Now check for a report specific file
    $base_file = $path . '/' . $desc['name'];

    if ($format && file_exists($base_file . '-' . $format . '.css')) {
      $css_files[] = $base_file . '-' . $format . '.css';
    }
    elseif (file_exists($base_file . '.css')) {

      $css_files[] = $base_file . '.css';
    }
    return $css_files;
  }

  /**
   * Clear session data because we want to reload.
   */
  public function clear_session() {
  	if (isset($_SESSION['forena_access'])) unset($_SESSION['forena_access']);
  }

  /**
   * Check access control using the block in a data block.  In this case
   * public assess returns true.
   * @param $block  Repository block used to test access
   * @param $path   xpath to user right within xml data.
   * @param $access Access to test
   * @return unknown_type
   */
  function block_access($block, $path, $access, $cache=TRUE) {
  	// PUBLIC always returns true.
    if ($access=='PUBLIC') return TRUE;
    if (!isset($_SESSION['forena_access'])) $_SESSION['forena_access'] = array();
    if ($cache && isset($_SESSION['forena_access'][$block])) {
      $rights = $_SESSION['forena_access'][$block];
    }
    else {
      $rights = array();
      // Get the block from the repository
      $frxData = FrxData::instance();
      $frxData->push(array(), 'frx-block-access');
      $data = $this->invoke_data_provider($block, array(), NULL);
      $frxData->pop();
      if ($data) {
        if (!$path) {
          $path ='*/*';
        }
        $nodes = $data->xpath($path);
        if ($nodes) foreach ($nodes as $node) {
          $rights[] = (string)$node;
        }
        $_SESSION['forena_access'][$block]=$rights;
      }
    }
    foreach ($rights as $right) {
      if ($access == $right) return TRUE;
    }
    return FALSE;
  }


  // Putting this in a function to sandbox the repository settings
  function load_repository(&$repo) {
    // First determine if the class file exisits
    $path = @$repo['path'];
    $conf = array();

    if (file_exists($path . '/settings.php')) {
      // Override with repository specific data
      include($path . '/settings.php');
    }

    $repo = array_merge($conf, $repo);
    // $repo['user callback'] = $conf['user callback'];
    if (!isset($repos['data'])||!is_object($repo['data'])) $repo['data'] = $this->load_provider($repo, $path);

    //$repo['auth'] = __forena_load_auth($security_provider);

  }

  /**
   * Load the data provider class based on the class name.
   *
   * @param string $name
   * @return object The data provider object
   */
  function load_provider($conf, $repo_path) {
    @$name = isset($conf['data provider'])? $conf['data provider'] : $conf['data_engine'];
    $this->define_plugins();
    // Instantiate the path
    if (class_exists($name)) {
      $o = new $name($conf, $repo_path);
      return $o;
    }
    else {
      $this->app->error('Data provider not found');
    }

  }

  /*
   * Get the repository data for an a repository name.
   * If no repository is specified the descriptors for all repositories are returned.
   */
  function repository($name='') {
    static $repos ='';
    global $_forena_repositories;
    // Empty repository so we need to initialize
    if (!$repos) {
      // Build the default sample one
      $path = $this->app->forena_path();
      $repos = array();
      // Load the repository list from the global settings.php file.

      $repos = $this->app->repositories();
      if ($_forena_repositories) {
      	array_merge($repos, $_forena_repositories);
      }

    }

    // Now determine if the object exists
    if ($name) {
      if (isset($repos[$name])) {

        if (!isset($repos[$name]['data']) || !is_object($repos[$name]['data'])|| !isset($repos[$name]['auth'])||!is_object($repos[$name]['auth'])) {
          $this->load_repository($repos[$name]);
        }
        return $repos[$name];
      }
      else {
        $this->app->error('Undefined repository' . $name, "Undefined Repository: $name <pre>" . print_r($repos, 1) . '</pre>');
      }
    }
    else {
      return $repos;
    }
  }

  /**
   * Accessor function named more conveniently
   * Enter description here ...
   * @param unknown_type $block_name
   * @param unknown_type $parms
   */
  public function block_xml($block_name, $parms=array()) {
	 return $this->invoke_data_provider($block_name, $parms);
  }

  /**
   * Extract the data by running a block
   *
   * @param unknown_type $data_block
   * @param unknown_type $parameters
   * @param string $clause order / where cause
   * @return unknown
   */
  function invoke_data_provider($data_block, $parameters=array(), $clause='') {
     list($provider, $block) = explode('/', $data_block, 2);
     // Get the data
     $repos = $this->repository($provider);
     if (isset($repos['enabled']) && !$repos['enabled']) {
     	return '';
     }
     if (isset($repos['user callback'])) {
       $user_fn = $repos['user callback'];
       if (is_callable($user_fn)) {
       	 $current_user =   $user_fn();
         FrxData::instance()->setValue('current_user', $current_user);
       }
     }
     if ($repos['data']) {
       $provider = $repos['data'];
       $access = TRUE;
       if (method_exists($provider, 'data')) {
         $xml = $provider->data($block, $parameters, $clause);
       }
       return $xml;
     }
  }

  /**
   * Returns an array of information about the data block
   * @param $data_block
   * @return unknown_type
   */
  function forena_load_block($data_block, $clause='') {

     @list($provider, $block) = explode('/', $data_block, 2);
     // Get the data

     $repos = forena_repository($provider);

     if (isset($repos['data'])) {
       $provider = $repos['data'];
       if (method_exists($provider, 'load_block')) {
         $block_info = $provider->load_block($block, $clause);

       }
       return $block_info;
     }
  }



  /**
   * Load the formatters for all initialized repositories.
   *
   */
  function get_formatter($fkey) {
    // Get all repositories
    $repos = $this->repository();
    $formats = array();
    foreach ($repos as $k => $r) {
      $provider = isset($r['data']) ? $r['data'] : NULL;
      if ($provider && method_exists($provider, 'formats'))  {
        $f = $provider->formats();
      if (isset($f[$fkey]) && method_exists($provider, $fkey)) {
          // We found an object with the advertised method return it
          return $provider;
        }
      }
    }
    //Did not find the formater in the data provider
    //Look to see if it's in a control class
    $controls = $this->define_controls();
    foreach ($controls as $k => $r) {
      $provider = $r;
      if ($provider && method_exists($provider, 'formats'))  {
        $f = $provider->formats();
      if (isset($f[$fkey]) && method_exists($provider, $fkey)) {
          // We found an object with the advertised method return it
          return $provider;
        }
      }
    }
    return $formats;
  }



  /**
   * Accepts the name of a file
   *
   * Returns a xml object of the file.
   *
  */
  function get_report($report_name, $data=array()) {
    $r=null;
    if ($report_name) {
      $r_text = $this->app->load_report($report_name);
    }
    if ($r_text) {
      $r = new FrxReport($r_text, $data);
    }
    return $r;
  }


  /**
   * Enter description here...
   *
   * @param simplexml $xml
   * @param string $tag
   * @return string
   */
  function inner_xml($xml, $tag) {
    if (is_object($xml) && is_object($xml->$tag)) {
      $xml_data = $xml->$tag->asXML();
      $xml_data = preg_replace("/<\/?" . $tag . "(.|\s)*?>/", "", $xml_data);
    };
    return $xml_data;
  }

  /**
   * Accepts the name of the html tag, and the string the tag is in.
   *
   * Returns the string within the html tag name
   *
  */
  function get_html($tag, $r_text) {
    $open = strpos($r_text, $tag);
    $close = strpos($r_text, '>', $open);
    $next = strpos($r_text, '<', $close + 1);
    $str = substr($r_text, $close + 1, $next - ($close + 1));

    return $str;
  }

  function format_data($value, $format, $format_str, $teng='') {
    $fo = $this->get_formatter($format);
    if ($fo) {
      $ret = $fo->$format($value, $format_str, $teng);
    }
    else {
      $ret = $value;
    }
    return $ret;
  }

/**
 * Returns an array of information about the data block
 * @param $data_block
 * @return unknown_type
 */
function load_block($data_block, $clause='') {

   list($provider, $block) = explode('/', $data_block, 2);
   // Get the data

   $repos = $this->repository($provider);

   if ($repos['data']) {
     $provider = $repos['data'];
     if (method_exists($provider, 'load_block')) {
       $block_info = $provider->load_block($block, $clause);

     }
     return $block_info;
   }
}


  /**
   * Load a block file form disk and introspect the comments to determine security
   * Return a structured array based on this.
   *
   * @param unknown_type $filepath
   * @param unknown_type $comment
   */
   function load_block_file($filepath) {
    if (!file_exists($filepath)) {
      return array();
    }

    $block_data = file_get_contents($filepath);
    return $block_data;
  }

  /**
   * Loads all of the include files that
   */
  function define_plugins( $class='') {
    $plugins = $this->app->plugins();
    foreach ($plugins as $p) {
      if (($class=='' || $class==$p['class'])) {
        if ($p['file'] ) {
          include_once( trim($p['file'], '/'));
        }
      }
    }
  }

  /**
   * returns a list of instances of the control classes
   * if there wasn't a class, returns an empty string.
   */
  function define_controls( $class='') {
    static $instances = '';
    if (!$instances) {
      $classes = $this->app->controls();
      foreach ($classes as $c) {
        if (($class=='' || $class==$c['class'])) {
          if ($c['file']) {
            include_once($c['file'] );
            if (class_exists($c['class'])) {
              $instances[$c['class']] =  new $c['class'];
            }
          }
        }
      }
    }
    if ($class) {
      return @$instances[$class];
    }
    return $instances;
  }

  /**
   * Gets the correct format function
   * to render the document and returns an
   * object of that class.
   *
   * If it fails it returns a 0;
   */

  function get_doctypes($fkey) {
    $controls = $this->define_controls();
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
   * @return returns an array of supported document format extensions
   *
   */
  function supported_doctypes() {
    $controls = $this->define_controls();
    $supported_doctypes = array();
    if ($controls) foreach ($controls as $k => $r) {
      $provider = $r;
      if ($provider && method_exists($provider, 'doc_types'))  {
        $f = $provider->doc_types();
        $supported_doctypes = array_merge($supported_doctypes, $f);
      }
    }
    $temp = array_keys($supported_doctypes);
    $temp[]='web';
    if ($supported_doctypes) $supported_doctypes = array_combine($temp, $temp);
    return $supported_doctypes;
  }

  /**
   * Returns an object of the template class
   * that has a method named templates.
   *
   * If it fails it returns a 0;
   */
  function get_templates($fkey) {
    $controls = $this->define_controls();
    foreach ($controls as $k => $r) {
      $provider = $r;
      if ($provider && method_exists($provider, 'templates'))  {
        $f = $provider->templates();
        if ($f[$fkey] && method_exists($provider, $fkey)) {
          // We found an object with the advertised method, return it
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
    $controls = $this->define_controls();
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

  function supported_formats() {
    $controls = $this->define_controls();
    $supported_formats = array();
    $f = array();
    foreach ($controls as $k => $r) {
      $provider = $r;
      if ($provider && method_exists($provider, 'formats'))  {
        $f = $provider->formats();
        $supported_formats = array_merge($supported_formats, $f);
      }
    }
    return $supported_formats;
  }

  /**
   *
   * @param unknown_type $format: The extension of the document to be rendered
   * @param unknown_type $output: The string of the page to be displayed
   * @return: The document in the requested format. Returns a string if not
   * able to format.
   */
  function  generate_doc($format, $output, $options = array()) {

    $doc = $this->get_doctypes($format);
    if ($doc) {
      $all_methods = $doc->doc_types();
      $method = $all_methods[$format];
      $ret = $doc->$method($output, $options);
    }
    if ($ret) {
    // If an object was found, set the appropriate mime type (header doctype).
   //   $flen = strlen($ret);
      switch ($format) {
        case 'doc':
          header('Content-Type: application/msword');
          header('Cache-Control:');
          header('Pragma:');
          header("Cache-Control: must-revalidate");
          print $ret;
          break;
        case 'pdf':
          header('Content-Type: application/pdf');
          header('Cache-Control:');
          header('Pragma:');
          header("Cache-Control: must-revalidate");
          header("Content-Disposition: attachment; filename=report.pdf");
          print $ret;
          break;

        case 'xls':
          header('Content-Type: application/msexcel');
          header('Cache-Control:');
          header('Pragma:');
          header("Cache-Control: must-revalidate");
          header("Content-Disposition: attachment; filename=report.xls");
          print $ret;
          break;
       case 'csv':
          header('Content-Type: application/csv');
          header('Cache-Control:');
          header('Pragma:');
          header("Cache-Control: must-revalidate");
          header("Content-Disposition: attachment; filename=report.csv");
          print $ret;
          break;
       case 'svg':
       	  header('Conent-Type: image/svg+xml');
       	  header('Cache-Control:');
       	  header('Pragma:');
       	  header('Cache-Control: must-revalidate');
       	  print $ret;
       	  break;
       default:
           return $ret;

      }
      // Print the output.

    }
    else {
      return $ret;
    }
  }


  /**
 * Load and render a report based on a drupal path.
 * In this function the arglist is used to get the full path to the report.
 *
 * @return unknown
 */
function report($name_in, $parms = array()) {
  $output = '';
  $desc = $this->report_desc($name_in);
  $name = $desc['name'];
  $format = isset($desc['format']) ? $desc['format'] : '';
  $filename = $desc['filename'];
  $css_files = array();

  // Determine the data to get.
  if (!$parms) {
  	$parms = array_merge($_GET, $_POST);
  	unset($parms['q']);
  }
  else $parms = (array)$parms;
  if ($name) {
    $r = $this->get_report($name, $parms);
    if (!$r || !$r->rpt_xml) {
    	$this->app->error('Could not load report. Check for invalid XML.');
    	return '';
    }

    //check for default parameters
    $r_params = $r->parameters;
    $reload_params = FALSE;
    $missing_parms = FALSE;

    //Set default Parameters
    if ($r_params) {

      //put default parameters in parms array
      foreach ($r_params as $key => $parm) {
        if (@!$parms[$key] && @$parm['value']) {
         $parms[$key] = $parm['value'];
         $reload_params = TRUE;
        }
          //do not show report if a required parameter does not have a value
          //force the user to input a parameter
        if ((@!$parms[$key]) && (@strcmp($parm['require'], "1") == 0) && (!$parm['value'])) {
          $missing_parms = TRUE;
        }
      }
    }

    //Reload report if parameters were missing
    if ($reload_params) {
      $r = $this->get_report($name, $parms);
    }

    $form = isset($r->options['form']) ? $r->options['form'] : '';
    $rpt_xml = $r->rpt_xml;

    // Default the form
    if (!$form) $form = $this->app->configuration('default_form');

    if (!$missing_parms) {
      $output .= $r->render($format);
      $css_files = $this->report_css($desc, $form, $format);
    }

    //put title on top of report
    $title = (string)$r->title;
    $this->title = $title;

    if ($format && $format !='web') { //a format was requested
        $header = '<h1>' . $title . '</h1>';
        $output = $header . $output;
        $css_text = '';
        $r_text = '';
        if ($css_files) foreach ($css_files as $css_file) {
          $css_text .= file_get_contents($css_file);
        }
        $options = array('css' => $css_text,
                         'docname' => str_replace(' ', '_', $title),
                         'xml' => $r_text,
                         'title' => $title,
                   );

        $output = $this->generate_doc($format, $output, $options);
        print $output;
        exit;
      }
      else {
        //Creating links for downloadable documents.
        //build querystring for document href
        $q='';
        foreach ($parms as $key => $value) {
          $q .= "&" . $key . '=' . $value;
        }
        $q = trim($q, '&');

        //Building the document links
        //@TODO: Move this to better location
        $rpt_xml = $r->rpt_xml;
        $nodes = $rpt_xml->xpath('//frx:docgen/frx:doc');
        $div = '<div class="doclinks">';
        $default_doctypes = $this->app->configuration('doc_defaults', array());

        if (!$missing_parms) {
          if (!$nodes) {
            //show the default.
            if ($default_doctypes) foreach ($default_doctypes as $value) if (is_object($this->get_doctypes($value))) {
              $div .= $this->link(strtoupper($value) . ' ', 'report_doc/' . $name_in . '.' . $value, array('query' => $parms, 'class' => 'doclinks'));
            }
          }
          else {
            //There were nodes. show the prefered doc types
            $doctypes = $this->supported_doctypes();
            foreach ($nodes as $value) {
              $arr = $value->attributes();
              $type = (string)$arr['type'];
              if (@$doctypes[$type]) {
                if (is_object($this->get_doctypes($type))) {
                  $div .= $this->link(strtoupper($type) . ' ', 'report_doc/' . $name_in . '.' . $type, array('query' => $parms, 'class' => 'doclinks'));
                }
              }
            }
          }
        }
        $div .= '</div>';
        $output = $div .  $this->app->theme($r, $title, $format);

        $this->app->add_css($this->app->forena_path() . '/forena.css');
        if ($css_files) foreach ($css_files as $css_file) {
          $this->app->add_css($css_file);
        }
        return $output;
      }
    }
    else {
      $this->app->not_found();
    }

  }

/*
 * Recieves a datablock and returns an array of values from the data block.
 * @data_block: name of the data block to be invoked for values
 * @field: Specific field name within the data block. The values returned will only
 * come from this field.
 * @params: filtering for the data block
 * @where_clause: Where clause for data block to be filtered against.
 *
 */
function data_block_params($data_block, $field, $label, $params=array(), $clause='') {
  $xml = $this->invoke_data_provider($data_block, $params, $clause);

  $list = array();
  if ($xml) {
    $path = ($field) ? $field : '*[1]';
    $label_path = ($label) ? $label : '*[2]';


    //walk through the $xml.
    $rows = $xml->xpath('*');
    if ($rows) foreach ($rows as $row) {
    	$value = $row->xpath($path);
    	$label_field = $row->xpath($label_path);
    	$label_value =  $label_field ? (string)$label_field[0] : (string)$value[0];
    	$list[(string)$value[0]] = (string)$label_value;

    }
  }
  return $list;
}

public function debug($short_message='', $log='') {
	$this->app->debug($short_message, $log);
}

public function error($short_message='', $log='') {
	$this->app->error($short_message, $log);
}

public function configuration($name) {
	return $this->app->configuration($name);
}

public function report_path() {
	return $this->app->configuration('report_repos');
}

  public function link($title, $path, $options=array()) {
    return $this->app->link($title, $path, $options);
  }

}