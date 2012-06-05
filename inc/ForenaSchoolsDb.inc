<?php
/**
 * This will be a generic libarary of database functions that are used by the
 */


/**
 * Database abstraction class for postgres
 */
class ForenaSchoolsDB {

protected $c;
protected $sql_cmd;
protected $bind_order;
protected $rs;
protected $token_exp = '/:([a-z]|[0-9]|{A-Z]|[-_])+/';
public $debug = TRUE;

/**
 * Connect to databse using global connect string
 * return object Database connection object.
 */
function __construct() {

  if (!$this->c) {
    $this->c = pg_connect(forena_schools_connection_string());
    if ($this->c) pg_query($this->c,"SET search_path=public,import");
  }
}

/**
 * Factory instance
 * Use to return instance of connection.
 */
static function instance() {
	static $db = '';
	if (!$db) {
		$db = new ForenaSchoolsDB();
	}
	return $db;
}


function first_row($rs) {
   if ($rs) {
     $row = pg_fetch_assoc($rs);
     $retvar = $row;
   }
   return $retvar;
  }


function first_column($rs) {
if ($rs) {
    $row = pg_fetch_row($rs);
    $retvar = $row[0];
  }
  return $retvar;
}

function all_rows($rs) {
	$ret = array();
	if ($rs) {
		$ret = pg_fetch_all($rs);
	}
	return $ret;
}

/**
 *
 * Load a file and execute ...
 * @param String $filename
 */
public function executeFile($filename) {
	$filename = drupal_get_path('module', 'forena_schools') . '/' . $filename;

	if (file_exists($filename)) {
    $sql = file_get_contents($filename);
    //$sql = trim($sql,';');
    $this->exec_query($sql);
	}
	else {
		drupal_set_message('File not found: ' .  $filename, 'error');
	}
}

/**
* Prepare a sql statment based on lightly typed bind variables.
* The bind variables can containg the following format:
*    {Bind variable}  The variable to bind to.
*  @param string $sql The sql command to prepare
*/
protected function prepare($sql)
{
  $match=array();
  $bind_order = array();
  $sql_cmd = $sql;
  if (preg_match_all($this->token_exp,$sql,$match))
  {
    //list($params) = $match[1];
    $i=0;
    foreach($match[0] as $param_num=>$param)
    {
       //* @TODO Rebuild the array into one that can be handled by pg_prepare()
       //$sql = str_replace($sql,$parm,$values[$idx]);
       $i++;
       $p = strpos($sql_cmd,$param);
       $parm_num = '$'.$i;
       $sql_cmd = substr_replace($sql_cmd,$parm_num,$p,strlen($param));
       $bind_order[] = trim($param,':');
    }

  }
  // Save the prepared statement info
  $this->bind_order = $bind_order;
  $this->sql_cmd = $sql_cmd;

  return $sql_cmd;
}

/**
* Return the variable for bind values
* @param  mixed $values Array or simplexml object containing values to extract
* @param  mixed $key    The key or xpath used to extract the item.
*/
protected function get_value($values,$key)
{
  $retvar = '';
  if (is_array($values))
  {
    $retvar = @$values[$key];
  }
  elseif (is_object($values)) {
    $retvar =$values->$key ->asXML();
     // Replace the root tag if we want to
    if ($retvar) {
      // Find the end of the first tag.
      //print $root_tag;
      $p = strpos($retvar,'>');
      $retvar = substr_replace($retvar,' ',0,$p+1);
      $p = strrpos($retvar,'<',-1);
      $retvar = substr_replace($retvar,'',$p,strlen($retvar) - $p);
    }
  }
  $retvar = trim($retvar);
  // Don't use '' but rather null instead
  if ($retvar==='')
    $retvar = null;
  return $retvar;
}

/**
 * Extract values from the array in the order that they appeared in the sql.
 * @values mixed Array or SimpleXML object containing parameters.
 * @return Array Bind values
 */
protected function get_bind_values($values) {
  $bind_order = $this->bind_order;
  $bind_values = array();
  // if it is an object, assume SimpleXML and extract based on xpath.
  if (is_object($values)||is_array($values)) {
    foreach( $bind_order as $bind_var) {
      $bind_values[] =$this->get_value($values,$bind_var);
    }
  }

  return $bind_values;
}

/**
 * Execute a safe SQL query and get the recordset in reponse
 * @param  string  $sql    SQL statement with {var} syntax bind variables
 * @param  array   $params A key/value array of parameter replacements
 * @return object  postgres recordset otbject
 */
public function exec_query($sql,$values=array()) {
  // Look for all parameter names and send back sql
  $sql = $this->prepare($sql);
  //print "$sql\n";
  // Extract values into single params variable
  $param_values = $this->get_bind_values($values);
  if ($this->debug) $this->log('Query:',$sql . "\n" . print_r($param_values,1),'debug');
  return pg_query_params($this->c,$sql,array_values($param_values));
}

/**
 * Execute a safe SQL query and get the recordset in reponse
 */
public function raw_query($sql) {
  if ($this->debug) $this->log('Query Executeed: ',$sql,'debug');
  return pg_query($this->c,$sql);
}

public function query($sql, $values=array()) {
	return $this->all_rows($this->exec_query($sql, $values));
}

/**
 * Performs a single function call expecting a single stream of data to be returned
 * This is usefull for getting xml data back from function calls.
 */
public function call($f_call,$values=array()) {
  $sql = 'select '.$f_call.';';
  $retvar = '';
  $rs = $this->exec_query($sql,$values);
  if ($rs) {
    $row = pg_fetch_row($rs);
    $retvar = $row[0];
  }
  return $retvar;
}

/**
* Prepare a sql statment based on lightly typed bind variables.
* The bind variables can containg the following format:
*    {Bind variable}  The variable to bind to.
*/
public function prepare_xml($sql,$values)
{
  $match=array();
  $bind_order = array();
  $sql_cmd = $sql;
  if (preg_match_all($this->token_exp,$sql,$match))
  {
    //list($params) = $match[1];
    $i=0;
    foreach($match[0] as $param_num=>$param)
    {
       //* @TODO Rebuild the array into one that can be handled by pg_prepare()
       //$sql = str_replace($sql,$parm,$values[$idx]);
       $i++;
       $p = strpos($sql_cmd,$param);
       $parm_num = '$'.$i;
       $key = trim($param,':');
       $value = $this->get_value($values,$key);

       if ($value=='') {
         $value = 'NULL';
       }
       else {
	       $value = "'".pg_escape_string($value)."'";
       }
       $sql_cmd = substr_replace($sql_cmd,$value,$p,strlen($param));
    }

  }
  // Save the prepared statement info
  $this->bind_order = $bind_order;
  $this->sql_cmd = $sql_cmd;

  return $sql_cmd;
}

/**
* Execute a query and obtain recordset results as XML
* @param string $sql       The sql command to execute using {} Bind syntax
* @param mixed  $values    The value
* @param string $root_tag  The root tag to use to replace the table tag with
*/
public function query_xml($sql, $values=array(),$root_tag='',$message='') {
  $retvar='';

  // Prepend select * so that full table queries are easy
  if (strpos(strtolower($sql),'select')===false)
    $sql = 'select * from '. $sql;

  // Prepare statmeent to protect against sql injection.
  $sql = $this->prepare_xml($sql,$values);

  // Execute the query using query_to_xml
  if ($this->debug) $this->log('Query Executeed',$sql . "\n" . print_r($values,1),'debug');

  $rs = pg_query_params($this->c,'select query_to_xml($1,true,false,$2);',array($sql,''));
  if ($rs) {
    $row = pg_fetch_row($rs);
    $retvar = $row[0];
  }

  if ($message) {
  	$msgAttr = ' message="'. htmlspecialchars($message) .'" ';
  } else $msgAttr='';

  // Replace the root tag if we want to
  if ($root_tag) {
    if ($retvar) {
      // Find the end of the first tag.
      //print $root_tag;
      $p = strpos($retvar,'>');
      $retvar = substr_replace($retvar,'<'.$root_tag .$msgAttr .' xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ',0,$p);
      $p = strrpos($retvar,'<',-1);
      $retvar = substr_replace($retvar,'</'. $root_tag .'>',$p,strlen($retvar) - $p);
    }
    else $retvar = "<$root_tag/>";
  }

  return $retvar;

}

/**
 * Execute a query for each row as identifeid by an xpath expression.
 * @param simpleXML $xml   The xml object to iterate through
 * @param string    $xpath Xpath expressions that defines the rows to iterate through.
 * @param string    $sql   The sql command to execute.
 * @param string    $test_xpath Optional xpath query test.
 * @param string    $false_query Optional SQL to execute if test returns false
 * This query will perform a sql query for every row defined by the xpath query.  This is useful for
 * performing bulk inserts or updates to a database when the xml object represents multiple rows
 * in the database.
 *
 * If the optional $test_xpath and $false_query parameters are passed, this fucntion will run the $false_query
 * instead of $sql when the $test_xpath expression returns no rows.  This can be userful for doing conditional
 * insert vs. update logic based on the presence of some id field contained in the row.
 */
public function query_foreach($xml,$xpath,$sql,$test_xpath='',$else_query='') {
  if ($xml && $xpath) {
    // If it isn't already parsed, parse it into simplexml object.
    if (!is_object($xml)) $xml=new SimpleXMLElement($xml);

    $rows = $xml->xpath($xpath);
    foreach((array)$rows as $row_xml) {
      $sql_cmd = $sql;

      // run the test if it is defined
      if ($test_xpath) {
        $test = $this->get_value($row_xml,$test_xpath);
         if (!(string)$test)
          $sql_cmd = $else_query;
      }

      // If we've still got sql after the test, execute it.
      if ($sql_cmd)
        $this->exec_query($sql_cmd,$row_xml);
    } // foreach
  }
}

public function log($title='', $message='', $type = 'debug') {

  watchdog($type, $title . '<br/> <pre>'.  $message . '</pre>');
  // Extract values into single params variable
}

/**
 * Generate insert statment from table.
 * Returns empty string if the table is not found.
 * @param $table string table to use for insert generation
 * @return string Sql insert statement
 */
function gen_insert($table, &$tab_columns) {
  $rs = $this->exec_query('SELECT column_name FROM information_schema.columns WHERE table_name=:table ',array('table'=>$table));
  $col_list='';
  $val_list='';
  while ($row = pg_fetch_row($rs)) {
  	 $col_name = strtolower($row[0]);
  	 $col_list .= $col_name . ',';
  	 $val_list .= ':'.$col_name.',';
  	 $tab_columns[]=$col_name;
  }
  $sql = '';
  if ($col_list) {
    $sql = 'INSERT INTO '.$table.' ('.trim($col_list,',') .') VALUES ('.trim($val_list,',') .')';
  }
  return $sql;
}


public function test_connection() {
	$result = pg_query($this->c,"SELECT i_install_step()");
	return db_first_column($result);
}
// End of class
}