<?php
// $Id$
/**
 * @file
 * Oracle specific driver that takes advantage of oracles native XML support
 *
 * In order to take advantage of XML support the following XML
 *
 */
class FrxOracle extends FrxDataProvider {


  private $db;
  private $use_oracle_xml;
  private $schema;

  /**
   * Object constructor
   *
   * @param unknown_type $uri Database connection string.
   * @param string $repos_path Path to location of data block definitions
   */
  public function __construct($conf, $repos_path) {

    parent::__construct($conf, $repos_path);
    $this->use_oracle_xml = FALSE;
    $uri = $conf['uri'];
    $this->debug = $conf['debug'];
    if (isset($conf['schema'])) $this->schema = $conf['schema'];
    if ($conf['oracle_xml']) $this->use_oracle_xml = TRUE;
    if ($uri) {
         // Test for postgres suport
      if (!is_callable('oci_connect')) {
        $this->error('OCI support not installed.', 'OCI support not installed.');
        return;
      }
      try {
        $db = oci_connect($conf['user'], $conf['password'], $uri, @$conf['character_set']);
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
    if ($block['source'] && $this->access($block['access']) && $db) {
      $sql = $block['source'];
      // See if this block matches a declare begin end; syntax.
      if (stripos($sql, 'end;')>= stripos($sql, 'begin') && stripos($sql, 'begin')!==FALSE) {
        $this->call($sql, $params, array('return' => 'clob'));
        $xml = $params['return'];
        if ($xml) $xml = new SimpleXMLElement($xml);
      }
      else {
        if ($clause) {
          $sql = 'SELECT * FROM (' . trim($sql, ' ;') . ') ' . $clause;
        }
        $sql = $this->te->replace($sql);

        if ($this->use_oracle_xml) {
          $xml = $this->oracle_xml($sql, $block_name);
        }
        else {
          $xml = $this->php_xml($sql);
        }
      }
      if ($this->debug) {
        $d = ($xml) ? htmlspecialchars($xml->asXML()) : '';
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
  private function oracle_xml($sql, $block) {
      $db = $this->db;

      //$rs->debugDumpParams();
      $fsql = 'declare x XMLTYPE; begin x := f_forena_xml(:p1); :ret_val := x.getClobVal();  end; ';
      $stmt = oci_parse($db, $fsql);
      $ret = oci_new_descriptor($db, OCI_D_LOB);
      oci_bind_by_name($stmt, ':ret_val', $ret, -1, OCI_B_CLOB);
      oci_bind_by_name($stmt, ':p1', $sql);
      $r = oci_execute($stmt, OCI_DEFAULT);
      // Report errors
      if (!$r) {
        $e = oci_error($stmt);  // For oci_execute errors pass the statement handle
        $msg .=  htmlentities($e['message']);
        $msg .= "\n<pre>\n";
        $msg .= htmlentities($e['sqltext']);
        //printf("\n%".($e['offset']+1)."s", "^");
        $msg .= "\n</pre>\n";
        $this->error('Database error in ' . $block . ' see logs for info', $msg);
        return NULL;
      }

      $xml_text = $ret->load();
      if ($xml_text) {
        $xml = new SimpleXMLElement($xml_text);
        if ($xml->getName() == 'error') {
          $msg = (string)$xml . ' in ' . $block . '.sql. ';
          $this->error($msg . 'See logs for more info', $msg . ' in <pre> ' . $sql . '</pre>');
        }
      }
      oci_free_statement($stmt);
      return $xml;
  }

  private function php_xml($sql) {
      $db = $this->db;
      $xml = new SimpleXMLElement('<table/>');

      //$rs->debugDumpParams();
      $stmt = oci_parse($db, $sql);
      oci_execute($stmt);

      while ($row = oci_fetch_array($stmt, OCI_ASSOC+OCI_RETURN_NULLS)) {
        $row_node = $xml->addChild('row');
        foreach ($row as $key => $value) {
          $row_node->addChild(strtolower($key), htmlspecialchars($value));
        }
      }
      oci_free_statement($stmt);
      return $xml;
  }

    /**
   * @param $sql string sql script containing function
   * @param $data array parameter array.
   * @param $types array containting types
   * @return oci statement
   * Call a pl/sql block of code.
   * The code snippet is expected to contain a begin/end data block as well as
   * any variable binding that is necessary.  Note that bind variables should appear
   * only once in the calling code.
   */
  public function call($sql, &$data, $types = array()) {
    $db = $this->db;
    $begin_end_block = stripos($sql, 'begin')===FALSE ? FALSE : TRUE;
    $match=array();
    if ($db) $stmt = oci_parse($db, $sql);
    if ($stmt) {
      if (preg_match_all(FRX_SQL_TOKEN, $sql, $match)) {
        //list($params) = $match[1];
        $i=0;
        foreach ($match[0] as $match_num => $token) {
           $name = trim($token, ':');
           $value = $data[$name];

           list($type, $subtype) = explode(' of ', $types[$name], 2);
           // Default varchar
           if (!$type )$type = 'varchar';
           switch (strtolower($type)) {
             // Handle arrays based on subtype.
             case 'array':
               $value = (array) $value;
               $bind_type = $this->oci_bind_type($subtype);
               $entries = count($value);
               if (!count($value)) {
                  $entries = 255;
               }
               oci_bind_array_by_name($stmt, $token, $value, $entries, -1, $bind_type);
               break;

             case 'clob':
               $c = oci_new_descriptor($db, OCI_D_LOB);
               $lobs[$name] = $c;
               oci_bind_by_name($stmt, $token, $lobs[$name], -1, OCI_B_CLOB);
               if (is_object($c)) {
                 $c->writeTemporary($value);
               }
               break;
             case 'number':
             case 'numeric':
             case 'varchar':
               oci_bind_by_name($stmt, $token, $data[$name], 32767);
               break;
             default:

               $o = oci_new_collection($db, strtoupper($type), $this->schema);
               $value = (array)$value;
               $collections[$name] = $o;
               if ($value && $o) foreach ($value as $element) {
                 $o->append($element);
               }
               oci_bind_by_name($stmt, $token, $o, -1, OCI_B_NTY);
               break;
           }
        }
      }

      // putting the @ operator before the oci_execute call will suppress php warnings.
      try {
       $r = @oci_execute($stmt, OCI_DEFAULT);

       // Report errors
       if (!$r) {
        $e = oci_error($stmt);  // For oci_execute errors pass the statement handle
        //drupal_set_message(e_display_array($e));
        if ($e['code'] != '1403') {
         $msg .=  htmlentities($e['message']);
         $msg .= "\n<pre>\n";
         $msg .= htmlentities($e['sqltext']);
         $msg .= "\n</pre>\n";
         ora_error('', $msg);
        }
       }
      }
      catch (Exception $e) {
         $msg .=  htmlentities($e['message']);
         $msg .= "\n<pre>\n";
         $msg .= htmlentities($e['sqltext']);
         //printf("\n%".($e['offset']+1)."s", "^");
         $msg .= "\n</pre>\n";
         ora_error('Database error, see logs for info', $msg);
       }

      // Retrieve any clob data.
      if ($lobs) foreach ($lobs as $name => $lob) {
        if (is_object($lob)) {
          $data[$name] = $lob->load();
          $lob->free();
        }
      }

      // Free any collections
      if ($collections) foreach ($collections as $col) {
        if ($col) $col->free();
      }
      if (!$begin_end_block) {
        $rows = array();
        $rows = oci_fetch_all($stmt, $return, NULL, NULL, OCI_FETCHSTATEMENT_BY_ROW );
      }

      oci_free_statement($stmt);
    }

    return $return;
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
    if ($value=='' || $value ==array())
      $value = 'NULL';
    else {
      if (is_array($value)) {
      	if ($value == array()) {
      		$value = 'NULL';
      	}
      	else {
      		// Build a array of values string
	      	$i=0;
	      	foreach ($value as $v) {
	      		$i++;
	      	  if ($i==1) {
	      			$val .= '(';
	      		}
	     		  else {
	    			  $val .= ',';
	     		  }
	    		$val .=  "'".  str_replace("'", "''", $v) ."'";
	    	  }
	    	$value = $val . ')';
        }
      }
      else {
        $value = trim($value);
        $value =  "'".  str_replace("'", "''", $value) ."'";
      }
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
      oci_close($db);
    }
  }



}