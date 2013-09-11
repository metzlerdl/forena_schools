<?php
require_once('ForenaSchoolsDb.inc');
class ForenaSchoolsScript {
	public $db;
	private $path;

	public function __construct() {
		$this->path = drupal_get_path('module', 'forena_schools') . '/db_api/';
		$this->db = new ForenaSchoolsDB();
	}

	/**
	 *
	 * Factory method for script object.
	 */
	static function instance() {
		static $object = '';
		if (!$object) {
			$object = new ForenaSchoolsScript();
		}
		return $object;
	}

	/**
	 *
	 * Run a pg_admin style script
	 * @param string $file_name
	 */
	public function dbScript($filename) {
		$data = file_get_contents($this->path . $filename);
		$lines = explode(';', $data);
		if ($lines) foreach ($lines as $line) {
		  $l = trim($line, " \n");
		  // Skip comment
		  if (strpos($l, '--')!==0) {
		  	if (strpos($l, '\\i') ===0) {
		  		list($i, $inc) = explode('\\i', $l, 2);
		  		$this->runInclude(trim($inc));
		  	}
		  	else {
		  	  if (trim($l, " \n")) $this->db->raw_query($l);
		  	}
		  }
		}
		return $this;

	}

	/**
	 * Run an ojbect installing include.  This is not parsed in the way the base file is.
	 * But rather is expected to run as an install script.
	 * @param string $filename
	 */
	public function runInclude($filename) {
		$data = file_get_contents($this->path . $filename);
		if ($data) $this->db->raw_query(trim($data, ';'));
		return $this;
	}
}