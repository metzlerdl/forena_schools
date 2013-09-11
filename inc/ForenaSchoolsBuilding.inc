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

  /**
   *
   * List the staff
   * @param $bldg_id int id of building for which to list staff.
   */
  public function listStaff($bldg_id) {
    $staff = array();
    $rs = db_query('select p.person_id, p.last_name, p.first_name from {p_staff} s JOIN {i_buildings} b ON s.bldg_id=b.bldg_id
      JOIN p_people p ON s.person_id=p.person_id
      WHERE s.bldg_id = :bldg_id
      ORDER BY last_name, first_name', array(':bldg_id' => $bldg_id)
    );
    foreach ($rs as $row) {
      $staff[$row->person_id] = $row->last_name . ', ' . $row->first_name;
    }
    return $staff;

  }
}