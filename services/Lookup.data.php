<?php
class Lookup extends ForenaSchools {
	public function auth() {
		return TRUE;
	}

	public function buildings() {
		return $this->db->query_xml('
		  SELECT * FROM i_buildings
		  ORDER BY min_grade,name
		');
	}

	public function buildingInfo() {
		return $this->db->query_xml('
		  SELECT * FROM i_buildings
		  WHERE bldg_id=:bldg_id
		', $_POST);
	}

	public function gradeLevels() {
		return $this->db->query_xml('
		  SELECT * FROM i_grade_levels
		    ORDER BY grade_level
		');
	}

	public function schoolYears() {
		return $this->db->query_xml(
		  'SELECT school_year, label FROM i_school_years ORDER BY school_year desc'
		);
	}

  public function tests() {
  	return $this->db->query_xml('
  	  SELECT * from a_tests
  	    ORDER BY name
  	');
  }

  public function measures()  {
  	return $this->db->query_xml('
  	  SELECT * from a_test_measures m
  	    WHERE m.test_id=:test_id
  	    ORDER BY m.name
  	', $_POST);
  }
}