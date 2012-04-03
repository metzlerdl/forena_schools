<?php
require_once('FrxHostApplication.inc');
global $forena_application_class;
$forena_application_class='FrxPedagoggle';

class FrxPedagoggle extends FrxHostApplication {

	public function controls() {
		$controls = parent::controls();
		$controls[] = array('file' => 'FrxPedagoggleScores.inc',
		                    'class' => 'FrxPedagoggleScores');
		$controls[] = array('file' => 'plugins/FrxSVGGraph.inc',
		  'class' => 'FrxSVGGraph');
		return $controls;
	}

	public function configuration($name) {
		$config = array(
		  'report_repos' => 'reports',
		  'default_form' => 'pedagoggle',
		  'library_path' => 'lib',
		  'doc_formats' => array('doc' => 'doc',
		     'xls' => 'xls',
		     'csv' => 'csv',
		     'svg' => 'svg')
		);
		return $config[$name];
	}

	function link($text, $path, $options=array()) {
	  if (is_array($options['attributes'])) {
	    $t = '';
	    foreach ($options['attributes'] as $key => $value) if ($key && $value) {
	      $t .= " $key=" . '"' . htmlentities($value) . '"';
	    }
	  }
	  return '<a href="' . htmlentities($this->url($path, $options)) . '" ' . $t . '>' . $text . '</a>';
	}

  function add_css($css_file) {
  	css_lib($css_file);
  }

}