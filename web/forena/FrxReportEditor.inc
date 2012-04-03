<?php
/**
 * Wrapper XML class for working with DOM object.
 * It provides helper
 * Enter description here ...
 * @author metzlerd
 *
 */
class FrxReportEditor {
  public $dom;
  public $document_root;
  public $simplexml;
  public $title;
  public $doc_prefix = '<?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE root [
    <!ENTITY nbsp "&#160;">
    ]>';
  public $xmlns = 'urn:FrxReports';
  private $xpq;

  /**
   * Create initial FRX report
   * Enter description here ...
   * @param unknown_type $xml_string
   */
  public function __construct($xml_string = '') {
    $this->dom = new DOMDocument('1.0', 'UTF-8');
    $dom = $this->dom;

    // Load a new one or build the empty XML Document
    if ($xml_string) {
    	// Eliminate the xml headers that don't have encoding info
    	$xml_string = str_ireplace('<?xml version="1.0"?>','',$xml_string);
      // If the <?xml code is missing assume UTF-8
      if (strpos($xml_string, '<?xml')===FALSE) {
        $xml_string = $this->doc_prefix . "\n" . $xml_string;
      }
    }
    else {
      $xml_string = $this->doc_prefix .
        '<html xmlns:frx="urn:FrxReports"><head/><body/></html>';
    }
    libxml_use_internal_errors();
    try {
      $dom->loadXML($xml_string);
    }
    catch (Exception $e) {
      forena_error('Invalid or malformed report document', '<pre>' .
        $e->getMessage() . $e->getTraceAsString() . '</pre>');
    }

    $this->verifyHeaderElements();
    $tnodes = $dom->getElementsByTagName('title');
    if ($tnodes->length) $this->title = $tnodes->item(0)->textContent;
    $this->document_root = $dom->documentElement;
    $this->simplexml = simplexml_import_dom($dom);
    $dom->formatOutput = TRUE;
    $this->xpq = new DOMXPath($dom);
    $this->xpq->registerNamespace('frx', $this->xmlns);
  }

  /**
   * Report the root element
   * Enter description here ...
   */
  public function asXML() {
    $dom =  $this->dom;
    $dom->formatOutput = TRUE;
    return $dom->saveXML();
  }

  /**
   * Set the value of an element within the report
   * @param String $xpath Xpath to element being saved
   * @param string $value Value to be saved.
   * @return unknown_type
   */
  public function setValue($xpath, $value) {
    $xml = $this->simplexml;
    $i = strrpos($xpath, '/');
    $path = substr($xpath, 0, $i);
    $key = substr($xpath, $i+1);
    $nodes = $xml->xpath($path);
    if ($nodes) {
      // if the last part of the xpath is a key then assume the key
      if (strpos($key, '@')===0) {
        $key = trim($key, '@');
        if (is_null($value)) {
          unset($nodes[0][$key]);
        }
        else {
          $nodes[0][$key] = $value;
        }
      }
      // We must be refering to the text element of a node.
      else {
        if (is_null($value)) {
          unset($nodes[0]->$key);
        }
        else {
          $nodes[0]->$key = $value;
        }
      }
      return TRUE;
    }
    else {
    	return FALSE;
    }
  }

  /**
   * Set the value of the body of the report
   * Will parse and set the value of the body of the report
   * using XML
   * @param html $body
   */
  public function setBody($body) {
  	$dom = $this->dom;
  	$nodes = $dom->getElementsByTagName('body');
  	$cur_body = $nodes->item(0);
  	// Make sure that we have a body tag.
    if (strpos($body, '<body')===FALSE) {
    	$body = '<body>' . $body . '</body>';
    }
    // Attempt to parse the xml
    $body_doc = new DOMDocument('1.0', 'UTF-8');
    $body_xml = $this->doc_prefix . '<html xmlns:frx="'. $this->xmlns.'">' . $body . '</html>';
    try {
      $body_doc->loadXML($body_xml);
      $new_body = $dom->importNode($body_doc->getElementsByTagName('body')->item(0), TRUE);
      $parent = $cur_body->parentNode;
      $parent->replaceChild($new_body, $cur_body);
    }
    catch (Exception $e) {

    	forena_error('Malformed report body', '<pre>' . $e->getMessage() .
    	  $e->getTraceAsString() . '</pre>');
    }
  }
  /**
   * Makes sure that the normal header elements for a report are there.
   * Enter description here ...
   */
  public function verifyHeaderElements($required_elements = array()) {
  	if (!$required_elements) $required_elements = array(
  	  'category',
  	  'options',
  	  'fields',
  	  'parameters',
  	  'docgen',
  	);
    $dom = $this->dom;
    $head = $dom->getElementsByTagName('head')->item(0);
    // Make sure the report title exists.
    if ($dom->getElementsByTagName('title')->length==0){
       $n = $dom->createElement('title');
       $head->appendChild($n);
    }
    // Make sure each of these exists in the header
    foreach ($required_elements as $tag) {
	    if($dom->getElementsByTagNameNS($this->xmlns, $tag)->length ==0 ) {
	      $n = $dom->createElementNS($this->xmlns, $tag);
	      $head->appendChild($n);
	    }
  	}
  }

  /**
   * Genreal utility for setting data in the header of a reprot
   *
   * @param string $parent Name of parent element
   * @param string $element Name of child element
   * @param array $element_array Data containing the elements
   * @param array $attributes array of attribute names to set
   * @param string $element_field name of field containing node data
   * @param unknown_type $id_field name of field containint node id
   */
  public function setFrxHeader($parent, $element, $element_array, $attributes, $element_field='', $id_field = 'id') {
  	$dom = $this->dom;
  	$xpq = $this->xpq;
  	$this->verifyHeaderElements(array($parent));
  	$pnode = $dom->getElementsByTagNameNS($this->xmlns, $parent)->item(0);

  	// Iterate through all child arrays in the header
  	$tnode = $dom->createTextNode("\n");
    $pnode->appendChild($tnode);
  	foreach ($element_array as $element_data) {
      $id = $element_data[$id_field];
      $path = '//frx:' . $parent . '/frx:' . $element . '[@' . $id_field . '="' . $id . '"]';
      $nodes = $xpq->query($path);
  	  $value = null;
      if ($element && isset($element_data[$element_field])) {
        $value = $element_data[$element_field];
      }

      $node = $dom->createElementNS($this->xmlns, $element,trim($value,"|"));
      if ($nodes->length == 0) {
      	$tnode = $dom->createTextNode("  ");
      	$pnode->appendChild($tnode);      	$pnode->appendChild($node);
      	$tnode = $dom->createTextNode("\n");
      	$pnode->appendChild($tnode);
      }
      else {
      	$src_node = $nodes->item(0);
      	$pnode->replaceChild($node, $src_node);
      }

      foreach ($attributes as $attribute) {
      	if (!empty($element_data[$attribute])) {
      		$node->setAttribute($attribute, $element_data[$attribute]);
      	}
      	else {
      		if ($node->hasAttribute( $attribute)) {
      			$node->removeAttribute($attribute);
      		}
      	}
      }
  	}



  }

  /**
   * Builds the fields from an array of elements.
   * Enter description here ...
   * @param $fieldElements
   */
  public function setFields($fieldElements) {
  	$dom = $this->dom;
    $newFields = $dom->createElementNS($this->xmlns, 'fields');
  	$this->verifyHeaderElements(array('fields'));
  	$fnode = $dom->getElementsByTagNameNS($this->xmlns, 'fields')->item(0);
  	$p = $fnode->parentNode;
  	$p->replaceChild($newFields, $fnode);

    $this->setFrxHeader('fields', 'field',
      $fieldElements,
      array('id', 'link', 'format', 'format-string', 'target'),
      'default');

  }

  /**
   * Set document generation types that apply to this report.
   * Enter description here ...
   * @param unknown_type $docgenElements
   */
  public function setDocgen($docgenElements) {
  	$dom = $this->dom;
    $newDocs = $dom->createElementNS($this->xmlns, 'docgen');
    $this->verifyHeaderElements(array('docgen'));
    $dnode = $dom->getElementsByTagNameNS($this->xmlns, 'docgen')->item(0);
    $p = $dnode->parentNode;
    $p->replaceChild($newDocs, $dnode);
    $this->setFrxHeader('docgen', 'doc',
      $docgenElements,
      array('type'),
      null,
      'type'
    );
  }

  /**
   * Set report parameters
   * Enter description here ...
   * @param array $parmElements array
   */
  public function setParameters($parmElements) {
  	$dom = $this->dom;
    $newParms = $dom->createElementNS($this->xmlns, 'parameters');
  	$this->verifyHeaderElements(array('parameters'));
  	$fnode = $dom->getElementsByTagNameNS($this->xmlns, 'parameters')->item(0);
  	$p = $fnode->parentNode;
  	$p->replaceChild($newParms, $fnode);

  	$this->setFrxHeader('parameters', 'parm',
      $parmElements,
      array('id', 'label', 'require', 'desc', 'data_source', 'data_field', 'type'),
      'value');
  }

  /**
   * Set the report title
   * @param String $title
   */
  public function setTitle($title) {
    $dom = $this->dom;
    $head = $dom->getElementsByTagName('head')->item(0);
    $tnode = $dom->getElementsByTagName( 'title')->item(0);
    $node = $dom->createElement( 'title', $title);
    $head->replaceChild($node, $tnode);
  }

  /**
   * Set the report category
   * Enter description here ...
   * @param unknown_type $cateogry
   */
  public function setCategory($category) {
  	$ret = array();
  	$dom = $this->dom;
  	$this->verifyHeaderElements(array('category'));
  	$head = $dom->getElementsByTagName('head')->item(0);
  	$cnode = $dom->getElementsByTagNameNS($this->xmlns, 'category')->item(0);
  	$node = $dom->createElementNS($this->xmlns, 'category', $category);
  	$head->replaceChild($node, $cnode);
  }

  public function getCategory() {
  	$dom = $this->dom;
  	$this->verifyHeaderElements(array('category'));
  	$cnode = $dom->getElementsByTagNameNS($this->xmlns, 'category')->item(0);
  	return $cnode->textContent;
  }

  /**
   * Retrieve options element in array form
   */
  public function getOptions() {
  	$dom = $this->dom;
  	$this->verifyHeaderElements(array('options'));
  	$opts = $dom->getElementsByTagNameNS($this->xmlns, 'options')->item(0);
    $ret = array();
  	// Simplexml is easier to work with
  	$options = simplexml_import_dom($opts);
  	foreach ($options->attributes() as $key =>$value) {
  	  $ret[(string)$key] = (string)$value;
  	}
    return $ret;
  }

  /**
   * Set the options list for the report
   * Enter description here ...
   * @param unknown_type $option_data
   */
  public function setOptions($option_data) {
  	$dom = $this->dom;
  	$this->verifyHeaderElements(array('options'));
  	$options = $dom->getElementsByTagNameNS($this->xmlns, 'options')->item(0);
    foreach ($option_data as $key => $value) {
    	if ($value) {
    		$options->setAttribute($key, $value);
    	}
    	else {
    		if ($options->hasAttribute($key)) {
    			$options->removeAttribute($key);
    		}
    	}
    }
  }

  public function removeParm($id) {
  	$dom = $this->dom;
  	$xpq = $this->xpq;

  	$pnode = $dom->getElementsByTagNameNS($this->xmlns, 'parameters')->item(0);
    $path = '//frx:parameters/frx:parm[@id="' . $id . '"]';

    $nodes = $xpq->query($path);
    if ($nodes->length) {
      foreach ($nodes as $node) $pnode->removeChild($node);
    }
  }

  /**
   * Make sure all xml elements have ids
   */
  private function parse_ids() {
     $i=0;
     if ($this->simplexml) {
       $this->simplexml->registerXPathNamespace('frx', FRX_NS);
       $frx_attributes = array();
       $frx_nodes = $this->simplexml->xpath('body//*[@frx:*]');

       if ($frx_nodes) foreach ($frx_nodes as $node) {
          $attr_nodes = $node->attributes(FRX_NS);
          if ($attr_nodes) {
            // Make sure every element has an id
            $i++;
            $id = 'forena-' . $i;

            if (!(string)$node['id']) {
              $node->addAttribute('id', $id);

            }
            else {
              if (strpos((string)$node['id'], 'forena-')===0) {
                // Reset the id to the numerically generated one
                $node['id'] = $id;
              }
              else {
                // Use the id of the element
                $id = (string)$node['id'];
              }
            }

            // Save away the frx attributes in case we need them later.
            $attr_nodes = $node->attributes(FRX_NS);
            $attrs = array();
            if ($attr_nodes) foreach ($attr_nodes as $key => $value) {
              $attrs[$key] = (string)$value;
            }
            // Save away the attributes
            $frx_attributes[$id] = $attrs;
          }
       }

       $this->frx_attributes = $frx_attributes;
     }
  }

  /**
   * Get the attributes by
   *
   * @return array Attributes
   *
   * This function will return an array for all of the frx attributes defined in the report body
   * These attributes can be saved away and added back in later using.
   */
  public function get_attributes_by_id() {
    $this->parse_ids();
    return $this->frx_attributes;
  }

  /**
   * Save attributes based on id match
   *
   * @param array $attributes
   *
   * The attributes array should be of the form
   * array( element_id => array( key1 => value1, key2 => value2)
   * The function restores the attributes based on the element id.
   */
  public function save_attributes_by_id($attributes) {
    $rpt_xml = $this->simplexml;
    if ($attributes) foreach ($attributes as $id => $att_list) {
      $id_search_path = '//*[@id="' . $id . '"]';
      $fnd = $rpt_xml->xpath($id_search_path);
      if ($fnd) {
        $node = $fnd[0];

        // Start attribute replacement
        $frx_attributes = $node->Attributes(FRX_NS);

        foreach ($att_list as $key => $value) {
          if (!$frx_attributes[$key]) {
            $node['frx:' . $key] = $value;
          }
          else {
            unset($frx_attributes[$key]);
            $node['frx:' . $key] = $value;
          }
        }
      }
    }
  }




}