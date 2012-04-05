<?php
class DataBroker {
	public $title='Test Entry';

	public function auth() {
		return access_level('teacher');
	}

	public function studentAssessments() {
		 $xml = db_query_xml('SELECT a.assessment_id, a.person_id, a.bldg_id, a.grade_level, a.test_id, a.date_taken, t.name AS test_name,
		   s.label AS sched_label,
		   e.student_id,
		   a_assessment_entry_xml(a.person_id, a.grade_level, a.test_id, a.date_taken) AS scores
		   FROM a_assessments a JOIN a_tests t on a.test_id=t.test_id
		     JOIN a_test_schedules s ON s.test_id=t.test_id AND a.seq=s.seq
		     LEFT JOIN p_students e ON e.person_id=a.person_id AND e.school_year=a.school_year and e.bldg_id=a.bldg_id
		   WHERE a.person_id=:person_id
		   ORDER BY a.date_taken desc', $_POST);
		 return $xml;
	}

	public function saveScores() {
		if (isset($_POST['to_remove'])) {
			$to_remove = $_POST['to_remove'];
			foreach ($to_remove as $assessment_id) {
				db_call('a_assessment_delete(:assessment_id)', array('assessment_id' => $assessment_id));
			}
		}

		db_call('a_test_entry_save_xml(:xml)', $_POST);
		return $this->studentAssessments();
	}

	public function tests() {
		return db_query_xml("SELECT t.* FROM a_tests t where :grade_level BETWEEN t.min_grade AND t.max_grade AND t.allow_data_entry=true ORDER BY name", $_POST);
	}

	public function schedules() {
		return db_query_xml('
		  SELECT sign(i_calc_school_day(CAST (now() as DATE)) - start_day) AS future,
		    abs(i_calc_school_day(CAST (now() as DATE)) - start_day) AS days_away,
		    s.*,
		    i_calc_school_date(s.start_day) AS start_date,
		    i_calc_school_date(s.end_day) AS end_date
		  FROM a_test_schedules s WHERE test_id=:test_id
		  ORDER BY 1 desc,2
		',
		$_POST);
	}

	public function buildings() {
		return db_query_xml("SELECT s.school_year,s.grade_level,b.* FROM i_buildings b LEFT JOIN p_students s ON b.bldg_id = s.bldg_id AND s.person_id=:person_id
		  WHERE :grade_level BETWEEN b.min_grade AND b.max_grade ORDER BY s.school_year DESC, b.name ", $_POST);
	}

	public function gradeLevels() {
		return db_query_xml('
		  SELECT g.grade_level, g.name, g.abbrev FROM i_grade_levels g
		    WHERE g.grade_level <= (SELECT MAX(grade_level) FROM p_students s WHERE s.person_id=:person_id)
		    ORDER BY grade_level DESC
		', $_POST);
	}

	public function newAssessment() {
		return db_query_xml('SELECT p.person_id, :bldg_id AS bldg_id, :grade_level AS grade_level, t.test_id, :date_taken AS date_taken, t.name AS test_name,
		  s.label AS sched_label,
		  a_assessment_entry_xml(p.person_id, :bldg_id, :grade_level, t.test_id, CAST(:date_taken AS DATE))
		 FROM p_people p JOIN a_tests t ON t.test_id=:test_id AND p.person_id = :person_id', $_POST);

	}

}