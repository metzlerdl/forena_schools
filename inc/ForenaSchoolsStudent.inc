<?php
class ForenaSchoolsStudent {
  /**
   * Factory for student model
   */
  static public function instance() {
    static $o = '';
    if (!$o) {
      $o = new ForenaSchoolsStudent();
      $o->init();
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
      array(':person_id' => $person_id, ':school_year' => $schooL_year)
    );
    if ($rs) {
      $student = $rs->fetchAssoc();
      $this->student = $student;
    }
    return $this->student;
  }

  public function loadByGradeLevel($person_id, $grade_level) {
    $rs = db_query(
      'SELECT s.*, last_name, first_name
      FROM {p_people} p JOIN {p_students} s on p.person_id = s.person_id
      WHERE p.person_id = :person_id AND s.grade_level = :grade_level
      ORDER BY school_year desc',
      array(':person_id' => $person_id, ':school_year' => $school_year)
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

  /**
   * Get grade levels for a student
   * @param $personid id of person to select.
   * @return array of grades in key value pairs suitable for a form.
   */
  public function studentGradeLevels($personid) {
    $rs = db_query('SELECT DISTINCT g.grade_level, g.abbrev FROM {p_students} s JOIN {i_grade_levels} g
      ON g.grade_level = s.grade_level
      WHERE person_id = :person_id ORDER BY g.grade_level', array(':person_id' => $personid));
    $grades = @$rs->fetchAllKeyed(0,1);
    if (!$grades) $grades = array();
    return $grades;
  }
}