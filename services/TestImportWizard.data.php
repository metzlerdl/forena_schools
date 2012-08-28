<?php
class TestImportWizard extends ForenaSchools {
	public $title = 'Test Import Wizard';
	public $csv;
	public function auth() {

		$auth = $this->access('dist_admin');
		if (@!$auth && !$_COOKIE && !$_SESSION) {
			// no cookies no sessioin.  Flex UPLOAD bug so circumvent security
			return true;
		}
		return $auth;
	}

	public function __construct() {
		parent::__construct();
		$m_path = drupal_get_path('module', 'forena_schools');
    require_once("$m_path/inc/ForenaSchoolsCSV.inc");
    $this->csv = ForenaSchoolsCSV::instance();
    set_time_limit(3600);

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
		$result = $this->db->call('etl_import_test_scores()');
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
    $import_directory = $this->import_directory();
    $this->verify_directory($import_directory);
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
    $file_path = $this->import_directory() . '/' . $file_name;
		return '<message>' . htmlspecialchars($this->csv->table_from_csv('imp_test_scores', $file_path,true, $defaults)). '</message>';
	}

	public function testCodes() {
		return $this->db->query_xml('select code,name from a_tests order by name');
	}

/**
 * Perform the file upload for a test import batch
 */
public function uploadFile() {

  watchdog('debug', 'Staring upload');
  $import_type = @$_REQUEST['import_type'];
  // Testing the file upload
  $file_temp = $_FILES['Filedata']['tmp_name'];
  $file_name = $_FILES['Filedata']['name'];
  $file_path = $this->import_directory() . '/' . $file_name;
   // Complete upload
  $filestatus = move_uploaded_file($file_temp, $file_path);
   if ($filestatus) {
     $retvar = 'Successful Upload' . $file_name;
   }
   else {
     $retvar = '<pre>Error uploading file' . print_r($_FILES,1) . '</pre>';
   }
   watchdog('debug', $retvar);
  return $retvar;
}

public function fileStats() {
	$file_path = $this->import_directory();
	if (is_writable($file_path)) {
		$xml = '<filepath>' . htmlspecialchars($file_path) . '</filepath>';
	}
	else {
		$xml = "<message>Upload directory $file_path is not writable.  You will be unable to upload files.  Contact your system administrator if you believe this to be in error</message>";
	}
	return $xml;
}

public function removeFile() {
	$file_path = $this->import_directory() . '/' . $_POST['file_name'];
	unlink($filename);
	return $this->listFiles();
}

public function verifyImport() {
	$sql = '
SELECT
  count(1) total,
  count(distinct i.test_code) total_tests,
  count(distinct t.test_id) matched_tests,
  count(distinct i.sis_id) total_students,
  count(distinct p.person_id) matched_students,
  count(distinct i.measure_code) total_measures,
  count(distinct m.measure_id) matched_measures,
  count(distinct COALESCE(bldg_code, bldg_school_code)) total_buildings,
  count(distinct b.bldg_id) matched_buildings,
  count(distinct date_taken) total_dates,
  COUNT(distinct CASE WHEN a_test_schedule_seq(t.test_id,cast (i.date_taken as date)) is not null then i.date_taken end) matched_dates,
  count(distinct i.grade_level) total_grades,
  count(distinct g.grade_level) matched_grades,
  count(case when p.person_id is not null
             and t.test_id is not null
             and b.bldg_id IS NOT NULL
             and t.test_id IS NOT NULL
             and m.measure_id IS NOT NULL
             and g.grade_level IS NOT NULL
             and a_test_schedule_seq(t.test_id,cast (i.date_taken as date)) is not null
             and i.date_taken is not null then 1 end) total_complete
FROM
  import.imp_test_scores i LEFT JOIN p_people p ON p.sis_id=i.sis_id
    LEFT JOIN a_tests t ON i.test_code=t.code
    LEFT JOIN import.imp_test_translations tl ON i.test_code=tl.test_code AND i.measure_code=tl.import_code
    LEFT JOIN a_test_measures m ON t.test_id=m.test_id AND COALESCE(tl.measure_code,i.measure_code)=m.code
    LEFT JOIN i_buildings b ON i.bldg_code = b.code or b.sis_code = i.bldg_school_code
    LEFT JOIN i_grade_levels g ON i.grade_level=g.grade_level
	';
	return $this->db->query_xml($sql);
}


}