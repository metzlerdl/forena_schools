<?php
class StudentSearch extends ForenaSchools
 {
	public $title = 'Student Search';
	public function auth() {
		$bldg_id = $_REQUEST['bldg_id'];
		if ($bldg_id) {
			return access('bldg_admin');
		}
		else {
			return access('dist_admin');
		}
	}
}