<?php
class ForenaSchoolsGroup { 
  
  public $group; 
  public $members; 

  /**
   * Factory for 
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
    
    //Search for the existing entry
    $rs = db_query(
    	'SELECT * FROM {s_groups} g 
    	  JOIN {i_buildings} b ON g.bldg_id=b.bldg_id
    	  JOIN {p_people} p ON g.owner_id = p.person_id
    	  WHERE g.code=:code AND g.school_year=:school_year and b.code=:bldg_code
    	    AND p.sis_id = :fac_sis_id', 
      array(':code' => $code, ':school_year' => $school_year, ':bldg_code' => $bldg_code, ':fac_sis_id' => $fac_sis_id)
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
    $group = array_merge($this->group, $group); 
    $this->group = $group; 
    
    // Insert/update the group. 
    if ($group['group_id']) { 
      db_update('s_groups')->
        fields(
          array(
          	'name' => $group['name'],
            'category' => $group['category'], 
            'bldg_id' => $group['bldg_id'], 
            'school_year' => $group['school_year'],
            'group_type' => $group['group_type'], 
            'source' => $group['source'], 
            'code' => $group['code'],
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
          'category' => $group['category'],
          'bldg_id' => $group['bldg_id'],
          'school_year' => $group['school_year'], 
          'group_type' => $group['group_type'], 
          'source' => $group['source'], 
          'code' => $group['code'], 
          'owner_id' => $group['owner_id'], 
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
  }
  
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
    
    // Uncomment to only do the first group
    //$data = array($data[0]); 
    foreach ($data as $group_array) { 
      $group_count++; 
      
      // Get first row as group
      $group = $group_array[0];
      if (isset($group['description'])) $group['name'] = $group['description']; 
      $group['group_code'] = @$group['group_code'] ? $group['group_code']: $group['course_code']; 
      $g->loadByCode($group['group_code'], $group['school_year'], $group['bldg_code'], $group['faculty_sis_id']); 
      
      // If we have enough data, save the group
      if (!$g->group['group_id'] && @$group['name']) { 
        $p->loadBySisId($group['fac_sis_id']); 
        $group['owner_id'] = $p->person['person_id'];    
        if ($group['owner_id']) $g->save($group); 
      }
      
      // If we have a description then save group data.
      if ($g->group['group_id']) { 
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
  

}