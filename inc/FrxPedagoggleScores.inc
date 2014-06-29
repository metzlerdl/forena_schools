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
    $output = '<table class="pedagoggle-scores table table-bordered">';
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
     for ($i=1; $i<= $depth; $i++) {
     	 $hrow = $this->headers[$i];
     	 $o .= '<tr>';

     	 foreach ($hrow as $hcol) {
     	 	 $label = $hcol['label'];
     	 	 $colspan = $hcol['colspan'];
     	 	 $rowspan = $depth - $hcol['depth'] + 1;
     	 	 $style = @$hcol['style'] ? 'style="' . $hcol['style'] . '"': '';
     	 	 $class = @$hcol['class'] ? 'class="' . $hcol['class'] . '"': '';
     	   $o .= "<th $style $class colspan='$colspan' rowspan='$rowspan'>$label</th>\n";
     	 }
     	 $o .= "<th>&nbsp;</th>";
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
  		$o .= "<td>&nbsp;</td>";
  		$o .= '</tr>';
  		$d->pop();

  	}
  	return $o;
  }

}