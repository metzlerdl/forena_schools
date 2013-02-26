<?php
class TestControlPanel extends ForenaSchools {
	public $title = 'Testing Control Panel';

	public function auth() {
		return $this->access('dist_admin');
	}


	public function scoreStats() {
		RETURN $this->db->query_xml('
			SELECT
			  s.label AS schedule, b.abbrev AS building, b.name as building_name, g.abbrev AS grade, c.grade_level, c.total_students, ss.stat_scores,
			  building_students, CASE WHEN c.total_students<=bc.building_students THEN CAST(100.00*c.total_students/bc.building_students AS NUMERIC(4,1)) END as coverage,
			  min_date_taken, max_date_taken
			FROM a_test_schedules s JOIN
			 (select bldg_id,test_id,school_year, grade_level,seq, min(date_taken) AS min_date_taken, max(date_taken) AS max_date_taken, count(distinct person_id) as total_students
			  FROM a_assessments
			  where test_id=:test_id
			    AND school_year=:school_year
			  GROUP BY bldg_id, test_id,school_year, grade_level, seq) c
			ON c.test_id=s.test_id AND c.seq=s.seq
			JOIN i_buildings b ON b.bldg_id=c.bldg_id
			JOIN i_grade_levels g ON c.grade_level=g.grade_Level
			LEFT JOIN (SELECT school_year, bldg_id, grade_level,  count(1) AS building_students FROM p_students
			  WHERE school_year = :school_year
			  GROUP BY school_year,bldg_id, grade_level) bc
			ON c.bldg_id=bc.bldg_id AND c.school_year = bc.school_year AND c.grade_level=bc.grade_level
			LEFT JOIN (
			  SELECT bldg_id, test_id,  school_year, grade_level, seq, MAX(total) AS stat_scores
			    FROM a_test_measures m
			      JOIN a_score_stats st
			        ON m.measure_id=st.measure_id
			    WHERE test_id=:test_id
			      AND school_year=:school_year
			    GROUP BY bldg_id, test_id, school_year, grade_level, seq
			    ) ss ON ss.bldg_id=c.bldg_id AND ss.grade_level = c.grade_level AND ss.test_id=c.test_id AND ss.school_year = c.school_year AND ss.seq=c.seq

			ORDER BY b.abbrev, c.grade_level,s.seq
					',
		$_POST);
	}

	public function recalcStats() {
		$this->db->call('a_calc_score_stats(:school_year, :test_id)', $_POST);
		return $this->scoreStats();
	}

	public function renormalizeScores() {
	  $message = $this->db->call('a_renormalize_scores(:test_id, :school_year)', $_POST);
	  return '<message>' . htmlspecialchars($message) . '</message>';
	}
}