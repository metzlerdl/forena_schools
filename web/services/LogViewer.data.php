<?php
class DataBroker {
	public $title= 'Log Viewer';
	public function auth() {
    return access_level('sys_admin');
	}

	public function recentLogs() {
		return db_query_xml('select * from logs ORDER BY log_time DESC LIMIT 1000');
	}

	public function clearLogs() {
		db_query('delete from logs');
		return $this->recentLogs();
	}

}