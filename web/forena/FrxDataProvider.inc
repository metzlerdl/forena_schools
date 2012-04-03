<?php
// $Id$
require_once('forena.common.inc');
/**
 * @file
 * Class that defines default methods for access control in an FrxDataProvider
 *
 */
class FrxDataProvider{

  public $conf;
  public $block_path;
  public $comment_prefix;
  public $comment_suffix;
  public $block_ext;
  protected $te;
  public function __construct($conf, $repos_path) {
    $this->conf = $conf;
    $this->comment_prefix = '--';
    $this->block_ext = 'sql';
    $this->block_path = $repos_path;
    $this->app = FrxReportGenerator::instance()->app;
  }

  /**
   * Implements the basic default security check of calling
   * an access method.
   *
   * @param unknown_type $arg
   * @return unknown
   */
  public function access($arg) {
    $f = @$this->conf['access callback'];
    if ($f && is_callable($f)) {
      return $f($arg);
    }
    elseif (isset($this->conf['access block'])) {
      $block = @$this->conf['access block'];
      $path='';
      if (isset($this->conf['access path'])) $path = $this->conf['access path'];
      return FrxReportGenerator::instance()->block_access($block, $path, $arg);
    }
    else {
      return user_access('access content');
    }
  }


  /**
   * Default block load
   * Loads the data block based on the block name from the file system.  The classes that
   * are derived from this will set the block_ext property, which in most cases is .sql but
   * might be something different.   The load of the block file should return data, access and
   * probably parameters to.
   * @param unknown_type $block_name
   * @return unknown
   */
  public function load_block($block_name, $clause='') {

    $filename = $this->block_path . '/' . $block_name . '.' . $this->block_ext;

    $contents = FrxReportGenerator::instance()->load_block_file($filename);
    $block = $this->parseXMLFile($contents);
    // If we have a regular expression token parser, then get the tokens out of the block.

    if ($this->te) {
      $tokens = @$this->te->tokens($block['source']);
      //drupal_set_message("tokens: ". print_r($tokens, 1));
      //check tokens in the where clause
      if ($clause) {
        $clause_tokens = $this->te->tokens($clause);
        //drupal_set_message("clause tokens: ". print_r($clause_tokens, 1));
        $temp = array();
        $temp = array_merge($tokens, $clause_tokens);
        //check for duplicates in block tokens
        if ($clause_tokens) foreach ($clause_tokens as $ct) {
          if (!isset($temp[$ct])) {
            array_push($tokens, $ct);
          }
        }
      }

      $block['tokens'] = $tokens;
    }

    return $block;
  }

  /**
   * Find all the blocks matching a provided search string
   *
   * @param string $search part block names to search for
   * @return unknown
   */
  public function list_blocks($search, $subdir='', $this_list=array()) {
    $block_list = $this_list;
    // First find files that match the search string
    $path = $this->block_path . '/';
    if ($subdir) $path = $subdir . '/';
    $block_path .= $path . '*' . $search . '*.' . $this->block_ext;

    // Find sql files
    $d = glob($block_path);
    if ($d) foreach ($d as $file_name) {
    	// Split off the extention
    	$p = strripos($file_name, '.');
      if ($p!==FALSE) {
  	    $ext = substr($file_name, $p+1);
  	    $block_name = substr($file_name, 0, $p);
      }
      else {
      	$ext = '';
      	$block_name = $file_name;
      }
    	$block_list[] = str_replace($this->block_path . '/', '', $block_name);
    }

    // Find directories
    $d = glob($path . '*');
    if ($d) foreach ($d as $dir_name) {
      if (is_dir($dir_name)) {
        $block_list += $this->list_blocks($search, $dir_name, $block_list);
      }
    }

    return $block_list;
  }

  public function debug($msg='', $log='') {
  	FrxReportGenerator::instance()->app->debug($msg, $log);
  }

  public function error($msg='', $log='') {
  	FrxReportGenerator::instance()->app->error($msg, $log);
  }

  public function parseXMLFile($contents) {
  	$comment = $this->comment_prefix;
  	$trim = $this->comment_suffix;
    $lines = explode("\n", $contents);
    $cnt = count($lines);
    $access = '';
    $i=0;
    $block = '';
    $data = '';
    while ($i<$cnt) {
      $l = trim($lines[$i], "\r");
      @list($d, $c) = explode($comment, $l, 2);
      if ($trim) $c = trim($c, $trim);
      if  ($c) {
        list($a, $o) = explode('=', $c, 2);
        $a = trim($a);
        if ($a && $o) {
	        switch ($a) {
	        	case 'ACCESS':
	        		$access = trim($o);
	        		break;
	        	default:


        }

        }

      }
      if (strpos($l, $comment)!==0) {
        $data .= "$l\n";
      }
      $i++;
    }
    return array('access' => $access, 'source' => $data);
  }

  public function parseSQLFile($contents) {
  	$comment = $this->comment_prefix;
  	$trim = $this->comment_suffix;
    $lines = explode("\n", $contents);
    $cnt = count($lines);
    $access = '';
    $i=0;
    $block = '';
    $parms = FrxData::instance()->currentContext();
    $data = '';
    while ($i<$cnt) {
      $l = trim($lines[$i], "\r");
      @list($d, $c) = explode($comment, $l, 2);
      if ($trim) $c = trim($c, $trim);
      if  ($c) {
        list($a, $o) = explode('=', $c, 2);
        $a = trim($a);
        if ($a && $o) {
	        switch ($a) {
	        	case 'ACCESS':
	        		$access = trim($a);
	        		break;
	        	default:

        }

        }

      }
      if (strpos($l, $comment)!==0) {
        $data .= "$l\n";
      }
      $i++;
    }


  }


}
