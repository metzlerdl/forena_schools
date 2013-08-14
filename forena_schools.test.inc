<?php
/**
 * @file
 * Test Definition forms.
 */

function forena_schools_test_delete_form($form, &$form_state, $test_id) {
  $t = ForenaSchoolsTest::instance();
  $test = $t->load($test_id);
  if (!$test) {
    drupal_not_found();
    exit;
  }

  $stats = $t->stats();
  $i=0;
  drupal_set_title('Delete ' . $test['name']);
  $form['heading'] = array('#type' => 'markup', '#markup' => '<h2>Current Test Statistics</h2>');

  foreach ($stats as $label => $count) {
    $i++;
    $form['stats_'. $i] = array(
      '#type' => 'item',
      '#title' => $label,
      '#markup' => $count,
    );
  }

  $form['confirm'] = array(
    '#type' => 'checkbox',
    '#required' => TRUE,
    '#title' => 'Confirm Delete',
    '#description' => 'This action cannot be undone.  Please review the number of assessments stored for each school year prior to checking this box. Assessments listed above will be deleted.'
  );

  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => 'Delete',
    '#submit' => array('forena_schools_test_delete'),
  );

  $form['cancel'] = array(
    '#value' => 'Cancel',
    '#type' => 'submit',
    '#submit' => array('forena_schools_form_cancel'),
    '#limit_validation_errors' => array(),
  );

  return $form;
}

function forena_schools_test_delete($form, &$form_state) {
  $t = ForenaSchoolsTest::instance();
  $test_id = $t->test_id;
  $name = $t->test['name'];
  $t->delete();
  $test = $t->load($test_id);
  if ($test) {
    drupal_set_message('Could not Delete '. $test_name, 'error');
  }
  else {
    drupal_set_message($test_name . ' Deleted.');
  }
}