<?php
require_once 'ForenaSchoolsPerson.inc';
require_once 'ForenaSchoolsBuilding.inc';
class ForenaSchoolsGroup {

  public $group;
  public $members;

  /**
   * Factory for group object
   * @return ForenaSchoolsGroup
   */
  static public function instance() {
    static $o = '';
    if (!$o) {
      $o = new ForenaSchoolsGroup();
      $o->init();
    }
    return $o;
  }

  /**
   * Initialize noew objects
   */
  public function init() {
    $this->group = array_fill_keys(array('group_id', 'group_type', 'name', 'category', 'code', 'owner_id', 'school_year'),'');
    $this->members = array();
  }

  /**
   * Load student group by code/school year
   * @param string $code The alpha code associated with this group.
   * @param string $school_year School year code identifying the current school-year
   */
  public function loadByCode($code, $school_year, $bldg_code, $fac_sis_id ) {
    $this->init();
    // Prepopulate the search values
    $this->group['code'] = $code;
    $this->group['school_year'] = $school_year;

    $parms = array(':code' => $code, ':school_year' => $school_year, ':bldg_code' => $bldg_code, ':fac_sis_id' => $fac_sis_id);

    //Search for the existing entry
    $rs = db_query(
    	'SELECT * FROM {s_groups} g
    	  JOIN {i_buildings} b ON g.bldg_id=b.bldg_id
    	  JOIN {p_people} p ON g.owner_id = p.person_id
    	  WHERE g.code=:code AND g.school_year=:school_year and b.code=:bldg_code
    	    AND p.sis_id = :fac_sis_id',
     $parms

    );

    // If data was returned then load the members
    if ($rs) {
      $this->group = $rs->fetchAssoc();
      $this->loadMembers();
    } else {
      //@TODO: load faculty data
    }
    return $this->group;
  }

  /**
   * Load group based on group id.
   * @param unknown_type $group_id
   */
  public function loadByGroupId($group_id) {
    $this->init();
    $rs = db_query(
      'SELECT * FROM {s_groups} WHERE group_id = :group_id',
      array(':group_id' => $group_id)
    );

    // If Data was returned then load members.
    if ($rs) {
      $this->group = $rs->fetchAssoc();
      $this->loadMembers();
    }
    return $this->group;
  }

  /**
   * Load memebers for existing roup into group array.
   */
  public function loadMembers() {
    if ($this->group['group_id']) {
      $rs = db_query(
        'SELECT m.*, last_name, first_name FROM {s_group_members} m
          JOIN {p_students} s ON s.student_id = m.student_id
          JOIN {p_people} p ON p.person_id=s.person_id
        WHERE group_id = :group_id
        ORDER BY last_name, first_name',
        array(':group_id' => $this->group['group_id'])
      );

      // Load memebers into an array keyed by student id.
      if ($rs) {
        $this->members = $rs->fetchAllAssoc('student_id', PDO::FETCH_ASSOC);
      }
      else {
        $this->members = array();
      }

    }
  }

  /**
   * Reload current group based on group id.
   */
  public function reload() {
    if ($this->group['group_id']) {
      $this->loadByGroupId($this->group['group_id']);
    }
  }

  /**
   * Build and save based on a group array.
   * @param array group
   */
  public function save($group = array()) {
    // First determine whether we need to add or insert.
    $new_members = @$group['members'];
    unset($group['members']);
    if (!$new_members) $new_members = array();

    // Only overwrite fields present in the group
    if ($this->group) $group = array_merge($this->group, $group);
    $this->group = $group;

    // Insert/update the group.
    if (@$group['group_id']) {
      db_update('s_groups')->
        fields(
          array(
          	'name' => $group['name'],
            'category' => @$group['category'],
            'bldg_id' => $group['bldg_id'],
            'school_year' => $group['school_year'],
            'source' => @$group['source'],
            'code' => @$group['code'],
            'owner_id' => $group['owner_id'],
          )
        )
        ->condition('group_id', $group['group_id'])
        ->execute();
    }
    else {
      $group_id = db_insert('s_groups')
        ->fields(array(
          'name' => $group['name'],
          'category' => @$group['category'],
          'bldg_id' => $group['bldg_id'],
          'school_year' => @$group['school_year'],
          'group_type' => $group['group_type'],
          'source' => @$group['source'],
          'code' => @$group['code'],
          'owner_id' => @$group['owner_id'],
          ))
        ->execute();
      $this->group['group_id'] = $group_id;
    }

  }

  /**
   * Update members based on group
   * @param array $members
   */
  public function saveMembers($new_members) {
    // Now save the members
    $group_id = $this->group['group_id'];
    // Verify group id exists
    if (!$group_id) {
      drupal_set_message('Missing group id in saveMembers', 'error');
      return;
    }

    //Get existing members
    $members = $this->members;
    $members_to_delete = array_diff_key($members, $new_members);
    $ids = array_keys($members_to_delete);
    if ($ids) {
      db_delete('s_group_members')
        ->condition('group_id', $group_id)
        ->condition('student_id', $ids)
        ->execute();
    }

    $members_to_add = array_diff_key($new_members, $members);
    foreach ($members_to_add as $key=>$value) {
      db_insert('s_group_members')
        ->fields(array(
          'group_id' => $group_id,
          'student_id' => $key
          ))
        ->execute();
    }

    // Update min and max grade levels if necessary
    $rs = db_query('SELECT MIN(grade_level) AS min_grade, max(grade_level) as max_grade
      FROM s_group_members m JOIN p_students s ON m.student_id=s.student_id where group_id=:group_id',
      array(':group_id' => $group_id)
    );
    if ($rs) {
      $data = $rs->fetchAssoc();
      if ($data['min_grade'] != @$this->group['min_grade_level'] || $data['max_grade']!= @$this->group['max_grade_level'])
      db_update('s_groups')
        ->fields(array(
            'max_grade_level' => $data['max_grade'],
            'min_grade_level' => $data['min_grade'],
          ))
        ->condition('group_id', $group_id)
        ->execute();
    }
  }

  /**
   * Save members based on a profided array of sis_ids along with a school year
   * @param $sis_ids Student System identifier
   * @param $school_year School year ofr current school records.
   */
  public function saveMembersBySISId($sis_ids, $school_year) {
    if ($sis_ids && $school_year) {
      $query =  db_select('p_students', 's');
      $query->join('p_people', 'p', 'p.person_id=s.person_id');
      $query->fields('s');
      $query->condition('sis_id', $sis_ids);
      $query->condition('school_year', $school_year);
      $rs = $query->execute();
      $members = array();
      if ($rs) {
        $members = $rs->fetchAllAssoc('student_id', PDO::FETCH_ASSOC);
      }

      $this->saveMembers($members);
    }
  }

  /**
   * Delete the current group by its group id.
   */
  public function delete() {
    if (@$this->group['group_id']) {
      db_delete('s_group_memebers')
       ->condition('group_id', $this->group['group_id'])
       ->execute();
      db_delete('s_groups')
       ->condition('group_id', $this->group['group_id'])
       ->execute();
    }
  }

  /**
   * This is a
   * Enter description here ...
   * @param unknown_type $data
   */
  public function import($data)  {

    $output = '';
    $group_count = 0;
    $student_count = 0;
    $p = ForenaSchoolsPerson::instance();
    $g = $this;
    $buildings = ForenaSchoolsBuilding::instance()->codeList();

    // Uncomment to only do the first group
    //$data = array($data[0]);
    foreach ($data as $group_array) {
      $group_count++;

      // Get first row as group
      $group = $group_array[0];

      // Rename fields for backward compatibilty
      if (isset($group['description'])) $group['name'] = $group['description'];
      $group['code'] = @$group['code'] ? $group['code']: $group['course_code'];
      // Default in group type
      if (!@$group['group_type']) $group['group_type'] = 'course';

      // Check for existing group.
      $cur_group = $g->loadByCode($group['code'], $group['school_year'], $group['bldg_code'], $group['faculty_sis_id']);
      if ($cur_group) {
        $group['group_id'] = $cur_group['group_id'];
      }

      // Determine building id based on bldg_code.
      $bldg = @$buildings[$group['bldg_code']];
      if ($bldg) {
        $group['bldg_id'] = $bldg['bldg_id'];
      }
      else {
        $group['bldg_id'] = -1;
      }

      // If we have enough data, save the group
      if (!$g->group['group_id'] && @$group['name'] && @$group['bldg_id']) {
        $p->loadBySisId($group['faculty_sis_id']);
        $group['owner_id'] = $p->person['person_id'];
        if ($group['owner_id']) $g->save($group);
      }

      // If we have a description then save group data.
      if ($g->group['group_id']  && isset($group['sis_id'])) {
        $sis_ids = array();
        foreach ($group_array as $member) {
          if (@$member['sis_id']) $sis_ids[] = $member['sis_id'];
        }
        $student_count+=count($sis_ids);
        $g->saveMembersBySISId($sis_ids, $group['school_year']);
      }
    }

    $output .= "Imported $student_count members into $group_count groups. ";
    return $output;
  }

  public function userGroups($type='intervention', $bldg_id='', $min_grade='', $max_grade='') {
    $groups = array();
    if (forena_schools_access('dist_admin') && !$bldg_id) {
      $rs = db_query('select * FROM s_groups g
        JOIN p_people p ON g.owner_id = p.person_id WHERE login=:login
          AND group_type = :type',
        array(':login' => forena_schools_current_login(), ':type' => $type)
        );
    }
    elseif (forena_schools_access('bldg_admin')) {
      $rs = db_query('select * from s_groups where bldg_id = :bldg_id
        AND (:min_grade BETWEEN min_grade_level AND max_grade_level
        OR :max_grade BETWEEN min_grade_level AND max_grade_level
        OR (min_grade_level > :min_grade AND max_grade_level < :max_grade))
        AND :type = group_type',
      array(':bldg_id' => $bldg_id, ':min_grade' => $min_grade, ':max_grade' => $max_grade,  ':type' => $type));
    }
    else {
      $rs = db_query('select * FROM s_groups g
        JOIN p_people p ON g.owner_id = p.person_id WHERE login=:login
          AND group_type = :type',
        array(':login' => forena_schools_current_login(), ':type' => $type)
        );
    }
    foreach ($rs as $row) {
      $groups[$row->group_id] = $row->name;
    }
    return $groups;

  }


}