<?php
class ForenaSchoolsQuery {
	static function instance() {
		static $o = '';
		if (!$o) {
			$o = new ForenaSchoolsQuery();
		}
		return $o;
	}

	/**
	 * Retrieve a single row from a table based on pimary keys
	 * Enter description here ...
	 * @param string $table The name of the table that we are selecting
	 * @param array $keys   Associative array containing keys for the table
	 */
	public function retrieve($table, $keys) {
		if (!$keys) return array();
		$q = db_select($table,'t')
		  -> fields('t');
		// Filter on all key values pairs
	  foreach ($keys as $key=> $value) {
	  	$q->condition($key, $value);
	  }
	  $rs = $q->execute();
	  // Retrun
	  if ($rs) {
	  	return $rs->fetchAssoc();
	  }
	  else {
	  	return array();
	  }
	}

	/**
	 * Update a single record based on a serialized key
	 * @param unknown_type $table
	 * @param unknown_type $key array of primary key values
	 * @param unknown_type $values
	 */
	public function update($table, $key, $values) {
		$pri_key = array($key => $values[$key]);
		$row = $this->retrieve($table, $pri_key);

		if ($row) {
			unset($values[$key]);
			db_update($table)
			  ->fields($values)
			  ->condition($key, $pri_key[$key])
			  ->execute();
		}
		else {
			db_insert($table)
			  ->fields($values)
			  ->exectue();
		}
	}

	public function options($result) {
		if ($result) {
			while ($row = $result->fetchAssoc()) {

			}
		}
	}


}