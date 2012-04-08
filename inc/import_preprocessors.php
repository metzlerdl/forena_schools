<?php

function process_imp_students($stu_row, $tab_columns) {
  // look for attributes and merge them into an array row

  foreach ($stu_row as $key=>$value) {
  	$attributes = array();
  	if (strpos($key,'attr')===0) {
      $attributes[] = $value;
  	}

  }
  $sis_id = $stu_row['sis_id'];
  // If there are attributes in the array insert them into the table
  $sql = 'INSERT INTO imp_student_attributes(sis_id,attribute) '.
      ' VALUES ( {sis_id},{attribute} )';
  $values = array('sis_id'=>$sid_id);
  if (count($attributes) > 0 ) {
  	foreach ($attributes as $attribute) {
      $values['attribute'] = $attribute;
  	  db_query($sql,$values);
  	}
  }
  $rows = array();
  $row[] =$stu_row;
  return $row;
}

/**
 * Test preprocessor
 * Enter description here ...
 * @param unknown_type $row
 */
function process_imp_test_scores($row, $tab_columns) {

  // look for attributes and merge them into an array row
  if ($row['mode']=='state_testing') {
    $test_codes= array();
  	$test_code='';
  	$last_test='';
  	$row['grade_level']= $row['reportinggrade'] ? $row['reportinggrade'] : $row['grade_level'];
  	$ts = $row;
    $ts['bldg_school_code']=$row['bldg_code_field'] ? $ts[$row['bldg_code_field']] : $row['schoolcode'];
    $ts['sis_id']=$row['sis_id_field'] ? $ts[$row['sis_id_field']] : $ts['districtstudentcode'];
    foreach ($row as $key=>$value) {
    	switch ($key) {
    		case 'readingtesttype':
    		case 'writingtesttype':
    		case 'sciencetesttype':
    		case 'mathtesttype':
    		case 'eocmathyr2testtype':
    		case 'eocmathyr1testtype':
    			$test_type = str_replace('testtype', '', $key);
    			// Make these two tests the same.
    			if ($value == 'MU1') $value = 'ALG';
    			$test_codes[$test_type] = $value;
    			$last_test=$value;
    			break;
    	}

    	// Now import scores that we categorized.
    	$new_test_code = '';
    	$type_key = $key;
    	$type_key = str_replace('eoc math year 1','eocmathyr1', $type_key);
    	$type_key = str_replace('eoc math year 2','eocmathyr2', $type_key);
      foreach ($test_codes as $subject => $type) {
      	if (strpos($type_key, $subject)!==FALSE) {
	      	$new_test_code=$type;
      	}
      }
      if ($new_test_code)  {
      	$test_code = $new_test_code;
      }
      else {
      		$test_code=$last_test;
      }
		  if ((strpos($key,'score') !==FALSE || strpos($key, 'percent')!==FALSE) && $test_code) {
		    	$ts['score'] = $value;
		    	$ts['test_code'] = $test_code;
		    	$ts['measure_code'] = $key;
		    	$test_rows[] = $ts;
		  }
    }
  }
  elseif ($row['mode']=='spreadsheet') {
  	$ts = $row;
  	foreach($row as $key => $value) {
  		if ($value && array_search($key, $tab_columns)==FALSE && $key!='mode') {
  			$ts['score'] = $value;
  			$ts['measure_code'] = $key;
  			$test_rows[] = $ts;
  		}
  	}
  }
  else {
    $test_rows = array();
    $test_rows[] = $row;
  }
  return $test_rows;
}
