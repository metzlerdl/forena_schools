<?php
/**
 * Create enrollment records for a student
 * establishing school year and grade level.
 * @param unknown_type $formid
 * @param unknown_type $form_state
 */
function forena_schools_student_enrollment_form($formid, &$form_state, $person_id='', $school_year='') {
  $grade_levels = array();
  if (!isset($form_state['storage'])) {
    $p = ForenaSchoolsPerson::instance();
    $buildings = $form_state['storage']['buildings'] = ForenaSchools::instance()->userBuildings();
    // looad the person
    $person = $form_state['storage']['person'] = $p->loadById($person_id);
    $student = $form_state['storage']['student'] = ForenaSchoolsStudent::instance()->loadByPersonId($person_id, $school_year);
    $grade_levels = $form_state['storage']['grade_levels'] = ForenaSchools::instance()->userGradeLevels();
    if (!$person) {
      drupal_not_found();
      exit;
    }
  }
  else {
    $person = $form_state['storage']['person'];
    $student = $form_state['storage']['student'];
    $grade_levels = $form_state['storage']['grade_levels'];
    $buildings = $form_state['storage']['buildings'];
  }

  if (isset($form_state['values']['grade_level'])) {
    $student['grade_level'] = $form_state['values']['grade_level'];
  }

  if ($student['grade_level']) {
    $buildings = ForenaSchoolsBuilding::instance()->buildingsByGradeLevel($student['grade_level']);
  }

  $form['school_year'] = array(
    '#type' => 'value',
    '#value' => $school_year,
  );

  $form['grade_level'] = array(
    '#type' => 'select',
    '#title' => 'Grade Level',
    '#options' => $grade_levels,
    '#required' => TRUE,
    '#default_value' => @$student['grade_level'],
    '#ajax' => array(
      'callback' => 'forena_schools_building_callback',
      'wrapper' => 'building',
      'method' => 'replace',
      'effect' => 'fade',
    ),
  );


  $form['bldg_id'] = array(
    '#prefix' => '<div id="building">',
    '#suffix' => '</div>',
    '#type' => 'select',
    '#title' => 'School',
    '#options' => $buildings,
    '#required' => TRUE,
    '#default_value' => @$student['bldg_id'],
    '#empty_label' => 'Select',
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => 'Save',
    '#submit' => array('forena_schools_student_enrollment_form_save'),
  );

  $form['cancel'] = array(
    '#type' => 'submit',
    '#limit_validation_errors' => array(),
    '#value' => 'Cancel',
    '#submit' => array('forena_schools_student_cancel'),
  );

  return $form;
}

function forena_schools_building_callback($form, &$form_state) {
  return $form['bldg_id'];
}

function forena_schools_student_cancel($form, &$form_state) {
  // Do nothing.
}

function forena_schools_student_enrollment_form_save($form, &$form_state) {
  extract($form_state['storage']['person']);
  extract($form_state['values']);
  $student = ForenaSchoolsStudent::instance()->loadByPersonId($person_id, $school_year);
  ForenaSchoolsStudent::instance()->save($bldg_id, $grade_level);
  drupal_set_message("Updated student enrollment for $first_name $last_name");
}