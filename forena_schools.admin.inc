<?php
require_once 'inc/ForenaSchools.inc';
require_once 'inc/ForenaSchoolsQuery.inc';
require_once 'inc/common.inc';
function forena_schools_school_year_form($formid, &$form_state, $school_year='') {
  $y = ForenaSchoolsYear::instance();
  if ($school_year) {
    $data = $y->load($school_year);
    if (!$data) {
      drupal_not_found();
    }
  }
  else {
    $data = array();
  }

  if ($school_year) {
    $form['school_year'] = array(
      '#type' => 'value',
      '#value' => $school_year,
    );
    $form['school_year_value'] = array(
      '#title' => 'School Year',
      '#markup' => $school_year,
    );
  }
  else {
    $form['school_year'] = array(
      '#type' => 'textfield',
      '#title' => 'School Year',
      '#description'  => 'A four digit number representing the year that the school year ends',
    );
  }

  $form['label'] = array(
    '#type' => 'textfield',
    '#maxlength' => 60,
    '#required' => TRUE,
    '#title' => 'Label',
    '#default_value' => @$data['label'],
    '#description' => 'Enter year as it is displayed to the user (.e.g. 2012-13)',
  );

  $form['start_date'] = array(
    '#type' => 'date_popup',
    '#date_format' => 'Y-m-d',
    '#title' => 'Start date',
    '#default_value' => @$data['start_date'],
    '#description' => 'Starting poing of school year. ',
  );

  $form['end_date'] = array(
    '#type' => 'date_popup',
    '#date_format' => 'Y-m-d',
    '#title' => 'End Date',
    '#default_value' => @$data['end_date'],
    '#description' => 'Ending date of the school year.  Leave time for tests to be entered. '
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#validate' => array('forena_schools_year_validate'),
    '#submit' => array('forena_schools_year_save'),
  );

  $form['cancel'] = array(
    '#value' => 'Cancel',
    '#type' => 'submit',
    '#submit' => array('forena_schools_form_cancel'),
    '#limit_validation_errors' => array(),
  );

  return $form;
}

function forena_schools_year_validate($form, &$form_state) {
 $y = ForenaSchoolsYear::instance();
 if (!$y->validDate($form_state['values']['start_date'])) {
   form_set_error('start_date', 'Invalid Start Date');
 }
 if (!$y->validDate($form_state['values']['end_date'])) {
   form_set_error('end_date', 'Invalid End Date');
 }

}

function forena_schools_year_save($form, &$form_state) {
  ForenaSchoolsYear::instance()->save($form_state['values']);
  drupal_set_message('School Year '. $form_state['values']['school_year'] . ' saved.');
}

/**
 * Delete conformation form.
 */
function forena_schools_year_delete_form($form, &$form_state, $year = '') {
  $y = ForenaSchoolsYear::instance();
  $year = $y->load($year);
  if (!$year) {
    drupal_not_found();
    exit;
  }

  $form['start_date'] = array(
    '#type' => 'item',
    '#title' => 'Start Date',
    '#markup' => $year['start_date'],
  );

  $form['end_date'] = array(
    '#type' => 'item',
    '#title' => 'End Date',
    '#markup' => $year['end_date'],
  );

  $stats = $y->stats();
  foreach ($stats as $key => $value) {
    $form[strtolower($key)] = array(
      '#type' => 'item',
      '#title' => $key,
      '#markup' => $value,
    );
  }

  $form['confirm'] = array(
    '#title' => 'Confirm Deletion of Data.',
    '#type' => 'checkbox',
    '#description' => 'All Data above will be deleted.  This action cannot be ondone.  Only delete if you are really sure you want to eliminate this data.
    Be sure and double check school year dates prior to checking this box.',
    '#required' => TRUE,
  );

  $form['delete'] = array(
    '#value' => 'Delete',
    '#type' => 'submit',
    '#submit' => array('forena_schools_year_delete'),
  );

  $form['cancel'] = array(
    '#value' => 'Cancel',
    '#type' => 'submit',
    '#submit' => array('forena_schools_form_cancel'),
    '#limit_validation_errors' => array(),
  );


  return $form;
}

function forena_schools_year_delete($form, &$form_state) {
  $y = ForenaSchoolsYear::instance();
  $year = $y->year['school_year'];
  $y->delete();
  $sy = $y->load($year);
  if (!$y) {
    drupal_set_message($year . ' Deleted');
  }
  else {
    drupal_set_message('Could not delete ' . $year, 'error');
  }

}

/**
 * School definition form
 * Enter description here ...
 * @param unknown_type $formid
 * @param unknown_type $form_state
 * @param unknown_type $bldg_id
 */
function forena_schools_building_form($formid, &$form_state, $bldg_id='') {

	$q = ForenaSchoolsQuery::instance();
	if ($bldg_id) {
	  $data = $q->retrieve('i_buildings', array('bldg_id' => $bldg_id));
	}
	else {
		$data = array('P');
	}
  $form['building']['#tree'] = TRUE;
	$form['building']['bldg_id'] = array(
	  '#type' => 'value',
	  '#value' => @$data['bldg_id'],
	);

	$form['building']['name'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Building Name'),
	  '#default_value' => @$data['name'],
	  '#required' => TRUE,
	);

	$form['building']['abbrev'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Abbreviation'),
	  '#default_value' => @$data['abbrev'],
	  '#description' => t('Short name for use in tabular displays and graphs'),
	  '#required' => TRUE,
	);

	$form['building']['code'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Code'),
	  '#description' => t('Used for import files to match building'),
	  '#default_value' => @$data['code'],
	);

	$form['building']['sis_code'] = array(
	  '#type' => 'textfield',
	  '#title' => t('SIS Code'),
	  '#description' => t('Typical numeric identifier used in the student information system'),
	  '#default_value' => @$data['sis_code'],
	);

	$grade_levels = db_query('select grade_level,abbrev from {i_grade_levels} ORDER by grade_level')
	  ->fetchAllKeyed();

	$form['building']['min_grade'] = array(
	  '#type' => 'select',
	  '#title' => t('Min Grade Level'),
	  '#default_value' => @$data['min_grade'],
	  '#options' => $grade_levels,
	  '#required' => TRUE,
	);

	$form['building']['max_grade'] = array(
	  '#type' => 'select',
	  '#title' => t('Maximum Grade Level'),
	  '#default_value' => @$data['max_grade'],
	  '#options' => $grade_levels,
	  '#required' => TRUE,
	);

	$form['building']['building_dashboard'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Building Dashboard'),
	  '#default_value' => @$data['building_dashboard'],
	  '#autocomplete_path' => 'forena/reports/autocomplete/link',
	);

	$form['building']['teacher_dashboard'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Teacher Dashboard'),
	  '#default_value' => @$data['teacher_dashboard'],
	  '#autocomplete_path' => 'forena/reports/autocomplete/link',
	);

	$form['submit'] = array(
	  '#type' => 'submit',
	  '#value' => t('Save'),
	);
	return $form;
}

function forena_schools_building_form_validate($form, &$form_state) {
	$data = $form_state['values']['building'];
	if ((int)$data['min_grade']>(int)$data['max_grade']) {
		form_set_error('building][max_grade', t('maximum grade must be greater than or equal to minimum grade'));
	}
}

function forena_schools_building_form_submit($form, &$form_state) {
	$q = ForenaSchoolsQuery::instance();
	$q->update('i_buildings', 'bldg_id', $form_state['values']['building']);
	$form_state['redirect']='reports/building.Schools';
}

/**
 * SIS Imports form
 */

function forena_schools_import_form($form_id, &$form_state) {
	$jobs = variable_get('forena_schools_import_jobs', array());
	$form['jobs'] = array('#tree' => TRUE,
	   '#prefix' => '<ul class="action-links"><li>'
	      . l('Add', 'admin/config/system/forena-schools/import/' . count($jobs)) . '</li></ul><table><tbody>',
	   '#suffix' => '</tbody></table>');
  $class='even';
	if ($jobs) foreach ($jobs as $num=>$job) {
		$class = $class=='even' ? 'odd' : 'even';
		$form['jobs'][] = array(
		  'run' => array(
		    '#type' => 'checkbox',
		    '#prefix' => "<tr class='$class'><td>",
		    '#suffix' => '</td>',


		  ),
		  'title' => array(
		    '#type' => 'markup',
		    '#markup' => '<td>' . check_plain($job['title']) . '</td>',
		   ),
		  'actions' => array(
		     '#type' => 'markup',
		     '#markup' => '<td>' . l('Edit', 'admin/config/system/forena-schools/import/'. $num) .
		        ' '. l('Delete', 'admin/config/system/forena-schools/import-delete/'. $num) . '</td></tr>',
		   ),

		);

	}
	$form['submit'] = array(
	  '#type' => 'submit',
	  '#value' => 'Run',
	);
	return $form;
}

function forena_schools_import_form_submit($form, &$form_state) {
	$jobs = variable_get('forena_schools_import_jobs');
	$values= $form_state['values'];
	$ops = array();
	foreach ($values['jobs'] as $i => $job) {
		if ($job['run']) {
			$ops[] = array('forena_schools_run_import_batch', array($jobs[$i]['title'], $jobs[$i]['script']));
		}
	}
	if ($ops) {
		$batch = array(
		  'title' => t('Running Forena Schools Jobs'),
		  'operations' => $ops,
		);
		batch_set($batch);
	}

}



/**
 * Edit Submission form
 * @param unknown_type $form_id
 * @param unknown_type $form_state
 * @param integer $job_num number of job to be edited.
 */
function forena_schools_import_edit_form($form_id, &$form_state, $job_num) {
	$jobs = variable_get('forena_schools_import_jobs', array());
	if ($job_num >= count($jobs)) {
		$job = count($jobs);
		$def = array('title' => 'New Job', 'script' => '');
	}
	else {
	  $job = @$jobs[$job_num];
	}

	$form['job_num'] = array(
	  '#type' => 'value',
	  '#value' => $job_num,
	);

	$form['title'] = array(
	  '#type' => 'textfield',
	  '#description' => t('Administrative title of the import script'),
	  '#title' => t('Title'),
	  '#default_value' => $job['title'],
	  '#required' => TRUE,
	);

	if (module_exists('elysia_cron')) {
		$form['cron_rule'] = array(
		  '#type' => 'textfield',
		  '#title' => t('Cron Rule'),
		  '#description' => t('Crontab rule for how often this job should be run'),
		  '#default_value' => @$job['cron_rule'],

		);
	}

	$form['script'] = array(
	  '#type' => 'textarea',
	  '#title' => 'Import Script',
	  '#required' => TRUE,
	  '#default_value' => $job['script'],
	  '#rows' => 30,
	  '#cols' => 80,
	);
	$form['save'] = array(
	  '#type' => 'submit',
	  '#value' => 'Save',
	);
	return $form;
}


function forena_schools_import_edit_form_submit($form, &$form_state) {
	$jobs = variable_get('forena_schools_import_jobs', array());
  $values = $form_state['values'];
	$jobs[$values['job_num']] = array('title' => $values['title'], 'script' => $values['script'], 'cron_rule' => $values['cron_rule']);
	variable_set('forena_schools_import_jobs', $jobs);
	$form_state['redirect'] = 'admin/config/system/forena-schools/imports';
}

function forena_schools_import_delete_form($form_id, &$form_state, $job_num) {
	$jobs = variable_get('forena_schools_import_jobs', array());
	if ($job_num >= count($jobs)) {
     drupal_not_found();
	}
	else {
		$job = $jobs[$job_num];
	}

	$form['job_num'] = array(
	  '#type' => 'value',
	  '#value' => $job_num,
	);

	$form['title'] = array(
	  '#type' => 'item',
	  '#description' => t('Administrative title of the import script'),
	  '#title' => t('Title'),
	  '#value' => $job['title'],
	  '#required' => TRUE,
	);

	$form = confirm_form($form, 'Are you sure you want to delete this job', 'admin/config/system/forena-schools/imports');

	return $form;
}

function forena_schools_import_delete_form_submit($form, &$form_state) {
	$jobs = variable_get('forena_schools_import_jobs', array());
  $values = $form_state['values'];
	unset($jobs[$values['job_num']]);
	variable_set('forena_schools_import_jobs', $jobs);


	$form_state['redirect'] = 'admin/config/system/forena-schools/imports';
}

function forena_schools_impersonate_form($formid, &$form_state) {
	$form['login'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Login'),
	  '#description' => t('Leave blank to undo impersonation, otherwise specify login to see forena schools as it exists for another user'),
	);
	$form['submit'] = array(
	  '#type' => 'submit',
	  '#value' => 'Impersonate',
	);
	return $form;
}

function forena_schools_impersonate_form_submit($formid, &$form_state) {
	$login = $form_state['values']['login'];
	if ($login) {
		$_SESSION['forena_schools_impersonate'] = $login;
	}
	else {
		unset($_SESSION['forena_schools_impersonate']);
	}
	$form_state['redirect'] = 'assessments';
}

/**
 * Test Definition General form,
 * @param unknown $form
 * @param unknown $form_state
 * @param unknown $test_id
 */
function forena_schools_test_edit_general_form($form, &$form_state, $test_id='') {
  $t = ForenaSchoolsTest::instance();
  if ($test_id) {
    $test = $t->load($test_id);
  }
  else {
    $test = $t->create();
  }

  $form['test_id'] = array(
    '#type' => 'value',
    '#value' => $test_id,
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Display nam of test'),
    '#max_length' => 75,
    '#required' => TRUE,
    '#default_value' => $test['name'],
  );

  $form['abbrev'] = array(
    '#type' => 'textfield',
    '#title' => t('Abbreviation'),
    '#description' => t('Short name to use in places where there is not much screen real estate for tests'),
    '#max_length' => 25,
    '#size' => 10,
    '#required' => TRUE,
    '#default_value' => $test['abbrev'],
  );

  $form['code'] = array(
    '#type' => 'textfield',
    '#title' => t('Interface Code'),
    '#description' => t('A machine interpretable name that is used when importing teset values from internal systems.'),
    '#default_value' => $test['code']
  );

  $form['inactive'] = array(
    '#type' => 'checkbox',
    '#title' => t('Inacive'),
    '#description' => t('Inacativate the test implies that the test is no longer used, but that you do not yet want to remove historical data for this test.'),
  	'#default_value' => $test['inactive'],
  );

  $grade_levels = ForenaSchools::instance()->userGradeLevels();
  $form['min_grade'] = array(
    '#type' => 'select',
    '#title' => t('Minimum grade'),
    '#options' => $grade_levels,
    '#description' => t('Minimum grade level that this assessment can be measured.'),
    '#default_value' => $test['min_grade'],
   );

  $form['max_grade'] = array(
  	'#type' => 'select',
    '#title' => t('Maximum grade'),
    '#description' => t('Maximimum grade level that this assessment can be measured'),
    '#options' => $grade_levels,
    '#default_value' => $test['max_grade'],
  );

  $form['weight'] = array(
   '#type' => 'weight',
   '#title' => t('Weight'),
   '#default_value' => $test['weight'],

  );

  $form['actions']['save'] = array(
  	'#type' => 'submit',
    '#value' => t('Save'),
    '#validate' => array('forena_schools_validate_test'),
    '#submit' => array('forena_schools_save_test_submit'),
  );

  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('forena_schools_test_edit_cancel'),
  );

  return $form;
}

function forena_schools_validate_test($form, $form_state) {
  if($form_state['values']['min_grade'] > $form_state['values']['max_grade']) {
    form_set_error('max_grade', t('Maximum grade level cannot be less than minimum grade level'));
  }
}

function forena_schools_save_test_submit($form, &$form_state) {
  $t = ForenaSchoolsTest::instance();
  $test_id = $form_state['values']['test_id'];
  $t->saveGeneral($form_state['values']);
  drupal_set_message(t('Changes saved.'));
  if (!$test_id) {
    $test_id = $form_state['values']['test_id'];
    $form_state['redirect'] = array("test/$test_id/edit");
  }

}

function forena_schools_test_edit_cancel($form, &$form_state) {
  $form_state['redirect'] = 'forenaschools/tests';
}

function forena_schools_measure_config_icon($test_id, $measure_id) {
  $image = array(
      'path' => file_create_url("misc/configure.png"),
      'alt' => t('Configure'),
      'title' => t('Configure'),
  );
  $image = theme('image', $image);
  $options = array('html' => TRUE);
  return l($image . $label, "test/$test_id/edit/rules/$measure_id", $options);
}

function forena_schools_test_edit_measures_form($form, &$form_state, $test_id) {
  if (!$test_id) {
    drupal_not_found();
    exit;
  }
  // build configu


  $t = ForenaSchoolsTest::instance();
  $t->load($test_id);
  $measures = $t->loadMeasures();
  $delta = count($measures);

  $form['test_id'] = array(
  	'#type' => 'value',
    '#value' => $test_id,
  );


  $form['measures'] = array(
   '#tree' => TRUE,
   '#theme' => 'forena_schools_draggable_table',
   '#tabledrag_table_id' => 'table-id',
   '#tabledrag_weight' => 'weight',
   '#tabledrag_id' => 'measure_id',
   '#tabledrag_parent' => 'parent_measure',
   '#tabledrag_hide' => TRUE,
   '#tabledrag_max_depth' => 1,
  );
  $root_measures = array();

  $w = 0;
  if ($measures) foreach ($measures as $measure_id => $measure) {
    $w++;
    $ctl = array();

    // If it is a root measure add it to the possible measures to add to.
    if ($measure['parent_measure'] == $measure['measure_id']) {
      $root_measures[$measure['measure_id']] = $measure['name'];
    }

    $ctl['measure_id'] = array(
    	'#type' => 'hidden',
      '#default_value' => $measure['measure_id'],
      '#disabled' => TRUE,
      '#size' => 4,
    );

    $ctl['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#maxlength' => 75,
      '#default_value' => $measure['name'],
    );

    $ctl['actions'] = array(
    	'#type' => 'item',
      '#title' => t('Config'),
      '#markup' => forena_schools_measure_config_icon($test_id, $measure_id),
    );

    $ctl['abbrev'] = array(
      '#type' => 'textfield',
      '#title' => t('Abbreviation'),
      '#maxlength' => 25,
      '#size' => 7,
      '#default_value' => $measure['abbrev'],
    );

    $ctl['code'] = array(
    	'#type' => 'textfield',
      '#title' => t('Code'),
      '#maxlength' => 128,
      '#size' => 20,
      '#default_value' => $measure['code'],
    );


    $ctl['inactive'] = array(
    	'#type' => 'checkbox',
      '#title' => t('Inactive'),
      '#default_value' => $measure['inactive'],
    );

    $ctl['parent_measure'] = array(
    	'#type' => 'hidden',
     // '#title' => t('Parent'),
      '#size' => 4,
      '#default_value' => ($measure['parent_measure'] == $measure['measure_id']) ? '0' : $measure['parent_measure'],
    );

    $ctl['weight'] = array(
    	'#type' => 'weight',
      '#title' => t('Sort Order'),
      '#delta' => $delta,
      '#default_value' => $w,
    );


    $form['measures'][$measure_id] = $ctl;
  }

  $form['new_measure'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Add Measure'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );

  $form['new_measure']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Name of measure to add'),
  );

  $form['new_measure']['abbrev'] = array(
  	'#type' => 'textfield',
    '#title' => t('Abbreviation'),
    '#description' => t('Short name for use when screen real estate is at a premium'),
  );

  $form['new_measure']['parent_measure'] = array(
  	'#type' => 'select',
    '#title' => t('Parent measure'),
    '#options' => $root_measures,
    '#empty_option' => '-None-',
    '#default_value' => '',
    '#disabled' => !count($root_measures),
  );

  $form['actions']['save'] = array(
  	'#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('forena_schools_save_measures_submit'),
  );

  $form['actions']['cancel'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
      '#submit' => array('forena_schools_test_edit_cancel'),
  );

  $form['new_measure']['actions'] = array(
    '#type' => 'submit',
    '#value' => 'add',
    '#validate' => array('forena_schools_add_measure_validate'),
    '#submit' => array('forena_schools_add_measure_submit', 'forena_schools_save_measures_submit'),
  );

  return $form;
}

function forena_schools_add_measure_validate($form, &$form_state) {
  $new_measure = $form_state['values']['new_measure'];
  if (!$new_measure['name']) {
    form_set_error('new_measure][name', t('You must specifiy a name for the measure.'));
  }
  if (!$new_measure['abbrev']) {
    form_set_error('new_measure][abbrev', t('You must specifiy an abbreviation for the measure.'));
  }
}

/**
 * Submit function for
 * @param unknown $form
 * @param unknown $form_state
 */
function forena_schools_add_measure_submit($form, &$form_state) {
  $measure = $form_state['values']['new_measure'];
  unset($measure['actions']);
  ForenaSchoolsTest::instance()->addMeasure($measure);
  $measure['weight'] = count($form_state['values']['measures']) + 1;
  $form_state['values']['measures'][$measure['measure_id']] = $measure;
}

function forena_schools_save_measures_submit($form, &$form_state) {
  ForenaSchoolsTest::instance()->saveMeasures($form_state['values']['measures']);
  drupal_set_message('Test Measures Saved');
}

/**
 * Schedule editing form for test definitions.
 * @return multitype:string
 */
function forena_schools_test_edit_schedules_form($form, &$form_state, $test_id) {
  if (!$test_id) {
    drupal_not_found();
    exit;
  }
  if (!$form_state['storage']) {
    $t = ForenaSchoolsTest::instance();
    $t->load($test_id);
    $schedules = $t->loadSchedules();
    $form_state['storage']['test_id'] = $test_id;
    $form_state['storage']['schedules'] = array_values($schedules);
  }
  else {
    extract($form_state['storage']);
  }
  $form['test_id'] = array(
    '#type' => 'value',
    '#value' => $test_id,
  );

  $form['schedules'] = array(
    '#tree' => TRUE,
    '#theme' => 'forena_schools_control_table',
  );

  foreach ($schedules as $schedule) {
    $ctl = array();
    $ctl['seq'] = array(
      '#type' => 'textfield',
      '#title' => t('Sequence'),
      '#required' => TRUE,
      '#size' => 3,
      '#max_length' => 3,
      '#default_value' => $schedule['seq'],
    );

    $ctl['label'] = array(
    	'#type' => 'textfield',
      '#title' => t('Label'),
      '#required' => TRUE,
      '#size' => 10,
      '#default_value' => $schedule['label'],
      '#max_length' => 25,
    );

    $ctl['start_date'] = array(
      '#type' => 'date_popup',
      '#title' => t('Start Date'),
      '#required' => TRUE,
      '#date_format' => 'Y-m-d',
      '#default_value' => $schedule['start_date'],
    );

    $ctl['end_date'] = array(
    	'#type' => 'date_popup',
      '#required' => TRUE,
      '#title' => t('End Date'),
      '#date_format' => 'Y-m-d',
      '#default_value' => $schedule['end_date'],
    );

    $ctl['remove'] = array(
    	'#type' => 'submit',
      '#title' => 'Action',
      '#value' => '-',
      '#limit_validation_errors' => array(),
      '#submit' => array('forena_schools_test_remove_schedule'),
      '#name' => "schedule-remove-${schedule['seq']}",
    );

    $form['schedules'][] = $ctl;
  }

  $form['add-schedule'] = array(
    '#type' => 'submit',
    '#value' => '+',
    '#limit_validation_errors' => array(),
    '#submit' => array('forena_schools_test_add_schedule'),
  );

  $form['instructions'] = array(
    '#markup' => '<p><strong>Start Date</strong> and <strong>End Date</strong> fields should be specified relative to the current
      school year.   Actual dates used will be based on the same dates relative to the beginning of the next school year each year after
      </p>',
  );

  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#validate' => array('forena_schools_save_schedules_validate'),
    '#submit' => array('forena_schools_save_schedules_submit'),
  );

  $form['actions']['cancel'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
      '#submit' => array('forena_schools_test_edit_cancel'),
  );

  return $form;
}

function forena_schools_test_remove_schedule($form, &$form_state) {
  $k = $form_state['triggering_element']['#parents'][1];
  unset($form_state['values']['schedules'][$k]);
  unset($form_state['input']['schedules'][$k]);
  unset($form_state['storage']['schedules'][$k]);
  $form_state['rebuild'] = TRUE;
}

function forena_schools_test_add_schedule($form, &$form_state) {
  $form_state['storage']['schedules'][] = array_fill_keys(array('seq', 'label', 'start_date', 'end_date'), '');
  $form_state['rebuild'] = TRUE;
}

function forena_schools_save_schedules_validate($form, &$form_state) {
  $schedules = $form_state['values']['schedules'];
  $year = ForenaSchoolsYear::instance()->year;

  foreach ($schedules as $k => $schedule) {
    if ($schedule['start_date'] < $year['start_date'] || $schedule['start_date'] > $year['end_date']) {
      form_set_error("schedulus][$k][start_date", 'Start date must be within the school year');
    }
    if ($schedule['end_date'] < $year['start_date'] || $schedule['end_date'] > $year['end_date']) {
      form_set_error("schedulus][$k][end_date", 'End date must be within the school year');
    }
    foreach ($schedules as $e => $sch) {
      if ($k!=$e) {
        if ($schedule['start_date'] >= $sch['start_date'] && $schedule['start_date'] <= $sch['end_date']) {
          form_set_error("schedules][$k][start_date", 'Schedules cannot overlap.');
        }
        if ($schedule['end_date'] >= $sch['start_date'] && $schedule['end_date'] <= $sch['end_date']) {
          form_set_error("schedules][$k][start_date", 'Schedules cannot overlap.');
        }
      }
    }
  }
}

/**
 * Saving schedules submit handler.
 */
function forena_schools_save_schedules_submit($form, &$form_state) {
  extract($form_state['storage']);
  $t = ForenaSchoolsTest::instance();
  $t->load($test_id);
  $t->loadSchedules();

  ForenaSchoolsTest::instance()->saveSchedules($form_state['values']['schedules']);
  drupal_set_message('Test Schedules Saved');
}

/**
 * Rules form.
 * @param unknown $form
 * @param unknown $form_state
 */
function forena_schools_test_edit_rules_form($form, &$form_state, $test_id='', $measure_id = '') {
  $t = ForenaSchoolsTest::instance();

  // Toss validation input if we've changed normalization strategies.
  if ($form_state['rebuild']  && $form['measure']['normalization'] != $form_state['values']['measure']['normalization']) {
    unset($form_state['input']['validations']);
    unset($form_state['values']['validations']);
  }

  if ($form_state['storage']) {
    extract($form_state['storage']);
  }
  else {
    $form_state['storage']['measure_id'] = $measure_id;
    $form_state['storage']['test_id'] = $test_id;
  }
  // Load test.
  $t->load($test_id);
  $t->loadSchedules();
  $measures = $t->loadMeasures();
  $measure_list = $t->listMeasures();
  $subjects = $t->subjectAreas();
  if ($measure_id) {
    $measure = $measures[$measure_id];
  }


  if ($measure_id) {


    if (isset($form_state['values']['measure'])) {
      $measure = array_merge($measure, $form_state['values']['measure']);
    }

    $t->load($test_id);

    $calc_choices = array(
    	'avg' => t('Average'),
      'sum' => t('Sum'),
    );

    $form['measure'] = array(
      '#type' => 'fieldset',
      '#title' => $measure['name'],
      '#prefix' => "<div id='measure-info'>",
      '#suffix' => "</div>",
      '#tree' => TRUE,
    );

    $form['measure']['calc_rule'] = array(
    	'#type' => 'select',
      '#empty_option' => t('-none-'),
      '#options' => $calc_choices,
      '#title' => t('Caculation rule method'),
      '#default_value' => $measure['calc_rule'],
      '#ajax' => array(
          'callback' => 'forena_schools_rules_ajax_callback',
          'wrapper' => 'measure-info',
      ),
    );

    $form['measure']['subject'] = array(
    	'#type' => 'select',
      '#options' => $subjects,
      '#empty_option' => t('--none--'),
      '#title' => 'Subject Area',
      '#default_value' => $measure['subject'],
      '#description' => 'Indicates subject growth graph that this measure counts towards.'
    );

    $form['measure']['grad_requirement'] = array(
    	'#type' => 'select',
      '#options' => $subjects,
      '#empty_option' => t('--none--'),
      '#title' => t('Graduation Requirement'),
      '#description' => t('Indicates subject this is a grad requirement for'),
      '#default_value' => $measure['grad_requirement'],
    );

    $calc_list = $measure_list;
    unset($calc_list[$measure_id]);
    $form['measure']['calc_measures'] = array(
      '#type' => 'checkboxes',
      '#options' => $calc_list,
      '#title' => t('Measured included in calculation'),
      '#access' => !empty($measure['calc_rule']),
      '#default_value' => $measure['calc_measures'],
    );

    // New validation type
    $norm_types = array(
      'normalized' => t('The same criteria are used for all grade levels and schedules. '),
      'grade' => t('The same criteria are used for all schedules within a particular grade level.'),
      'custom' => t('Different criteria are used per grade level and per schedule'),
    );

    $norm = @$measure['normalization'] ? $measure['normalization'] : 'custom';
    $form['measure']['normalization'] = array(
    	'#type' => 'radios',
      '#options' => $norm_types,
      '#title' => t('Scoring Strategy'),
      '#default_value' => $norm,
      '#ajax' => array(
    	  'callback' => 'forena_schools_validation_ajax_callback',
        'wrapper' => 'validation-info',
      ),
    );

    // Build validations
    $validations  = $t->loadValidations();
    $validations = $validations[$measure_id];
    $form['validations'] = array(
      '#prefix' => '<div id="validation-info">',
      '#suffix' => '</div>',
    	'#tree' => TRUE,
      '#theme' => 'forena_schools_control_table',
    );

    $schedules = $t->loadSchedules();
    $grades = ForenaSchools::instance()->userGradeLevels();
    // Display validations based on normalization type
    $val_table = array();
    $i=0;

    foreach ($validations as $grade_level => $s)
      foreach ($s as $seq => $val) {
        $i++;
        switch($norm) {
        	case 'normalized':
        	  $key = $measure_id;
        	  break;
        	case 'grade':
        	  $key = $grade_level;
        	  break;
        	case 'custom':
        	  $key = $i;
        	  break;
        }
        if (isset($val_table[$key])) {
          foreach($val as $k => $v) {
            if ($v !== '') {
              $val_table[$key][$k] = $v;
            }
          }
        }
        else {
          $val_table[$key] = $val;

        }
    }

    foreach ($val_table as $val) {
      $ctl = array();
      $seq = $val['seq'];
      $grade_level = $val['grade_level'];
      if ($norm != 'normalized') {
        $ctl['grade_label'] = array(
          '#type' => 'item',
          '#title' => t('Grade'),
          '#markup' => $grades[$grade_level],
        );
        $ctl['grade_level'] = array(
        	'#type' => 'value',
          '#value' => $grade_level,
        );
      }

      if ($norm == 'custom') {
        $ctl['sched_label'] = array(
          '#type' => 'item',
          '#title' => t('Schedule'),
          '#markup' => $schedules[$seq]['label'],
        );
        $ctl['sched'] = array(
        	'#type' => 'value',
          '#value' => $seq,
        );
      }

      $ctl['level_1'] = array(
      	'#type' => 'textfield',
        '#title' => 'Struggling',
        '#default_value' => $val['level_1'],
        '#size' => 3,
        '#max_length' => 6,
      );

      $ctl['level_2'] = array(
      	'#type' => 'textfield',
        '#title' => 'Below Expectations',
        '#size' => 3,
        '#default_value' => $val['level_2'],
        '#max_length' => 6,
      );

      $ctl['level_3'] = array(
      	'#type' => 'textfield',
        '#title' => 'Meets Expectation',
        '#size' => 3,
        '#max_length' => 6,
        '#default_value' => $val['level_3'],
      );

      $ctl['level_4'] = array(
          '#type' => 'textfield',
          '#title' => 'Exceeds Expectation',
          '#size' => 3,
          '#max_length' => 6,
          '#default_value' => $val['level_4'],
      );
      $ctl['max_score'] = array(
          '#type' => 'textfield',
          '#title' => 'Maximum Score',
          '#size' => 3,
          '#max_length' => 6,
          '#default_value' => $val['max_score']
      );

      $form['validations'][] = $ctl;

    }

    $form['actions']['save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
  }

  return $form;
}

/**
 * Redirect to php.
 * @param unknown $form
 * @param unknown $form_state
 */
function forena_schools_test_rules_select_measure_submit($form, &$form_state) {
  extract($form_state['storage']);
  $measure_id = $form_state['values']['select']['measure_id'];
  $form_state['redirect'] = "test/$test_id/edit/rules/$measure_id";
}

function forena_schools_validation_ajax_callback($form, &$form_state) {
  return $form['validations'];
}

function forena_schools_rules_ajax_callback($form, &$form_state) {
  // @TODO: Persist form state values
  return $form['measure'];
}

function forena_schools_test_edit_rules_form_validate($form, &$form_state) {
  $measure = $form_state['values']['measure'];
  if ($measure['calc_rule']) {
    if (!array_filter($measure['calc_measures'])) {
      form_set_error('measure][calc_measures', 'You must select at least on measure for calculation.');
    }
  }
}

/**
 * Save changes to rules form.
 * @param unknown $form
 * @param unknown $form_id
 */
function forena_schools_test_edit_rules_form_submit($form, &$form_state) {
  extract($form_state['storage']);
  $t = ForenaSchoolsTest::instance();

  // Update the measure.
  $measure_data = $form_state['values']['measure'];
  $measure_data['measure_id'] = $measure_id;
  $t->load($test_id);
  $t->loadSchedules();
  $t->saveMeasure($measure_data);

  // Update Validations.
  $t->saveValidations($measure_id, $measure_data['normalization'], $form_state['values']['validations']);
  $form_state['redirect'] = "test/$test_id/edit/measures";

}
