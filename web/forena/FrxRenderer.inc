<?php
/**
 * Base class for Frx custom renderers
 * @author davidmetzler
 *
 */
class FrxRenderer {
	public $teng;           // Token replacement engine.
	public $reportDocDomNode; //A Dom node version of the element.  This is important if you want to walk text nodes.
	public $reportDocNode;  // SimpleXML Report Document Node -- The node of the report we are rendering
	public $frxAttributes;  // Frx Attributes of the node we are rendering.
	public $htmlAttributes;  // Html attributes of the node that we are rendering
	public $dataProvider;   // An FrxData instance that provides the data assiated with the report.
	public $name;
	public $id;


	public function __construct($domNode = null, $teng = null, $frxReport = null) {
		if ($domNode) {
      $this->initReportNode($domNode, $teng);
		}
	}

	public function initReportNode($domNode, $frxReport) {
		$this->reportDocDomNode = $domNode;
    $this->dataProvider = FrxData::instance();
    $this->reportDocNode = simplexml_import_dom($domNode);
    $node = $this->reportDocNode;
    $this->name = $node->getName();
    $this->htmlAttributes = $node->attributes();
    $this->id = (string)$this->htmlAttributes['id'];
    $this->frxAttributes = $node->attributes(FRX_NS);
    $this->teng = $frxReport->teng;
    $this->frxReport = $frxReport;
	}

	public function replaceTokens($text, $raw_mode=FALSE) {
		return $this->teng->replace($text, $raw_mode);
	}

	public function render() {
		$this->initNode($domNode, $teng);
		// Get the data that we're working with
		$xml = $this->dataProvider->getCurrentContext();
		$node = $this->nodes;
    // We can render so lets do it.
    $text = $this->xmlNode->asXML();
    $o = $this->teng->replace($text);
    return $o;
	}

	/**
	 * Standard php array containing merged attributes
	 * Enter description here ...
	 */
	public function mergedAttributes() {
		$attributes = array();
		if (isset($this->frxAttributes)) foreach ($this->frxAttributes as $key=>$data) {
			$attributes[$key] = (string)$data;
		}
		if (isset($this->htmlAttributes)) foreach ($this->htmlAttributes as $key=>$data) {
			$attributes[$key] = (String)$data;
		}
    return $attributes;
	}


}