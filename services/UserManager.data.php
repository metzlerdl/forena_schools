<?php
class UserManager extends ForenaSchools {

	public $title= 'User Manager';
	public function auth() {
    return $this->access_level('sys_admin');
	}

	public function searchPeople() {
				$q = $_REQUEST['q'];
		if (strpos($q,','))
		// Break down the building identifier.
		{
			list($last, $first) = @explode(',', $q);
			$parms = array(
			  'last_name' => trim($last) . '%',
			  'first_name' => trim($first) . '%',
			);
		}
		else {
			$parms = array(
			  'last_name' => trim($q) . '%',
			  'first_name' => '%'
			);
		}
    $result= $this->db->query_xml('
      SELECT first_name, last_name, middle_name, login,sis_id, state_student_id, person_id
      FROM p_people
      WHERE UPPER(last_name) like UPPER(:last_name)
        AND (:first_name IS NULL or UPPER(first_name) LIKE UPPER(:first_name))
    ', $parms);


    return $result;
	}
}