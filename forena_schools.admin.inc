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
    '#type' => 'textfield',
    '#title' => 'Start date',
    '#default_value' => @$data['start_date'],
    '#description' => 'Starting poing of school year. ',
  );

  $form['end_date'] = array(
    '#type' => 'textfield',
    '#title' => 'End Date',
    '#default_value' => @$data['end_date'],
    '#description' => 'Ending date of the school year.  Leave time for tests to be entered. '
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('forena_schools_year_save'),
  );

  return $form;
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

	$grade_levels = db_query('select grade_level,abbrev from {i_grade_levels}')
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
