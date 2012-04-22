<?php
class ForenaSchools {

	public $db;

	public function __construct() {
		$this->db = ForenaSchoolsDB::instance();
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
		if (user_access('administer forena schools')) return TRUE;
		if (!$roles) {
		  $roles = array();
			$login = $this->current_login();
		  $parms = array('login' => $login);
			$rows = $this->db->query('SELECT s.role FROM p_people p JOIN p_staff s
			  ON p.person_id = s.person_id
			  WHERE login=:login
			  ORDER BY bldg_id ASC', $parms);
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

	function current_user() {
	  $user =  $_SESSION['pedagoggle_user'];
		return $user;
	}

	/**
	 * Return the currently logged in user
	 * Enter description here ...
	 */
	function current_login() {
    global $user;
    $login = '';
    if (is_callable('cas_current_user')) {
    	$login = cas_current_user();
    }
    if (!$login) {
    	if ($user->uid) $login = $user->name;
    }
	}
}