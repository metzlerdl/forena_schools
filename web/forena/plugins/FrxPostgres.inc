<?php
// $Id$
/**
 * @file
 * Oracle specific driver that takes advantage of oracles native XML support
 *
 * In order to take advantage of XML support the following XML
 *
 */
class FrxPostgres extends FrxDataProvider {


  private $db;
  private $use_postgres_xml=FALSE;

  /**
   * Object constructor
   *
   * @param unknown_type $uri Database connection string.
   * @param string $repos_path Path to location of data block definitions
   */
  public function __construct($conf, $repos_path) {
    parent::__construct($conf, $repos_path);
    $this->use_postgres_xml = FALSE;
    $uri = $conf['uri'];
    if (!empty($conf['password'])) $uri = trim($uri) . ' password='. $conf['password'];
    $this->debug = @$conf['debug'];
    if (isset($conf['postgres_xml'])) $this->use_postgres_xml = $conf['postgres_xml'];
    if ($uri) {
      // Test for postgres suport
      if (!is_callable('pg_connect')) {
        $this->error('PHP Postgres support not installed.', 'PHP Postgres support not installed.');
        return;
      }
      try {
        $db = pg_connect($uri);
        if (isset($conf['search path'])) pg_query($db, "SET search_path=" . $conf['search path']);
        $this->db = $db;
      } catch (Exception $e) {
        $this->error('Unable to connect to database ' . $conf['title'], $e->getMessage());
      }

    }
    else {
      $this->error('No database connection string specified', 'No database connection: ' . print_r($conf, 1));
    }

    // Set up the stuff required to translate.
    $this->te = new FrxSyntaxEngine(FRX_SQL_TOKEN, ':', $this);
  }
  /**
   * Get data based on file data block in the repository.
   *
   * @param String $block_name
   * @param Array $parm_data
   * @param Query $subQuery
   */
  public function data($block_name, $params=array(), $clause='') {
    // Load the block from the file
    $db = $this->db;
    $block = $this->load_block($block_name);
    $xml ='';
    if (@$block['source'] && $this->access($block['access']) && $db) {
      $sql = $block['source'];
      if ($clause) {
        $sql = 'SELECT * FROM (' . trim($sql, ' ;') . ') vvv ' . $clause;
      }

      $sql = $this->te->replace($sql);

      if ($this->use_postgres_xml) {
        $xml = $this->postgres_xml($sql, $block_name);
      }
      else {
        $xml = $this->php_xml($sql);
      }
      if ($this->debug) {
      	$d = '';
        if ($xml) $d = htmlspecialchars($xml->asXML());
        $this->debug('SQL: ' . $sql, '<pre> SQL:' . $sql . "\n XML: " . $d . "\n</pre>");
      }
      return $xml;
    }

  }

  /**
   * Generate xml from sql using the provided f_forena
   *
   * @param unknown_type $sql
   * @return unknown
   */
  private function postgres_xml($sql, $block) {
      $db = $this->db;
      //$rs->debugDumpParams();
      $fsql = 'select query_to_xml($1,true,false,$2);';
      $rs = pg_query_params($db, $fsql, array($sql, ''));
      $xml_text='';
      if ($rs) {
        $row = pg_fetch_row($rs);
        $xml_text = $row[0];
      }
      $xml = null;
      if ($xml_text) {
        $xml = new SimpleXMLElement($xml_text);
        if ($xml->getName() == 'error') {
          $msg = (string)$xml . ' in ' . $block . '.sql. ';
          $this->error($msg . 'See logs for more info', $msg . ' in <pre> ' . $sql . '</pre>');
        }
        if (!$xml->children()) $xml ='';
      }
      if ($rs) pg_free_result($rs);
      return $xml;
  }

  private function php_xml($sql) {
      $db = $this->db;
      $xml = new SimpleXMLElement('<table/>');
      $rs = pg_query($sql);

      if ($rs) while ($row = pg_fetch_assoc($rs)) {
        $row_node = $xml->addChild('row');
        foreach ($row as $key => $value) {
          $row_node->addChild(strtolower($key), htmlspecialchars($value));
        }
      }
      if ($rs) pg_free_result($rs);
      return $xml;
  }

  /**
   * Implement custom SQL formatter to make sure that strings are properly escaped.
   * Ideally we'd replace this with something that handles prepared statements, but it
   * wouldn't work for
   *
   * @param unknown_type $value
   * @param unknown_type $key
   * @param unknown_type $data
   */
  public function format($value, $key, $data) {
    if ($value =='')
      $value = 'NULL';
    else {
      if (is_array($value)) {
      		// Build a array of values string
	      	$i=0;
	      	$val ='';
	      	foreach ($value as $v) {
	      		$i++;
	      	  if ($i!=1) {
	    			  $val .= ',';
	     		  }
	     		  if ($value=='') {

		     		  $val .= 'NULL';
	     		  }
	     		  else {
			    		$val .=  $v;
	     		  }
	    	  }
	    	$value = $val;
      }
	    $value = trim($value);
	    $value =  "'".   pg_escape_string($value) ."'";
    }

    return $value;
  }
  /**
   * Destructor - Closes database connections.
   *
   */
  public function __destruct() {
    $db = $this->db;
    if ($db) {
      pg_close($db);
    }
  }
}