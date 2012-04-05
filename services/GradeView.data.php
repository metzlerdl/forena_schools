<?php
class DataBroker {
	// Verify authentication
	public $title='Grade View';

	public function __construct() {
		if (!isset($_POST['school_year'])) {
			$_POST['school_year'] = db_call('i_school_year()');
		}
	}

	public function auth() {
		return access_level('teacher');
	}

	public function profiles() {
		return db_query_xml(
		  'SELECT profile_id, name,  a_profile_measures_xml(profile_id) AS measures from a_profiles p
		    WHERE (bldg_id=:bldg_id OR bldg_id=-1) AND :grade_level BETWEEN min_grade and max_grade
		      AND analysis_only = false
		    ORDER BY p.weight, p.min_grade, p.max_grade, p.name' ,
		  $_POST
		);
	}

	public function scores() {
		$profile_id = @$_POST['profile_id'];
		if (!$profile_id) {
			$x = $this->profiles();
			if ($x) {
				$px = new SimpleXMLElement($x);
				$profile_id = (string)$px->row[0]->profile_id;
				$_POST['profile_id'] = $profile_id;
			}
		}

		return db_query_xml(
		  'SELECT s.student_id, p.person_id, p.first_name, p.last_name, a_profile_student_scores(p.person_id,:profile_id, COALESCE(:school_year, i_school_year())) AS scores FROM
		     p_students s JOIN p_people p ON p.person_id=s.person_id
		     WHERE s.school_year = COALESCE(:school_year,i_school_year()) AND s.grade_level = :grade_level AND s.bldg_id=:bldg_id
		     ORDER BY last_name, first_name',
		  $_POST
		);
	}
}