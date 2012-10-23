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

    $g = ForenaSchoolsGroup::instance(); 
    $p = ForenaSchoolsPerson::instance(); 
    
    // Uncomment to only do the first group
    //$data = array($data[0]); 
    foreach ($data as $group_array) { 
      $group_count++; 
      
      // Get first row as group
      $group = $group_array[0];
      if (isset($group['description'])) $group['name'] = $group['description']; 
      $group['group_code'] = @$group['group_code'] ? $group['group_code']: $group['course_code']; 
      $g->loadByCode($group['group_code'], $group['school_year'], $group['bldg_code'], $group['faculty_sis_id']); 
      
      // If we have enough data, save the group
      if (!$g->group['group_id'] && @$group['name']) { 
        $p->loadBySisId($group['fac_sis_id']); 
        $group['owner_id'] = $p->person['person_id'];    
        if ($group['owner_id']) $g->save($group); 
      }
      
      // If we have a description then save group data.
      if ($g->group['group_id']) { 
        $sis_ids = array(); 
        foreach ($group_array as $member) { 
          if (@$member['sis_id']) $sis_ids[] = $member['sis_id']; 
        }
        $student_count+=count($sis_ids); 
        $g->saveMembersBySISId($sis_ids, $group['school_year']); 
      }
    } 
    
    $output .= "Imported $student_count members into $group_count groups. "; 
    return $output; 
  }
  
}