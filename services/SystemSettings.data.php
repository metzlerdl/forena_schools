<?php
class SystemSettings extends ForenaSchools {
	/*
	 * Get District information
	 */
	public $title = 'System Settings';

	public function auth() {
		return access_level('sys_admin');
	}


	public function buildingInfo($msg='') {
		return $this->db->query_xml(
		  'select bldg_id,
		    name,
		    nts(abbrev) AS abbrev,
		    nts(code) as code,
		    nts(sis_code) as sis_code,
		    nts(min_grade) AS min_grade,
		    nts(max_grade) AS max_grade,
		    nts(address) AS address,
		    nts(city) as city,
		    nts(state) as state,
		    nts(zip) as zip,
		    nts(phone) as phone,
		    nts(fax) as fax   from i_buildings',$_REQUEST,'buildings',$msg);
	}

	public function saveBuildings() {
		$this->db->call('i_buildings_save_xml(:xml)',$_REQUEST);
		return $this->buildingInfo('Buildings Saved.');
	}

	public function schoolYears($msg='') {
		return $this->db->query_xml('select * from i_school_years order by school_year desc',null,'school_years',$msg);
	}

	public function saveYears() {
		$xml  = $this->db->call('i_school_years_save(:xml)',$_REQUEST);
		return $this->schoolYears();
	}
}