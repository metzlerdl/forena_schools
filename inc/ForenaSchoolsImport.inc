<?php
require_once 'ForenaSchoolsCSV.inc';
require_once 'ForenaSchoolsGroup.inc'; 
require_once 'ForenaSchoolsPerson.inc'; 
class ForenaSchoolsImport { 
  public static function instance() { 
    static $o =''; 
    if (!$o) {
      $o = new ForenaSchoolsImport(); 
    }
    return $o; 
  }
  
  /**
   * Import courses or schedules. 
   * @param $vars default data array.  Must at least contain filename to load. 
   */
  public function importCourseData($vars) { 
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
  
  /**
   * Delete all courses for a source in a given school year. 
   * @param $data varables containing school_year and source to delete. 
   */
  public function deleteCoursesForSource($data) {
    if (@$data['school_year'] && @$data['source']) {
      db_delete('s_groups')
        ->condition('source', $data['source'])
        ->condition('school_year', $data['school_year'])
        ->execute(); 
    }
  }
}
