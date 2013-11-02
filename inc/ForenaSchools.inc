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
	 * @return ForenaSchools
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
        AND (
			    ((s.role = 'sys_admin'
          OR s.role='dist_admin') AND v.code IN ('dist_admin','bldg_admin','teacher'))
          OR (s.role='bldg_admin' AND v.code IN ('bldg_admin', 'teacher'))
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
	  static $years = '';
	  if (!$years) {
	    $years = array();
  	  $rs = db_query('SELECT school_year, label FROM i_school_years ORDER BY start_date desc');
  	  if ($rs) foreach ($rs as $row) {
  	    $years[$row->school_year] = $row->label;
  	  }
	  }
	  return $years;
	}

	/**
	 * Return key value array for buildings the user has access to.
	 * @return array
	 */
	public function userBuildings() {
    $buildings = array();
	  if ($this->access('dist_admin')) {
	    //
	    $rs = db_query('select b.bldg_id, b.name from i_buildings b WHERE bldg_id<>-1 order by name') ;
	  }
	  else {
	    $rs= db_query('select b.bldg_id, b.name FROM i_buildings b JOIN p_staff s ON s.bldg_id = b.bldg_id
	      JOIN p_people p ON p.person_id = s.person_id
	      WHERE p.login = :login
	      order by name',
	      array(':login' => $this->current_login()));
	  }
    foreach ($rs as $row) {
      $buildings[$row->bldg_id] = $row->name;
    }
    return $buildings;
	}

	/**
	 * Return key value array for grade levels
	 * that the user has access to.
	 * @return array
	 */
	public function userGradeLevels() {
	  $grade_levels = array();
	  if ($this->access('dist_admin')) {
	    // All defined grade levels.
	    $rs = db_query('select grade_level, abbrev from i_grade_levels order by grade_level');
	  }
	  else {
	    // In this case we're going to get min and  max grade levels
	    // for the staff member.
	    $rs = db_query('select grade_level, abbrev FROM i_grade_levels g
	      JOIN
	      (SELECT MIN(COALESCE(s.min_grade_level,b.min_grade)) AS min_grade,
	        MAX(COALESCE(s.max_grade_level, b.max_grade)) AS max_grade
	       FROM p_staff s JOIN i_buildings b ON b.bldg_id=s.bldg_id
	         JOIN p_people p ON s.person_id=p.person_id
	       WHERE p.login=:login
	       ) v ON g.grade_level BETWEEN COALESCE(v.min_grade,0)
	         AND COALESCE(v.max_grade, 12)',
	      array(':login' => $this->current_login())
	    );

	  }

	  foreach ($rs as $row) {
	    $grade_levels[$row->grade_level] = $row->abbrev;
	  }
	  return $grade_levels;
	}

	/**
	 * List of tests based on min and max grades
	 * @param $min_grade int 0 - 12
	 * @param $max_grade int 0 - 12
	 */
	public function userTests($min_grade, $max_grade) {
	  $tests = array();
    if (!$min_grade) {
      $grade_levels = array_keys($this->userGradeLevels());
      $min_grade= array_shift($grade_levels);
      $max_grade= array_pop($grade_levels);
    }
	  $rs = db_query('SELECT test_id, name FROM a_tests t WHERE
	    :min_grade BETWEEN min_grade and max_grade
	    OR :max_grade BETWEEN min_grade and max_grade
	    OR (:min_grade < min_grade AND :max_grade > max_grade)
	    ORDER BY name
	    ',
	    array(':min_grade' => $min_grade,':max_grade' => $max_grade)
	    );
	  foreach ($rs as $row) {
	    $tests[$row->test_id] = $row->name;
	  }
	  return $tests;
	}

	public function time() {
	  return date('Y-m-d h:i');

	}

	public function date() {
	  return date('Y-m-d');
	}

	public function fieldToArray($value) {
	  $value = trim($value, '{}');
	  $value = explode(',', $value);
	  return $value;
	}

	public function arrayToField($array) {
	  $field = implode(',' , $array);
	  $field = '{' . $field . '}';
	  return $field;
	}






}