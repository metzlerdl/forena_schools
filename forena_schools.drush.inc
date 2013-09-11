<?php
/**
 * @file forena.drush.inc
 * Implementation of drush command hook.
 */
function forena_schools_drush_command() {
  $items = array();

  $items['forena-schools-job'] = array(
    'description' => 'Run a forena schools import job',
    'examples' => array('drush fsjob import'),
    'arguments' => array( array('name' => 'Job name to run')),
    'aliases' => array('fsjob')
  );

  return $items;
}

/**
 * Run the import job
 */
function drush_forena_schools_job($name = '') {
	require_once 'inc/ForenaSchoolsImport.inc';
	dsm($name);
	$jobs = variable_get('forena_schools_import_jobs', array());
	$j=-1;
	foreach ($jobs as $job_num=>$job) {
	  if ($name == $job['title']) $j = $job_num;
	}
	$job = @$jobs[$j];
	if ($job) {
	  drupal_set_message('running ' . $job['title']);
    $ops[] = array('forena_schools_run_import_batch', array($job['title'], $job['script']));
  	$batch = array(
  		  'title' => t('Running Forena Schools Import jobs'),
  		  'operations' => $ops,
  		);
  	batch_set($batch);
  	if (is_callable('drush_backend_batch_process')) {
  	 drush_backend_batch_process();
  	}
	}
	else {
	  drupal_set_message($name . 'not found', 'error');
	}

}

