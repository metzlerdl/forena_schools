<?php
class FrxChart extends FrxRenderer {
	public function render() {
		// Get data from source
		$options = array();
		$series = array();
		$seriesx = array();
		$output;
		$data = array();
		$attributes = $this->mergedAttributes();
		$path = isset($attributes['xpath']) ? $attributes['xpath'] :  '*';
		$label = '';
		$links = '';
		$type = $attributes['type'];
		$link = @$attributes['link'];
		if (isset($attributes['options'])) {
		  parse_str($attributes['options'], $options);
		  $series = @$options['series'];
		  if (isset($options['seriesx']))  {
			  $seriesx = $options['seriesx'];
		  	unset($options['seriesx']);
		  }
		  if (isset($options['label'])) {
			  $label =  $options['label'];
			  unset($options['label']);
		  }
		  foreach($options as $key=>$value) {
		  	$options[$key] = $this->teng->replace($value);
		  }
		}

	 if (isset($attributes['height'])) $options['height'] = $this->teng->replace($attributes['height']);
	 if (isset($attributes['width'])) $options['width'] = $this->teng->replace($attributes['width']);

		// Extract chart series data from xml.
		if ($series) {
			foreach((array)$series as $i=>$d) {
				if ($seriesx) {
					$data[] = $this->xmlToValues($path, $d, $seriesx[$i]);
				}
				else {
				  $data[] = $this->xmlToValues($path, $d, $label);
				}
			}
		}
		if ($link) {
			$links = $this->xmlToValues($path, $link, $label);
		}
		if ($data) {
			if (method_exists($this,'renderChart')) {
				$output = $this->renderChart($type, $data, $options, $links);
			}

		}
		return $output;
	}


	public function xmlToValues($path, $data_path, $label_path='') {
		$do = FrxData::instance();
		$data = $do->currentContext();
		$values = array();
		if (is_object($data)) {
			$nodes = $data->xpath($path);
			if ($nodes) foreach ($nodes as $i=>$node) {
				$do->push($node, $this->id);

				$val = $this->teng->replace($data_path, TRUE);
				if ($label_path) {
					$key = $this->teng->replace($label_path, TRUE);
				}
				else {
					$key = $i;
				}
	      $values[$key] = $val;
				$do->pop();
			}
		}
    return $values;
	}
}