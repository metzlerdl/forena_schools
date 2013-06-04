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

  public function __construct() {
    $this->test = new ForenaSchoolsTest();
  }


  public function normalize($score, $validation) {

  }



  public function setStudent($student) {
    $this->student = $student;
  }

  /**
   * Load the scores for the assessment id
   */
  private function loadScores() {
    $rs = db_query('SELECT s.* FROM {a_assessments} a
      JOIN {a_test_measures} m  ON m.test_id = a.test_id
      LEFT JOIN {a_scores} s ON a.assesment_id = s.assessment_id AND m.measure_id = s.measure_id
      WHERE s.assessment_id = :assessment_id ORDER BY m.sort_order',
      array(':assessment_id' => $this->assessment['assessment_id'])
    );
    if ($rs) {
      $this->scores = $rs->fetchAllKeyed('measure_id', 'score');
    }
    dpm($this->scores);
  }

  public function fillScores() {
    $this->scores = array();
    foreach ($this->test->measures as $key => $value) {
      $this->scores[$key] = '';
    }
    return $this->scores;
  }

  public function load($test_id, $seq) {
    $student = $this->student;

    $rs = db_query('SELECT * FROM {a_assessments} WHERE person_id=:person_id
      school_year = :school_year AND test_id = :test_id AND seq = :seq',
      array(
        ':person_id' => $sudent->$person_id,
        ':school_year' => $student->school_year,
        ':test_id' => $test_id,
        ':seq' => $seq)
    );
    if ($rs) {
      $this->assessment = $rs->fetchAssoc();
      $this->loadScores();
    }
    else {
      $this->fillScores();
    }
  }

  // Save an assessment with its test scores.
  public function saveAssessment($data) {
    $a_id = @$data['assessment_id'];
    $test_id = @$data['test_id'];

    //Make sure we have required info to store assessemnts
    // Cannot save a test without an error.

    if (!@$data['person_id'] || !@$data['grade_level'] || !@$data['bldg_id'] || !@$data['school_year'] || !@$data['test_id'] || !$data['seq']) {
      drupal_set_message('Missing Required data to save test.  Please Contact Your adminiistrator', 'error');
      watchdog('forena', 'Missing assessment data: <pre> %data </pre>', array('%data' => print_r($data,1)));
      return;
    }

    // Load assessments.
    if ($this->test->test_id != $test_id) {
       $test = $this->test->load($test_id);
       $measures = $this->test->loadMeasures();
       $schedules = $this->test->loadSchedules();
       $validations = $this->test->loadValidations();
    }

    $scores = $data['sccores'];

    // Precaclulate calculated scores and put them in the data array.
    foreach ($measures as $measure) {
      if ($measure['calc_rule']) {
        dpm($measure['calc_measure']);
      }
    }

    // Validate all measures
    if ($data['scores']) foreach($data['scores'] as $key=>$value) {
      if (array_key_exists($key, $measures) && $value) {
        //The measure exists.
        $measure = $measures[$key];

      }
    }

    // Actually Save the data.

  }


}