<?php
require_once ('FrxChart.inc');
class FrxSVGGraph extends FrxChart {
	private $graph;
  private $links;
	function __construct() {
		  $directory = FrxReportGenerator::instance()->configuration('library_path');
			$library = rtrim($directory,'/') . '/SVGGraph/SVGGraph.php';
		  require_once $library;
	}

	function renderChart($type, $data, $options, $links ) {
		$this->links = $links;
		if ($data) {
				switch ($type) {
					case 'bargraph':
						$output = $this->barGraph($data[0], $options);
					  break;
					case 'piechart':
						$output = $this->pieChart($data[0], $options);
						break;
					case 'stackedbargraph':
						$output = $this->stackedBarGraph($data, $options);
						break;
					case 'groupedbargraph':
						$output = $this->groupedBarGraph($data, $options);
						break;
					case 'linegraph':
						$output = $this->lineGraph($data[0], $options);
						break;
					case 'multilinegraph':
						$output = $this->multiLineGraph($data, $options);
						break;
					case 'scatterplot':
						$output = $this->scatterPlot($data, $options);
						break;
					case 'multiscatterplot':
						$output = $this->multiScatterPlot($data, $options);
						break;
				}
		}
		return $output;
	}

  // Generate a bar graph.
	function barGraph($data, $options) {
    if (@$options['style']=='3D') {
      $type = 'Bar3DGraph';
    }
    elseif (@$options['style']=='horizontal') {
    	$type = 'HorizontalBarGraph';
    }
    else {
      $type = 'BarGraph';
    }
    return $this->renderGraph($type, $options, $data);
	}

	function stackedBarGraph($data, $options) {
		if (@$options['style'] == 'horizontal') {
			$type = 'HorizontalStackedBarGraph';
		}
		else {
			$type = 'StackedBarGraph';
		}
		return $this->renderGraph($type, $options, $data);
	}

	function groupedBarGraph($data, $options) {
		if (@$options['style'] == 'horizontal') {
			$type = 'HorizontalGroupedBarGraph';
		}
		else {
			$type = 'GroupedBarGraph';
		}
		return $this->renderGraph($type, $options, $data);
	}

	// Generate a pie chart
	function pieChart($data, $options) {
    if (@$options['style']=='3D') {
      $type = 'Pie3DGraph';
    }
    else {
      $type = 'PieGraph';
    }
    return $this->renderGraph($type, $options, $data);
	}

	function lineGraph($data, $options) {
		return $this->renderGraph('LineGraph', $options, $data);
	}

	function multiLineGraph($data, $options) {
		return $this->renderGraph('MultiLineGraph', $options, $data);
	}

	// Scatter plot
	function scatterPlot($data, $options) {
		return $this->renderGraph('ScatterGraph', $options, $data);
	}

	// Multi scatter plot
	function multiScatterPlot($data, $options) {
		return $this->renderGraph('MultiScatterGraph', $options, $data);
	}

	function renderGraph($type, $options, $data) {
		$links = $this->links;
	  $width = (@$options['width']) ? @$options['width'] : 600;
	  $height = (@$options['height']) ? @$options['height'] : 400;
		$graph = new SVGGraph($width, $height, $options);
		$this->graph = $graph;
    $graph->Values($data);
    if (is_array(@$options['colors'])) {
      $graph->colours = $options['colors'];
    }
    if ($links) {
    	$graph->Links($links);
    }
    ob_start();
		$graph->Render($type, FALSE);
		$output = ob_get_contents();
		ob_end_clean();
		return $output;
	}


}