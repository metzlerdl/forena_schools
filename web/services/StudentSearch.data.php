<?php
class DataBroker {
	public $title = 'Student Search';
	public function auth() {
		$bldg_id = $_REQUEST['bldg_id'];
		if ($bldg_id) {
			return access_level('bldg_admin');
		}
		else {
			return access_level('dist_admin');
		}
	}
}