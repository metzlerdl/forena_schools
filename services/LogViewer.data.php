<?php
class LogViewer extends ForenaSchools {
	public $title= 'Log Viewer';
	public function auth() {
    return $this->access('sys_admin');
	}

	public function recentLogs() {
		return $this->db->query_xml('select * from logs ORDER BY log_time DESC LIMIT 1000');
	}

	public function clearLogs() {
		$this->db->query('delete from logs');
		return $this->recentLogs();
	}

}