<?php
class FrxPedagoggleScores extends FrxRenderer {

	public $headers = array();
	public $columns = array();
	public $def_columns = array();
	public $def_column_styles = array();
	public $td_text = '<td class="score l{@norm_group}">{@score}</td>';

  public function render() {
    $frx = $this->frxAttributes;
    // Default controling attributes
    $profile_source = (string)$frx['profile'];
    $measure_field = (string)$frx['measureField'];
    $measure_field = $measure_field ? $measure_field : 'measures';
    $this->defaultHeaders();

    if ($profile_source) {
    	$profile_xml = $this->dataProvider->getContext($profile_source);
    	if ($profile_xml) {
    		//Get the depth of the profile xml.
    	  $measures = $profile_xml->$measure_field;
    	  $depth = 0;
    	  $this->parseFields($measures,$depth);
    	}
    }
    $width = (count($this->columns) + count($this->def_columns) +1) * 75 + 320;
    $output = "<table class='pedagoggle-scores table table-bordered' style='width:${width}px;'>";
    $output .= $this->renderHeaders();
    $output .= $this->renderStudents();
    $output .= '</table>';
    return $output;
  }
  /*
   * Determine field depth and set up columns.
   */
  public function parseFields($fields, &$depth) {
  	$depth++;
  	$max_depth = $depth;
  	foreach ($fields->children() as $tag => $value) {
  		if ($tag == 'parent') {
  			$hcol = array();
  			$hcol['label'] = (string)$value['label'];
  			$span = count($value->xpath('.//measure'));
  			$hcol['colspan'] = $span;
  			$max_depth = $this->parseFields($value, $depth);
  			$hcol['depth'] = $max_depth;
  			$this->headers[$depth][] = $hcol;
  		}
  		// We have a measure column so we need to set it up.
  		else {
  			$hcol['label'] = (string)$value['abbrev'];
  			$hcol['colspan'] = 1;
  			$prefix = 'scores/measure[@measure_id="';
  			if ((int)$value['seq']==0) {
  			  $col = $prefix . (string)$value['measure_id'] . '"]';
  			} else {
  			  $col = $prefix . (string)$value['measure_id'] . '" and @seq="' . (string)$value['seq'] . '"]';
  			}
  			$hcol['depth'] = $depth;
  			$hcol['class'] = 'm-abbrev';
  			$this->columns[] = $col;
  			$this->headers[$depth][] = $hcol;
  		}
  	}
    $depth--;
    return $max_depth;
  }

  private function defaultHeaders() {
  	$node = $this->reportDocNode;
  	if ($node->thead && $node->thead->tr) {
  		foreach ($node->thead->tr->children() as $th) {
  			$hcol['label'] = (string)$th;
  			$hcol['depth'] = 1;
  			$hcol['colspan'] = 1;
  			$hcol['class'] = @(string)$th['class'];
  			$hcol['style'] = @(string)$th['style'];
        $this->headers[1][] = $hcol;

  		}
  	}
  	if ($node->tbody && $node->tbody->tr) {
  		foreach ($node->tbody->tr->children() as $td) {
  			$col['data'] = (string)$td;
  			$col['class'] = (string)$td['class'];
  			$col['style'] = (string)$td['style'];
  			$this->def_columns[] = $col;
  		}
  	}
  }

  private function renderHeaders() {
     $o = '<thead>';
     $depth = count($this->headers);
     $max_rowspan = 1;
     for ($i=1; $i<= $depth; $i++) {
     	 $hrow = $this->headers[$i];
     	 $o .= '<tr>';

     	 foreach ($hrow as $hcol) {
     	 	 $label = $hcol['label'];
     	 	 $colspanstr = '';
     	 	 $rowspanstr = '';
     	 	 $colspan = $hcol['colspan'];
     	 	 $rowspan = $depth - $hcol['depth'] + 1;
     	 	 if ($rowspan > $max_rowspan) $max_rowspan = $rowspan;

     	 	 $style = @trim($hcol['style']) ? 'style="' . $hcol['style'] . '"': '';
     	 	 $class = @trim($hcol['class']) ? 'class="' . $hcol['class'] . '"': '';
     	 	 $attr_str = trim($style . ' ' . $class);
   	 	   if ($colspan > 1) $colspanstr = "colspan=$colspan";
   	 	   if ($rowspan > 1) $rowspanstr = "rowspan=$rowspan";
   	 	   $attr_str = trim($attr_str . ' ' . $colspanstr . ' ' . $rowspanstr);
     	   $o .= "<th $attr_str>$label</th>\n";
     	 }
     	 if ($i == 1) $o .= "<th rowspan='$max_rowspan'></th>";
     	 $o .= '</tr>';
     }
     $o .= '</thead>';
     return $o;
  }

  // Put out the scores that we find.
  private function renderScores($xml) {
  	$d = FrxData::instance();
  	$o='';
  	foreach($this->columns AS $col) {
  		// Xpath out the score based on the data passed.
  		$x = $xml->xpath($col);
  		$score = @$x[0];

  		$d->push($score, 'score');
  		$o .= $this->replaceTokens($this->td_text);
  		$d->pop();
  	}
  	return $o;
  }

  private function renderStudents() {
  	$d = FrxData::instance();
  	$o = '';
  	$xml = $d->currentContext();
  	foreach ($xml->children() as $row) {
  		$d->push($row,'student');
  		$o .= "<tr>\n";
  		foreach($this->def_columns as $col) {
  		  $style = $col['style'] ? "style='" . $col['style'] . "'" : '';
  		  $class = $col['class'] ? "class='" . $col['class'] . "'" : '';
  			$o .= $this->replaceTokens("<td $style $class >". $col['data'] . '</td>');
  		}

  		$o .= $this->renderScores($row);
  		$o .= "<td></td>";
  		$o .= '</tr>';
  		$d->pop();

  	}
  	return $o;
  }

}