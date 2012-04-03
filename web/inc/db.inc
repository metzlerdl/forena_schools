<?php
/**
 * This will be a generic libarary of database functions that are used by the
 */


/**
 * factory for getting the applicatioins database connection.
 */
function db_get() {
  static $db = '';
  if (!$db) {
    $db = new pg_database();
  }
  return $db;
}


function db_query($sql,$values=array()) {
 GLOBAL $db_debug;
 $db = db_get();
 if ($db) return $db->query($sql,$values);
}

function db_query_xml($sql,$values = array(),$root_tag='',$message='') {
 $db = db_get();
 if ($db) return $db->query_xml($sql,$values,$root_tag,$message);
}

function db_call($fn_call,$values = array()) {

  $db = db_get();
  if ($db) return $db->call($fn_call,$values);
}

/**
 * Generate insert statment from table.
 * Returns empty string if the table is not found.
 * @param $table string table to use for insert generation
 * @return string Sql insert statement
 */
function db_gen_insert($table, &$tab_columns) {
  $rs = db_query('SELECT column_name FROM information_schema.columns WHERE table_name=:table ',array('table'=>$table));
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

/**
 * Generate the list of columns from a table
 * @param $table string Name of table to introspect
 * @return array Array of column names
 */
function db_column_list($table) {
  $rs = db_query('SELECT column_name FROM information_schema.columns WHERE table_name ={table}',array('table'=>$table));
  $col_list=array();

  while ($row = pg_fetch_row($rs)) {
  	 $col_list[] = strtolower($row[0]);
  }

  return $col_list;
}

function db_first_row($rs) {
   if ($rs) {
     $row = pg_fetch_assoc($rs);
     $retvar = $row;
   }
   return $retvar;
  }


function db_first_column($rs) {
if ($rs) {
    $row = pg_fetch_row($rs);
    $retvar = $row[0];
  }
  return $retvar;
}
function db_query_foreach($xml,$xpath,$sql,$test_xpath='',$false_query='') {
  $db = db_get();
  if ($db) $db->query_foreach($xml,$xpath,$sql,$test_xpath,$false_query);
}
/**
 * Check database connection returning the error message if it fails.
 *
 */
function db_check_connection_failed() {
	global $install_step;
	ob_start();
	// Execute a simple select 'x' to test the connection
	$db = db_get();
    $result = $db->test_connection();
	$error_text = ob_get_contents();
	ob_end_clean();
	if ($result === NULL) {
		$error_text .= 'Database Connection Failed';
	}
  $install_step=$result;

	return $error_text;
}
/**
 * Database logging procedure.  Writes a log entry to the database.
 *
 * @param String $title Short title of this log message
 * @param String $message Full tet of message
 * @param String $type Defaults to error
 */
function db_log($title,$message='',$type='error') {
	$user=current_user();
    ob_start();
    $db = db_get();
    $db->log($type,$user['user_id'],$title,substr($_SERVER['REMOTE_ADDR'],-15),$message);
    $error_text = ob_get_contents();
    ob_end_clean();
    if ($error_text) {
    	print "<pre>";
    	print "Failed to write to database logs";
    	print $error_text;
    	print "</pre>";
    }
}
/**
 * Database abstraction class for postgres
 */
class pg_database {

protected $c;
protected $sql_cmd;
protected $bind_order;
protected $token_exp = '/:([a-z]|[0-9]|{A-Z]|[-_])+/';

/**
 * Connect to databse using global connect string
 * return object Database connection object.
 */
function __construct() {
  global $db_connection_string;
  if (!$this->c) {
    $this->c = pg_connect($db_connection_string);
    if ($this->c) pg_query($this->c,"SET search_path=public,import");
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
    $retvar = $values[$key];
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
public function query($sql,$values=array()) {
  // Look for all parameter names and send back sql
  GLOBAL $db_debug;
  $sql = $this->prepare($sql);
  //print "$sql\n";
  // Extract values into single params variable
  $param_values = $this->get_bind_values($values);
  if ($db_debug) db_log('Query Executeed',$sql . "\n" . print_r($param_values,1),'debug');
  return pg_query_params($this->c,$sql,array_values($param_values));
}

/**
 * Performs a single function call expecting a single stream of data to be returned
 * This is usefull for getting xml data back from function calls.
 */
public function call($f_call,$values=array()) {
  $sql = 'select '.$f_call.';';
  $retvar = '';
  $rs = $this->query($sql,$values);
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
  GLOBAL $db_debug;
  $retvar='';

  // Prepend select * so that full table queries are easy
  if (strpos(strtolower($sql),'select')===false)
    $sql = 'select * from '. $sql;

  // Prepare statmeent to protect against sql injection.
  $sql = $this->prepare_xml($sql,$values);

  // Execute the query using query_to_xml
  if ($db_debug) db_log('Query Executeed',$sql . "\n" . print_r($param_values,1),'debug');

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
        $this->query($sql_cmd,$row_xml);
    } // foreach
  }
}

public function log($type,$user_id='',$title='',$ip='',$message='') {

  $sql = 'insert into logs(msg_type,user_id,title,ip,message) VALUES ($1,$2,$3,$4,$5)';
  // Extract values into single params variable
  $param_values = array($type,$user_id,$title,$ip,$message);
  //print_r($param_values);
  if ($this->c) pg_query_params($this->c,$sql,array_values($param_values));
}

public function test_connection() {
	$result = pg_query($this->c,"SELECT i_install_step()");
	return db_first_column($result);
}
// End of class
}