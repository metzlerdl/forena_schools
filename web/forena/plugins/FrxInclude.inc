<?php
class FrxInclude extends FrxRenderer {
	public function render() {
		// Get data from source
		$attributes = $this->mergedAttributes();
		$output = '';

		// Determine data type
		$include = @$attributes['src'];
		$include = $this->teng->replace($include);
		list($url, $query)=@explode('?', $include);
		$parts = @explode('/', $url);
		$file = @$parts[count($parts) - 1];
		$parts = explode('.', $file);
		// Determine file extention
		$ext = count($parts) > 1 ? $parts[count($parts) - 1] : '';

    $doc_types = FrxReportGenerator::instance()->configuration('doc_formats');
    if (@$doc_types[$ext]) {
      $output = $this->render_reference($include, $ext, $attributes);
    }
		else {
			$output = $this->teng->replace($this->reportDocNode->asXML());
		}
		return $output;
	}

  function render_reference($url, $ext, $attributes) {
  	$ext = strtolower($ext);
  	$attributes = $this->teng->replace($attributes);
  	switch($ext) {
  		case 'png':
  		case 'gif':
  		case 'jpg':
  		case 'jpeg':
      	$x = new SimpleXMLElement('<img/>');
      	$x['src'] = $url;
      	if (isset($attributes['height'])) $x['height'] = $attributes['height'];
      	if (isset($attributes['width'])) $x['width'] = $attributes['width'];
  		  break;
  		case 'svg':
  			$x = new SimpleXMLElement('<embed/>');
  			$x['src'] = $url;
  			$x['type'] = 'image/svg+xml';
  			$x['pluginspage'] = "http://www.adobe.com/svg/viewer/install/";
      	if (isset($attributes['height'])) $x['height'] = $attributes['height'];
      	if (isset($attributes['width'])) $x['width'] = $attributes['width'];
  			break;
  		default:
  			$x = new SimpleXMLElement('<a>' . $ext . ' document</a>' );
  			$x['href'] = $url;
  	}

  	if (isset($attributes['id'])) $x['id'] = $attributes['id'];
  	return $x->asXML();
  }


}