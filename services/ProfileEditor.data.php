<?php
class ProfileEditor extends ForenaSchools {
	public $title = 'Profile Editor';
	public $id;


	public function auth() {
		return $this->access('dist_admin');
	}

	public function __construct() {
		parent::__construct();
		$this->id= @$_POST['profile_id'];
	}

	public function profile() {
		$parms = array('id' => $this->id);
		return $this->db->call('a_profile_xml(:id)', $parms);
	}

	public function saveProfile() {
		return $this->db->call('a_profile_save(:xml)', $_POST);
	}

	public function deleteProfile() {
	 $this->db->call('a_profile_delete(:id)', $_POST);
	 return '<profile/>';
	}

	public function preview() {
		return $this->db->query_xml('SELECT name,a_profile_measures_xml(profile_id) AS measures FROM a_profiles WHERE profile_id=:profile_id', $_POST);
	}

	/**
	 * Retreiv tests for adding the profile
	 * The tests will be in the same format that they need to be added into the profile in.
	 */
	public function tests() {
		$min_grade = @$_POST['min_grade'];
		$max_grade = @$_POST['max_grade'];
		if ($min_grade && $max_grade) {
			$sql = 'SELECT t.*, a_profile_test_measures(test_id) AS measures,
			        a_profile_test_schedules(test_id) AS schedules
			        FROM
			         a_tests t
			           WHERE min_grade BETWEEN :min_grade AND :max_grade
			         OR max_grade BETWEEN :min_grade AND :max_grade
			         OR :min_grade BETWEEN min_grade AND max_grade
			         ORDER BY name';
		}
		else {
			$sql = 'SELECT t.*, a_profile_test_schedules(test_id) AS schedules,
			  a_profile_test_measures(test_id) AS measures FROM a_tests t ORDER BY name';
		}
    return $this->db->query_xml($sql, $_POST);
	}



	/**
	 * Retreive a list of building for possible profile selection
	 */
	public function buildings() {
		$sql = "SELECT bldg_id, name FROM i_buildings
		  UNION ALL
		    SELECT -1,'District'";
		return $this->db->query_xml($sql);
	}
}