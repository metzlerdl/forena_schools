<?php
include_once('import_preprocessors.php');
class ForenaSchoolsCSV {

	public $db;

	public function __construct() {
		$this->db = ForenaSchoolsDB::instance();
	}

	static function instance() {
		static $o = '';
		if (!$o) {
			$o = new ForenaSchoolsCSV();
		}
		return $o;
	}

	function preview_csv($file_name,$max_lines=10) {
	  // open the file
	  $file = fopen($file_name,'r');

	  // Read in a preview of the file
	  $i=0;
	  if ($file){
	      while ( ($i <= $max_lines) && (($data=fgetcsv($file))!=false )) {
	        $lines[] = $data;
	        $i++;
	      }
	  }
	  return $lines;
	}

	function default_csv_descriptor()
	{
	  return $field;
	}

	function csv_descriptor($lines) {
	   // Determine the filed headers if possible
	   $headers = $lines[0];
	   // Set the defaults for the descriptor
	   $field = array(
	      'label' => '',
	      'field_name' => '',
	      'pre_format' => '',
	      'cross_walk' => '',
	    );

	    // Load the header field entries into an array
	   foreach ((array)$headers as $idx=>$text) {
	     $desc[$idx] = $field;
	     $desc[$idx]['label'] = $text;
	   }

	   // Load the first 10 sample values into an array
	   foreach($desc as $key=>$field) {
	      $i=1;
	      while ($i <= count($lines)) {
	         $desc[$key]['row_'. $i] = $lines[$i][$key];
	         $i++;
	      }
	   }

	   return $desc;

	}

	/**
	 * Import a table from the contents of a csv file.
	 *
	 * This function will assume that the first row contains headers that are in match the column names
	 * in the table in the database.  The columns list is introspected from the table name and the insert statement is automatically
	 * generated based on the list of columns in the databse.  Any fields not in the csv file will be
	 * considered to be null.
	 *
	 * @return unknown_type
	 */
	function table_from_csv($table_name,$file_name,$del=false,$defaults=array(),$sep=",",$enc='"') {
		if (!trim($sep)) {
			$sep =',';
		}

		// Delete if we're supposed to
		if ($del) {
			$sql = 'delete from '.$table_name;
		    $this->db->query($sql);
		}

		// Need to get the fields from a view or table
		$tab_columns = array();
		$sql = $this->db->gen_insert($table_name, $tab_columns);
		if (!$sql) {
			return "Database table $table_name not found\n";
		}

	    if (!file_exists($file_name)) {
	    	return "Could not find file $file_name\n";
	    }
	    // Check for preprocessor
	    $fn_name = 'process_'.$table_name;
	    if (is_callable($fn_name)) {
	    	$proc_fn = $fn_name;
	    } else {
	    	$proc_fn = '';
	    }

		// Start reading the file.
		$file = fopen($file_name,'r');

		// Read in the header line for fields
	    $header = fgetcsv($file,null,$sep,$enc);
	    $fields = array();
	   	foreach ($header as $key=>$column) {
				$fields[$key] = strtolower($column);
		}
		$line = 1;
		// Import the file
	    while (($data=fgetcsv($file,null,$sep,$enc))!=false ) {
		  	$line++;

			$row = $defaults;
	    	foreach ($fields as $key=>$column) {
				$row[$column] = @$data[$key];
			}
	    if ($proc_fn) {
	      $rows = call_user_func($proc_fn, $row, $tab_columns);
	      if ($rows) foreach ($rows as $r) {
	      	$this->db->query($sql,$r);
	      }
	    }
	    else {
				$this->db->query($sql,$row);
	    }
	    }

	    return "$file_name: $line records imported";
	}
	
	/**
	 * Read in a full csv grouping rows as needed. 

	 */
	function readCSV($file_name, $defaults=array(), $group_by = array(),  $sep = ',', $enc='"') { 
		if (!trim($sep)) {
			$sep =',';
		}
    
		// Load File name
    if (!file_exists($file_name)) {
    	drupal_set_message( "Could not find file $file_name", 'error');
    	return; 
    }
    
		// Start reading the file.
		$file = fopen($file_name,'r');

		// Read in the header line for fields
	  $header = fgetcsv($file,null,$sep,$enc);
	  $fields = array();
	  foreach ($header as $key=>$column) {
				$fields[$key] = strtolower($column);
		}
		
		if ($group_by) { 
		  $empty_group = array_fill_keys($group_by, ''); 
		  $last_group = $empty_group; 
		  $group = array(); 
		}
	  $data = array(); 
		$line = 1;
		// Import the file
    while (($cols=fgetcsv($file,null,$sep,$enc))!=false ) {
	  	$line++;

  	  // Row starts out with default values passed in
  		$row = $defaults;
      foreach ($fields as $key=>$column) {
  			$row[$column] = @$cols[$key];
  		}
  		
  		//If we're grouping extract keys based on data
  		if ($group_by) { 
  		  $cur_values = array_intersect_key($row, $empty_group); 
  		  if ($cur_values != $last_group) { 
  		    if ($last_group!=$empty_group) {
  		      $data[] = $group; 
  		    }
  		    $last_group = $cur_values; 
		      $group = array(); 
  		  } 
    		$group[] = $row; 
  		} 
  		else { 
  		  $data[] = $row; 
  		}
  		
    }  
    // If we're grouping we need to save away the last group. 
    if ($group_by) { 
      $data[] = $group; 
    }
    
    return $data; 
	}

}
