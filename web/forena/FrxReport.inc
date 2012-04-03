<?php
// $Id$
/**
 * @file
 * Basic report provider.  Controls the rendering of the report.
 */
define('FRX_NS', 'urn:FrxReports');
require_once('FrxSyntaxEngine.inc');
require_once('FrxRenderer.inc');

class FrxReport {
  public $blocks_loaded;
  public $rpt_xml;
  public $fields;
  public $category;
  public $descriptor;
  public $form;
  public $access;
  public $parameters;
  public $options;
  public $body;
  public $html;
  private $ids;
  public $teng;
  public $frx_data; // Data object
  private $root_data;
  private $dom;


  public function __construct($xhtml, $data=array()) {
    $this->access = array();
    $this->parameters = array();
    $this->options = array();
    $this->root_data = $data;
    $this->frx_data = FrxData::instance();
    $this->frx_data;
    $this->teng = new FrxSyntaxEngine(FRX_TOKEN_EXP, '{}', $this, $this->frx_data);
    if ($xhtml) {
      $dom = $this->dom = new DOMDocument('1.0', 'UTF-8');
      // Old assumption is an ojbect is a simplexml one
      if (is_object($xhtml)) $xhtml = $xhtml->asXML();

      // Load document and simplexml representation
      $dom->loadXML($xhtml);
      $dom->formatOutput = TRUE;
      $rpt_xml = $this->rpt_xml = simplexml_import_dom($dom);

      // Loaod data if its there.
      $this->frx_data->push($data, 'parm');
      // Load header data
      $this->body = $rpt_xml->body;
      if ($rpt_xml->head) {
        $this->title = (string)$rpt_xml->head->title;

        foreach ($rpt_xml->head->children(FRX_NS) as $name => $node) {
          switch ($name) {
            case 'fields':
              $this->fields = $node;
              break;
            case 'category':
              $this->category = (string)$node;
              break;
            case 'descriptor':
            	$this->descriptor = (string)$node;
            	break;
            case 'options':
              foreach ($node->attributes() as $key => $value) {
                $this->options[$key] = (string)$value;
              }
              break;
            case 'form':
              $this->form = (string) $value;
              break;
            case 'parameters':
              foreach ($node->children(FRX_NS) as $key => $node) {
                $parm = array();
                foreach ($node->attributes() as $akey => $attr) {
                  $parm[$akey] = (string)$attr;
                }
                $id = $parm['id'];
                $val = isset($parm['value']) ? $parm['value'] : '';
                $parm['value']= ((string)$node) ? (string)$node : $val;
                $this->parameters[$id] = $parm;
              }
              break;
            case 'doctypes':
              $this->doctypes = $value;
              break;
          }
        }
      }
    }
  }

  /**
   * Get the data block
   * @param $block
   * @return unknown_type
   */
  private function get_data($block, $clause='', $id='', $data_uri='') {
    //@TODO: Merge xml data parameters into the report paramters
    if ($data_uri) {
    	parse_str($data_uri, $data);
    	if (is_array($data)) foreach ($data as $key=>$value) {
    		$data[$key] = $this->teng->replace($value);

    	}
    	$id .= '-parm';
    	FrxData::instance()->push($data, $id);
    }

    $xml = FrxReportGenerator::instance()->invoke_data_provider($block, null, $clause);

    if ($xml) {
    	$this->blocks_loaded = TRUE;
    }
    return $xml;
  }

  /**
   * Recursive report renderer
   * Walks the nodes rendering the report.
   */
  public function render_section(DOMNode $dom_node) {
    $continue = TRUE;
    $is_data_block = FALSE;
    $node_type = $dom_node->nodeType;
    $o = '';

    // Shortcut process a text node
    if ($node_type == XML_TEXT_NODE
        || $node_type == XML_ENTITY_REF_NODE
        || $node_type == XML_ENTITY_NODE
        ) {     $text = $dom_node->textContent;
      $o .= $this->teng->replace($text);
      return $o;
    }

    //Ignore certain node types
    if ($node_type == XML_COMMENT_NODE) {
      return '';
    }
    // Continue processing non text nodes
    $node = simplexml_import_dom($dom_node);
    // Special catch to make sure we don't process bad nodes
    if (!is_object($node)) {
	    return '';
    }


    $frx = $node->attributes(FRX_NS);
    $elements = $dom_node->childNodes->length;
    // Test to see if we have any nodes that contain data url
    if ($node->xpath('*//@frx:*') || $frx) {
      $attrs = $node->attributes();
      $id = (string)$attrs['id'];
      $frx = $node->attributes(FRX_NS);
      $tag = $node->getName();

      if ((string)$frx['block']) {
        $is_data_block = TRUE;
        $xml = $this->get_data((string)$frx['block'], (string)$frx['clause'], $id, (string)$frx['parameters']);
        if ($xml) {
          $this->frx_data->push($xml, $id);
        }
        else $continue = FALSE;
      }

      // Preserve non frx attributes
      $attr_text='';
      $tmp_attrs = array();
      if ($attrs) foreach ($attrs as $key => $value) {
        $attr_text .=  ' ' . $key . '="' . (string)$value . '"';
        $tmp_attr[$key] = (string)$value;
      }

      // Determine if we have a custom renderer
      $renderer = (string)$frx['renderer'];
       // if we have a foreach in this node, we need to iterate the children
      if ((string)$frx['foreach'] ) {
        // Save xml
        $path = (string)$frx['foreach'];
        $data = $this->frx_data->currentContext();

        if (is_object($data)) $nodes = $data->xpath($path);
        $i=0;

        //$tmp_attrs = (array)$attrs;
        if ($nodes) foreach ($nodes as $x) {
          $this->frx_data->push($x, $id);
          $i++;
          $odd = $i & 1;
          $row_class = $odd ? 'odd' : 'even';
          $tmp_attrs['class'] = trim($attrs['class'] . ' ' . $row_class);
          $r_attr_text = '';

          foreach ($tmp_attrs as $key => $value) {
             $r_attr_text .=  ' ' . $key . '="' . (string)$value . '"';
          }
          $o .= $this->teng->replace('<' . $tag . $r_attr_text . '>');
          foreach ($dom_node->childNodes as $child) {
            $o .= $this->render_section($child);
          }
          $o .= '</' . $tag . '>';
	        $this->frx_data->pop();
        }
      }
      elseif ($continue) {
      	if ($renderer) {
      		// Implement custom renderer.
          $co = FrxReportGenerator::instance()->define_controls($renderer);
      	  if ($co) {
      	  	$co->initReportNode($dom_node, $this);
      	  	$o = $co->render();
      	  }
      	}
      	else {
	        $o .= $this->teng->replace('<' . $tag . $attr_text . '>');

	        // None found, so render children
	        foreach ($dom_node->childNodes as $child) {
	          $o .= $this->render_section($child);
	        }
	        $o .= '</' . $tag . '>';
      	}
      }
      if ($is_data_block && $continue) {
      	$this->frx_data->pop();
      }
    }
    else {

      // We can render so lets do it.
      $text = $node->asXML();
      $o .= $this->teng->replace($text);
    }

    return $o;
  }

  /**
   * Render the report
   * @return unknown_type
   */
  public function render($format) {
    $dom = $this->dom;
    $o='';
    $body = $dom->getElementsByTagName('body')->item(0);

    foreach ($body->childNodes as $node) {
      $o .= $this->render_section($node);
    }
    $this->html = $o;
    return $o ;
  }
  /*
   * Formatter used by the syntax engine to alter data that gets extracted.
   * This invokes the field translation
   */
  public function format($value, $key, $data) {
    // Determine if there is a field overide entry
    $default='';
    $link ='';
    $format='';
    $format_str='';
    $target='';
    if ($this->fields) {

      $path = 'frx:field[@id="' . $key . '"]';
      $formatters = $this->fields->xpath($path);
      if ($formatters) foreach ($formatters as $formatter) {
        if ((isset($formatter['block']) &&(string)$formatter['block'] == $this->block) || (!(string)$formatter['block'])) {

           //@TODO: Replace the default extraction with something that will get sub elements of the string
           $default = (string)$formatter;
           $link = (string)$formatter['link'];
           $format = (string) $formatter['format'];
           $format_str = (string) $formatter['format-string'];
           $target = (string) $formatter['target'];
        }
      }
    }

    if ($format) {
      $value = FrxReportGenerator::$instance->format_data($value, $format, $format_str, $this->teng);
    }


    // Default if specified
    if (!$value && $default) {
      $value = $default;
    }

    if ($link) {
    	$target = $this->teng->replace($target, TRUE);
    	// use the target attribute to open links in new tabs or as popups.
      if (@strpos(strtolower($target),'popup')===0) {
        $opts = 'status=1';


      	$options = "status=1";
        $attributes = array('onclick' =>
          'window.open(this.href,\''. $target . '\', "' . $options . '"); return false;');
      }
      else {
      	$attributes = array('target' => $target);
      }
      $link = $this->teng->replace($link, TRUE);
      @list($url, $query) = explode('?', $link);
      @list($query, $queryFrag) = explode('#', $query);
      @list($url, $fragment) = explode('#', $url);
      $fragment = $fragment . $queryFrag;
      $data = array();
      parse_str($query, $data);
      if (trim($url)) $value = FrxReportGenerator::instance()->link(
        htmlspecialchars_decode($value), $url,
        array('fragment' => $fragment, 'query' => $data, 'attributes' => $attributes, 'absolute' => TRUE));
    }
    return $value;
  }

  /**
   * Delete a node based on id
   * @param unknown_type $id
   * @return unknown_type
   */
  public function deleteNode($id) {
    $path = '//*[@id="' . $id . '"]';
    $nodes = $this->rpt_xml->xpath($path);
    $node = $nodes[0];
    $dom=dom_import_simplexml($node);
    $dom->parentNode->removeChild($dom);
  }

  /**
   * Return the xml data for the report.
   *
   * @return unknown
   */
  public function asXML() {
     if ($this->rpt_xml) {
       return $this->rpt_xml->asXML();
     }
     else {
       return '';
     }
  }
  /**
   * Make sure all xml elements have ids
   */
  private function parse_ids() {
     $i=0;
     if ($this->rpt_xml) {
       $this->rpt_xml->registerXPathNamespace('frx', FRX_NS);
       $frx_attributes = array();
       $frx_nodes = $this->rpt_xml->xpath('body//*[@frx:*]');

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
    $rpt_xml = $this->rpt_xml;
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

  /**
   * Set the value of an element within the report
   * @param String $xpath Xpath to element being saved
   * @param string $value Value to be saved.
   * @return unknown_type
   */
public function set_value($xpath, $value) {
  $xml = $this->rpt_xml;
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
  }
}


}


