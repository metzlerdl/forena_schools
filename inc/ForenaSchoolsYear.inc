<?php
class ForenaSchoolsYear {
  public $year;

  static public function instance($year='') {
    static $o = '';
    if (!$o) {
      $o = new ForenaSchoolsYear();
    }
    if ($year && $year != $o->year['school_year']) {
      $o->load($year);
    }
    return $o;
  }

  /**
   * Delete School year
   * @param $year
   *   School year to delete.
   */
  public function delete() {
    if ($this->year) {
      $year = $this->year['school_year'];

      //Groups
      db_delete('s_groups')
        ->condition('school_year', $year)
        ->execute();

      //Check for assessments
      db_delete('a_assessments')
        ->condition('school_year', $year)
        ->execute();

      //Check for assessments
      db_delete('a_score_stats')
        ->condition('school_year', $year)
        ->execute();
      //Students
      db_delete('p_students')
        ->condition('school_year', $year)
        ->execute();

      db_delete('i_school_years')
        ->condition('school_year', $year)
        ->execute();
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
    $data =  array('year' => $year);
    if ($rs = db_query('SELECT * from i_school_years where school_year = :year', array(':year' => $year))) {
      $data = $rs->fetchAssoc();
    }
    $this->year = $data;
    return $data;
  }

  /**
   * Get statistics for the school year.
   */
  public function stats() {
    //Check for groups
    $year = $this->year['school_year'];
    $rs = db_query ('SELECT COUNT(1) FROM {s_groups} WHERE school_year = :school_year',
      array(':school_year' => $year));

    $stats['Groups'] = $rs->fetchColumn();

    //Check for students
    $rs = db_query('SELECT COUNT(1) FROM {p_students} WHERE school_year = :school_year', array(':school_year' => $year));
    $stats['Students'] = $rs->fetchColumn();

    //Check for assessments
    $rs = db_query('SELECT COUNT(1) FROM {a_assessments} WHERE school_year = :school_year', array(':school_year' => $year));
    $stats['Assessments'] = $rs->fetchColumn();

    return $stats;

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
      $date =$date ->format('Y-m-d');
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
      $fields['start_date'] = $this->validDate($fields['start_date']);
    }
    if (isset($fields['end_date'])) {
      $fields['end_date'] = $this->validDate($fields['end_date']);
    }
    if ($this->year) {
      db_update('i_school_years')
       ->fields(array(
         'label' => $fields['label'],
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
          'label' => $fields['label'],
          'start_date' => $fields['start_date'],
          'end_date' => $fields['end_date'],
        ))
        ->execute();
    }
  }

  /**
   * Return an offset date from the school year.
   * @param  $offset integer Number of days.
   * @return string Formatted date string.
   */
  public function date($offset) {
    $date = new DateTime($this->year['start_date']);
    $date->add(new DateInterval('P' . $offset . 'D'));
    return $date->format('Y-m-d');
  }
}