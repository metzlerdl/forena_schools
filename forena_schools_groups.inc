<?php
require_once 'inc/ForenaSchools.inc';
require_once 'inc/common.inc';
require_once 'inc/ForenaSchoolsCSV.inc';

function forena_schools_import_group_display($data) {
  // Check to make sure groups will be imported
  if (!@$data[0][0]['name']) {
    drupal_set_message('Group names not found.  No new groups will be created!', 'warning');
  }

  // Check for required building code.
  if (@!$data[0][0]['bldg_code']) {
    drupal_set_message('Missing required buidling code! Data cannot be imported', 'error');
  }

  // Set row headers
  $headers = array('sis_id' => 'SIS ID', 'last_name' => 'Last', 'first_name' => 'first', 'name' => 'name');

  $output ='<div class="group-preview">';

  // Loop through each group.
  foreach ($data as $group) {
    // Build just those keys
    $g = $group[0];
    $members = count($group);
    $tdata = array();
    $code = @$g['course_code'] ? $g['course_code'] : @$g['code'];
    $output .= '<h2>' . @$g['bldg_code'] . '-' . $code . ':' . @$g['name'] . '</h2>';
    $owner = @$g['owner_id'] ? $g['owner_id']: @$g['faculty_sis_id'];
     
     
    if ($owner) {
      $p = ForenaSchoolsPerson::instance()->loadBySisId($g['sis_id']);
      $output .= '<p>'  . $p['last_name'] . ', ' . $p['first_name'] . '</p>';
    }
    foreach ($group as $members) {
      $tdata[] = array_intersect_key($members, $headers);
    }
    $output .= @theme_table(array('rows' => $tdata, 'attributes' => array()));
  }
  $output .= '</div>';
  return $output;
}

function forena_schools_import_groups($formid, &$form_state) {

  $group_types = array(
    'course' => 'Courses', 
    'analysis' => 'Analysis/Invervention', 
  );

  $school_years = ForenaSchools::instance()->schoolYearList();
  $form['defaults'] = array(
    '#type' => 'fieldset', 
    '#title' => 'Defaults', 
    '#tree' => TRUE, 
  ); 

  $form['defaults']['type'] = array(
    '#type' => 'select', 
    '#title' => t('Student Group Type'), 
    '#description' => t('Select type of group to create can be overriende by type column in the import file.'), 
    '#options' => $group_types, 
  );

  $form['defaults']['category'] = array(
    '#type' => 'textfield', 
    '#title' => 'Category', 
    '#description' => 'Can be overriden by specifying a category column in the import file.', 
    '#size' => 60,
  );

  $form['defaults']['school_year'] = array(
    '#type' => 'select', 
    '#title' => 'School Year', 
    '#description' => 'Can be overridden by including school_year column in the import file.', 
    '#options' => $school_years,  
  );
  
 
  
  if (!@$form_state['storage']['file_name']) {
    $form['upload'] = array(
      '#type' => 'file', 
      '#title' => t('File Upload'), 
      '#description' => 'Files to upload.  Files may contain the follwoing headers', 
    );
  }

  $form['preview'] = array(
    '#type' => 'submit',
    '#value' => 'Preview', 
    '#submit' => array('forena_schools_import_groups_preview'), 
  );

  if (@$form_state['storage']['data']) {
    $form['import'] = array(
      '#type' => 'submit',
      '#value' => 'Import', 
      '#submit' => array('forena_schools_import_group_submit')
    ); 
    $form['preview_markup'] = array(
      '#type' => 'markup', 
      '#markup' => forena_schools_import_group_display($form_state['storage']['data']), 
    );
  }



  return $form;
}

// Preview
function forena_schools_import_groups_preview($form, &$form_state) {
  $values = $form_state['values'];
  $form_state['rebuild'] = TRUE;
  if (@$_FILES['files']['tmp_name']['upload']) {

    $validators = array('file_validate_extensions' => array('txt csv'));
    if ($file = file_save_upload('upload', $validators, null, FILE_EXISTS_REPLACE)) {
      $form_state['storage']['file_name'] = $path =  $file->uri;
    }
    else {
      drupal_set_message('Could not uplaod file', 'error');
    }
  }
  else {
    $path = @$form_state['storage']['file_name'];
  }
  if ($path) {
    $csv = ForenaSchoolsCSV::instance();
    $group_by = array('school_code',  'bldg_code', 'group_code', 'course_code', 'faculty_sis_id');
    $data = $csv->readCSV($path, $form_state['values']['defaults'], $group_by, 10);
    $form_state['storage']['data'] = $data;

  }
}

function forena_schools_import_group_submit($form, &$form_state) {

  $path = @$form_state['storage']['file_name'];
  if ($path) {
    $mpath = drupal_get_path('module', 'forena_schools'); 
    $batch = array(
      'title' => t('Importing Groups'),
      'operations' => array(
        array('forena_schools_process_group_import', array($path, $form_state['values']['defaults'])),
        ),
      'file' => $mpath . '/forena_schools_groups.inc', 
    );
    
    batch_set($batch); 
  }

}

/**
 * Batch import of data from impot group
 * $filename specifies the name of the file to import 
 * $default is an array of defaults to include.
 */
function forena_schools_process_group_import($filename, $defaults,  &$context) {
  // Reparse file because it may be too big to cache. 
  $csv = ForenaSchoolsCSV::instance();
  $group_by = array('school_code',  'bldg_code', 'group_code', 'course_code', 'faculty_sis_id');
  $data = $csv->readCSV($filename, $defaults, $group_by);
  // Break into groups of 100. 
  $data = array_chunk($data, 100);   
    
  // Determine how many passes are required. 
  if (@!$context['sandbox']) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = count($data); 
  } 
  $cur = $context['sandbox']['progress']; 
  $max = $context['sandbox']['max'];
  
  // Import in groups of 100. 
  $context['message'] = ForenaSchoolsGroup::instance()->import($data[$cur]); 

  $cur++; 
  if ($cur != $context['sandbox']['max']) {
    $context['finished'] = $cur / $context['sandbox']['max']; 
    $context['sandbox']['progress'] = $cur; 
  } 
  else {
    drupal_set_message('Group Import Completed'); 
  }
  

}