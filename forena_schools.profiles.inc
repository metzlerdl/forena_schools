<?php
/**
 * @file
 * Profile form functions
 */
require_once 'inc/ForenaSchools.inc';
require_once 'inc/ForenaSchoolsProfile.inc';
require_once 'inc/ForenaSchoolsTest.inc';
/**
 * Impelmentation of forena profile form
 */
function forena_schools_profile_form($formid, &$form_state, $profile_id='') {
  $data = array();
  // Load form
  $p = ForenaSchoolsProfile::instance();
  // Load form data if necessary.
  if (@!$form_state['storage']['profile']) {
    $form_state['storage']['profile'] = $p->load($profile_id);
  }

  $data = $form_state['storage']['profile'];
  $form = array();
  $fs = ForenaSchools::instance();

  // Get the validations that we need.

  // Grade Levels
  $grade_levels = $fs->userGradeLevels();
  $offset_years = array(
    '0' => 'Current Year',
    '-1' => 'Prior Year',
    '-12' => 'Any Year',
  );

  // Buidings
  $buildings = $fs->userBuildings();
  $buildings['-1'] = 'District Wide';

  // Displays
  $displays = array('Performance', 'Group', 'Grade Level');
  $displays = array_combine($displays, $displays);

  // Tests
  $tests = $fs->userTests($data['min_grade'], $data['max_grade']);
  $form['profile_id'] = array(
    '#type' => 'value',
    '#value' => $data['profile_id'],
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => 'Name',
    '#description' => 'Indicate the name as it will appear to users who select this profile.' ,
    '#maxlength' => 75,
    '#size' => 75,
    '#default_value' => $data['name'],
  );

  $form['min_grade'] = array(
    '#type' => 'select',
    '#title' => 'Mininum Grade',
    '#options' => $grade_levels,
    '#required' => TRUE,
    '#default_value' => $data['min_grade'],
  );

  $form['max_grade'] = array(
    '#type' => 'select',
    '#title' => t('Maximum Grade'),
    '#required' => TRUE,
    '#options' => $grade_levels,
    '#default_value' => $data['max_grade'],
  );

  $form['school_year_offset'] = array(
    '#type' => 'select',
    '#title' => 'School Years',
    '#options' => $offset_years,
    '#description' => 'Controls which years to display data for',
    '#default_value' => $data['school_year_offset'],
  );

  $form['bldg_id'] = array(
    '#type' => 'select',
    '#title' => 'Building',
    '#options' => $buildings,
    '#required' => TRUE,
    '#default_value' => $data['bldg_id'],
  );


  $form['weight'] = array(
    '#type' => 'textfield',
    '#title' => 'Weight',
    '#description' => 'Use to override the order of display of profiles.  Note
      that it is usually best practice to leave these displayed in alpha
      order',
    '#default_value' => $data['weight'],
  );

  $form['displays'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Displays',
    '#options' => $displays,
    '#default_value' => $data['displays'],
    '#description' => t('Check any displays that this profile should show up in. ')
  );

  $form['save'] = array(
    '#type' => 'submit',
    "#submit" => array('forena_schools_profile_save'),
    '#value' => t('Save'),
  );
  $form['save_close'] = array(
    '#type' => 'submit',
    "#submit" => array('forena_schools_profile_save'),
    '#value' => t('Save and Close'),
  );

  return $form;
}

function forena_schools_profile_save($form, &$form_state) {

  $p = ForenaSchoolsProfile::instance();
  $profile_id = $form_state['values']['profile_id'];
  if ($form_state['values']['profile_id']) $p->load($form_state['values']['profile_id']);
  unset($form_state['values']['measures']['measures__active_tab']);
  $p->save($form_state['values']);
  if (strpos($_POST['op'], 'Close')!==FALSE) {

    $form_state['redirect'] = 'reports/district.Profiles';
  }
  else {
    if ($profile_id != $p->profile_id) {
      $form_state['redirect'] = 'profile/' . $p->profile_id . '/edit';
    }
  }
}

function forena_schools_profile_measures($formid, &$form_state, $profile_id = '') {
  $data = array();
  // Load form
  $p = ForenaSchoolsProfile::instance();
  // Load form data if necessary.
  if (@!$form_state['storage']['profile']) {
    $form_state['storage']['profile'] = $p->load($profile_id);
  }
  $data = $form_state['storage']['profile'];

  // Tests
  $tests = ForenaSchools::instance()->userTests($data['min_grade'], $data['max_grade']);

  $form = array();

  $form['profile_id'] = array(
      '#type' => 'value',
      '#value' => $data['profile_id'],
  );

  $form['measures'] = array(
      '#tree' => TRUE,
  );

  $i = 0;
  // Measures section of display.
  if ($data['measures']) foreach($data['measures'] as $key=>$measure) {
    $i++;
    $label = @$measure['label'] ? $measure['label'] . '*' : $measure['name'];
    $ctl = array(
        '#tree' => TRUE,
    );
    $ctl['measure_id'] = array(
        '#type' => 'value',
        '#value' => $measure['measure_id'],
    );

    $ctl['seq'] = array(
        '#type' => 'value',
        '#value' => $measure['seq'],
    );

    $ctl['name'] = array(
        '#type' => 'item',
        '#markup' => $measure['name'] . ' (' . $measure['seq'] . ')',
    );

    $ctl['label'] = array(
        '#type' => 'textfield',
        '#title' => 'Label',
        '#title_display' => 'invisible',
        '#maxlength' => 30,
        '#default_value' => $measure['label'],
    );

    $ctl['remove'] = array(
        '#type' => 'submit',
        '#value' => 'Remove',
        '#submit' => array('forena_schools_profile_remove_measure'),
        '#name' => $i,
    );

    $ctl['weight'] = array(
        '#type' => 'weight',
        '#title' => 'Weight',
        '#title_display' => 'invisible',
        '#delta' => 100,
        '#default_value' => $measure['sort_order'],
    );

    $form['measures'][] = $ctl;

  }

  // Add measures control
  $form['add_measures'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => !isset($form_state['values']['add_measures']['test_id']),
      '#prefix' => '<div id="add-measures-fieldset">',
      '#suffix' => '</div>',
      '#tree' => TRUE,
      '#title' => 'Add Measures',
  );

  $form['add_measures']['test_id'] = array(
      '#type' => 'select',
      '#title' => 'Test',
      '#options' => $tests,
      '#empty_value' => '',
      '#ajax' => array(
          'wrapper' => 'add-measures-fieldset',
          'callback' => 'forena_schools_add_measure_callback',
      )
  );

  $test_id = @$form_state['values']['add_measures']['test_id'];

  $measures = array();
  $schedules = array('0' => 'Any');

  $t = ForenaSchoolsTest::instance();
  if ($test_id) $measures = $t->listMeasures($test_id);
  if ($test_id) $schedules = array_merge($schedules,  $t->listSchedules($test_id));
  $form['add_measures']['seq'] = array(
      '#title' => 'Schedule',
      '#type' => 'select',
      '#options' => $schedules,
      '#defalut_value' => '',
  );

  $form['add_measures']['measures'] = array(
      '#title' => 'Measure',
      '#type' => 'select',
      '#multiple' => TRUE,
      '#options' => $measures,
      '#validated' => TRUE,
      '#disabled' => !$test_id,
  );


  $form['add_measures']['add'] = array(
      '#type' => 'submit',
      '#value' => 'Add',
      '#submit' => array('forena_schools_profile_add_measures'),
  );

  $form['add_measures']['add_all'] = array(
      '#type' => 'submit',
      '#value' => 'Add All',
      '#submit' => array('forena_schools_profile_add_all_measures'),
  );

  $form['save'] = array(
  	'#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('forena_schools_profile_save_measures'),
  );
  $form['save_close'] = array(
      '#type' => 'submit',
      '#value' => t('Save and Close'),
      '#submit' => array('forena_schools_profile_save_measures'),
  );

  return $form;
}

function forena_schools_profile_save_measures($form, &$form_state) {
  $p = ForenaSchoolsProfile::instance();
  $profile_id = $form_state['values']['profile_id'];
  if ($form_state['values']['profile_id']) $p->load($form_state['values']['profile_id']);
  //unset($form_state['values']['measures']['measures__active_tab']);
  //Caclulate sort orders from weights:
  if ($form_state['values']['measures']) {
    foreach ($form_state['values']['measures'] as $id => $measure) {
      $weight[$id] = $measure['weight'];
    }
    asort($weight);
    $measures = array();
    foreach ($weight as $id => $value) {
      $measures[] = $form_state['values']['measures'][$id];
    }
    $form_state['values']['measures'] = $measures;
  }

  $p->save($form_state['values']);
  if (strpos($_POST['op'], 'Close')!==FALSE) {

    $form_state['redirect'] = 'reports/district.Profiles';
  }
}

/**
 * Remove the measure from the list.
 * @param unknown_type $form
 * @param unknown_type $form_state
 */
function forena_schools_profile_remove_measure($form, &$form_state) {
  // Determine triggering parent.
  $data = $form_state['storage']['profile'];
  $parents = $form_state['triggering_element']['#parents'];
  array_shift($parents);
  $idx = array_shift($parents);
  unset($data['measures'][$idx]);
  unset($form_state['input']['measures'][$idx]);
  // Reset input to make sure that labels still stay alligned.
  $form_state['input']['measures'] = array_values($form_state['input']['measures']);
  $form_state['storage']['profile']['measures'] = array_values($data['measures']);
  $form_state['rebuild'] = TRUE;
}

// Add selected measures
function forena_schools_profile_add_measures($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $measures = $form_state['storage']['profile']['measures'];
  $test_id = $form_state['values']['add_measures']['test_id'];
  $valid_measures = ForenaSchoolsTest::instance()->listMeasures($test_id);
  $new_measures = $form_state['values']['add_measures']['measures'];
  // Schedule
  $seq = $form_state['values']['add_measures']['seq'];
  $valid_schedules = ForenaSchoolsTest::instance()->listSchedules($test_id);
  $seq = isset($valid_schedules[$seq]) ? $seq : '0';
  $label = isset($valid_schedules[$seq]) ? $valid_schedules[$seq] : 'Any';

  if ($new_measures) foreach ($new_measures as $measure_id) {
    if (isset($valid_measures[$measure_id])) {
      $measure = array(
        'measure_id' => $measure_id,
        'name' => $valid_measures[$measure_id],
        'seq' => $seq,
        'label' => '',
      );
      $measures[] = $measure;
    }
  }
  $form_state['storage']['profile']['measures'] = $measures;
}

function forena_schools_profile_add_all_measures($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $measures = $form_state['storage']['profile']['measures'];
  $test_id = $form_state['values']['add_measures']['test_id'];
  $valid_measures = ForenaSchoolsTest::instance()->listMeasures($test_id);
  $seq = $form_state['values']['add_measures']['seq'];
  $valid_schedules = ForenaSchoolsTest::instance()->listSchedules($test_id);
  $seq = isset($valid_schedules[$seq]) ? $seq : '';
  $label = isset($valid_schedules[$seq]) ? $valid_schedules[$seq] : 'Any';
  if ($valid_measures) foreach ($valid_measures as $measure_id => $name) {
    $measure = array(
      'measure_id' => $measure_id,
      'name' => $valid_measures[$measure_id],
      'seq' => $seq,
      'label' => '',
    );
    $measures[] = $measure;
  }
  $form_state['storage']['profile']['measures'] = $measures;
}

function forena_schools_add_measure_callback($form, &$form_state) {
  return $form['add_measures'];
}

function forena_schools_profile_delete($formid, &$form_state, $profile_id) {
  $profile = ForenaSchoolsProfile::instance()->load($profile_id);
  $form = array();
  if ($profile) {
    $form['profile_id'] = array(
      '#type' => 'value',
      '#value' => $profile_id,
    );

    $path = 'reports/district.Profiles';
    $form = confirm_form($form, 'Delete profile ' . $profile['name'],  $path,  'Deleting a profile is permanent.  Are you sure', 'Delete', 'Cancel');
  }
  return $form;
}

function forena_schools_profile_delete_submit($form, &$form_state) {
  $p = ForenaSchoolsProfile::instance();
  if ($p->load($form_state['values']['profile_id'])) {
    $p->delete();
    $form_state['redirect'] = 'reports/district.Profiles';
  }
}

/**
 * Theme callback for the derived from tabledrag_example_simple_form form.
 *
 *
 * @return
 *   The rendered tabledrag form
 *
 * @ingroup tabledrag_example
 */
function theme_forena_schools_profile_measures($variables) {
  $form = $variables['form'];

  // Initialize the variable which will store our table rows.
  $rows = array();

  // Iterate over each element in our $form['example_items'] array.
  foreach (element_children($form['measures']) as $id) {

    // Before we add our 'weight' column to the row, we need to give the
    // element a custom class so that it can be identified in the
    // drupal_add_tabledrag call.
    //
    // This could also have been done during the form declaration by adding
    // '#attributes' => array('class' => 'example-item-weight'),
    // directy to the 'weight' element in tabledrag_example_simple_form().
    $form['measures'][$id]['weight']['#attributes']['class'] = array('profile-measures-item-weight');

    // We are now ready to add each element of our $form data to the $rows
    // array, so that they end up as individual table cells when rendered
    // in the final table.  We run each element through the drupal_render()
    // function to generate the final html markup for that element.
    $rows[] = array(
        'data' => array(
            // Add our 'name' column.
            drupal_render($form['measures'][$id]['name']),
            // Add our 'description' column.
            drupal_render($form['measures'][$id]['label']),
            // Add our 'weight' column.
            drupal_render($form['measures'][$id]['remove']),

            drupal_render($form['measures'][$id]['weight']),
        ),
        // To support the tabledrag behaviour, we need to assign each row of the
        // table a class attribute of 'draggable'. This will add the 'draggable'
        // class to the <tr> element for that row when the final table is
        // rendered.
        'class' => array('draggable'),
    );
  }

  // We now define the table header values.  Ensure that the 'header' count
  // matches the final column count for your table.
  $header = array(t('Name'), t('Label'), t('Action'), t('Weight'));

  // We also need to pass the drupal_add_tabledrag() function an id which will
  // be used to identify the <table> element containing our tabledrag form.
  // Because an element's 'id' should be unique on a page, make sure the value
  // you select is NOT the same as the form ID used in your form declaration.
  $table_id = 'profile-measures-items-table';

  // We can render our tabledrag table for output.
  $output = theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array('id' => $table_id),
  ));

  // And then render any remaining form elements (such as our submit button).
  $output .= drupal_render_children($form);

  // We now call the drupal_add_tabledrag() function in order to add the
  // tabledrag.js goodness onto our page.
  //
  // For a basic sortable table, we need to pass it:
  // - the $table_id of our <table> element,
  // - the $action to be performed on our form items ('order'),
  // - a string describing where $action should be applied ('siblings'),
  // - and the class of the element containing our 'weight' element.
  drupal_add_tabledrag($table_id, 'order', 'sibling', 'profile-measures-item-weight');

  return $output;
}
