<?php
// $Id$
/**
 * @file
 * Provides data blocks for native drupal connections using the default
 * drupal connections.
 *
 */
class FrxDrupal extends FrxDataProvider {
  /**
	 * Implements hooks into the drupal applications
	 */

  private $db;

  /**
   * Object constructor
   *
   * @param unknown_type $uri Database connection string.
   * @param string $repos_path Path to location of data block definitions
   */
  public function __construct($conf, $repos_path) {
    parent::__construct($conf, $repos_path);

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
    $block = $this->load_block($block_name, $clause);
    $xml ='';
    if ($block['source'] && $this->access($block['access'])) {
      $sql = $block['source'];
      if ($clause) {
        $sql = 'SELECT * FROM (' . trim($sql, ' ;') . ') forena_table ' . $clause;
      }
      $sql = $this->te->replace($sql);
      $rs = db_query($sql);

      $xml = new SimpleXMLElement('<table/>');
      foreach ($rs as $data) {
          $row_node = $xml->addChild('row');

          foreach ($data as $key => $value) {
            $row_node->addChild($key, htmlspecialchars($value));
            //$row_node->$key = $value;
          }
      }

    }

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
    $db = Database::getConnection('default');
    if ($db) {
      if ($value=='') $value = 'NULL'; else  $value =   $db -> quote($value);
    }
    return $value;
  }

  /**
   * @section
   * Formatters
   * Below here are formatting types
   */
  public function formats() {
    $formats = array('drupal_filter' => 'Drupal Input Filter'
                     );
    return $formats;
  }


  public function drupal_filter($nid, $format) {
    $node = node_load($nid, NULL, TRUE);
    if (!$node) {
      return 'N/A';
    }
    if ((strcasecmp($format, 'teaser') == 0) || (!$format)) {
      $bool = TRUE;
    }
    else {
      $bool = FALSE;
    }
    //If true, render the teaser
    //else render the body
    $node = node_prepare($node, $bool);
    if ($bool) {
      return $node->teaser;
    }
    return $node->body;
  }
}













