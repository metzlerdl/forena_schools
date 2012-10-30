<?php
class ForenaSchoolsPerson {
  public $person; 
  
  /**
   * Singleton Factory method
   */
  static public function instance() { 
    static $o = ''; 
    if (!$o) { 
      $o = new ForenaSchoolsPerson(); 
    }
    return $o; 
  }
  
   
  /**
   * Load person based on sis id. 
   */
  public function loadBySisId($sis_id) { 
    $rs = db_query(
      'SELECT *
      FROM {p_people} p 
      WHERE p.sis_id = :sis_id',
      array(':sis_id' => $sis_id)
    ); 
    
    if ($rs) {
      $person = $rs->fetchAssoc(); 
      $this->person = $person; 
    }
    return $this->person; 
  }
  
  
}