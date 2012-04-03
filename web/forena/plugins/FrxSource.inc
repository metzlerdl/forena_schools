<?php
/**
	* Look at the FrxRender class to see a full list of
  * properties that can be used here.
  */
class FrxSource extends FrxRenderer {
	public function render() {
		$node = $this->reportDocNode;
		$output = htmlspecialchars($node->asXML());
		return $output;
	}
}