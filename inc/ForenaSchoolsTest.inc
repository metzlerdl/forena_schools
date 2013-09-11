<?php
class ForenaSchoolsTest {
  public $test_id;
  public $test;
  public $measures;
  public $schedules;

  /**
   * Singleton Factory.
   * @return ForenaSchoolsTest
   */
  public static function instance() {
    static $o = '';
    if (!$o) {
      $o = new ForenaSchoolsTest();
    }
    return $o;
  }

  /**
   * Determine the number of assessment records for each test
   * @return array Statistics array.
   */
  function stats() {

    $test_id = $this->test_id;
    $stats = array();

    $years = ForenaSchools::instance()->schoolYearList();
    foreach ($years as $year => $label) {
      // Check for assessments
      $rs = db_query('SELECT COUNT(1) FROM {a_assessments} WHERE test_id = :test_id and school_year = :school_year',
         array(':test_id' => $test_id, ':school_year' => $year));
      $stats[$label] = $rs->fetchColumn();

    }
    return $stats;

  }

  function recalcStats($school_year) {
    db_query('SELECT a_calc_score_stats(:school_year, :test_id)',
      array(':school_year' => $school_year, ':test_id' => $this->test_id));
  }

  // Load Test information
  function load($test_id) {
    $this->test_id = $test_id;
    $rs = db_query('SELECT * from {a_tests} WHERE test_id = :test_id', array(':test_id' => $test_id));
    if ($rs) {
      $this->test = $rs->fetchAssoc(PDO::FETCH_ASSOC);
    }
    else {
      $this->test = array();
    }
    return $this->test;
  }

  function listMeasures($test_id='') {
      if (!$test_id) $test_id = $this->test_id;
      $rs = db_query('SELECT measure_id, name FROM {a_test_measures}
      WHERE test_id = :test_id
        ORDER by sort_order', array(':test_id' => $test_id)
    );
    if ($rs) {
      return  $rs->fetchAllKeyed(0,1);

    }
    else {
      return array();
    }
  }

  function loadMeasures() {
    $rs = db_query('SELECT * FROM {a_test_measures}
      WHERE test_id = :test_id
        ORDER by sort_order', array(':test_id' => $this->test_id)
    );

    if ($rs) {
      $this->measures = $rs->fetchAllAssoc('measure_id', PDO::FETCH_ASSOC);
    }
    else {
      $this->measures = array();
    }
    return $this->measures;
  }

  // Load Schedules
  function loadSchedules() {
    $rs = db_query('SELECT * FROM {a_test_schedules} WHERE test_id = :test_id', array(':test_id' => $this->test_id));
    if (!$this->schedules = $rs->fetchAllAssoc('seq', PDO::FETCH_ASSOC)) {
      $this->schedules = array();
    }
    return $this->schedules;
  }

  // Load all validations
  function loadValidations() {
    $validations = array();
    $rs = db_query('SELECT * FROM {a_test_rules} r JOIN {a_test_measures} m ON m.measure_id=r.measure_id WHERE m.test_id = :test_id
      ORDER BY r.measure_id, r.grade_level', array(':test_id' => $this->test_id));
    if ($rs) {
      $rows = $rs->fetchAll(PDO::FETCH_ASSOC);
    }
    else {
      $this->validations = array();
    }
    foreach ($rows as $valid) {
      $validations[$valid['measure_id']][$valid['grade_level']][$valid['seq']] = $valid;
    }
    $this->validations = $validations;
    return $this->validations;
  }

  // Load Schedules
  function listSchedules($test_id='') {
    if (!$test_id) $test_id = $this->test_id;
    $rs = db_query('SELECT seq, label FROM {a_test_schedules} WHERE test_id = :test_id', array(':test_id' => $test_id));
    if ($rs) {
      return $rs->fetchAllKeyed(0, 1);
    }
    else {
      return array();
    }
  }

  /**
   * Return list of tests for a grade
   * Enter description here ...
   * @param  $grade_level Qualifying grade level.
   */
  function gradeLevelTests($grade_level) {
    $rs = db_query('SELECT test_id, name FROM {a_tests} WHERE :grade_level BETWEEN min_grade AND max_grade ORDER BY name', array(':grade_level' => $grade_level));
    if ($rs) {
      return $rs->fetchAllKeyed(0, 1);
    }
    else {
      return array();
    }
  }

  /**
   * Delete a test permanently
   */
  function delete() {
    $test_id = $this->test_id;
    $measures = $this->listMeasures($test_id);
    // Measure Data
    if ($measures) {
      $measure_ids = array_keys($measures);

      db_delete('a_scores')
        ->condition('measure_id', $measure_ids)
        ->execute();

      db_delete('a_score_stats')
        ->condition('measure_id', $measure_ids)
        ->execute();

      db_delete('a_test_rules')
        ->condition('measure_id', $measure_ids)
        ->execute();
    }

    db_delete('a_assessments')
      ->condition('test_id', $test_id)
      ->execute();

    db_delete('a_test_schedules')
      ->condition('test_id', $test_id)
      ->execute();

    db_delete('a_test_measures')
      ->condition('test_id', $test_id)
      ->execute();

    db_delete('a_tests')
      ->condition('test_id', $test_id)
      ->execute();
  }

  /**
   * Generate a help message from the validation
   * @param $measure array containing measure data
   * @param $validation array containing validation data.
   * @return string
   */
  static function scoreHelp($validation) {
    extract($validation);
    $help = '';
    $messages = array();
    if ($level_1) {
      $messages[] = "$level_1 struggling";
    }
    if ($level_2) {
      $messages[] = "$level_2 below ";
    }
    if ($level_3) {
      $messages[] = "$level_3 meets";
    }
    if ($level_4) {
      $messages[] = "$level_4 exceeds";
    }
    if ($max_score) {
      $message[] = "$max_score maximum";
    }
    $help = implode(', ', $messages);
    return $help;
  }
}