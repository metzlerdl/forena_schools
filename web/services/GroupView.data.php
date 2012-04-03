<?php
class DataBroker {
	// Verify authentication
	public $title='Group View';
	public function __construct() {
		$_POST['school_year'] = db_call('i_school_year()');
	}

	public function auth() {
		return access_level('teacher');
	}

	public function profiles() {
		return db_query_xml(
		  'SELECT p.profile_id, p.name,  a_profile_measures_xml(p.profile_id) AS measures from a_profiles p
		    JOIN s_groups g ON (g.bldg_id=p.bldg_id OR p.bldg_id=-1)
		      AND (
		        g.min_grade_level BETWEEN p.min_grade AND p.max_grade
		        OR g.max_grade_level BETWEEN p.min_grade AND p.max_grade
		        OR (p.min_grade <= g.max_grade_level AND p.max_grade >= g.max_grade_level)
		      )
		   WHERE g.group_id=:group_id AND analysis_only=false
		   ORDER BY p.weight, p.min_grade, p.max_grade, p.max_grade, p.name',
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

		  'SELECT g.student_id, g.person_id, g.first_name, g.last_name, a_profile_student_scores(person_id,:profile_id, g.school_year) AS scores FROM s_group_members_v g
		    JOIN a_profiles p ON p.profile_id=:profile_id
		    WHERE group_id=:group_id
		    ORDER BY last_name, first_name',
		  $_POST
		);
	}
}