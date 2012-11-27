<?php
class ForenaSchoolsStudent { 
  /**
   * Factory for student model
   */
  static public function instance() { 
    static $o = ''; 
    if (!$o) { 
      $o = new ForenaSchoolsStudent(); 
      $this->init(); 
    }
    return $o; 
  }
  
  public function init() { 
    $this->student = array_fill_keys(array('person_id', 'student_id', 'last_name', 
      'school_year', 'bldg_id', 'grade_level'),''); 
  }
  
  /**
   * Load a student based on SIS id and school_year
   * @param string $sis_id
   * @param string $schooL_year
   */
  public function loadBySisId($sis_id, $schooL_year) { 
    $rs = db_query(
      'SELECT s.*, last_name, first_name 
      FROM {p_people} p 
      	JOIN {p_students} s ON s.person_id=p.person_id 
      WHERE p.sis_id = :sis_id AND s.school_year = :school_year',
      array(':sis_id' => $sis_id, ':school_year' => $schooL_year)
    ); 
    if ($rs) {
      $student = $rs->fetchAssoc(); 
      $this->student = $student; 
    }
    return $this->student; 
  }
 
  public function loadByStudentId($student_id) { 
      $rs = db_query(
      'SELECT s.*, last_name, first_name 
      FROM {p_people} p 
      	JOIN {p_students} s ON s.person_id=p.person_id 
      WHERE s.student_id = :student_id',
      array(':student_id' => $student_id)
    ); 
    if ($rs) {
      $student = $rs->fetchAssoc(); 
      $this->student = $student; 
    }
    return $this->student; 
  }
  
  public function loadByPersonId($person_id, $school_year) { 
    $rs = db_query(
      'SELECT s.*, last_name, first_name 
      FROM {p_people} p 
      	JOIN {p_students} s ON s.person_id=p.person_id 
      WHERE p.sis_id = :sis_id AND s.school_year = :school_year',
      array(':person_id' => $person_id, ':schooL_ear' => $schooL_year)
    ); 
    if ($rs) {
      $student = $rs->fetchAssoc(); 
      $this->student = $student; 
    }
    return $this->student;     
  }
  
  /**
   * Import list of students
   * @param $data Array of person records (each as an associative array). 
   */
  public function import($data) { 
    // Get building list 
    
    
    // If we have data import i
    $p = ForenaSchoolsPerson::instance(); 
    if ($data) foreach ($data as $student) {
      if ($data['sis_id']) { 
        $p->loadBySisId($data['sis_id']); 
        $person = $p->save($student); 
        
        $this->loadByStudentId($student_id);
        //* @TODO: Complete import logic
      }
    }
  }
}