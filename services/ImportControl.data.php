<?php
require_once('csvimport.inc');
class ImportControl extends ForenaSchools {
	public $title = 'Import Control';
	public function auth() {
		return $this->access_level('dist_admin');
	}

	public function __construct() {
    GLOBAL $import_directory;
    $import_directory = $import_directory ? $import_directory : 'scripts/import';
    $this->import_directory = rtrim($import_directory,'/');
	}

	public function tests() {
		return $this->db->query_xml('
			SELECT
			  i.test_code,
			  i.measure_code,
			  s.seq,
			  s.label,
			  t.test_id,
			  count(1) as scores,
			  max(m.code) AS matched_code,
			  min(parse_numeric(score)) AS min_score,
			  max(parse_numeric(score)) AS max_score,
			  min(cast(date_taken AS DATE)) min_date,
			  max(cast(date_taken AS DATE)) max_date,
			  min(i_calc_school_day(cast(i.date_taken AS date))) AS min_day,
			  max(i_calc_school_day(cast(i.date_taken AS date))) AS max_day,
			  max(i.description) as description
			  FROM imp_test_scores i
			  LEFT JOIN a_tests t ON i.test_code=t.code
			  LEFT JOIN a_test_schedules s  ON t.test_id = s.test_id AND i_calc_school_day(cast(i.date_taken AS date), false)  BETWEEN
			    s.start_day AND s.end_day
			  LEFT JOIN import.imp_test_translations tt ON i.measure_code=tt.import_code
			  LEFT JOIN a_test_measures m ON t.test_id = m.test_id AND COALESCE(tt.measure_code, i.measure_code) = m.code
			  GROUP BY i.test_code, i.measure_code, t.test_id, s.seq, s.label
			  ORDER BY test_code, seq, matched_code
		',
		$_POST);
	}

	public function importScores() {
		$result = $this->db->call('etl_merge_test_scores()');
		return '<message>' . htmlspecialchars($result) . '</message>';
	}

	public function saveTranslations() {
    $this->db->call('etl_save_translations(:xml)', $_POST);
	  return $this->tests();
	}

	public function translateScores() {
		$this->db->call('etl_translate_scores()');
		return $this->tests();
	}

	/*
	 * Generate a listing of files that need to be uploaded.
	 */
	public function listFiles() {
    $import_directory = $this->import_directory;
    $d = @dir($import_directory);
    if (!$d) {
    	return '<error> Could not read ' . htmlspecialchars($import_directory) . '.</error>';
    }
    $xml = new SimpleXMLElement('<directory/>');
    while (FALSE !== ($entry=$d->read())) {
     if (strpos($entry,'.')!==0) {
	     $f = $xml->addChild('file', '');
	     $f['name'] = $entry;
     }
    }
    return $xml->asXML();
	}

	public function uploadTestFile() {
		$file_name = $_POST['file_name'];
	  $defaults = $_POST;
	  unset($defaults['file_name']);
	  unset($defaults['service']);
	  unset($defaults['method']);
    $file_path = $this->import_directory . '/' . $file_name;
		return '<message>' . htmlspecialchars(table_from_csv('imp_test_scores', $file_path,true, $defaults)). '</message>';
	}

	public function testCodes() {
		return $this->db->query_xml('select code,name from a_tests order by name');
	}

}