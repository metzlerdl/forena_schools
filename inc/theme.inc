<?php
/**
* Render a partial template from an array.
* @param $info an two dimensional array to iterate through for partial
* @param $template the template to execute for the partial.
*/
function render_template($template,$content='',$extract=true) {
  if (is_array($content) && $extract==true)  {
    extract($content);
  }
  include($template.".tpl.php");
}

/**
* Render a partial template from an array.
* @param mixed $collectin an array to itterate through as the partical
* @param string $template the template to execute for the partial.
*/
function render_repeating_template($template,$collection='',$extract=true) {
  foreach ($collection as $item)  {
    render_template($template,$item,$name,$extract);
  }
}
/**
 * Static caching structure for javascript files
 */
function javascript_lib($js_file='') {
  static $js_array = '';
  if ($js_file)
    $js_array[] = $js_file;
  return $js_array;
}

/**
 * Render an array of javascript files
 */
function render_javascript($js_array=array())
{
  $output = '';
  if ($js_array) foreach ((array)$js_array as $js_file)
  {
    $output .= "\n".'  <script src="'.$js_file.'" language="javascript" type="text/javascript"></script>';
  }
  return $output;
}

/**
 * Static caching structure for list of css files
 * @param string $css_file  The file to add tot he css Libary
 */
function css_lib($css_file='') {
  static $css_array='';
  if ($css_file)
    $css_array[] = $css_file;
  return $css_array;
}

/**
* render the css files in an array
*/
function render_css($css_array=array())
{
  $output = '';
  if ($css_array) foreach ($css_array as $css_file)
  {
    $output .= "\n".'  <link rel="stylesheet" type="text/css" href="'.$css_file.'" />';
  }
  return $output;
}


function encode_array($a) {
	if (is_array($a)) {
		foreach ($a as $key=>$value) {
			$ret .= '&' . $key . '=' .urlencode($value);
		}
	}
	return ltrim($ret,'&');
}
function render_flex($application,$title='', $popup=false)
{
  global $base_url;
  $fvar = $_GET;
  $fvar['dataService'] = $base_url.'/data.php';
  $fvar['basePath'] = rtrim($base_url,'/');
  $flexPath = "flex/";
  if ($_GET['debug']=='true') {
  	$flexPath = "flex-debug/";
  }
  $vars['flexPath'] = $flexPath;
  unset($fvar['q']);
  $flashfile = $flexPath . $application . '.swf';
  $vars['flashfile'] = $flashfile . '?d=' . filemtime($flashfile);
  $vars['application'] = $application;
  $vars['div_id'] = $application. '_div';
  $vars['flashvars'] = encode_array($fvar);
  $vars['flashvars_json'] = json_encode($fvar, JSON_FORCE_OBJECT);
  css_lib('flex/history/history.css');
  if ($popup) $vars['height']='100%'; else $vars['height']='85%';
  if (!$popup) $output .= '<div class="flex-application grid_12" style="height: 700px;">';
  $output .= theme_template('flex', $vars);
  if (!$popup) $output .= '</div>';
  render_page('flex',$title,$output, $popup);
}

/**
 * Rendering a page with optional page variables
 *
 * String $page  the base file_name of the template
 * String $title The title of the page to be displayed.
 * Array  $vars  An array containing the variables to be instantiated forthe template
 */
function render_page($page,$title='',$page='', $popup=false) {
  GLOBAL $base_url;
  javascript_lib('flex/swfobject.js');
  javascript_lib('inc/pedaflex.js');
  $content = $vars;
  $content['scripts'] = render_javascript(javascript_lib());
  $content['home'] = $base_url . 'index.php';
  $content['logout'] = $base_url . 'index.php?q=logout';
  $content['css'] = render_css(css_lib());
  $content['title'] = $title;
  $content['content'] = $page;
  if ($popup) {
    render_template('popup',$content);
  }
  else {
  	render_template('page',$content);
  }

}


/**
 * Rendering a page with optional page variables
 *
 * String $page  the base file_name of the template
 * String $title The title of the page to be displayed.
 * Array  $vars  An array containing the variables to be instantiated forthe template
 */
function render_print($page,$title='',$vars='') {
  ob_start();
  render_template($page,$vars);
  $content['scripts'] = render_javascript(javascript_lib());
  $content['css'] = render_css(css_lib());
  $content['title'] = $title;
  $content['content'] = ob_get_contents();
  ob_end_clean();
  render_template('print',$content);
}

/**
 * Rendering a page with optional page variables
 *
 * String $page  the base file_name of the template
 * String $title The title of the page to be displayed.
 * Array  $vars  An array containing the variables to be instantiated forthe template
 */
function render_inline($page,$title='',$vars='') {
  ob_start();
  render_template($page,$vars);
  $content['css'] = inline_css('print.css');
  $content['title'] = $title;
  $content['content'] = ob_get_contents();
  ob_end_clean();
  render_template('print',$content);
}


function theme_template($page,$content) {
  ob_start();
  render_template($page,$content);
  $output = ob_get_contents();
  ob_end_clean();
  return $output;
}

function inline_css($file) {
	$output .= '<style type="text/css">';
	ob_start();
	include($file);
	$output .= ob_get_contents();
	ob_end_clean();
	$output .= "</style>";
	return $output;
}

