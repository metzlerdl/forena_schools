<?php
require_once 'ForenaSchoolsBuilding.inc';

/* 
 * Contains some forena schools utility fucn
 */
class ForenaSchoolsUtil extends ForenaSchools { 
  static public function instance() { 
    static $o = ''; 
    if (!$o) { 
      $o = new ForenaSchoolsUtil(); 
    }
    return $o; 
  }
  
  /* 
   * Test function for testing interface
   */
  public function test() { 
    drupal_set_message('testing...'); 
    return 'Hello'; 
  }
  
  /*
   * Delete buildings based on parameters
   */
  public function deleteBuildings($default) { 
    ForenaSchoolsBuilding::instance()->delete($default['bldg_id']); 
  }
  

}