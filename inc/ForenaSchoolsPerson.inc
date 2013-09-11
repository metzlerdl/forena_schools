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
   * Load person by person_id
   * @param unknown_type $id
   */
  public function loadById($id) { 
    $rs = db_query(
      'SELECT *
      FROM {p_people} p 
      WHERE p.person_id = :person_id',
      array(':person_id' => $id)
    ); 
    
    if ($rs) {
      $person = $rs->fetchAssoc(); 
      $this->person = $person; 
    }
    return $this->person;     
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
  
  /**
   * Reload current record from database. 
   */
  public function reload() { 
    $id = $this->person['person_id']; 
    $this->loadById($id); 
  }
  
  /**
   * Save a person 
   * @param $person Assocative array containing person row. 
   * 
   * Save should never be called prior to calling one of the load methods
   * to determine whether the object exists. 
   */
  public function save($person) { 
    // Make sure we have minimum required fields. 
    if ($person_id || isset($person['last_name'])) { 
      // Make fields right. 
      if ($this->person) array_merge($this->person, $person); 
      
      if ($this->person['person_id']) { 
        db_update('p_people')
          ->fields(array(
              'sis_id' => @$person['sis_id'], 
              'last_name' => $person['last_name'], 
              'first_name' => @$person['first_name'],
              'middle_name' => @$person['middle_name'], 
              'address_street' => @$person['address_street'], 
              'address_city' => @$person['address_city'], 
              'address_state' => @$person['address_state'], 
              'address_zip' => @$person['address_zip'], 
              'phone' => @$person['phone'], 
              'email' => @$person['email'],
              'login' => @$person['login'], 
              'gender' => @$person['gender'], 
              'birthdate' => @$person['birthdate'], 
              'last_modified' => @$person['last_modified'], 
              'ethnicity_code' => @$person['ethnicity_code'], 
              'state_student_id' => @$person['state_student_id'], 
            ))
          ->condition('person_id', $this->person['person_id'])
          ->execute(); 
      }
      else { 
        $this->person['person_id'] = db_insert('p_people')
          ->fields(array(
              'sis_id' => @$person['sis_id'], 
              'last_name' => $person['last_name'], 
              'first_name' => @$person['first_name'],
              'middle_name' => @$person['middle_name'], 
              'address_street' => @$person['address_street'], 
              'address_city' => @$person['address_city'], 
              'address_state' => @$person['address_state'], 
              'address_zip' => @$person['address_zip'], 
              'phone' => @$person['phone'], 
              'email' => @$person['email'],
              'login' => @$person['login'], 
              'gender' => @$person['gender'], 
              'birthdate' => @$person['birthdate'], 
              'last_modified' => @$person['last_modified'], 
              'ethnicity_code' => @$person['ethnicity_code'], 
              'state_student_id' => @$person['state_student_id'], 
            ))
          ->execute(); 
      }
      $this->reload(); 
      return $this->person; 
    }
  }
  
  
  
  
}