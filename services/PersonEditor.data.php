<?php
class PersonEditor extends ForenaSchools {

	public function auth() {
		return $this->access('dist_admin');
	}

	public function getPerson() {
		return $this->db->query_xml('select p.*, p_staff_xml(person_id) AS staff_info, p_student_xml(person_id) AS student_info FROM p_people p WHERE person_id=:person_id', $_POST);
	}

	/**
	 * Save the collections.
	 */
	public function save() {
		$data = array('login' => $this->current_login());
		$data['xml'] = $_POST['xml'];
		$person_id = $this->db->call('p_save_person(:xml,:login)', $data);
		$_POST['person_id'] = $person_id;
		return $this->getPerson();
	}

}