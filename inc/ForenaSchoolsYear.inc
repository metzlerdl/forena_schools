<?php
class ForenaSchoolsYear { 
  static public function instance() { 
    static $o = ''; 
    if (!$o) { 
      $o = new ForenaSchoolYear(); 
    }
    return $o; 
  }
  
  /**
   * Delete School year 
   * @param $year
   *   School year to delete. 
   */
  public function delete($year) { 
    if ($year) { 
      //Check for groups
      $rs = db_query ('SELECT COUNT(1) FROM {s_groups} WHERE school_year = :school_year', 
        array(':school_year' => $year)); 
      $count = $rs->fetchColumn();
      if ($count) { 
        drupal_set_message(t('Cannot delete year while groups exist'), 'eror'); 
        return; 
      }
      
      //Check for students
      $rs = db_query('SELECT COUNT(1) FROM {p_students} WHERE school_year = :school_year', array(':school_year' => $year)); 
      $count = $rs->fetchColumn(); 
      if ($count) { 
        drupal_set_message(t('Cannot delete year while students exist'), 'error'); 
        return; 
      }
      
      //Check for assessments
      $rs = db_query('SELECT COUNT(1) FROM {a_assessments} WHERE school_year = :school_year', array(':school_year' => $year)); 
      $count = $rs->fetchColumn(); 
      if ($count) { 
        drupal_set_message(t('Cannot delete year while assessments exist'), 'error'); 
        return; 
      }
      
      // Perform the actual delete; 
      db_delete('school_year')->condition('school_year', $year); 
    }
  }
  
  /**
   * Load school year from database. 
   * @param $year 
   *   School year to load... typicaly the year that the school year ends in. 
   *  @return 
   *    Associative array representation of school year. 
   */
  public function load($year) {
    $data =  array('year' => year); 
    if ($rs = db_query('SELECT * from i_school_year where school_year = :year', array(':year' => $year))) { 
      $data = $rs->fetchAssoc(); 
    } 
    $this->year = $data; 
    return $data; 
  }
  
  /**
   * Load the school year for the data
   */
  public function selectList() { 
    
  }
  
  /**
   * Check to make sure date is valid
   * @param $date
   * 		Date to check 
   */
  public function validDate($date) { 
    if ($date) { 
      $date = date_create($date); 
      if (!$date) {
        drupal_set_message(t('Invalid date format'), 'error'); 
      }
    }
    return $date; 
  }
  
  /**
   * Save school year
   * @param $fields 
   *   associative array of school year fields. 
   */
  public function save($fields = array()) {
    // Check to make sure dates are reasoblae
    if (isset($fields['start_date'])) { 
      $fields['start_date'] = $this->valdDate($fields['start_date']); 
    }
    if (isset($fields['end_date'])) { 
      $fields['end_date'] = $this->validDate($fields['end_date']); 
    }
    if ($this->year) { 
      db_update('i_school_years')
       ->fields(array(
         'name' => $fields['name'],
         'abbrev' => $fields['abbrev'], 
         'start_date' => $fields['start_date'], 
         'end_date' => $fields['end_date'], 
       ))
       ->condition('school_year', $fields['school_year'])
       ->execute(); 
    } 
    else {
      db_insert('i_school_years')
        ->fields(array(
          'school_year' => $fields['school_year'], 
          'name' => $fields['name'], 
          'abbrev' => $firlds['abbrev'], 
          'start_date' => $fields['start_date'], 
          'end_date' => $fields['end_date'], 
        ))
        ->execute(); 
    }
  }
}