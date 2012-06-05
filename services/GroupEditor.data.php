<?php
class GroupEditor extends ForenaSchools {
  public $title = 'Group Editor';
  public $group_id;

  public function __construct() {
  	parent::__construct();
  	//Determine which group we might be talking about
  	$this->group_id = $_REQUEST['group_id'];

  }

  public function auth() {
  	return $this->access('dist_admin');
  }

  /**
   * Extract the current group definitioin
   * Group id is extracted from the post parameter.  We're assuming that the access check has taken care of security issues.
   */
  public function group() {
  	if ($_POST['group_id']) {
	  	$sql = '
	  	  SELECT g.*, s_group_members_xml(group_id) AS members FROM s_groups g WHERE g.group_id=:group_id
	  	';
	  	$parms = array(
	  	  'group_id' => $_POST['group_id'],
	  	);
  	}
  	else {
  		$user = current_user();
  		$person_id = $user['person_id'];
  		if (access_level('bldg_admin') && @$_POST['person_id']) {
  			$person_id = $_POST['person_id'];
  		}

  		$parms = array(
  		           'bldg_id' => $_POST['bldg_id'],
  		           'person_id' => $person_id,
  		           'school_year' => $_POST['school_year'],
  		           'group_type' => $_POST['group_type'],
  		          );
  		$sql = "
  		  SELECT
  		    :bldg_id AS bldg_id,
  		    :person_id as owner_id,
  		    '' as name,
  		    '' as members,
  		    CASE WHEN :group_type='course' THEN 'course'
  		      ELSE 'analysis' END as group_type,
  		    COALESCE(CAST(:school_year AS integer), i_school_year()) as school_year
  		  ";
  	}
  	return $this->db->query_xml($sql, $parms);
  }

  public function delete() {
    if (access_level('bldg_admin')) {
  	   $this->db->query('DELETE FROM s_groups WHERE group_id=:group_id', $_POST);
    }
    else {
    	$user = current_user();
    	$parms = array('group_id'=> $_POST['group_id'],
    	  'person_id' => $user['person_id']);
    	$this->db->query('DELETE FROM s_groups WHERE group_id=:group_id AND owner_id = :person_id', $parms);
    }
    return $this->group();


  }

  /**
   * Save the group
   * Enter description here ...
   */
  public function save() {
     $group_id = $this->db->call('s_group_save(:xml)', $_POST);
     // Toss back in the id of any inserted group_id
     if ($group_id) $_POST['group_id'] = $group_id;
     return $this->group();
  }

  public function staff() {
  	$user = current_user();
    $parms = array('bldg_id' => $_POST['bldg_id'],
        'person_id' => $user['person_id'],);
  	return $this->db->query_xml("
  	  SELECT p.person_id, p.last_name || ', ' || p.first_name AS name
  	    FROM p_staff s JOIN p_people p on s.person_id = p.person_id
  	    WHERE s.bldg_id = :bldg_id
  	      OR (s.person_id = :person_id and s.bldg_id=-1)
  	    ORDER BY name
  	", $parms);

  }

}