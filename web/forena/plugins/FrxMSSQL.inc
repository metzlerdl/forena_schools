<?php
// $Id$
/**
 * @file
 * Oracle specific driver that takes advantage of oracles native XML support
 *
 * In order to take advantage of XML support the following XML
 *
 */
class FrxMSSQL extends FrxDataProvider {


  private $db;
  private $use_mssql_xml;

  /**
   * Object constructor
   *
   * @param unknown_type $uri Database connection string.
   * @param string $repos_path Path to location of data block definitions
   */
  public function __construct($conf, $repos_path) {

    parent::__construct($conf, $repos_path);
    $this->use_mssql_xml = FALSE;
    $uri = $conf['uri'];
    $this->debug = $conf['debug'];
    if ($conf['mssql_xml']) $this->use_mssql_xml = TRUE;
    if ($uri) {
      // Test for mssql suport
      if (!is_callable('mssql_connect')) {
        FrxReportGenerator::instance()->error('MSSQL support not installed.', 'MSSQL mssql support not installed.');
        return;
      }
      try {
        $db = mssql_connect($uri, $conf['user'], $conf['password']);
        $this->db = $db;
        if ($db) mssql_select_db($conf['database'], $db);
      } catch (Exception $e) {
        FrxReportGenerator::instance()->error('Unable to connect to database ' . $conf['title'], $e->getMessage());
      }

    }
    else {
      FrxReportGenerator::instance()->error('No database connection string specified', 'No database connection: ' . print_r($conf, 1));
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
      if ($clause) {
        $sql = 'SELECT * FROM (' . trim($sql, ' ;') . ') ' . $clause;
      }
      $sql = $this->te->replace($sql);

      if ($this->use_mssql_xml) {
        $xml = $this->mssql_xml($sql, $block_name);
      }
      else {
        $xml = $this->php_xml($sql);
      }
      if ($this->debug) {
        if ($xml) $d = htmlspecialchars($xml->asXML());
        FrxReportGenerator::instance()->debug('SQL: ' . $sql, '<pre> SQL:' . $sql . "\n XML: " . $d . "\n</pre>");
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
  private function mssql_xml($sql, $block) {
      $db = $this->db;

      //$rs->debugDumpParams();
      $fsql = $sql . ' FOR XML AUTO';
      $rs = mssql_query($db, $fsql, array($sql, ''));
      if ($rs) {
        $row = mssql_fetch_row($rs);
        $xml_text = $row[0];
      }
      if ($xml_text) {
        $xml = new SimpleXMLElement($xml_text);
        if ($xml->getName() == 'error') {
          $msg = (string)$xml . ' in ' . $block . '.sql. ';
          FrxReportGenerator::instance()->error($msg . 'See logs for more info', $msg . ' in <pre> ' . $sql . '</pre>');
        }
      }
      if ($rs) mssql_free_result($rs);
      return $xml;
  }

  private function php_xml($sql) {
      $db = $this->db;
      $xml = new SimpleXMLElement('<table/>');

      $rs = mssql_query($sql);

      while ($row = mssql_fetch_assoc($rs)) {
        $row_node = $xml->addChild('row');
        foreach ($row as $key => $value) {
          $row_node->addChild(strtolower($key), htmlspecialchars($value));
        }
      }
      if ($rs) mssql_free_result($rs);
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
    if ($value=='') $value = 'NULL'; else $value = "'" . str_replace("'", "''", $value) . "'";
    return $value;
  }

    /**
   * Destructor - Closes database connections.
   *
   */
  public function __destruct() {
    $db = $this->db;
    if ($db) {
      mssql_close($db);
    }
  }
}