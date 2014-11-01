<?php
class ForenaSchoolsTest {
  public $test_id;
  public $test;
  public $measures;
  public $schedules;
  public $validations;

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

  // Create empty shell of a test.
  function create() {
    $test = array_fill_keys(array('test_id', 'name', 'code', 'abbrev', 'sbuject_area', 'inactive', 'min_grade', 'max_grade'), NULL);
    return $test;
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

    /**
     * Post process the measures
     */
    foreach ($this->measures as $mid => $measure) {
      $calc_measures = trim($measure['calc_measures'], "{}");
      if ($calc_measures) {
        $calc_measures = explode(',', $calc_measures);
      }
      else {
        $calc_measures = array();
      }
      $this->measures[$mid]['calc_measures'] = $calc_measures;
    }
    return $this->measures;
  }

  // Load Schedules
  function loadSchedules() {

    $rs = db_query('SELECT * FROM {a_test_schedules} WHERE test_id = :test_id ORDER BY seq',
        array(':test_id' => $this->test_id)
    );
    if (!$this->schedules = $rs->fetchAllAssoc('seq', PDO::FETCH_ASSOC)) {
      $this->schedules = array();
    }
    $y = ForenaSchoolsYear::instance();
    foreach($this->schedules as $k=>$schedule) {
      $this->schedules[$k]['start_date'] = $y->date($schedule['start_day']);
      $this->schedules[$k]['end_date'] = $y->date($schedule['end_day']);
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


  /**
   * Saves general information about a test.
   * @param unknown $data
   */
  function saveGeneral(&$data) {
    $fields = $this->create();
    // Make sure we only save the appropriate keys.
    $data = array_intersect_key($data, $fields);

    if ($data['test_id']) {
      db_update('a_tests')
        ->fields($data)
        ->condition('test_id', $data['test_id'])
        ->execute();
    }
    else {
      unset($data['test_id']);
      $test_id = db_insert('a_tests')
        ->fields($data)
        ->execute();
      $this->test_id = $data['test_id'] = $test_id;
    }
    $this->load($data['test_id']);
  }

  /**
   * A comparison fuction that will sort measures by grouping and weight.
   * @param unknown $a
   * @param unknown $b
   * @return number
   */
  public function msortCompare($a, $b) {
    $a_is_child = !empty($a['parent_measure']);
    $b_is_child = !empty($b['parent_measure']);

    // We can simply compare based on weights if we're of the same level and the same parent or not parents at all.
    if (($a_is_child && $b_is_child && $a['parent_measure'] == $b['parent_measure']) || (!$a_is_child && !$b_is_child) ) {
      $a_weight = $this->weights[$a['measure_id']];
      $b_weight = $this->weights[$b['measure_id']];
      if ($a_weight == $b_weight) return $a['measure_id'] < $b_measure_id ? -1 : 1;
      return ($a_weight < $b_weight) ? -1 : 1;
    }

    // Compare parents
    $a_id = $a['parent_measure'] ? $a['parent_measure'] : $a['measure_id'];
    $b_id = $b['parent_measure'] ? $b['parent_measure'] : $b['measure_id'];
    $a_weight = $this->weights[$a_id];
    $b_weight = $this->weights[$b_id];
    if ($a_id != $b_id) {
      if ($a_weight == $b_weight) return $a_id < $b_id ? -1 : 1;
      return $a_weight < $b_weight ? -1 : 1;
    }

    // If its the same parent then the child is always greater than the non child
    return $b_is_child ? -1 : 1;


  }


  /**
   * Save all measures retaining their sort order.
   * @param unknown $measures
   */
  function saveMeasures(&$measures) {
    // Sort by weight.
    $this->weights = array();
    foreach ($measures as $mid => $measure) {
      if (!$measure['measure_id']) $measures[$mid]['measure_id'] = $mid;
      if (isset($measure['weight'])) $this->weights[$mid] = $measure['weight'];
    }
    usort($measures, array($this, 'msortCompare'));

    //Rebuild sort and parent criteria:
    $parent_no = 0;
    $child_no = 0;

    foreach ($measures as $mid=>$measure) {
      if (isset($measure['parent_measure'])) {
        if (!$measure['parent_measure']) {
          $measures[$mid]['parent_measure'] = $measure['parent_measure'] = $measure['measure_id'];
        }
        if (!$measure['parent_measure'] || $measure['parent_measure'] == $measure['measure_id']) {
          $parent_no++;
        }
        else {
          $child_no++;
        }
        $measures[$mid]['sort_order'] = $measure['sort_order'] = sprintf('%d.%02d', $parent_no, $child_no);
        // Remove the weight column
        unset($measure['weight']);
      }

      $measure_id = $measure['measure_id'];
      unset($measure['measure_id']);

      // Preprocess the calculated measures if they are in the field set.
      if (isset($measure['calc_measures'])) {
        $calc_measures = $measure['calc_measures'];
        if (is_array($calc_measures)) {
          $calc_measures = '{' . implode(',', array_filter($calc_measures)) . '}';
        }
        elseif (!$calc_measures) {
          $calc_measures = '{}';
        }
        $measure['calc_measures'] = $calc_measures;
      }
      db_update('a_test_measures')
        ->fields($measure)
        ->condition('measure_id', $measure_id)
        ->execute();
    }

  }

  /**
   * Helper method to save a single measure.
   * @param unknown $measure
   */
  function saveMeasure($measure) {
    if (isset($measure['measure_id'])) {
      $measures = array($measure['measure_id'] => $measure);
      $this->saveMeasures($measures);
    }
  }

  /**
   * Save the test schedules.
   * @param unknown $schedules
   */
  function saveSchedules(&$schedules) {
    $y=ForenaSchoolsYear::instance();
    $seqs = array();
    if ($schedules) foreach($schedules as $k => $schedule) {

      $schedule['start_day'] = $y->day($schedule['start_date']);
      $schedule['end_day'] = $y->day($schedule['end_date']);
      $fields = array_fill_keys(array('seq', 'label', 'start_day', 'end_day'), '');
      $schedule = array_intersect_key($schedule, $fields);
      $seqs[] = $schedule['seq'];
      if (isset($this->schedules[$schedule['seq']])) {
        $seq = $schedule['seq'];
        unset($schedule['seq']);
        db_update('a_test_schedules')
          ->fields($schedule)
          ->condition('test_id', $this->test_id)
          ->condition('seq', $seq)
          ->execute();
      }
      else {
        $schedule['test_id'] = $this->test_id;
        db_insert('a_test_schedules')
          ->fields($schedule)
          ->execute();
      }
    }
    db_delete('a_test_schedules')
      ->condition('test_id', $this->test_id)
      ->condition('seq', $seqs , 'NOT IN')
      ->execute();
  }


  function saveValidations($measure_id, $normalization, $validations) {

    if (!$this->schedules) {
      $this->loadSchedules();
    }
    if (!$this->validations) {
      $this->loadValidations();
    }
    $m_val = array();
    $min_grade = $this->test['min_grade'];
    $max_grade = $this->test['max_grade'];
    foreach($validations as $val) {
      // Build validations from array.
      if ($normalization == 'custom') {
        $m_val[$val['grade_level']][$val['seq']] = $val;
      }
      else if ($normalization != 'grade') {
        // We need to set each sequence for each grade level to be the same.
        for ($g=$min_grade; $g <= $max_grade; $g++) {
          $val['grade_level'] = $g;
          foreach($this->schedules as $seq => $sched) {
            $val['seq'] = $seq;
            $m_val[$g][$seq] = $val;
          }
        }
      }
      else {
        // We need to set each sequence
        foreach($this->schedules as $seq => $sched) {
          $val['seq'] = $seq;
          $m_val[$val['grade_level']][$seq] = $val;
        }
      }
    }

    // Wipe existing validations and save.
    if ($m_val) {
      db_delete('a_test_rules')
        ->condition('measure_id', $measure_id)
        ->execute();
      foreach($m_val as $grade_level => $grade_vals) {
        foreach($grade_vals as $seq => $val) {
          db_insert('a_test_rules')
            ->fields(array(
              'measure_id' => $measure_id,
              'grade_level' => $grade_level,
              'seq' => $seq,
              'level_1' => $this->val($val['level_1']),
              'level_2' => $this->val($val['level_2']),
              'level_3' => $this->val($val['level_3']),
              'level_4' => $this->val($val['level_4']),
              'max_score' => $this->val($val['max_score']),
            ))
            ->execute();
        }
      }
    }

  }

  /**
   * Insert a new measure based on exising data.
   * @param $data the assocaitive array containing new measure to create.
   * Fields include:
   *   name - The name of hte measure
   *   abbrev - The short name of the field
   *   parent_measure - The id of the parent measure to set this to.
   * The array is altered to cotain the new measure_id of
   */
  function addMeasure(&$measure) {
    // Determine default sort
    $measure['test_id'] = $this->test_id;
    $measure_id = db_insert('a_test_measures')
      ->fields($measure)
      ->execute();
    $measure['measure_id'] = $measure_id;
  }

  /**
   * Convert numeric value to NULL or numeric
   * @param string $val
   * @return number
   */
  public function val($val) {
    return $val === '' ? NULL : floatval($val);
  }

  /**
   * Return the subject areas defined in the system.
   * @return areas:
   */
  public function subjectAreas() {
    $subjects = array();
    $rs = db_query('SELECT subject FROM i_subjects ');
    if ($rs) {
      $vals = $rs->fetchCol();
      $subjects = array_combine($vals, $vals);
    }
    return $subjects;
  }
}