<?php
/**
	* Look at the FrxRender class to see a full list of
  * properties that can be used here.
  */
class FrxXMLSource extends FrxRenderer {
	public function render() {
		$node = $this->reportDocNode;
		$xml = FrxData::instance()->currentContext();
		if ($xml) $output = htmlspecialchars($xml->asXML());
		return $output;
	}
}