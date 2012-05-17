<?php
class TestDefinition extends ForenaSchools {
	public $title = 'Test Definition';
	public function auth() {
		return $this->access('dist_admin');
	}

	public function getTest() {
		$parms = array('test_id' => $_REQUEST['test_id']);
		return $this->db->call('a_test_xml(:test_id)', $parms);
	}

	public function saveTest() {
		$parms = array('xml' => $_POST['xml']);
		return $this->db->call('a_test_save_xml(:xml)', $parms);
	}
  /**
   * Generate rules for all measure/grade_level/schedules
   * This does so based on the xml data.
   */
	public function generateRules() {
		$parms = array(
		  'm_xml' => $_POST['m_xml'],
		  's_xml' => $_POST['s_xml'],
		  'r_xml' => $_POST['r_xml'],
		  'min_grade' => $_POST['min_grade'],
		  'max_grade' => $_POST['max_grade']
		);
		return $this->db->call('a_test_generate_rules_xml(:m_xml, :s_xml, :r_xml, :min_grade, :max_grade)', $parms);
	}

	public function gradeLevels() {
		$parms = array(
		  'min_grade' => $_REQUEST['min_grade'],
		  'max_grade' => $_REQUEST['max_grade'],
		);
		return $this->db->query_xml('select i.* from i_grade_levels i JOIN (SELECT g AS grade_level FROM generate_series(CAST(:min_grade AS INTEGER),CAST(:max_grade AS INTEGER)) g) gl on gl.grade_level=i.grade_level', $parms, 'grades');
	}

	public function yearDates() {
		return $this->db->query_xml('
		  select * from i_school_years WHERE schooL_year=i_school_year()
		');
	}

	public function subjects() {
		return $this->db->query_xml('select * FROM i_subjects ORDER BY subject');
	}
}