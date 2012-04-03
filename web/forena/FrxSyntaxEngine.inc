<?php
// $Id$
/**
 * @file
 * FrXSytnaxEngine defines how regular expression procesing/token substitution takes place.
 * It includes support for passing in a formatter oobject that will escape strings properly
 * before substituting them.
 *
 */
define('FRX_TOKEN_EXP', '/\{[^\n^\r^}]+}/');
define('FRX_SQL_TOKEN', '/:([a-z]|[0-9]|{A-Z]|[_])+/');
class FrxSyntaxEngine {
  private $tpattern;
  private $trim_chars;
  private $formatter; // Object used to format the data
  private $frxData ;
  /**
   * Class for doing syntax replacements;
   * @param $regexp
   * @return unknown_type
   */
  public function __construct($regexp, $trim, $formatter=NULL, $frxObject=NULL) {
    $this->tpattern = $regexp;
    $this->trim_chars = $trim;
    if (is_object($formatter)) {
      $this->formatter=$formatter;
    }
    if ($frxObject) {
    	$this->frxData = $frxObject;
    }
    else {
      $this->frxData = FrxData::instance();
    }

  }


  /**
   * Provides an api to the {=xpath} syntax that can be used
   * to evaluat expressions such as sum and count in a report.  We
   * need to use the DOM object here, because this method isn't exposed
   * with simplexml.
   *
   * @param $xml
   * @param $path
   * @return unknown_type
   */
  protected function simplexml_evaluate($xml, $path) {
    $dom_node = dom_import_simplexml($xml);
    $dom_doc = new DOMDocument('');
    $dom_node = $dom_doc->importNode($dom_node, TRUE);
    $dom_doc->appendChild($dom_node);
    // Do we also need to call AppendChild?
    $xpath = new DOMXpath($dom_doc);
    $ret = $xpath->evaluate($path, $dom_node);
    return $ret;
  }


  /**
  * Get the value from the data.
  * This is used by token_replace method to extract the data based on the path provided.
  * @param $data
  * @param $key
  * @return unknown_type
  */
  protected function get_value( $key, $raw=FALSE) {
    $retvar = '';
    // Default to theo current context
    $data = $this->frxData->currentContext();

    // Determine if we have a . syntax for the id.
    $raw_key=$key;
    if ($key && strpos($key, '.')) {
      @list($id, $path) = explode('.', $key, 2);
      if ($this->frxData->contextExists($id)) {
        $data = $this->frxData->getContext($id);
        $key = $path;
      }
    }

    if (is_array($data)) {
      $retvar = @$data[$key];
    }
    elseif (is_object($data)) {

      if (strpos($key, '=')===0) {
        $retvar = $this->simplexml_evaluate($data, ltrim($key, '='));
      }
      else {
        $rows = $data->xpath($key);
        $x ='';
        if ($rows) $x = $rows[0];

        if ($x) $retvar = $x->asXML();
        // Check to see if there are child nodes
        // If so use asXML otherwise string cast.
        if ($retvar && strpos($retvar, '<')!==FALSE) {
          // Find the end of the first tag.
          $p = strpos($retvar, '>');
          $retvar = substr_replace($retvar, '', 0, $p+1);
          $p = strrpos($retvar, '<', -1);
          $retvar = substr_replace($retvar, '', $p, strlen($retvar) - $p);

        }
        else {
          $retvar = (string)$x;
        }
      }

    }
    // Call the formatter object if neccessary
    $f = $this->formatter;
    if (!$raw && is_object($f) && method_exists($f, 'format')) {
      $retvar = $f->format($retvar, $raw_key, $data);
    }

    $retvar = trim((string)$retvar);
    return $retvar;
  }


  /**
   *
   * @param $text text that needs replacing
   * @param $data
   * @return unknown_type
   */
  public function replace($text, $raw=FALSE) {
  	if (is_array($text)) {
  	  foreach ($text as $key => $value) {
  		  $text[$key] = $this->replace($value, $raw);
  	  }
      return $text;
  	}
  	//Otherswise assume text
    $match=array();
    $o_text = $text;
    // Put the data on the stack.
    if (preg_match_all($this->tpattern, $o_text, $match)) {
      //list($params) = $match[1];
      $i=0;

      foreach ($match[0] as $match_num => $token) {
         $path = trim($token, $this->trim_chars);
         $value = $this->get_value($path, $raw);

          $pos = strpos($text, $token);
          if ($pos !== FALSE) {
            $text = substr_replace($text, $value, $pos, strlen($token));
          }
      }
    }
    return $text;
  }

  /**
   * List all of the tokens used in a piece of text, ignoring duplicates.
   *
   * @param string $text
   * @return array tokens contained in the text according to the regular expression.
   */
  public function tokens($text) {
   $match=array();
   $tokens = array();

    if (preg_match_all($this->tpattern, $text, $match)) {
      $i=0;
      foreach ($match[0] as $match_num => $token) {
         $path = trim($token, $this->trim_chars);
         if (array_search($path, $tokens)===FALSE) {
           $tokens[] = $path;
         }
      }
    }
    return $tokens;
  }
}
