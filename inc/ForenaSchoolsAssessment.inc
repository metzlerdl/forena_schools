<?php
/**
 * Manipulate assessment scores for forena schools
 * @author davidmetzler
 *
 */
class ForenaSchoolsAssessment {
  public $test;
  public $assessment;

  public function __construct() {
    $this->test = new ForenaSchoolsTest();
  }

  public function normalize($score, $validation) {

  }

  // Save an assessment with its test scores.
  function saveAssessment($data) {
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