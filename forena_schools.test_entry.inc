<?php
require_once 'inc/common.inc';

function forena_schools_single_entry_form($form, &$form_state, $person_id, $test_id, $school_year, $seq) {
  // Initialize multi-part form data
  if (!isset($form_state['storage'])) {
    $s = ForenaSchoolsStudent::instance();
    $student = $form_state['storage']['student'] = $s->loadByPersonId($person_id, $school_year);

    $a = ForenaSchoolsAssessment::instance();
    $a->setStudent($student);
    $assessment = $form_state['storage']['assessment'] = $a->load($test_id, $seq);
    $measures = $form_state['storage']['measures'] = $a->test->measures;
    $validations = $form_state['storage']['validations'] = $a->test->validations;
    $test = $form_state['storage']['test'] = $a->test->test;
    $schedule = $form_state['storage']['schedule'] = $a->test->schedules[$seq];
  } else {
    $student = $form_state['storage']['student'];
    $test = $form_state['storage']['test'];
    $assessment = $form_state['storage']['assessment'];
    $validations = $form_state['storage']['validations'];
    $measures = $form_state['storage']['measures'];
    $schedule = $form_state['storage']['schedule'];
  }

  $heading =  $test['name'] . ' - ' . $schedule['label'];
  drupal_set_title($student['last_name'] . ', ' . $student['first_name'] . ' - ' . $heading);


  $form['scores'] = array('#type' => 'fieldset', '#title' => 'Scores',  '#tree' => TRUE);

  foreach ($measures as $measure_id => $measure) {
    $form['scores'][$measure_id] = array(
      '#type' => 'textfield',
      '#title' => $measure['name'],
      '#default_value' => @$assessment['scores'][$measure_id],
      '#disabled' => $measure['calc_rule'],
    );
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}

function forena_schools_single_entry_form_validate($form, &$form_state) {
  $assessment = $form_state['storage']['assessment'];
  extract($assessment);
  $scores = $form_state['values']['scores'];
  $validations = @$form_state['storage']['validations'];
  $measures = $form_state['storage']['measures'];
  foreach ($scores as $measure_id => $score) {
    $measure_label = $measures[$measure_id]['name'];
    if ($score !== '' && !is_numeric($score)) {
      form_set_error("scores][$measure_id", $measure_label . ' must be a number');
    }
    else {
      $rule = $validations[$measure_id][$grade_level][$seq];
      if ($rule['max_score'] && (float)$score > (float)$rule['max_score'])  {
        form_set_error("scores][$measure_id", $measure_label . ' cannot be greater than ' . $rule['max_score']);
      }
    }
  }
}

function forena_schools_single_entry_form_submit ($form, &$form_state) {
  $a = ForenaSchoolsAssessment::instance();
  $a->setStudent($form_state['storage']['student']);
  $assessment = $form_state['storage']['assessment'];
  $assessment['scores'] = $form_state['values']['scores'];
  $a->saveAssessment($assessment);
  drupal_set_message('Saved Scores');
}

