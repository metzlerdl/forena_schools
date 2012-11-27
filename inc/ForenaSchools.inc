<?php
require_once('ForenaSchoolsDb.inc');
class ForenaSchools {

	public $db;
	public $user;

	public function __construct() {
		//Obtain a database connection
		$this->db = ForenaSchoolsDB::instance();
		//Determine current user
		GLOBAL $user;
		$login = @$user->name; 
		if ($this->db && $user->uid) {
			$email = $user->mail;
			$sql = 'SELECT p.*,s.role,s.bldg_id FROM p_people p JOIN p_staff s
         ON p.person_id=s.person_id';
			if (is_callable('cas_current_user')) {
				$login = strtolower(cas_current_user());
			}

			if (isset($_SESSION['forena_schools_impersonate'])) {
				$login = strtolower($_SESSION['forena_schools_impersonate']);
			}
			// Use cas user preferentially
			if ($login) {
				$sql .= ' WHERE login=:login';
			}
			// if no cas user then load based on email
			else {
				$sql .= ' WHERE email=:email';
			}
			$rs = $this->db->exec_query($sql, array('login' => $login, 'email' => $email));
			if ($rs) {
				$this->user = $this->db->first_row($rs);

			}
			else {
				$this->user = array();
			}

		}
		else {
			$this->user = array();
		}
	}

	// Determine whether a user has a buidling role
	function building_access($login, $bldg_id, $role) {
		$role = $this->building_role($login, $bldg_id);
	}

	/**
	 * Factory method for generating application instance
	 */
	static function instance() {
		static $instance = '';
		if (!$instance) {
			$instance = new ForenaSchools();
		}
		return $instance;
	}

	/**
	 * access
	 * Check to see if a user has this forena_schools_role.  Static caching is invoked to make
	 * sure this happens only once per page load.
	 * @param String $role
	 */
	public function access($role) {
		static $roles = '';
		if ($role=='PUBLIC') return TRUE;
		if (user_access('administer forena schools') && !isset($_SESSION['forena_schools_impersonate'])) return TRUE;
		if (!$roles) {
		  $roles = array();
			$login = $this->current_login();
		  $parms = array('login' => $login);

			$rows = $this->db->query("
			select v.code AS role FROM p_staff s JOIN p_people p
        ON p.person_id=s.person_id
      JOIN i_validations v ON v.var='role'
        AND (s.role = 'sys_admin'
          OR s.role='dist_admin' AND v.code IN ('dist_admin','bldg_admin','teacher')
          OR s.role='bldg_admin' AND v.code IN ('bldg_admin', 'teacher')
          OR s.role=v.code)
			  WHERE login=:login
			  ORDER BY bldg_id ASC", $parms);
			if ($rows) foreach ($rows as $row) {
				if ($row['role']) $roles[] = $row['role'];
			}
		}

		// Cascade down checking roles.
		switch ($role) {
			case 'teacher':
				if (array_search('teacher', $roles) !== FALSE) return TRUE;
			case 'bldg_admin':
				if (array_search('bldg_admin', $roles) !== FALSE) return TRUE;
			case 'dist_admin':
				if (array_search('dist_admin', $roles) !== FALSE) return TRUE;
			case 'sys_admin':
				if (array_search('sys_admin', $roles) !== FALSE) return TRUE;
		}
		return FALSE;
	}

	/**
	 * Get the defined role for a building
	 * @param string $login  login name.
	 * @param string $bldg_id building id of the building to test
	 */
	function building_role($login, $bldg_id='') {
		$db = $this->db;
		// Check to see if the username is in the database
		$parms = array('login' => $login, 'bldg_id' => $bldg_id);
		$user = $db->first_row($db->query('SELECT p.*,s.role,s.bldg_id FROM p_people p JOIN p_staff s
			 ON p.person_id=s.person_id
			 where login=:login AND :bldg_id = s.bldg_id
			 ORDER BY bldg_id DESC',$parms));
		if ($user) {
		  // Set up session for currently logged in user.
		  return $user['role'];
		}
		else {
			$user = $db->first_row($db->query('SELECT p.*,s.role,s.bldg_id FROM p_people p JOIN p_staff s
				 ON p.person_id=s.person_id
				 where login=:login AND s.bldg_id = -1
				 ORDER BY bldg_id DESC',$parms));
			if ($user) {
				return $user['role'];
			}
			else {
				return FALSE;
			}
		}
	}


	/**
	 * Return the currently logged in user
	 * Enter description here ...
	 */
	function current_login() {
    return strtolower(@$this->user['login']);
	}

  function verify_directory($fullpath) {
	  static $path='';
	  $success = TRUE;

	  @list($dir, $file) = explode('/', $fullpath, 2);
	  if ($path) {
		  $path .= '/' . $dir;
	  }
	  else {
	  	$path = $dir;
	  }
 	  // Path
	  if (!file_exists($path)) {
	    @mkdir($path);
	    if (!@file_exists($path)) {
	    	drupal_set_message($path . ' not found');
	    	return FALSE;
	    }

	  }
	  // Recurse to next file.
	  if ($file) {
	    $this->verify_directory($file);
	  }
	  return TRUE;
	}

	/**
	 * Return the configured import directory
	 */
	public function import_directory() {
		$dir = variable_get('file_private_path' , conf_path() . '/files'). '/import';
	  if (!$dir) {
	    $dir = variable_get('file_' . file_default_scheme() . '_path' , conf_path() . '/files'). '/import';
	  }
	  return trim($dir, '/');
	}
	
	/**
	 * List All School years for sleection. 
	 */
	public function schoolYearList() { 
	  $years = array(); 
	  $rs = db_query('SELECT school_year, label FROM i_school_years ORDER BY start_date desc'); 
	  if ($rs) foreach ($rs as $row) { 
	    $years[$row->school_year] = $row->label; 
	  }
	  return $years; 
	}

}