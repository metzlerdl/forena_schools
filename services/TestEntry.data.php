<?php
class TestEntry extends ForenaSchools {
	public $title='Test Entry';

	public function auth() {
		return $this->access('teacher');
	}

	public function info() {
		return $this->db->query_xml("
		  SELECT p.first_name || ' ' || p.last_name as owner, g.name as group_name, group_id, y.label AS year_label,
		    s.label AS sched_label,
		    i_calc_school_date(s.target_day, g.school_year) target_date
		   FROM s_groups g
		     LEFT JOIN i_school_years y ON g.school_year=y.school_year
		     LEFT JOIN a_test_schedules s ON s.test_id = :test_id
		       AND s.seq = :seq
		     LEFT JOIN p_people ON g.owner_id=p.person_id
		   where group_id=:group_id
		", $_POST);
	}

	public function groupMembers() {
		$test_date = @$_POST['date_taken'];
		$test_id = @$_POST['test_id'];
		if ($test_date && $test_id) {
		  $xml = $this->db->query_xml('
	    SELECT * FROM
		   (SELECT
		      row_number() OVER (partition by g.person_id,
		        :test_id,
		        :seq
		         ORDER BY a.date_taken desc) AS r,
		      g.*,
		      a.date_taken,
		      a_assessment_entry_xml(g.person_id, g.grade_level, :test_id,
		        COALESCE(a.date_taken,
		          CAST(:date_taken
		            AS date)))
		         AS scores
		    FROM s_group_members_v g
		      JOIN a_tests t ON g.grade_level BETWEEN t.min_grade AND t.max_grade
		        AND t.test_id=:test_id
		      LEFT JOIN a_assessments a
		      ON g.school_year = a.school_year
		        AND g.bldg_id = a.bldg_id
		        AND a.seq = :seq
		        AND a.person_id = g.person_id
		        AND a.test_id= t.test_id
		        AND a.date_taken > CAST(:date_taken AS date)

		    WHERE group_id= :group_id
		    ) S
		    ORDER BY  last_name, first_name
		  ', $_POST);
		}
		else {
			$xml = $this->db->query_xml('SELECT g.* from s_group_members_v g WHERE group_id = :group_id', $_POST);
		}
		return $xml;
	}

	/**
	 * Get the tests that are possible for this collection
	 */
	public function tests() {
	  return $this->db->query_xml ('
	    SELECT t.test_id, t.name, a_test_entry_measures_xml(t.test_id) AS measures
	      FROM s_groups g JOIN a_tests t ON (g.min_grade_level BETWEEN t.min_grade AND t.max_grade )
	        OR (g.max_grade_level BETWEEN t.min_grade AND t.max_grade)
          OR (t.min_grade BETWEEN g.min_grade_level AND g.max_grade_level)
        WHERE g.group_id=:group_id
          AND t.allow_data_entry=true', $_POST);

	}

	public function saveScores() {
		$this->db->call('a_test_entry_save_xml(:xml)', $_POST);
		return $this->groupMembers();
	}

	public function schedules() {
		return $this->db->query_xml('
		  SELECT sign(i_calc_school_day(CAST (now() as DATE)) - start_day) AS future,
		    abs(i_calc_school_day(CAST (now() as DATE)) - target_day) AS days_away,
		    s.*,
		    i_calc_school_date(s.start_day, g.school_year) AS start_date,
		    i_calc_school_date(s.target_day, g.school_year) AS target_date,
		    i_calc_school_date(s.end_day, g.school_year) AS end_date
		  FROM s_groups g JOIN a_test_schedules s on test_id=:test_id
		    AND g.group_id=:group_id
		  ORDER BY 1 desc,2
		',
		$_POST);
	}
}