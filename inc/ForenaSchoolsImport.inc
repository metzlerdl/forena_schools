<?php
require_once 'ForenaSchoolsCSV.inc';
require_once 'ForenaSchoolsGroup.inc'; 
require_once 'ForenaSchoolsPerson.inc'; 
class ForenaSchoolsImport { 
  public function importCourseSchedules($vars) { 
    $output = ''; 
    $group_count = 0; 
    $student_count = 0; 
    $path = rtrim($vars['import_directory']); 
    $filename=$vars['filename']; 
    $filepath = $path . '/' . $filename; 
    $sep = @$vars['sep']; 
    $enc = @$vars['enc']; 
    $csv = ForenaSchoolsCSV::instance(); 
    $group_by = array('school_code',  'bldg_code', 'group_code', 'course_code', 'faculty_sis_id'); 
    $data = $csv->readCSV($filepath, $vars, $group_by);

    return ForenaSchoolsGroup::instance()->import($data); 
  }
  
}