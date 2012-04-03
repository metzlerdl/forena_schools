<?php
class DataBroker {
	public $title = 'Assessment Analysis';
	public function auth() {
		return access_level('teacher');
	}
  public function profiles() {
		return db_query_xml(
		  'SELECT p.profile_id, p.name,  a_profile_measures_xml(profile_id) AS measures from a_profiles p
		    WHERE :grade_level BETWEEN min_grade AND max_grade
		      AND school_year_offset=0
		      and p.analysis_only = true
		    ORDER BY p.weight, p.min_grade, p.max_grade, p.name',
		  $_POST
		);
	}

	public function gradeLevels() {
    return db_query_xml(
      'SELECT grade_level,name FROM i_grade_levels order by grade_level',
      $_POST
    );
	}

	public function schoolYears() {
		return db_query_xml(
		  'SELECT school_year, label FROM i_school_years'
		);
	}

	public function districtScores() {
	  return db_query_xml(
	    "SELECT pm.profile_id,
      s.*,
      g.name AS grade,
      t.name AS test,
      m.name as measure,
      m.abbrev as abbrev,
      sc.label AS schedule,
      m.name || ' ' || sc.label AS label
FROM
  a_profile_measures pm JOIN a_score_stats s on pm.measure_id=s.measure_id
    AND pm.seq=s.seq OR pm.seq=0
    JOIN a_test_measures m ON m.measure_id = s.measure_id
    JOIN a_tests t ON t.test_id=m.test_id
    JOIN a_test_schedules sc ON sc.test_id = m.test_id AND sc.seq=s.seq
    JOIN i_grade_levels g ON g.grade_level=s.grade_level
    WHERE s.bldg_id=-1 AND s.grade_level= :grade_level
      AND pm.profile_id=:profile_id
      AND s.school_year = :school_year",
	    $_POST
	  );
	}

	public function buildingScores() {
	  return db_query_xml(
	    "SELECT pm.profile_id,
      s.*,
      g.name AS grade,
      t.name AS test,
      m.name as measure,
      b.name AS building,
      m.abbrev as abbrev,
      sc.label AS schedule,
      b.abbrev AS label
FROM
  a_profile_measures pm JOIN a_score_stats s on pm.measure_id=s.measure_id
    AND pm.seq=s.seq OR pm.seq=0
    JOIN i_buildings b ON b.bldg_id=s.bldg_id
    JOIN a_test_measures m ON m.measure_id = s.measure_id
    JOIN a_tests t ON t.test_id=m.test_id
    JOIN a_test_schedules sc ON sc.test_id = m.test_id AND sc.seq=s.seq
    JOIN i_grade_levels g ON g.grade_level=s.grade_level
    WHERE s.measure_id=:measure_id AND s.grade_level= :grade_level
      AND s.seq=:seq
      AND s.school_year = :school_year
      AND pm.profile_id=:profile_id",
	    $_POST
	  );
	}
}