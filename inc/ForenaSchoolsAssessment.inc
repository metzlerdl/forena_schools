<?php
/**
 * Manipulate assessment scores for forena schools
 * @author davidmetzler
 *
 */
class ForenaSchoolsAssessment {
  public $test;
  public $assessment;
  public $scores;
  public $student;

  public function __construct() {
    $this->test = new ForenaSchoolsTest();
  }

  /**
   * Factory Method
   * @return ForenaSchoolsAssessment
   */
  static public function instance() {
    static $o = '';
    if (!$o) {
      $o = new ForenaSchoolsAssessment();
    }
    return $o;
  }


  //Normalize The test result.
  public function normalize($score, $validation) {
    $score = (float) $score;
    $max = (float)$validation['max_score'];
    $l1 = (float)$validation['level_1'];
    $l2 = (float)$validation['level_2'];
    $l3 = (float)$validation['level_3'];
    $l4 = (float)$validation['level_4'];
    if ($max && $score > $max) {
      $norm_score = 4.99;
    }
    elseif ($l4 && $score >= $l4 && $max > $l4) {
      $norm_score = 4.00 + (($score - $l4) / ($max - $l4));
    }
    elseif ($l3 && $score >= $l3 && $l4 > $l3) {
      $norm_score = 3.00 + (($score - $l3) / ($l4 - $l3));
    }
    elseif ($l3 && $score >= $l3)
      $norm_score = 3.00;
    elseif ($l2 && $score >= $l2 && $l3 > $l2) {
      $norm_score = 2.00 + (($score - $l2) / ($l3 - $l2));
    }
    elseif ($l1 && $score >= $l1 && $l2 > $l1) {
      $norm_score = 1.00 + (($score - $l1) / ($l2 - $l1));
    }
    else {
      $norm_score = 1.00;
    }
    return $norm_score;
  }


  // Get array contstructor and load student.
  public function setStudent($student) {
    $this->student = $student;
  }

  /**
   * Load the scores for the assessment id
   */
  private function loadScores() {
    $rs = db_query('SELECT s.* FROM {a_assessments} a
      JOIN {a_test_measures} m  ON m.test_id = a.test_id
      LEFT JOIN {a_scores} s ON a.assessment_id = s.assessment_id AND m.measure_id = s.measure_id
      WHERE s.assessment_id = :assessment_id ORDER BY m.sort_order',
      array(':assessment_id' => $this->assessment['assessment_id'])
    );
    if ($rs) {
      $this->scores = $rs->fetchAllKeyed('measure_id', 'score');
    } else {
      dpm($this->scores);
    }
  }

  // Load the scores.
  public function fillScores() {
    $this->scores = array();
    foreach ($this->test->measures as $key => $value) {
      $this->scores[$key] = '';
    }
    return $this->scores;
  }

  public function load($test_id, $seq) {
    $student = $this->student;
    $this->loadTest($test_id);
    $rs = db_query('SELECT * FROM {a_assessments} WHERE person_id=:person_id
      AND school_year = :school_year AND test_id = :test_id AND seq = :seq',
      array(
        ':person_id' => $student['person_id'],
        ':school_year' => $student['school_year'],
        ':test_id' => $test_id,
        ':seq' => $seq)
    );
    if ($rs) {
      $this->assessment = $rs->fetchAssoc();
      $this->loadScores();
    }
    else {
      $this->assessment = array(
        'person_id' => $student['person_id'],
        'bldg_id' => $student['bldg_id'],
        'school_year' => $student['school_year'],
        'grade_level' => $student['grade_level'],
        'test_id' => $test_id,
        'seq' => $seq,
      );
      $this->fillScores();
    }
  }


  // Load test information
  public function loadTest($test_id) {
    if ($this->test->test_id != $test_id) {
      $this->test->load($test_id);
      $t = $this->test;
      $this->measures = $this->test->loadMeasures();
      $this->schedules = $this->test->loadSchedules();
      $this->validations = $this->test->loadValidations();
    }
  }

  // Save an assessment with its test scores.
  public function saveAssessment($data) {
    $a_id = @$data['assessment_id'];
    $test_id = @$data['test_id'];


    //Make sure we have required info to store assessemnts
    // Cannot save a test without an error.

    if (!@$data['person_id'] || !@$data['grade_level'] || !@$data['bldg_id'] || !@$data['school_year'] || !@$data['test_id'] || !$data['seq']) {
      drupal_set_message('Missing Required data to save test.', 'error');
      watchdog('forena', 'Missing assessment data: <pre> %data </pre>', array('%data' => print_r($data,1)));
      return;
    }

    $grade_level = $data['grade_level'];
    $seq = $data['seq'];

    // Load assessments.
    $this->loadTest($test_id);
    $test = $this->test->load($test_id);
    $measures = $this->measures;
    $schedules = $this->schedules;
    $validations = $this->validations;
    $scores = $data['scores'];

    // Precaclulate calculated scores and put them in the data array.
    foreach ($measures as $measure) {
      if ($measure['calc_rule']) {
        dpm($measure['calc_measure']);
      }
    }

    // Validate all measures
    if ($data['scores']) foreach($data['scores'] as $key=>$score) {
      if (array_key_exists($key, $measures) && $value) {
        $norm_score ='';
        //The measure exists.
        $measure = $measures[$key];
        $validation = $this->validations[$grade_level][$seq];
        if ($validation) {
          $norm_score = $this->normalize($score, $validation);
        }

      }
    }

    // Actually Save the data.

  }


}