<?php
require_once('forena/forena.standalone.inc');
class DataBroker {
	public function auth() {
		return access_level('teacher');
	}

	public function search() {
		$b = new FrxSqlQueryBuilder();
		return $b->query('ims/search/students', $_POST)
		  ->filter('school_year', '=')
		  ->filter_not_null('bldg_id', '=')
		  ->where('grade_level BETWEEN :min_grade AND COALESCE(:max_grade, grade_level)', @$_POST['min_grade'])
		  ->where('EXISTS(select 1 FROM s_group_members m JOIN s_groups g ON g.group_id=m.group_id WHERE owner_id=:owner_id
		            AND m.student_id=s.student_id AND g.school_year = v.school_year)', @$_POST['owner_id'])
		  ->execute();

	}

	public function buildings() {
		$_POST['current_user'] = current_login();
		if (access_level('dist_admin')) {
			$sql = 'select bldg_id AS data, name as label from i_buildings b';
		} else {
			$sql = 'select bldg_id AS data, name as label from i_buildings b JOIN
			  p_staff s ON b.bldg_id=s.bldg_id JOIN p_people on s.person_id=p.person_id
			  WHERE login=:current_user';
		}
		return db_query_xml($sql);
	}

	public function gradeLevels() {
		if (isset($_POST['bldg_id'])) {
		  $sql = 'select grade_level AS data, abbrev AS label FROM i_grade_levels g
		    JOIN i_buildings b ON grade_level BETWEEN b.min_grade AND b.max_grade
		    ORDER BY g.grade_level';
		}
		else {
			$sql = 'select grade_level AS data, abbrev AS label FROM i_grade_levels
			  ORDER BY grade_level';
		}
		return db_query_xml($sql);
	}

	public function schoolYears() {
		return db_query_xml('SELECT school_year AS data, label FROM i_school_years ORDER BY start_date desc ');
	}
}