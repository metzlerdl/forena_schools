<?php
require_once 'inc/ForenaSchools.inc';
require_once 'inc/ForenaSchoolsQuery.inc';

/**
 * School definition form
 * Enter description here ...
 * @param unknown_type $formid
 * @param unknown_type $form_state
 * @param unknown_type $bldg_id
 */
function forena_schools_building_form($formid, &$form_state, $bldg_id='') {

	$q = ForenaSchoolsQuery::instance();
	$data = $q->retrieve('i_buildings', array('bldg_id' => $bldg_id));
  $form['building']['#tree'] = TRUE;
	$form['building']['bldg_id'] = array(
	  '#type' => 'value',
	  '#value' => $data['bldg_id'],
	);

	$form['building']['name'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Building Name'),
	  '#default_value' => $data['name'],
	);

	$form['building']['abbrev'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Abbreviation'),
	  '#default_value' => $data['abbrev'],
	  '#description' => t('Short name for use in tabular displays and graphs'),
	);

	$form['building']['code'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Code'),
	  '#description' => t('Used for import files to match building'),
	  '#default_value' => $data['code'],
	);

	$form['building']['sis_code'] = array(
	  '#type' => 'textfield',
	  '#title' => t('SIS Code'),
	  '#description' => t('Typical numeric identifier used in the student information system'),
	  '#default_value' => $data['sis_code'],
	);

	$grade_levels = db_query('select grade_level,abbrev from {i_grade_levels}')
	  ->fetchAllKeyed();

	$form['building']['min_grade'] = array(
	  '#type' => 'select',
	  '#title' => t('Min Grade Level'),
	  '#default_value' => $data['min_grade'],
	  '#options' => $grade_levels,
	  '#required' => TRUE,
	);

	$form['building']['max_grade'] = array(
	  '#type' => 'select',
	  '#title' => t('Maximum Grade Level'),
	  '#default_value' => $data['max_grade'],
	  '#options' => $grade_levels,
	  '#required' => TRUE,
	);

	$form['building']['building_dashboard'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Building Dashboard'),
	  '#default_value' => $data['building_dashboard'],
	  '#autocomplete_path' => 'forena/reports/autocomplete/link',
	);

	$form['building']['teacher_dashboard'] = array(
	  '#type' => 'textfield',
	  '#title' => t('Building Dashboard'),
	  '#default_value' => $data['teacher_dashboard'],
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
