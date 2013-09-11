<?php
class Identify extends ForenaSchools {

	public function auth() {
		return $this->access('teacher');
	}

	/**
	 * Retrieve profile
	 * Get the profile that has been requested for identification.
	 */
	public function profile() {
		return $this->db->query_xml(
		  'SELECT profile_id, name,  a_profile_measures_xml(profile_id) AS measures from a_profiles where profile_id=:profile_id',
		  $_POST
		);
	}

	/**
	 * Retrieve scores for the identified group of people.
	 * Enter description here ...
	 */
	public function scores() {
		$measure_id = $_POST['measure_id'];
		// Perform identification
    $sql = "
      SELECT student_id, last_name, first_name, a_profile_student_scores(person_id,:profile_id,:school_year) AS scores FROM (
    	SELECT
    	  row_number() over (partition by p.person_id order by a.date_taken desc) AS r,
    	  p.*,
    	  s.norm_score,
    	  ss.student_id
    	FROM a_scores s JOIN a_assessments a ON a.assessment_id = s.assessment_id
        JOIN p_people p ON p.person_id = a.person_id
        LEFT JOIN p_students ss ON a.person_id=ss.person_id
          AND a.bldg_id=ss.bldg_id AND a.school_year = ss.school_year
    	WHERE a.bldg_id = COALESCE(:bldg_id, a.bldg_id)
    	  AND a.grade_level = :grade_level
    	  AND a.school_year = :school_year
    	  AND a.seq = :seq
    	  AND s.measure_id = :measure_id
    	  ) v
    	WHERE r=1
    	  AND TRUNC(norm_score) = :norm_score
    	ORDER BY last_name, first_name
    ";
   return $this->db->query_xml($sql, $_POST);
	}

	public function groups() {
		//@TDODO: Verify permissions on groups
		return $this->db->query_xml("
		  SELECT -1 as group_id, 'Create Group' AS name, 9999 AS school_year
		  UNION ALL
		  SELECT group_id, name, school_year
		  FROM s_groups
		  WHERE group_type<>'course'
		  ORDER BY school_year desc, name
		");
	}

	public function addGroupMembers() {
    $group_id = $this->db->call('s_group_add_members(:group_id, :xml)', $_POST);
    RETURN $this->db->query_xml('select group_id, name FROM s_groups WHERE group_id=:group_id', array('group_id' => $group_id));

	}
}