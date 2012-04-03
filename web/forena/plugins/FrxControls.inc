<?php
// $Id$
/**
 * @file contains various methods for extending report
 * formating, layout, transformation and design.
 *
 */

class FrxControls {
  /**
   * @section
   * Below here are advertising methods
   */

  //date formats
  public function formats() {
    $formats = array('drupal_date_format' => 'Drupal Date',
                     'iso_date' => 'ISO Date',
                     'sprintf' => 'PHP sprintf()',
                     'number' => 'Number',
                     'drupal_translation' => 'Translatable Text'
                     );
    return $formats;
  }

  //document transformations
  public function doc_types() {
    $doc_types = array('doc' => 'htmlDocument',
                       'html' => 'htmlDocument',
                       'csv' => 'csvDocument',
                       'xls' => 'xlsDocument',
                       'email' => 'emailDocument',
                       'svg' => 'svgDocument'
    );
    return $doc_types;
  }

  //Report templates
  public function templates() {
    $templates = array('table' => 'table',
                       'vertical_list' => 'list',
                       'email_merge' => 'email merge',
                      );

    return $templates;
  }
  // Custom formats based on translation string
  public function drupal_translation($value, $format_str, $teng) {
  	 $field = '';
     if ($format_str) $field = '{' . $format_str . '}';
     $field = $teng->replace($field, null, TRUE);
     $vars = array();
     if ($field) {
     	$vars = (array) @unserialize($field);
     }
     return t($value, $vars);
  }

  //Date format methods
  public function drupal_date_format($value, $format_str, $teng) {
     if (!$format_str) $format_str = 'small';
     switch ($format_str) {
       case 'medium':
         $type = $format_str;
         $format='';
         break;
        case 'small':
         $type = $format_str;
         $format='';
         break;
        case 'large':
         $type = $format_str;
         $format='';
         break;
       default:
         $type = 'custom';
         $format = $format_str;
     }
     if ($value) {
     	  $value = function_exists('format_date') ? format_date($value, $type, $format) : date($format, $value);
     }
     return $value;
  }

  public function iso_date($value, $format_str, $teng) {
    if ($value) $date = strtotime($value);
    return $this->drupal_date_format($date, $format_str, $teng);
  }

  public function sprintf($value, $format_str, $teng) {
    if ($value) {
      $value = sprintf($format_str, $value);
    }
    return $value;
  }

  public function number($value, $format_str, $data='') {

    $dec= 2;
    $dec_sep='.';
    $th_sep = ',';
    if ($format_str && !trim($format_str, '9.,$')) {
      //Determine the decimal places.
      $dec_pos = strrpos($format_str, $dec_sep);
      if ($dec_pos) {
        $dec = strlen($format_str) - $dec_pos -1;
      }
      else {
        $dec = 0;
       $dec_sep = '';
      }
    }

    if ($value && $this->is_number($value)) $value = number_format($value, $dec, $dec_sep, $th_sep);
    return $value;
  }

  private function is_number($value) {
           $non_numeric_chars = trim($value, ' +-.,0123456789');
          // Determine if it contains +- in the interior
          // Zero is ok here bu
          $inner_symbols = FALSE;
          if (strpos($value, '+') || strpos($value, '-') || strpos($value, ' ')) $inner_symbols =  TRUE;
          return (empty($non_numeric_chars) && trim($value)!=='' && !$inner_symbols) ? TRUE : FALSE;
  }

  //Document transformation methods
  public function htmlDocument($body, $options) {
    $output = '<html><head>';

    if ($options['css']) {
      $output .= '<style type="text/css">';
      $output .= $options['css'];
      $output .= '</style>';
    }
    $output .= '<title>' . $options['title'] . '</title></head><body class="forena-report">' . $body . '</body></html>';
    return $output;
  }

  /**
   * Fill columns with rowspan tags.
   * @param unknown_type $colspan
   */
  private function fillColumns($colspan) {
  	$f = '';
  	for ($i=2; $i<=$colspan; $i++) {
  		$f .= ',';
  	}
  	return $f;
  }


  /**
   * Generate the CSV document.
   * @param XML $body
   * @param unknown_type $options
   */
  public function csvDocument($body, $options) {
    $doc = new DOMDocument();
    $doc->strictErrorChecking = FALSE;
    $xmlBody = '<?xml version="1.0" encoding="UTF-8"?>' . $body;
    @$doc->loadHTML($xmlBody);
    $xml = simplexml_import_dom($doc);
    $output = '';
    $rows = $xml->xpath('//tr');
    $rowspans = array();
    if ($rows) foreach ($rows as $row) {
    	$c = 0;
      foreach ($row as $column) {
      	$c++;
      	if (@$rowspans[$c]) {
      		$cont = true;
      		while ($rowspans[$c] && $cont) {
      			$rowspans[$c]--;
      			$output .= ',';
      			$c++;
      		}
      	}
        $value = $column->asXML();
        $value = strip_tags($value);
        $value = str_replace('"', '""', $value);
        $value = str_replace(array("\n"), '', $value);
        $value =  '"' . $value . '",';
        $output .= $value;
        // Add Column span elements
        if ((int)$column['colspan'] > 1) {
          for ($i=2; $i<=(int)$column['colspan']; $i++) {
          	$c++;
  		      $output .= ',';
  	      }
        }
        // Check to see if we have some rowspans that we need to save
        if ((int)$column['rowspan'] > 1) {
        	$rowspans[$c] = (int)$column['rowspan'] - 1;
        }
      }
      $output .= "\n";
    }
    return $output;
  }

  public function svgDocument($body, $options) {
  	$output = '<?xml version="1.0" encoding="UTF-8" standalone="no" ?>';
  	$doc = new DomDocument();
  	$doc->strictErrorChecking = FALSE;
  	@$doc->loadHTML($body);
  	$xml = simplexml_import_dom($doc);
  	$svg = $xml->xpath('//svg');
  	if ($svg) foreach($svg as $graph) {
  		$output .= $graph->asXML();
  	}
  	return $output;
  }


  public function xlsDocument($body, $options) {
    $output = '<?xml version="1.0" encoding="UTF-8"?>' . "\n";
    $output .= '<?mso-application progid="Excel.Sheet"?>' . "\n";
    $output .= '<Workbook xmlns="urn:schemas-microsoft-com:office:spreadsheet"' . "\n";
    $output .= '  xmlns:o="urn:schemas-microsoft-com:office:office"' . "\n";
    $output .= '  xmlns:x="urn:schemas-microsoft-com:office:excel"' . "\n";
    $output .= '  xmlns:ss="urn:schemas-microsoft-com:office:spreadsheet"' . "\n";
    $output .= '  xmlns:html="http://www.w3.org/TR/REC-html40">' . "\n";
    $output .= '<Styles>' . "\n";
    $output .= '  <Style ss:ID="Default" ss:Name="Normal">' . "\n";
    $output .= '    <Alignment ss:Vertical="Bottom"/>' . "\n";
    $output .= '    <Borders/>' . "\n";
    $output .= '    <Font/>' . "\n";
    $output .= '    <Interior/>' . "\n";
    $output .= '    <NumberFormat/>' . "\n";
    $output .= '    <Protection/>' . "\n";
    $output .= '  </Style>' . "\n";
    $output .= '</Styles>' . "\n";

    $doc = new DOMDocument();
    $doc->strictErrorChecking = FALSE;
    $doc->loadHTML($body);
    $xml = simplexml_import_dom($doc);

    $tables = $xml->xpath('//table');
    $count = 1;
    if ($tables) foreach ($tables as $table) {
      $output .= '<Worksheet ss:Name="sheet' . ' ' . $count . '">' . "\n";
      $count ++;
      $output .= '  <Table>' . "\n";

      $rows = $table->xpath('descendant::tr');
      if ($rows) foreach ($rows as $row) {
        $output .= '    <Row>' . "\n";
        foreach ($row as $column) {
          $value = $column->asXML();
          $value = strip_tags($value);
          $tval = trim($value);
          // Find if it contains invalid number characters
          $non_numeric_chars = trim($value, ' +-.,0123456789');
          // Determine if it contains +- in the interior
          // Zero is ok here bu
          $inner_symbols = FALSE;
          if (strpos($tval, '+') || strpos($tval, '-') || strpos($tval, ' ')) $inner_symbols =  TRUE;
          if (substr_count($tval, '.') > 1) $inner_symbols = TRUE;
          if (empty($non_numeric_chars) && trim($value)!=='' && !$inner_symbols) {
            $output .= '      <Cell><Data ss:Type="Number">' . $tval . '</Data></Cell>' . "\n";
          }
          else {
            $output .= '      <Cell><Data ss:Type="String">' . $value . '</Data></Cell>' . "\n";
          }
        }
        $output .= '    </Row>' . "\n";
      }
      $output .= '  </Table>' . "\n";
      $output .= '  <WorksheetOptions xmlns="urn:schemas-microsoft-com:office:excel">' . "\n";
      $output .= '    <Selected/>' . "\n";
      $output .= '    <ProtectObjects>False</ProtectObjects>' . "\n";
      $output .= '    <ProtectScenarios>False</ProtectScenarios>' . "\n";
      $output .= '  </WorksheetOptions>' . "\n";
      $output .= '</Worksheet>' . "\n";
    }
    $output .= '</Workbook>';
    return $output;
 }

 public function emailDocument($body, $options) {
   $doc = new DOMDocument('1.0', 'UTF-8');
   $doc->strictErrorChecking = FALSE;
   $doc->loadHTML($body);
   $xml = simplexml_import_dom($doc);
   $docs = $xml->xpath('.//*[@class="email-document"]');
   foreach ($docs as $doc) {
      $from = $doc->xpath('.//*[@class="email-header-from"]');
      $from = $from ? (string)$from[0] : '';
      $subject = $doc->xpath('.//*[@class="email-header-subject"]');
      $subject = $subject ? (string)$subject[0] : '';
      $to = $doc->xpath('.//*[@class="email-header-to"]');
      $to = $to ? (string)$to[0] : '';
      $body = $doc->xpath('.//*[@class="email-body"]');
      $body = $body ? $body[0]->asXML() : $body;
      $email = array(
        'to' => $to,
        'from' => $from,
        'parms' => array('subject' => $subject,
        'body' => $body),
         );
      $emails [] = $email;
   }
   $count = count($docs);
   if ($count) {
     $output = drupal_get_form('forena_confirm_email', $emails, $count);
   }
   else {
      $output = $body;
   }
   return $output;
 }
 //template methods
  public function table($columns, $data_block, $clause='') {
    if ($clause) $clause = 'frx:clause ="' . $clause . '"';
    $xml = '<div frx:block="' . $data_block . '" ' . $clause . '>';
    $xml .= '<table>';
    $xml .= '<thead>';
    $xml .= '<tr>';
    //Create the headers
    foreach ($columns as $header) {
     $xml .= '<th>' . $header . '</th>';
    }
    $xml .= '</tr>';
    $xml .= '</thead>';
    $xml .= '<tbody>';
    $xml .= '<tr frx:foreach="*">';
    foreach ($columns as $column) {
      $xml .= '<td>{' . $column . '}</td>';
    }
    $xml .= '</tr>';
    $xml .= '</tbody>';
    $xml .= '</table>';
    $xml .= '</div>';
    return $xml;
  }

  public function vertical_list($columns, $data_block, $clause) {
    if ($clause) $clause = 'frx:clause ="' . $clause . '"';
    $xml = '<div frx:block="' . $data_block . '" ' . $clause . '>';
    $xml .= '<table frx:foreach="*">';
    $xml .= '<tbody>';

    foreach ($columns as $column) {
      $xml .= '<tr >';
      $xml .= '<th>' . $column . '</th><td>{' . $column . '}</td>';
      $xml .= '</tr>';
    }

    $xml .= '</tbody>';
    $xml .= '</table>';
    $xml .= '</div>';

    return $xml;
  }

  public function email_merge($columns, $data_block, $clause) {
    if ($clause) $clause = 'frx:clause ="'. $clause .'"';
    $xml = '<div frx:block="'. $data_block .'" '. $clause .' frx:foreach="*">';
    $xml .='<div class="email-document">';
    $xml .='<div class="email-header">
    <table>
      <tbody>
        <tr>
          <th>From</th>
          <td class="email-header-from">metzlerd@evergreen.edu</td>
        </tr>
       <tr>
         <th>To</th>
         <td class="email-header-to">{email}</td>
       </tr>
       <tr>
         <th>Subject</th>
         <td class="email-header-subject"></td>
       </tr>
     </tbody>
    </table>
   </div>';
   $xml .= '<div class="email-body"><p>';

   foreach ($columns as $column) {
    $xml .= '{' . $column . '} ';
   }

   $xml .= '</p></div>';
   $xml .= '</div>';
   $xml .= '</div>';
   return $xml;
  }

}
