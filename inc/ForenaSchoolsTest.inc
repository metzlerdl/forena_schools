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

  // Load Test information
  function load($test_id) {
    $this->test_id = $test_id;
    $rs = db_query('SELECT * from {a_tests} WHERE test_id = :test_id', array(':test_id' => $test_id));
    if ($rs) {
      $this->test = $rs->fetchAll(PDO::FETCH_ASSOC);
    }
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
      $rs->fetchAll(PDO::FETCH_ASSOC);
    }
    else {
      $this->measures = array();
    }
  }

  // Load Schedules
  function loadSchedules() {
    $rs = db_query('SELECT * FROM {a_test_schedules} WHERE test_id = :test_id', array(':test_id' => $this->test_id));
    if ($rs) {
      $this->schedules = $rs->fetchAll(PDO::FETCH_ASSOC);
    }
    else {
      $this->schedules = array();
    }
    return $this->schedules;
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
}