<?php
/**
 * @file
 * Profile management
 * @author metzlerd
 *
 */
class ForenaSchoolsProfile {
  public $profile_id = '';    //Unique key to profile
  public $profile = array();
  public $measures = array();
  public $displays = array();

  /**
   * Singleton factory method
   * @return ForenaSchoolsProfile
   */
  public static function instance() {
    static $o = '';
    if (!$o) {
      $o = new ForenaSchoolsProfile();
    }
    return $o;
  }

  /**
   * Save the profle
   * Enter description here ...
   * @param $profile associate array containg profile data.
   */
  public function save($profile) {
    // First insert or update the base profile record.
    $profile = array_merge($this->profile, $profile);
    if ($this->profile_id) {
      db_update('a_profiles')
        ->fields(array(
          'name' => $profile['name'],
          'min_grade' => $profile['min_grade'],
          'max_grade' => $profile['max_grade'],
          'school_year_offset' => $profile['school_year_offset'],
          'bldg_id' => $profile['bldg_id'],
          'weight' => $profile['weight'],
        ))
        ->condition('profile_id', $this->profile_id)
        ->execute();
    }
    else {
      $this->profile_id =  db_insert('a_profiles')
        ->fields(array(
          'name' => $profile['name'],
          'min_grade' => $profile['min_grade'],
          'max_grade' => $profile['max_grade'],
          'school_year_offset' => $profile['school_year_offset'],
          'bldg_id' => $profile['bldg_id'],
          'weight' => $profile['weight'],
        ))
        ->execute();
    }

    //Now insert update the measures
    $existing_measures = $this->measures;
    $i=0;
    // Update
    if ($profile['measures']) foreach ($profile['measures'] as $k=>$measure) {
      if(isset($existing_measures[$k])) {
        db_update('a_profile_measures')
          ->fields(array(
            'measure_id' => $measure['measure_id'],
            'seq' => $measure['seq'],
            'label' => $measure['label'],
          ))
          ->condition('profile_id', $this->profile_id)
          ->condition('sort_order', $i)
          ->execute();
      }
      else {
        db_insert('a_profile_measures')->
          fields(array(
            'profile_id' => $this->profile_id,
            'measure_id' => $measure['measure_id'],
            'seq' => $measure['seq'],
            'sort_order' => $i,
          ))
          ->execute();
      }
      $i++;
    }

    if ($profile['measures']) {
      $m1 = array_keys($this->measures);
      $m2 = array_keys($profile['measures']);
      $measures_to_delete = array_diff($m1, $m2);
      if ($measures_to_delete) {
        db_delete('a_profile_measures')
          ->condition('profile_id', $this->profile_id)
          ->condition('sort_order', $measures_to_delete)
          ->execute();
      }
    }
    else {
      db_delete('a_profile_measures')
        ->condition('profile_id', $this->profile_id)
        ->execute();
    }

    //Update the displays
    $new_displays = array();
    if (@$profile['displays']) foreach ($profile['displays'] as $display) {
      if ($display) $new_displays[] = $display;
    }
    $displays_to_add = array_diff($new_displays, $this->displays);

    foreach ($displays_to_add as $display) {
      db_insert('a_profile_displays')
        ->fields(array(
        	'profile_id' => $this->profile_id,
          'display' => $display,
        ))
        ->execute();
    }

    if ($new_displays) {
      db_delete('a_profile_displays')
        ->condition('profile_id', $this->profile_id)
        ->condition('display', $new_displays, 'NOT IN')
        ->execute();
    }
    else {
      db_delete('a_profile_displays')
        ->condition('profile_id', $this->profile_id)
        ->execute();
    }

  }

  /**
   * Load measures into an array structure.
   */
  public function loadMeasures() {
    $rs = db_query("select pm.*,
        m.name,
        m.abbrev,
        COALESCE(s.label, 'Any') AS sched_label
      FROM {a_profile_measures} pm
      JOIN  {a_test_measures} m
        ON pm.measure_id = m.measure_id
      LEFT JOIN  {a_test_schedules} s
        ON s.seq = pm.seq
      WHERE profile_id = :profile_id
      ORDER BY sort_order", array(':profile_id' => $this->profile_id));
    $measures = array();
    foreach ($rs as $row) {
      $measures[(int)$row->sort_order] = array(
        'sort_order' => (int)$row->sort_order,
        'measure_id' => $row->measure_id,
        'seq' => $row->seq,
        'label' => $row->label,
        'name' => $row->name,
        'abbrev' => $row->abbrev,
        'sched_label' => $row->sched_label,
      );
    }
    $this->measures = $this->profile['measures']  = $measures;
    return $this->measures;
  }

  /**
   * Load List of displays that apply to this measure
   */
  public function loadDisplays() {
    $rs = db_query('SELECT * FROM {a_profile_displays} WHERE profile_id = :profile_id ORDER BY display', array(':profile_id' => $this->profile_id));
    $displays = array();
    foreach ($rs as $row) {
      $displays[] = $row->display;
    }
    $this->displays = $this->profile['displays'] = $displays;
    return $this->displays;
  }

  /**
   * Load profile data structure
   * @param $profile_id string key of profile to load
   */
  public function load($profile_id) {
    // Fetch basic profile fields
    if ($profile_id) {
      $this->profile_id = $profile_id;
      $rs = db_query('SELECT * FROM {a_profiles} WHERE profile_id=:profile_id', array(':profile_id' => $profile_id));
      if ($rs) {
        $this->profile = $rs->fetchAssoc();
      }
      $this->loadMeasures();
      $this->loadDisplays();
    }
    else {
      $this->profile =
        array('profile_id' => '', 'school_year_offset' => 0, 'name' => '','code' => '', 'min_grade' => '', 'max_grade' => '', 'bldg_id' => -1, 'weight' => 0);
      $this->displays = $this->profile['displays'] = array();
      $this->measures = $this->profile['measures'] =  array();
    }
    return $this->profile;
  }

  /**
   * Delete all profile entries for a profile.
   */
  public function delete() {
    if ($this->profile_id) {
      db_delete('a_profile_measures')
        ->condition('profile_id', $this->profile_id)
        ->execute();
      db_delete('a_profile_displays')
        ->condition('profile_id', $this->profile_id)
        ->execute();
      db_delete('a_profiles')
        ->condition('profile_id', $this->profile_id)
        ->execute();
    }
  }

}