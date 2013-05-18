<?php
require_once 'inc/common.inc';
function forena_schools_single_entry_form($form, &$form_state, $person_id) {
  // Initialize multi-part form data
  if (!isset($form_state['storage'])) {
    $person = $form_state['storage']['person'] = ForenaSchoolsPerson::instance()->loadById($person_id);
    $grades = $form_state['storage']['grades'] = ForenaSchoolsStudent::instance()->studentGradeLevels($person['person_id']);
    $form_state['storage']['test_id'] = '';
  } else {
    $person = $form_state['storage']['person'];
    $grades = $form_state['storage']['grades'];
  }

  $grade_values = array_keys($grades);
  $grade_level = isset($form_state['values']['grade_level']) ? $form_state['values']['grade_level'] : array_pop($grade_values);
  $to = ForenaSchoolsTest::instance();
  $tests = $to->gradeLevelTests($grade_level);
  $test_values = array_keys($tests);
  $schedules = array();
  $test_id = @$form_state['values']['test_id'];
  if ($test_id) $schedules = $to->listSchedules($test_id);

  drupal_set_title($person['last_name'] . ', ' . $person['first_name']);

  $form['select'] = array(
    '#prefix' => '<div id="test_select">',
    '#suffix' => '</div>',
    '#type' => 'fieldset', '#title' => 'Select Test'
  );

  $form['select']['grade_level'] = array(
    '#title' => t('Grade Level'),
    '#type' => 'select',
    '#options' => $grades,
    '#ajax' =>  array(
      'callback' => 'forena_schools_test_select_ajax',
      'event' => 'change',
      'wrapper' => 'test_select',
    ),
    '#default_value' => $grade_level,
  );

  $form['select']['test_id'] = array(
    '#type' => 'select',
    '#title' => t('Assessment'),
    '#options' => $tests,
    '#default_value' => $test_id,
    '#empty_value' => 'Select Test',
    '#required' => TRUE,
    '#ajax' => array(
      'callback' => 'forena_schools_test_select_ajax',
      'event' => 'change',
      'wrapper' => 'test_select',
    ),
  );

  $form['select']['seq'] = array(
    '#type' => 'select',
    '#title' => t('Schedule'),
    '#empty_value' => t('Select Schedule'),
    '#required' => TRUE,
    '#options' => $schedules,
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  return $form;
}

function forena_schools_test_select_ajax($form, &$form_state) {
  // Clear out sequence if test_id has changed.
  if ($form_state['storage']['test_id'] != $form_state['values']['test_id']) {
    $form_state['input']['seq'] = '';
  }
  $form_state['storage']['test_id'] = $form_state['values']['test_id'];
  return $form['select'];
}