<?php
class StudentSearch extends ForenaSchools
 {
	public $title = 'Student Search';
	public function auth() {
		$bldg_id = @$_REQUEST['bldg_id'];
		if ($bldg_id) {
			return $this->access('bldg_admin');
		}
		else {
			return $this->access('dist_admin');
		}
	}

 	public function execSearch() {
		$q = $_REQUEST['q'];
		if (strpos($q,','))
		// Break down the building identifier.
		{
			list($last, $first) = @explode(',', $q);
			$parms = array(
			  'last_name' => trim($last) . '%',
			  'first_name' => trim($first) . '%',
			  'bldg_id' => @$_REQUEST['bldg_id'],
			);
		}
		else {
			$parms = array(
			  'last_name' => trim($q) . '%',
			  'bldg_id' => @$_REQUEST['bldg_id'],
			);
		}

		$result = $this->db->call('p_student_search(:last_name, :first_name, :bldg_id)', $parms);

    return $result;
	}
}