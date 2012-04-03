<?php
// $Id$
/**
 * @file
 * File class for loading raw xml files as Sample data
 * This class is used as a reference implementation of a data engine, but is
 * also used to provide sample data files and reports.
 * @author metzlerd
 *
 */
class FrxFiles extends FrxDataProvider {
  private $path;
  public function __construct($conf, $repos_path) {
    parent::__construct($conf, $repos_path);
    $this->comment_prefix = '<!--';
    $this->comment_suffix = '-->';
    $this->block_ext = 'xml';
    list($protocol, $path) = explode('://', $conf['uri'], 2);
    if (!$path) $path = $protocol;
    $this->path = $path;
  }

  public function getName() {
    return 'XML File';
  }

  public function data($block_name, $parm_data='', $subQuery='') {

     $block = $this->load_block($block_name);
    $xml ='';
    if (isset($block['source']) && $this->access($block['access'])) {
      try {
        $xmlData = $block['source'];
        $xml = new SimpleXMLElement($xmlData);
      } catch (Exception $e) {
        FrxReportGenerator::instance()->error("Error processing xml\n", $e->getMessage() . "\n" . $xmlData);
      }
    }

    //@TODO: Implment xpath subquery parameter
    // Should filter returned nodes, but within the original root element?
    return $xml;
  }

}