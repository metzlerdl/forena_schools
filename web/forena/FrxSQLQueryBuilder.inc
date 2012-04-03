<?php
/**
 * FrxSQLQuery SQL Builder
 * This class defines a common query builder that is used to
 * make SQL safe queries based on named column filteres.
 * @author metzlerd
 */
class FrxSQLQueryBuilder {
	private $clauses = array();
	private $descriptions = array();
	private $where_clause = array();
	private $comparison_operators= array(
	  '<' => 'is less than',
	  '>' => 'is greater than',
	  '=' => 'is equal to',
	  '<>' => 'is not equal to',
	  '= any' => 'is one of',
	  'LIKE' => 'is like',
	);
	private $data;

	/**
	 * Starts the query builder
	 * Enter description here ...
	 */
	public function query($block, $values = array()) {
		$this->applied_filters = array();
		$this->block=$block;
		$this->data = $values;
		return $this;
	}

	/**
	 * Executes the query that was started with query method
	 * Enter description here ...
	 */
	public function execute() {
    return FrxReportGenerator::instance()->invoke_data_provider($this->block, $this->data, $this->sql());
	}

	/**
	 * Return the last where clause used for the data block.
	 */
	public function sql() {
    $where = '';
    $i=0;
    if (count($this->where_clause)>0) {
			foreach($this->where_clause as $clause) {
	    	$i++;
	      $where .= $i==1 ? 'WHERE ' : ' AND ';
				$where .= $clause;
			}
    }
		return $where;
	}

	public function where($condition, $include = TRUE ) {
		if ($include) $this->where_clause[] = $condition;
		return $this;
	}

	/**
	 * Filters the query that is being executed
	 * Enter description here ...
	 * @param string $field
	 * @param string $comparison
	 */
  public function filter($field, $comparison, $include=TRUE) {
  	if ($include===null) $include = (!empty($this->data[$field]));
  	if ($include && @$this->comparison_operators[$comparison]) {
	  	$this->where_clause[] = $field . ' ' . $comparison . ' :'.$field ;
  	}
  	return $this;
  }

  /**
   * Removing
   * Enter description here ...
   * @param unknown_type $field
   * @param unknown_type $comparison
   * @param unknown_type $value
   */
  public function condition($field, $comparison, $value) {
    $this->data[$field] = $value;
    $this->filter($field, $comparison);
  }

  /**
   * Filters the query if the value is present
   * Enter description here ...
   * @param string $field
   * @param string $comparison
   */
  public function filter_not_null($field, $comparison) {
  	if (!empty($this->data[$field])) {
  		$this->filter($field, $comparison);
  	}
  	return $this;
  }

}