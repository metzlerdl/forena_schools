<?php


class ForenaSchoolsBuilding { 
  
  public $bulding; 
  static public function instance() { 
    static $o = ''; 
    if (!$o) { 
      $o = new ForenaSchoolsBuilding(); 
    }
    return $o; 
  }
  
  
  /**
   * Delete a school
   * Removes all data associated with a partcicluar school 
   * @param mixed $schools Reference the building id or ids for schools that need to be deleted. 
   */  
  public function delete($schools = array()) { 
    
    $rows = 0;    
    if ($schools) {  
      $rows += db_delete('a_assessments')
        ->condition('bldg_id', $schools)
        ->execute();
        
      $rows += db_delete('s_groups')
        ->condition('bldg_id', $schools)
        ->execute(); 
        
      $rows += db_delete('p_students')
        ->condition('bldg_id', $schools)
        ->execute();
        
      $rows += db_delete('i_buildings')
        ->condition('bldg_id', $schools)
        ->execute(); 
      drupal_set_message("Deleted building data of $rows rows."); 
      watchdog('Forena Schoools',  "Deleted build data of $rows rows." ); 
      return "$rows records deleted from database"; 
    }
  }
  
  /**
   * Return an array of buildings keyed by buidling id. 
   */
  public function idList() { 
    $buildings = array(); 
    $rs = db_query('select * from {i_buildings} order by name'); 
    if ($rs) { 
      $buildings = $rs->fetchAllAssoc('bldg_id',PDO::FETCH_ASSOC); 
    }
    return $buildings; 
  }
  
  /**
   * Retrun a list of all buildings indexed by code. 
   */
  public function codeList() { 
    $buildings = array(); 
    $rs = db_query('select * from i_buildings  where code is not null order by name'); 
    if ($rs) { 
      $buildings = $rs->fetchAllAssoc('code',PDO::FETCH_ASSOC); 
    }
    return $buildings;  
  }
  
  /**
   * Load individual building by code
   * @param $code Building code to load. 
   */
  public function loadByCode($code) {
    $rs = db_query('select * from i_buildings where code=:code',
      array(':code' => $code)
    ); 
    if ($rs) { 
      $bldg = $rs->fetchAssoc(); 
    }
    else { 
      $bldg =  array(); 
    }
    $this->building = $bldg; 
    return $bldg; 
  }
}