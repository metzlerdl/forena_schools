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
  public $schedules;
  public $validations;

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
    $this->fillScores();
    if (@$this->assessment['assessment_id']) {
      $rs = db_query('SELECT measure_id,score FROM a_scores
        WHERE assessment_id = :assessment_id',
        array(':assessment_id' => $this->assessment['assessment_id'])
      );
      if ($rs) {
        $scores = $rs->fetchAllKeyed();
        foreach ($scores as $measure_id=>$score) {
          $this->scores[$measure_id] = $score;
        }
        //$this->scores = array_merge($this->scores, $scores);

      }
    }
    $this->assessment['scores'] = $this->scores;
    return $this->scores;
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
        ':seq' => $seq));
    if ($rs) $this->assessment = $rs->fetchAssoc();


    if (!$this->assessment) {
      $this->assessment = array(
        'person_id' => $student['person_id'],
        'bldg_id' => $student['bldg_id'],
        'school_year' => $student['school_year'],
        'grade_level' => $student['grade_level'],
        'test_id' => $test_id,
        'seq' => $seq,
      );
    }
    $this->assessment['scores'] = $this->loadScores();
    return ($this->assessment);
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
    $assessment = $this->load($test_id, $seq);
    $measures = $this->measures;
    $schedules = $this->schedules;
    $validations = $this->validations;
    $scores = $data['scores'];
    $schedule = $schedules[$seq];

    if (!$schedule) {
      drupal_set_message('Invalid Schedule', 'error');
      watchdog('forena', 'Invalid Schedule data: <pre> %data </pre>', array('%data' => print_r($data,1)));
      return;
    }


    // Precaclulate calculated scores and put them in the data array.
    if ($measures) foreach ($measures as $measure_id => $measure) {
      if ($measure['calc_rule'] && $measure['calc_measures']) {
        $score = 0.0;
        $count = 0;
        $calc_measures = ForenaSchools::instance()->fieldToArray($measure['calc_measures']);
        foreach ($calc_measures as $calc_id) {
          if ($scores[$calc_id]!=='') {
            $count++;
            $score += (float)$scores[$calc_id];
          }
        }
        if ($measure['calc_rule'] == 'avg' && $count) {
            $score = $score/$count;
        }
        $scores[$measure_id] = $score;
      }
    }

    $norm_scores = array();
    // Validate all measures
    if ($data['scores']) foreach($data['scores'] as $key=>$score) {
      if (array_key_exists($key, $measures) &&  $score !=='') {
        $norm_score = NULL;
        //The measure exists.
        $measure = $measures[$key];
        $validation = $this->validations[$key][$grade_level][$seq];
        if ($validation) {
          $norm_score = $this->normalize($score, $validation);
        }
        $norm_scores[$key] = $norm_score;
      }
    }



    // Actually Save the data.

    $date_taken = ForenaSchoolsYear::instance($data['school_year'])->date($schedule['target_day']);
    $user = ForenaSchools::instance()->current_login();
    $modified = ForenaSchools::instance()->time();
    if ($this->assessment['assessment_id']) {
      // If we have an asssessment id we're updating
      $assessment_id = $this->assessment['assessment_id'];
      db_update('a_assessments')
        ->fields(array(
          'modified' => $modified,
          'modified_by' => $user,
          'date_taken' => $date_taken
          ))
        ->condition('assessment_id', $assessment_id)
        ->execute();

    }
    else {
      // We're inserting

      $assessment_id = db_insert('a_assessments')
        ->fields(array(
          'person_id' => $data['person_id'],
          'bldg_id' => $data['bldg_id'],
          'school_year' => $data['school_year'],
          'grade_level' => $data['grade_level'],
          'test_id' => $data['test_id'],
          'seq' => $data['seq'],
          'modified' => $modified,
          'modified_by' => $user,
          'date_taken' => $date_taken,
        ))
        ->execute();
    }

    // Save Scores
    $existing_scores = $this->loadScores();
    $scores_to_delete = array();
    foreach ($scores as $measure_id => $score) {
      if (@$existing_scores[$measure_id] && ($score !== '') ) {
        db_update('a_scores')
          ->fields(array(
            'score' => $score,
            'norm_score' => $norm_scores[$measure_id],
            ))
          ->condition('assessment_id', $assessment_id)
          ->condition('measure_id', $measure_id)
          ->execute();
      }
      elseif ($score !== '') {
        db_insert('a_scores')
          ->fields(array(
            'assessment_id' => $assessment_id,
            'measure_id' => $measure_id,
            'score' => $score,
            'norm_score' => $norm_scores[$measure_id]
            ))
          ->execute();
      } else {
        $scores_to_delete[] = $measure_id;
      }

    }
    if ($scores_to_delete) {
      db_delete('a_scores')
        ->condition('assessment_id', $assessment_id)
        ->condition('measure_id', $scores_to_delete)
        ->execute();
    }
    $this->test->recalcStats($data['school_year']);
    $this->load($test_id, $seq);
    return $this->assessment;
  }


}