<?php
// $Id$
/**
 * @file
 * Report administration forms and functions.
 */
require_once('forena.common.inc');

/**
 * Make sure a drectory exists in the report path prior to save.
 *
 * @param unknown_type $fullpath
 * @param unknown_type $recursive
 */
function _forena_verify_directory($fullpath, $recursive=FALSE) {
  static $path='';
  if (!$recursive) $path = forena_report_path();
  @list($dir, $file) = explode('/', $fullpath, 2);
  $path .= '/' . $dir;
  // Path
  if (!file_exists($path) && $file) {
    mkdir($path);
  }
  // Recurse to next file.
  if ($file && strpos($file, '/')) {
    watchdog('debug', $file);
    _forena_verify_directory($file, TRUE);
  }
}

/**
 * Recursively copy all report files from the source directory to the destination directory
 *
 * @param string $src_dir  Source directory
 */
function _forena_copy_reports($src_dir) {
  static $dest_dir = '';
  static $i = 0;
  if (!$dest_dir) {
    $dest_dir = forena_report_path();
  }
  if (!file_exists($dest_dir)) {
    mkdir($dest_dir);
  }

  $d = dir($src_dir);
  while (false !== ($rpt_file = $d->read())) {
    $src_file = $d->path . '/' . $rpt_file;
    $dest_file = $dest_dir . '/' . $rpt_file;

    if (is_file($src_file)) {
      file_put_contents($dest_file, file_get_contents($src_file));
      $i++;
    }
    else {
      if (strpos($rpt_file, '.')!==0) {

        // Recurse into sub directory
        $src_save = $src_dir;
        $dest_save = $dest_dir;
        $dest_dir .= '/' . $rpt_file;
        $src_dir .= '/' . $rpt_file;
        _forena_copy_reports($src_dir);
        $src_dir = $src_save;
        $dest_dir = $dest_save;
      }
    }
  }
  $d->close();
  return $i;
}

/**
 * Save the report file to disk
 *
 * @param string $name File name  to save report to
 * @param unknown_type $data
 */
function forena_save_report($report_name, $report, $save_file = FALSE, $altered = 0) {
  static $save_count=0;

  if ($report && !is_object($report)) {
    try {
      $report = new SimpleXMLElement($report);
    }
    catch (Exception $e) {
      forena_error(t('Could not save %s because XML was malformed', htmlspecialchars($report_name)),
        "<p>Invalid XML</p><pre>XML:" . htmlspecialchars($report) . "\n" . $e->getMessage() . "</pre>");
      return;
    }
  }

  _forena_verify_directory($report_name);
  $report_path = forena_report_path();
  $r = new FrxReport($report);
  $data['title'] = $r->title;
  $data['category'] = $r->category;
  $data['options'] = $r->options;
  $data['descriptor'] = $r->descriptor;
  $data['name'] = $report_name;
  $data['altered'] = $altered;
  $r_xml = $report->asXML();
  //@TODO: Clean up filename to make sure
  $name = $data['name'];
  $filepath = $report_path . '/' . $report_name . '.frx';
  // If we need to save this to the file system

  if ($save_file) {
    // Serialize the report for saving

    try {
      file_put_contents($filepath, $r_xml);
    } catch (Exception $e) {
      fornea_error('Error Saving Report', $e->getMessage());
    }
  }

  // Get the security caches from the reports
  if ($report) $cache = forena_load_cache($report); else $cache='';
  $rpt_cache='';
  if ($cache) $rpt_cache = serialize($cache);

  // Set default interpretations of data
  $data['enabled'] = (isset($data['enabled']) && $data['enabled']) ? 1 : 0;
  if (isset($data['options']['hidden']) && (string)$data['options']['hidden']) {
    $data['hidden'] =  ($data['options']['hidden'] && $data['options']['hidden']!='N' && $data['options']['hidden']!='0') ? 1:0;
    if (!$data['category']) $data['category'] = 'All';
  }
  else {
    // Set hidden based on category
    $data['hidden'] = ($data['category'])? 0 : 1;
  }

  // Save language info
  $lang = 'en';
  if (module_exists('locale')) {
  	@list($tlang, $tname) = explode('/', $name,2);
  	if (array_key_exists($tlang, language_list())) {
  		$lang = $tlang;
  		$name = $tname;
  	}
  }


  // Save to the Database
  if (file_exists($filepath)) {
    $modified = filemtime($filepath);
    $result = db_query("SELECT report_name FROM {forena_reports} WHERE report_name=:report_name AND language=:language", array(':report_name' => $name, 'language' => $lang) );
    if ($rpt = $result->fetchObject() ) {
      db_update('forena_reports')
        -> condition('report_name', $name)
        -> condition('language', $lang)
        ->fields(array(
          'title' => $data['title'],
          'descriptor' => $data['descriptor'],
          'category' => $data['category'],
          'hidden' => $data['hidden'],
          'cache' => $rpt_cache,
          'src' => $r_xml,
          'altered' => $data['altered'],
          'modified' => $modified))
        ->execute();
    }
    else {
      db_insert('forena_reports')->
        fields( array('report_name' => $name,
          'language' => $lang,
          'title' => $data['title'],
          'descriptor' => $data['descriptor'],
          'category' => $data['category'],
          'hidden' => $data['hidden'],
          'cache' => $rpt_cache,
          'src' => $r_xml,
          'altered' => $data['altered'],
          'modified' => $modified,
          ))
        ->execute();
    }

    $save_count++;
  }

  $r = NULL;
  $result = NULL;
  $r_xml = NULL;
  $report = NULL;
  $data = NULL;
  $rpt = NULL;
  $cache = NULL;
  $rpt_cache = NULL;


  return $save_count;
}

/**
 * Remove the report from the database and file system.
 * @param string $report_name
 */
function forena_delete_report($report_name, $delete_file = TRUE) {
  $report_path = forena_report_path();
  $language = 'en';


  $filepath = $report_path .'/'. $report_name .'.frx';
  $info = pathinfo($filepath);
  $do = TRUE;
  if (file_exists($filepath)) {
    chdir($info['dirname']);
    if ($delete_file) $do = unlink($info['basename']);
  }
  if (module_exists('locale')) {
    @list($tlang, $tname) = explode('/', $report_name);
    if (array_key_exists($tlang, language_list())) {
    	$report_name = $tname;
    	$language = $tlang;
    }
  }
  if ($do) {
    db_delete('forena_reports')
      ->condition('report_name', $report_name)
      ->condition('language', $language)
      ->execute();
  }
  else {
    drupal_set_message('Unable to delete file '. $info['basename'],'error');
  }
}

/**
 * Syncronize the data
 *
 */
function forena_db_sync($subdir='') {
  static $prefix = '';
  if (!$subdir) {
    $prefix = '';
     db_delete('forena_reports')
       ->execute();
  }
  $path =forena_report_path() . '/' . $subdir;
  $d = dir($path);
  if ($d) while (false !== ($rpt_file = $d->read())) {
    $src_file = trim($d->path, '/') . '/' . trim($rpt_file, '/');
    $dest_file = $path . '/' . trim($rpt_file, '/');
    if (is_file($src_file)) {
      list($report_name, $ext) = explode('.', $rpt_file, 2);
      if ($ext == 'frx') {
        $report_name = trim($prefix . '/' . $report_name, '/');

        try {
          $r_xml =file_get_contents($src_file);
        } catch (Exception $e) {
          $s = t('unable to load Report %s', $r_xml);
          forena_error($s, $s . $e->getMessage());

        }

        // Load the report

        $r = new FrxReport($r_xml);
        $save_count = forena_save_report($report_name, $r_xml, FALSE);
      }
    }
    elseif (is_dir($src_file)) {
      if (strpos($rpt_file, '.')!==0) {
        $save_prefix = $prefix;
        $prefix .=  '/' . $rpt_file;
        $prefix = trim($prefix, '/');
        forena_db_sync($prefix);
        $prefix = $save_prefix;
      }
    }
  }
  if ($d) $d->close();

  return $save_count;
}

/**
 * Accepts the name of a file
 *
 * Returns an editor object of the file.
 *
*/
function forena_get_report_editor($report_name) {
  require_once('FrxReportEditor.inc');
  if ($report_name) {
    $r_text='';
    $report_path = forena_report_path();
    $filename = $report_path . '/' . $report_name . '.frx';
    if (file_exists($filename)) {
      $r_text = file_get_contents($filename);
    }
    $r = new FrxReportEditor($r_text);
    return $r;
  }
  else {
    return new FrxReportEditor();
  }
}


function forena_filter_element($fmt) {
  global $user;
  $element['format'] =  array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Input formats'),
  );
    // Get a list of formats that the current user has access to.
  $formats = filter_formats($user);
  foreach ($formats as $format) {
    $options[$format->format] = $format->name;
    $element['format']['guidelines'][$format->format] = array(
      '#theme' => 'filter_guidelines',
      '#format' => $format,
    );
  }
  $element['format']['guidelines']['#weight']=12;

  $element['format']['forena_input_format'] = array(
    '#type' => 'select',
    '#title' => t('Text format'),
    '#options' => $options,
    '#default_value' => $fmt,
    '#access' => count($formats) > 1,
    '#weight' => 10,
    '#attributes' => array('class' => array('filter-list')),
  );

  $element['format']['help'] = array(
    '#type' => 'container',
    '#theme' => 'filter_tips_more_info',
    '#attributes' => array('class' => array('filter-help')),
    '#weight' => 11,
  );
  return $element['format'];
}

/**
 * Forena admin settings form
 *
 */
function forena_settings() {
  $report_path = forena_report_path();


  $form['forena_report_repos'] = array(
    '#type' => 'textfield',
    '#title' => t('Report Repository'),
    '#description' => t('Indicate the directory that you want to use for your reports.  In order for you to ' .
                        'to be able to save reports, this directory should be writable by the web user. Relative' .
                        'paths should be entered relative to the base path of your drupal installation.'),
    '#default_value' => $report_path,
  );

  $form['forena_input_format'] = forena_filter_element(variable_get('forena_input_format', filter_default_format()));

  $form['instructions'] = array(
    '#type' => 'item',
    '#title' => t('Data Sources'),
    '#value' => '<p>' . t('Database connections and data block repositories are configured directly in the file system for ' .
                         'security reasons.  See the Forena Reports README.txt file for more information.') . '</p>',
  );

  $form['forena_default_form'] = array(
    '#type' => 'textfield',
    '#title' => t('Default report form'),
    '#description' => t('Indicates the default style sheet(s) that will be used when no tag is present in the report.'),
  );


  $form =  system_settings_form($form);
  $form['#submit'][] = 'forena_settings_submit';
  return $form;

}
/**
 * Added submit handler to create directories and clear menu cache
 *
 * @param unknown_type $form
 * @param unknown_type $form_state
 */
function forena_settings_submit($form, &$form_state) {
  $values = $form_state['values'];
  $path = $values['forena_report_repos'];
  $src_dir = drupal_get_path('module', 'forena') . '/repos/reports';
  if (!file_exists($path)) {
    try {

      if (file_exists($path)) {
        drupal_set_message(t('Created directory %s', array($path))) ;
      }
      mkdir($path);

    } catch (Exception $e) {
      forena_error(t('Unable to create report directory'), $e->getMessage());
    }

  }

  if (file_exists($path) && $path != $src_dir) {
    // Copy the reports from the

    $i = _forena_copy_reports($src_dir);
    drupal_set_message($i . ' delivered reports copied from ' . $src_dir . ' to ' . $path);
  }
  $save_count = forena_db_sync();

  menu_cache_clear();
}


function forenea_general_form($formid, $form_state, $new_report='') {
  $desc = forena_report_desc();
  $name = $desc['name'];
  $add_new_rpt;
  //determine if this is an add new report request
  if ($new_report == 'add') {
    $add_new_rpt = TRUE;
  }

  $filename = $desc['filename'];
  $format = $desc['format'];
}


/**
 * Form function for the edit report form
 * @param $form_state
 * @return the form
 */
function forena_layout_form($formid, $form_state, $new_report='') {
  $desc = forena_report_desc();
  $name = $desc['name'];
  global $language;
  //determine if this is an add new report request
  if ($new_report) {
    $add_new_rpt = TRUE;
  }
  else $add_new_rpt = FALSE;
  $filename = $desc['filename'];
  $format = @$desc['format'];
  if ($name) {
    if ((isset($desc['exists']) && $desc['exists']) || $add_new_rpt) {
    	$save_name = $name;
      //set the name to empty string for new reports
      if ($add_new_rpt) $name = '';
      $r = forena_get_report_editor($name);

      if ($add_new_rpt) {
        $title = '';
        $options = '';
        $attributes ='';
        $report_form ='';
        $hidden = '0';
        $category='';
        $head = '';
        $body = '';
        $lang = $language->language;

      }
      else {
        $title = (string)$r->title;
        if (module_exists('locale')) {
	        @list($tlang,	$tsave_name) = explode('/', $name, 2);
	        // FInd out if the starting name of the report is an installed language.
	        if (array_key_exists($tlang, language_list() )) {
	        	$lang = $tlang;
	        	$save_name = $tsave_name;
	        }
	        else {
	        	$lang = 'en';
	        }
        }
        drupal_set_title(filter_xss($r->title));
        // Need to get all option attributes
        $frx_options = $r->getOptions();
        $hidden = @$frx_options['hidden']=='1' ? 1 : 0;
        $report_form = @$frx_options['form'];
        $attributes = $r->get_attributes_by_id();
        $category = $r->getCategory();
        $body = $r->simplexml->body->asXML();
      }
      $form = array();

      //array of xml attributes that are required to have a value
      $required = array('id' => TRUE, 'label' => TRUE);
      //list of supported document formats
      $supported = FrxReportGenerator::instance()->supported_doctypes();
      $doclist = variable_get('forena_doc_formats', array());

      $form['report_name'] = array(
        '#type' => 'value',
        '#value' => $name,
      );


      $form['save_report_name'] = array(
        '#type' => 'textfield',
        '#title' => t('Report Name'),
        '#default_value' => $save_name,
        '#description' => t('Enter only letters, numbers, and special characters:  - _ /
                             <br/>White space is not permitted.
                             Create a directory using the format: (directory name) / (report name). Save multiple reports to the same directory
                             by referencing the same name.'),
        '#required' => TRUE,
      );

      $languages = array();
      if (module_exists('locale')) {
      	foreach (language_list() as $key=>$obj) {
          $languages[$key] = $obj->native;
      	}
      	$form['report_lang'] = array(
      	  '#type' => 'value',
      	  '#value' => $lang,
      	);
      	$def_lang = isset($_GET['language']) ? $_GET['language'] : $lang;
	      $form['save_report_language'] = array(
	        '#type' => 'select',
	        '#title' => t('Language'),
	        '#options' => $languages,
	        '#default_value' => $def_lang,
	      );
      }

      $form['title'] = array(
        '#type' => 'textfield',
        '#title' => t('Title'),
        '#default_value' => $title,
      );

      $form['category'] = array(
        '#type' => 'textfield',
        '#title' => t('Category'),
        '#default_value' => $category,
        '#autocomplete_path' => 'forena/categories/autocomplete',
        '#description' => t('The heading your report will be grouped under on the report list.'),
      );

      $form['form'] = array(
        '#type' => 'textfield',
        '#title' => t('Form'),
        '#default_value' => $report_form,
        '#description' => t('The page style of your report, such as letter or landscape. The default form is letter.')
      );

      $form['hidden'] = array(
        '#type' => 'checkbox',
        '#title' => t('Hidden'),
        '#default_value' => $hidden,
        '#description' => t('Hide your report from showing up on the report list.'),
      );

      //begin checking doc generation options
      if ($r) $nodes = $r->simplexml->head->xpath('frx:docgen/frx:doc');
      if ($doclist) {
        $form['docgen'] = array(
         '#tree' => TRUE,
         '#type' => 'fieldset',
         '#title' => t('Document Options'),
         '#collapsible' => TRUE,
         '#collapsed' => TRUE,
         '#description' => t('These are document transformation options. Options selected will display as links in your report view.')
        );

        //build the options and default list
        $options = array();
        $default = array();
        foreach ($doclist as $key => $value) {
          if (is_object(FrxReportGenerator::instance()->get_doctypes($value))) {
            $options[$value] = strtoupper($value);
            $doc =  isset($r) ? $r->simplexml->head->xpath('frx:docgen/frx:doc[@type="' . $value . '"]') : '';
            if ($doc && $doclist[$value]) {
              $default[$value] = $value;
            }
          }
        }
        //display checkboxes
        $form['docgen']['docs'] = array(
          '#type' => 'checkboxes',
          '#description' => t('If no options are selected, the system will display all of the above as available for this report.'),
          '#options' => $options,
          '#default_value' => $default
        );
      }

      $form['attributes'] = array(
        '#type' => 'value',
        '#value' => $attributes,
      );

      $form['body'] = array(
        '#type' => 'text_format',
        '#title' => t('Body'),
        '#default_value' => $body,

        '#rows' => 25,
      '#format' => variable_get('forena_input_format', filter_default_format())
      );

//      $form['format'] = forena_filter_element(variable_get('forena_input_format', FILTER_DEFAULT));
      $form['#validate'][]='forena_layout_form_validate';
      $form['buttons']['save'] = array(
        '#type' => 'submit',
        '#value' => 'Save',
        '#submit' => array('forena_layout_form_submit'),
      );

      if (user_access('delete report')) {
        $form['buttons']['delete'] = array(
          '#type' => 'submit',
          '#value' => 'Delete',
          '#submit' => array('forena_edit_delete_submit'),
        );
      }
      return $form;
    }
    else {
      drupal_not_found();
    }
  }
  else {
    drupal_not_found();
  }
}

function forena_layout_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $regexp = "/^[A-Za-z0-9\/\_\-]*$/";
  $save_report_name = $values['save_report_name'];
  $report_name = $values['report_name'];

  //checking illegal characters
  if (!preg_match($regexp, $save_report_name)) {
    form_set_error('save_report_name', t('Invalid character entered in Report Name'));
  }
  if (@$values['save_report_language']) $save_report_name = $values['save_report_language'] . '/' . $save_report_name;
  //comparing the report names to see if they have changed.
  //If they have, making sure the new name does not already exist.
  if ($report_name != $save_report_name) {
    $report_path = forena_report_path();
    $filename = $report_path . '/' . $save_report_name . '.frx';
    if (file_exists($filename)) {
      form_set_error('save_report_name', t('The file ' . $save_report_name . ' already exists. Please enter another name.'));
    }
  }
}

/**
 * builds a string of the xml document,
 * submits it to forena_save_report.
 */
function forena_layout_form_submit($form, &$form_state) {
  $nodes = array();
  $values = $form_state['values'];
  $report_name = $values['save_report_name'];
  if (@$values['save_report_language'] && @$values['save_report_language']!='en') $report_name = $values['save_report_language'] . '/' . $report_name;
  $r = forena_get_report_editor($values['report_name']);
  // Title and category
  $r->setTitle($values['title']);
  $r->setCategory($values['category']);
  // Form options
  $options = array(
    'hidden' => $values['hidden'],
    'form' => $values['form']);
  $r->setOptions($options);

  // Doc gen settings.
  if (isset($form['docgen'])) {
    $docgen = array();
    if($selected = array_filter(@$values['docgen']['docs'])) {
      if ($selected) foreach ($selected as $key => $value) {
       if ($value) $docgen[] = array('type' => $key);
      }
    }
    $r->setDocgen($docgen);
  }

  // Body
  $r->setBody($values['body']['value']);
  // If there are no frx attributes in the body then replace them with the old values.
  $frx_nodes = $r->simplexml->xpath('body//*[@frx:block]');
  if (!$frx_nodes) {
    $r->save_attributes_by_id($values['attributes']);
  }

  //determine redirection.
  $report_path = forena_report_path();
  $filename = $report_path . '/' . $report_name . '.frx';
  if (!file_exists($filename)) {
    $new_rpt = TRUE;
  }
  else {
    $new_rpt = FALSE;
  }

  if (forena_save_report($report_name, $r->asXML(), TRUE) == 1) {
    drupal_set_message(t('Your report, %s has been saved.', array('%s'=>$report_name)));
    //if this is a new report then redirect to data blocks

    if ($new_rpt) {
      $form_state['redirect']= 'reports/' . str_replace('/', '.', $report_name) . '/data';
    }
    else{
      $form_state['redirect']= 'reports/' . str_replace('/', '.', $report_name) . '/layout';
    }
  }
}

/*
 * administer the settings for document format options
 */
function forena_doc_formats_settings() {
  $supported_doctypes = FrxReportGenerator::instance()->supported_doctypes();
  $form['forena_doc_formats'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Allowed Document Formats'),
    '#default_value' => variable_get('forena_doc_formats', $supported_doctypes),
    '#description' => t('check your desired document format'),
    '#options' => $supported_doctypes,
  );

  $form['forena_doc_defaults'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Default Document Formats'),
    '#default_value' => variable_get('forena_doc_defaults', array()),
    '#description' => t('check your desired document format'),
    '#options' => $supported_doctypes,
  );

  $form['forena_email_override'] = array(
    '#type' => 'checkbox',
    '#title' => 'Run email merges in test mode' ,
    '#default_value' => variable_get('forena_email_override',FALSE),
    '#description' => t('When this box is checked emails are sent to the currently logged in user.  Useful for testing environments.'),
  );

  return system_settings_form($form);
}

/*
 * administer the settings for document format options
 */
function forena_data_settings() {
  $repos = FrxReportGenerator::instance()->app->repositories();
  $r_list = array();
  $headers = array(t('Name'),t('Description'), t('Path'), t('Operation'));
  foreach ($repos as $name=>$r) {
    $r_list[] = array(
      $name,
      $r['title'],
      $r['path'],
      l('configure', 'admin/config/content/forena/data/configure/'. $name)
    );

  }

  $output = '<ul class="action-links"><li>' . l('Add data source', 'admin/config/content/forena/data/add') . '</li></ul>';
  $output .= theme_table(array('header' => $headers, 'rows' => $r_list, 'attributes' => array(), 'caption' => '', 'sticky' => true, 'colgroups' => array(), 'empty' => ''));
  return $output;
}

function forena_data_settings_edit($form, &$form_state, $source=-1) {
  $adding = ($source === -1);
  if (!@$form_state['storage']) {
    if ($adding) {
      $form_state['storage'] = array(
        'name' => '',
        'title' => '',
        'path' => '',
        'config' => array(
          'source' => 'user',
          'data provider' => 'FrxDrupal',
          'access callback' => 'user_access',
          'user callback' => 'forena_current_user_id'
        ),
      );
    }
    else {
      $repos = FrxReportGenerator::instance()->app->repositories();
      $r = $repos[$source];
      $load = FrxReportGenerator::instance()->repository($source);
      // Remove teh object from the data.
      unset($load['data']);
      $form_state['storage'] = array(
        'name' => $source,
        'title' => $r['title'],
        'path' => @$r['path'],
        'config' => $load,
      );

    }
  }
  $data = $form_state['storage'];
  $config = $data['config'];

  $locked = !($adding || (@$config['source'] == 'user'));
  $values = @$form_state['values'];

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Machine readable name.  Used in referencing all data used by this source. must should not contain any special characters or spaces.'),
    '#disabled' => !$adding,
    '#default_value' => $data['name'],
    '#required' => TRUE,
   );

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#required' => TRUE,
    '#description' => t('Human readable name that describes the data source.  This primarily occurs in error messages where the data source cannot be accessed.'),
    '#default_value' => $data['title'],
  );

  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#description' => t('Disabling will cause all queries to return no data.'),
    '#default_value' => @$data['enabled']!==0,
  );

  $form['debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Debug'),
    '#description' => t('Write information to the screen and logs for each query executed.'),
    '#default_value' => @$config['debug'],
  );

  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path'),
    '#required' => TRUE,
    '#disabled' => $locked,
    '#description' => t('Directory containing data block files.'),
    '#default_value' => @$data['path'],
  );

  $user_options = array(
    '' => 'None',
    'forena_current_user_id' => 'UID',
    'forena_current_user_name' => 'User name',
  );

  $form['user_callback'] = array(
    '#type' => 'select',
    '#title' =>  'Current user',
    '#disabled' => $locked,
    '#description' => t('Can be refererenced as :current_user in each data block.'),
    '#options' => $user_options,
    '#default_value' => @$config['user callback'],
    '#disabled' => $locked,
  );


  // Access method list
  $access = array(
    'callback' => t('Use drupal permissions'),
    'block' => t('Match values provided by a data block. '),
  );

  $form['access_method'] = array(
    '#type' => 'select',
    '#options' => $access,
    '#disabled' => $locked,
    '#title' => t('Data security method'),
    '#default_value' => empty($config['access block']) ? 'callback' : 'block',
    '#description' => t('Specify how the ACCESS defined for a data block is to be interpreted. '),
    '#ajax' => array(
      'callback' => 'forena_access_info_callback',
      'wrapper' => 'access-details',
    ),
  );

  $form['access_details'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<div id="access-details">',
    '#suffix' => '</div>',
    '#title' => t('Details'),
  ) ;

  switch (!empty($values['access_method']) ? $values['access_method'] : $form['access_method']['#default_value']) {
	  case 'block':
	  	$form['access_details']['access_block'] = array(
	  	  '#type' => 'textfield',
	  	  '#title' => 'Data block providing permissions list',
	  	  '#disabled' => $locked,
	  	  '#autocomplete_path' => 'forena/data_block/autocomplete',
	  	  '#description' => t('The datablock to be used to interpret permissions.  This should return a single column of permissions based on the current user.   May be provided by another repository.'),
	  	  '#default_value' => @$config['access block'],
	  	);
	  	break;
	  default:
	  	$form['access_details']['access_callback'] = array(
	  	  '#type' => 'item',
	  	  '#title' => 'Access callback',
	  	  '#disabled' => $locked,
	  	  '#markup' => @$config['access callback'],
	  	);
  }



  // Driver list
  $drivers = array(
    'FrxDrupal' => t('Drupal'),
    'FrxOracle' => t('Oracle Database'),
    'FrxPDO' => t('PDO other than Drupal '),
    'FrxPostgres' => t('Postgres Database'),
    'FrxMSSQL' => t('MSSQL Database'),
    'FrxFiles' => t('XML Files'),
  );

  $form['data_provider'] = array(
    '#type' => 'select',
    '#title' => t('Driver'),
    '#description' => t('Forena data connection type'),
    '#options' => $drivers,
    '#disabled' => $locked,
    '#default_value' => $config['data provider'],
    '#ajax' => array(
      'callback' => 'forena_connection_info_callback',
      'wrapper' => 'conn-div',
    ),
  );

  $form['connection'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => 'Connection info',
   '#prefix' =>  '<div id="conn-div">',
   '#suffix' => '</div>',
  );
  $data_provider = (!empty($form_state['values']['data_provider']) ? $form_state['values']['data_provider'] : $config['data provider']);

  // Common controls used in mulitple providers.
  $uri = array(
    '#type' => 'textfield',
    '#title' => t('uri'),
    '#descripton' => t('Connection string: see appropriate php documentation for more details.'),
    '#default_value' => @$config['uri'],
    '#required' => TRUE,
    '#disabled' => $locked,
  );

  $user = array(
    '#type' => 'textfield',
    '#title' => t('User'),
    '#default_value' => @$config['user'],
  );
  $password= array(
    '#type' => 'password',
    '#title' => t('Password'),
    '#default_value' => @$config['password'],
   );

  switch ($data_provider) {
    case 'FrxDrupal':
      $form['connection']['uri'] = array(
        '#type' => 'item',
        '#title' => t('URI'),
        '#disabled' => $locked,
        '#markup' => 'Determined by Drupal settings.php file',
      );
      break;
    case 'FrxMSSQL':
    	$form['connection']['uri'] = $uri;
    	$form['connection']['user'] = $user;
    	$form['connection']['new_password'] = $password;
      $form['connection']['mssql_xml'] = array(
        '#type' => 'checkbox',
        '#disabled' => $locked,
        '#title' => t('Microsoft SQL native XML'),
        '#description' => t('Use for XML auto queries to generate XML.'),
        '#default_value' => $config['mssql_xml'],
      );
    	    	break;
    case 'FrxOracle':
    	$form['connection']['uri'] = $uri;
      $form['connection']['user'] = $user;
      $form['connection']['new_password'] = $password;
      $form['connection']['character_set'] = array(
        '#type' => 'textfield',
        '#title' => t('Character Set'),
        '#disabled' => $locked,
        '#description' => t('Leave blank for default character set'),
        '#default_value' => @$config['character_set'],

      );
      $form['connection']['oracle_xml'] = array(
        '#type' => 'checkbox',
        '#title' => t('Oracle native XML'),
        '#disabled' => $locked,
        '#description' => t('Use the function provided with Forena to generate XML.  Requires installing a function into the database'),
        '#default_value' => $config['oracle_xml'],
      );
      break;
    case 'FrxPDO':
      $form['connection']['uri'] = $uri;
     	$form['connection']['user'] = $user;
     	$form['connection']['new_password'] = $password;
     	break;
    case 'FrxPostgres':
    	$form['connection']['uri'] = $uri;
    	$form['connection']['new_password'] = $password;
      $form['connection']['postgres_xml'] = array(
        '#type' => 'checkbox',
        '#title' => t('Postgres native XML'),
        '#disabled' => $locked,
        '#default_value' => @$config['postgres_xml'],
        '#description' => t('Use Postgres native XML support.  Requires Posgres 8.3 or better'),
      );

    default:
      $form['connection']['uri'] = $uri;
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('forena_data_settings_save'),
  );

  $form['delete'] = array(
    "#type" => 'submit',
    '#value' => t('Delete'),
    '#submit' => array('forena_data_settings_delete'),
  );

  return $form;
}

/**
 * Submit handler to cause save events to happen.
 * Enter description here ...
 */
function forena_data_settings_save($form, &$form_state) {
  $values = $form_state['values'];
  $name = $values['name'];
  $config = $form_state['storage']['config'];
  $config['path'] = @$values['path'];
  $config['data provider'] = $values['data_provider'];
  $config['user callback'] = $values['user_callback'];
  $config['debug'] = $values['debug'];
  if (@$values['connection']['new_password']) {
	  $values['connection']['password'] = $values['connection']['new_password'];
  }
  if (isset($values['connection']['new_password'])) unset($values['connection']['new_password']);
  if (is_array($values['connection'])) $config = array_merge($config, $values['connection']);
  if (empty($values['access_block'])) {
  	$config['access callback'] = empty($values['access_callback']) ? 'user_access' : $values['access_callback'];
  }
  else {
  	$config['access block'] = $values['access_block'];
  }
  $config_str = serialize($config);
  $result = db_query('SELECT * FROM {forena_repositories} WHERE repository = :name', array(':name' => $name));

  if ($result) drupal_set_message(t('Data connection settings saved'));
  if ($repos = $result->fetchObject()) {
    db_update('forena_repositories')
      ->fields(array(
        'title' => $values['title'],
        'enabled' => $values['enabled'],
        'config' => $config_str,
       ))
      ->condition('repository', $name, '=')
      ->execute();
  }
  else {
  	$form_state['#redirect'] = 'admin/forena/content/data';
    db_insert('forena_repositories')
      ->fields(array(
        'repository' => $name,
        'title' => $values['title'],
        'enabled' => $values['enabled'],
        'config' => $config_str,
      ))
      ->execute();
  }
  $form_state['redirect'] = 'admin/config/content/forena/data';
}

/**
 * Delete the function
 * Enter description here ...
 * @param unknown_type $form
 * @param unknown_type $form_state
 */
function forena_data_settings_delete($form, &$form_state) {
	if (!@$form_state['storage]']['locked']) {
		$form_state['redirect'] = 'admin/config/content/forena/data';
		db_delete('forena_repositories')
		 ->condition('repository', $form_state['values']['name'], '=')
		 ->execute();
	}

}

/**
 * Generates the controls for external databases.
 * Enter description here ...
 */
function forena_connection_info_callback($form, $form_state) {
  return $form['connection'];

}

function forena_access_info_callback($form, $form_state) {
	return $form['access_details'];
}


/**
 *
 * @param $form_state
 * @return a form to edit the fields of the report
 */
function forena_fields_form($formid, $form_state) {
  $desc = forena_report_desc();
  $name = $desc['name'];
  $filename = $desc['filename'];
  @$format = $desc['format'];
  if ($name) {
    if (file_exists($filename)) {
      $r = forena_get_report_editor($name);
      drupal_set_title($r->title);
      $regexp = FRX_TOKEN_EXP;
      $match = array("{", "}");

      /*search the body, looking for {}*/
      $body = $r->simplexml->body->asXML();
      $head = $r->simplexml->head;

      $fields = array();
      preg_match_all($regexp, $body, $out);
      foreach ($out as $key => $value) {
        foreach ($value as $el) {
          $clean_element = str_replace($match, "", $el);
          $fields[$clean_element] = $clean_element;
        }
      }

      $form = array();

      $form['report_name'] = array(
        '#type' => 'value',
        '#value' => $name,
      );

      $form['fields'] = array('#tree' => TRUE, );
      /*Now check the fields in the body against the xml*/
      $i=0;
      foreach ($fields as $field) {
        $i++;
        $field_ids[$i] = $field;
        $form['fields'][$i] = array(
          '#tree' => TRUE,
          '#type' => 'fieldset',
          '#title' => $field,
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
        );

        $path = 'frx:fields/frx:field[@id="' . $field . '"]';
        $node =  $head->xpath($path);
        $attr= array();
        $default = '';
        if ($node) {
          $attr = $node[0];
          $default = (string) $node[0];
        }

        $form['fields'][$i]['id'] = array(
          '#type' => 'value',
          '#value' => $field,
        );

        $form['fields'][$i]['format'] = array(
          '#type' => 'textfield',
          '#title' => t('format'),
          '#default_value' => @$attr['format'],
          '#size' => 30,
          '#autocomplete_path' => 'forena/fields/format/autocomplete',
          '#description' => t('Format a date and time field by entering the name of a supported format function. Enter a "*" to see all available formats.')
        );

        $form['fields'][$i]['format-string'] = array(
          '#type' => 'textfield',
          '#title' => t('format-string'),
          '#default_value' => @$attr['format-string'],
          '#size' => 30,
          '#description' => t('The display type of your format.')
        );

        $form['fields'][$i]['link'] = array(
          '#type' => 'textfield',
          '#title' => t('link'),
          '#default_value' => @$attr['link'],
          '#size' => 100,
          '#maxlength' => 256,
          '#description' => t('Create a link that incorporates this field, e.g "profile/{field_name}" will create a link to this field_name\'s profile. *Note the field must be wrapped in {}.')
        );

        $form['fields'][$i]['target'] = array(
          '#type' => 'textfield',
          '#title' => t('target'),
          '#default_value' => @$attr['target'],
          '#size' => 30,
          '#description' => t('Link target eg. _BLANK')
        );

        $form['fields'][$i]['default'] = array(
          '#type' => 'textfield',
          '#title' => t('default value'),
          '#default_value' => $default,
          '#size' => 30,
          '#description' => t('The value to be displayed in the report in the place of the field.')
        );
      }
      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Save',
      );
      return $form;
    }
    else {
      drupal_not_found();
    }
  }
  else {
    drupal_not_found();
  }
}


function forena_fields_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $name = $values['report_name'];

  $r = forena_get_report_editor($name);
  /*now build the fields*/
  $fields = $values['fields'];
  $r->setFields($fields);

  if (forena_save_report($name, $r->asXML(), TRUE) == 1) {
    drupal_set_message(t('Your report, %s has been saved.', array('%s' => $name)));
  }
  else {
    drupal_set_message(t('There was an error saving your report, %s to the database.', array('%s' => $name)));
  }

}

/**
 * A form to preview and add data blocks to an existing report
 * @param unknown_type $form_state
 * @return unknown_type
 */
function forena_data_block_form($formid, &$form_state) {
  $desc = forena_report_desc();
  $name = $desc['name'];
  $filename = $desc['filename'];
  $format = isset($desc['format']) ? $desc['format'] : '';
  if ($desc['exists']) {
    $r = forena_get_report_editor($name);
    drupal_set_title($r->title);
    $form = array();
    $title = (string)$r->title;

    $template_array = FrxReportGenerator::instance()->supported_templates();
    @$default_template = $form_state['storage']['template'];
    @$params = $form_state['storage']['parameters'];
    @$param_values = $form_state['storage']['param_values'];

    $form = array();

    $form['report_name'] = array(
        '#type' => 'value',
        '#value' => $name,
     );


    //The default submit handler
    //If someone presses enter, this handler will execute
    $form['default_submit'] = array(
      '#type' => 'submit',
      '#submit' => array('forena_data_block_form_submit'),
      '#prefix' => '<div style="visibility:hidden;float:right">',
      '#suffix' => '</div>',
    );

    //find the datablocks in the existing report
    $data_block_array = array();
    if ($r) $r->get_attributes_by_id();
    $body = $r->simplexml->body;
    $path = '//*[@frx:block]';

    if ($body) foreach ($body->xpath($path) as $node) {
      $attrs = $node->attributes(FRX_NS);
      $id = (string)$node['id'];
      $block = (string)$attrs['block'];
      $clause = ($attrs['clause']) ?  "(" . (string)$attrs['clause'] . ")" : '';
      $block_info = FrxReportGenerator::instance()->load_block($block, $clause);
      $access = $block_info['access'];
      $data_block_array[$id] = $block . ": <i>" . $clause . ' security "' . $access . '"</i>';
    }

    $form['delete_blocks'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Data Blocks In Report'),
      '#options' => $data_block_array,
      '#description' => ($data_block_array) ? t('Check the data block to be deleted from your report.') : '',
    );

    if ($data_block_array) {
      $form['delete'] = array(
        '#type' => 'submit',
        '#value' => 'Delete',
        '#submit' => array('forena_data_block_delete'),
      );
    }

    $form['data_block'] = array(
      '#type' => 'textfield',
      '#title' => t('Data Block'),
      '#default_value' => @$form_state['storage']['data_block'],
      '#autocomplete_path' => 'forena/data_block/autocomplete',
      '#description' => t('Enter a data block, a preview will appear below'),
    );

    $form['where_clause'] = array(
      '#type' => 'textfield',
      '#title' => t('Where Clause'),
      '#default_value' => @$form_state['storage']['where_clause'],
      '#description' => t('Enter a where clause to filter your report.'),
    );

    $form['templates'] = array(
      '#type' => 'radios',
      '#title' => t('Templates'),
      '#default_value' => ($default_template) ? $default_template : $template_array['table'],
      '#options' => $template_array,
      '#required' => TRUE,
      '#description' => t('Select a template to preview the data block in.'),
    );

    //If there are parameters in the data block allow the user to input data
    //Check if there is already a parameter of the same name in the existing report. If there is, use the report value.
    if ($params) {
      $rpt_params = array();
      //returns a 2 dimensional array of report parameters from the report
      if ($r) $rpt_params = @$r->parameters;

      $form['params'] = array(
        '#tree' => TRUE,
        '#title' => 'Parameters',
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => ($form_state['storage']['output']) ? TRUE : FALSE,
      );

      foreach ($params as $param) {
        $p_val = isset($rpt_params[$param]) ? $rpt_params[$param]: '';
        $form['params'][$param] = array(
          '#type' => 'textfield',
          '#title' => t($param),
          '#default_value' => (isset($param_values[$param])) ? $param_values[$param] : $p_val,
        );
      }
    }

    if (isset($form_state['storage']['output'])) {
      $form['output_text'] = array(
        '#type' => 'markup',
        '#markup' => '</p>' . $form_state['storage']['output'] . '</p>',
      );
    }

    $form['preview'] = array(
      '#type' => 'submit',
      '#value' => 'Preview',
    );

    if (@$form_state['storage']['output'] != '') {
      $form['add'] = array(
        '#type' => 'submit',
        '#value' => 'Add',
        '#submit' => array('forena_data_block_add'),
      );
    }
    return $form;
  }
  else {
    drupal_not_found();
   }
}

/**
 * Validates the forena_data_block_form's data_block field.
 * @param $form
 * @param $form_state
 * @return unknown_type
 */

function forena_data_block_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $data_block = $values['data_block'];
  $delete_blocks = $values['delete_blocks'];

  //validate the data block only if delete block was not chosen
  if (!$delete_blocks) {
    $block_info = FrxReportGenerator::instance()->load_block($data_block);
    if (!$block_info) {
      form_set_error('data_block', t('Invalid data block.'));
    }
  }
}



/**
 * The Preview submit handler for forena_add_block_form
 * Renders datablock into a report
 * @param $form
 * @param $form_state
 * @return unknown_type
 */
function forena_data_block_form_submit($form, &$form_state) {
	$xml='';
	$block_info='';
  $values = $form_state['values'];
  $data_block = $values['data_block'];
  $template = $values['templates'];
  $params = isset($values['params']) ? $values['params'] : array();
  $name = $values['report_name'];
  $where_clause = $values['where_clause'];
  $r = FrxReportGenerator::instance()->get_report($name);

  $form_state['storage']['param_values'] = $params;
  $form_state['storage']['data_block'] = $data_block;
  $form_state['storage']['template'] = $template;
  $form_state['storage']['where_clause'] = $where_clause;
  $form_state['rebuild'] = TRUE;
  if ($data_block)  {
     //get xml from data block
    //check for and store parameters
    $block_info = FrxReportGenerator::instance()->load_block($data_block, $where_clause);
    if ($block_info) {
      //if there were not any parameters passed. Use the report parameters
      if (!$params) {
        $rpt_params = $r->parameters;
        if ($rpt_params) foreach ($block_info['tokens'] as $index => $id) {
          $params[$id] = @$rpt_params[$id]['value'];
        }
      }

      //now invoke the data provider with the correct params
      FrxData::instance()->push($params, 'parm');
      $xml = FrxReportGenerator::instance()->invoke_data_provider($data_block, $params, $where_clause);
    }
  }
  if ($xml) {

    //create an array of columns
    $rows = $xml->xpath('//row');
    $column_array = array();

    foreach ($rows as $columns) {
      foreach ($columns as $name => $value) {
        $column_array[$name] = $name;
      }
    }

    //create xml from template object
    $template_obj = FrxReportGenerator::instance()->get_templates($template);
    if ($template_obj) {
      $body = $template_obj->$template($column_array, $data_block, $where_clause);
    }
    $rpt_xml =  '<html xmlns:frx="urn:FrxReports"><body>' . $body . '</body></html>';

    //render the xml
    $output = forena_render_report($rpt_xml, NULL, $params);
    $form_state['storage']['output'] = $output;
    $form_state['storage']['template_body'] = $body;

  }
  else { //there wasn't any xml returned. clear the storage
    $form_state['storage']['output'] = '';
  }
  $form_state['storage']['parameters'] = $block_info ? $block_info['tokens'] :array();
}


/**
 * submit handler for forena_data_block_form.
 * This adds the datablock to an existing report.
 *
 */
function forena_data_block_add($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $values = $form_state['values'];
  $report_name = $values['report_name'];
  $data_block = $values['data_block'];
  $template = $values['templates'];
  $where_clause = $values['where_clause'];
  $r = forena_get_report_editor($report_name);
  $head = $r->simplexml->head;

  //Get the xml body of the added data block
  $added_report = $form_state['storage']['template_body'];

  //Get the data block parameters
  $block_info = FrxReportGenerator::instance()->load_block($data_block, $where_clause);
  $data_block_parms = $block_info['tokens'];

  //Build the parameters
  $xml = "";
  $path_parent = '//frx:parameters';
  $parent_node = $head->xpath($path_parent);

  $path_child = '//frx:parm';
  $child_nodes = $head->xpath($path_child);
  $found_rpt_param = FALSE;

  $parm_name = 'parm';
  //loop through the data block parameters
  foreach ($data_block_parms as $index => $id) {
    //check if rpt_params exist
    if ($child_nodes) {
      $path_id = '//frx:parm[@id="' . $id . '"]';
      $found_rpt_param = $child_nodes[0]->xpath($path_id);
    }
    //add block params if none are found in report
    if (!$found_rpt_param) {
      if ($parent_node) {
        $parm = $parent_node[0]->addChild($parm_name);
        $parm->addAttribute('id', $id);
      }
    }
  }

  $main_report = forena_inner_xml($r->simplexml, 'body');
  $added_report = forena_clean_xhtml($added_report);

  $r->setBody($main_report . "\n" . $added_report);

  if (forena_save_report($report_name, $r->asXML(), TRUE) == 1) {
    drupal_set_message(t('Your report has been saved'));
  }
}

function forena_report_translations($name_in) {
	$desc = forena_report_desc();
	global $language;
	$cur_lang = $language->language=='en' ? '' : $language->language;
  $name = $desc['name'];
  $filename = $desc['filename'];
	$languages = language_list();
	@list($lang, $base_name) = explode('/', $name, 2);
	if (array_key_exists($lang, $languages)) {
		$name = $base_name;
	}
	elseif ($cur_lang &&  file_exists(FrxReportGenerator::instance()->report_path() . '/' . $cur_lang . '/' . str_replace('.', '/', $name_in) . '.frx')) {
		$name_in = $cur_lang . '.' . $name_in;
	}
	$data = array();
	$headers = array(t('Language'), t('Title'), t('Last Modified'), t('Operation'));
	// Generate list of all existing tranlsations and their status.
	$report_path = FrxReportGenerator::instance()->report_path();
	foreach ($languages as $key=>$language) {
	  $i_name = ($key=='en') ? $name : $key . '/' . $name;
    if (file_exists($report_path . '/' . $i_name . '.frx')) {
      $modified = format_date(filemtime($report_path . '/' . $i_name . '.frx'));
      $r = forena_get_report_editor($i_name);
      $title = $r->title;
      $edit = l('Edit', 'reports/' . str_replace('/', '.', $i_name) . '/layout',array('query' => array('language' => $key)));
    }
    else {
    	$modified = '';
    	$title ='';
    	$edit = l('Add', 'reports/' .$name_in. '/layout', array('query' => array('language' => $key)));
    }
    $data[] = array($language->native, $title, $modified, $edit);

	}

  return theme_table(array('header' => $headers, 'rows' => $data, 'attributes' => array(), 'caption' => '', 'sticky' => true, 'colgroups' => array(), 'empty' => ''));
}

/**
 * Delete submit handler to delete report blocks
 * @param $form
 * @param $form_state
 * @return unknown_type
 */
function forena_data_block_delete($form, &$form_state) {
  $values = $form_state['values'];
  $delete_blocks = array_filter($values['delete_blocks']);

  $report_name = $values['report_name'];
  $r = FrxReportGenerator::instance()->get_report($report_name);

  //find the datablocks in the existing report
  if ($r) $r->get_attributes_by_id();
  $body = $r->body;

  foreach ($delete_blocks as $id) {
    if ($r) $r->deleteNode($id);
  }

 if (forena_save_report($report_name, $r->rpt_xml, TRUE) == 1) {
    drupal_set_message(t('Your report has been saved.'));
  }
}

/**
 * Form to confirm the delete of a form.
 * @param $form_state
 * @return unknown_type
 */
function forena_delete_form() {
  // Parse names from url
  $desc = forena_report_desc();
  $report_name = $desc['name'];
  $link = $desc['link'];
  $form['report_name'] = array(
    '#type' => 'value',
    '#value' => $report_name
  );
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $report_name)),
    isset($_GET['destination']) ? $_GET['destination'] : $link,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
  return $form;
}

function forena_delete_form_submit($form, &$form_state) {
  $report_name = $form_state['values']['report_name'];

  drupal_set_message(t('Deleted %s', array($report_name)));
  forena_delete_report($report_name);
  $form_state['redirect'] = 'forena';
}
/**
 * Handle delete buttons from edit forms.
 * @return unknown_type
 */
function forena_edit_delete_submit($form, &$form_state) {
  $desc = forena_report_desc();
  $link = $desc['link'];
  $report_name = $form_state['values']['report_name'];
  $destination = '';
  if (isset($_REQUEST['destination'])) {
    $destination = drupal_get_destination();
    unset($_REQUEST['destination']);
  }
  $form_state['redirect'] = array('path' => $link . '/delete', 'query' => array('destination' => $destination));
}

/**
 * Clean xhtml
 *
 * @param unknown_type $xhtml
 * @return unknown
 */
function forena_clean_xhtml($xhtml) {
  $ret = $xhtml;
  // If tidy is installed lets clean the html using that.
  if (is_callable('tidy_repair_string')) {
    $config = array(
      'doctype'            => 'omit',
      'indent-spaces'             => 2,
      'indent' => 'auto',
      'input-xml'          => TRUE,
      'output-xml'         => TRUE,
      'indent-attributes'  => FALSE,
      'indent-spaces'      => 2,
      'add-xml-space'      => TRUE,
      'wrap'               => 135 );
    $ret = tidy_repair_string($xhtml, $config, 'utf8');
  }
  else {
    $ret = str_replace('&nbsp;', '&#160;', $ret);
  }
  return $ret;
}

/**
 * Provides list of blocks that a user has access to that are in any repository matching
 * a specified search string
 *
 * @param string $search block to search for.
 * @return array list of blocks the user has access to.
 */
function forena_user_data_blocks($search) {
  $repos = FrxReportGenerator::instance()->app->repositories();
  foreach ($repos as $name => $r) {

    // Make really sure the data provider objects have been instantiated
    $provider = $r;
    if (!@$provider['data']) {
      $provider = FrxReportGenerator::instance()->repository($name);
    }
    $repos[$name] = $provider;
    // Invoke the list block function to find out all of the block names
    $o = $provider['data'];
    if (method_exists($o, 'list_blocks')) {
      $blocks = $o->list_blocks($search);
      foreach ($blocks as $block) {
        $block_info = $o->load_block($block);
        if (method_exists($o, 'access')) {
          $allow = $o->access($block_info['access']);
          if ($allow) $user_blocks[] = $name . '/' . $block;
        }
      }
    }
  }
  return $user_blocks;
}

/**
 *  Prettifies an XML string into a human-readable and indented work of art
 *  @param string $xml The XML as a string
 *  @param boolean $html_output True if the output should be escaped (for use in HTML)
 */
function forena_xmlpp($xml, $html_output=FALSE) {
    $xml_obj = new SimpleXMLElement($xml);
    $level = 4;
    $indent = 0; // current indentation level
    $pretty = array();

    // get an array containing each XML element
    $xml = explode("\n", preg_replace('/>\s*</', ">\n<", $xml_obj->asXML()));

    // shift off opening XML tag if present
    if (count($xml) && preg_match('/^<\?\s*xml/', $xml[0])) {
      $pretty[] = array_shift($xml);
    }

    foreach ($xml as $el) {
      if (preg_match('/^<([\w])+[^>\/]*>$/U', $el)) {
          // opening tag, increase indent
          $pretty[] = str_repeat(' ', $indent) . $el;
          $indent += $level;
      }
      else {
        if (preg_match('/^<\/.+>$/', $el)) {
          $indent -= $level;  // closing tag, decrease indent
        }
        if ($indent < 0) {
          $indent += $level;
        }
        $pretty[] = str_repeat(' ', $indent) . $el;
      }
    }
    $xml = implode("\n", $pretty);
    return ($html_output) ? htmlentities($xml) : $xml;
}

function forena_admin_params_form($formid, $form_state) {

  $desc = forena_report_desc();
  $name = $desc['name'];
  $filename = $desc['filename'];
  $format = @$desc['format'];
  if ($desc['exists'] && $name) {
    $r = forena_get_report_editor($name);
    drupal_set_title($r->title);
    $form = array();

    $form['rpt_name'] = array(
      '#type' => 'value',
      '#value' => $name,
    );

    if ($r) $nodes = $r->simplexml->xpath('head/frx:parameters/frx:parm');
    if ($nodes) {
      $form['params'] = array('#tree' => TRUE, );

      foreach ($nodes as $node) {
        $attrs = $node->attributes();
        $id = (string)$attrs['id'];
        $label = (string)$attrs['label'];
        $value = (string)$attrs['value'] ? (string)$attrs['value'] : (string)$node;
        $require = (string)$attrs['require'];
        $data_source = (string)$attrs['data_source'];
        $data_field = (string)$attrs['data_field'];
        $desc = (string)$attrs['desc'];
        $type = (string)$attrs['type'];

        //make a subtree for each param
        $form['params'][$id] = array(
          '#tree' => TRUE,
          '#type' => 'fieldset',
          '#title' => ($label) ? $label : $id,
          '#collapsible' => TRUE,
          '#collapsed' => TRUE
        );

        $form['params'][$id]['id'] = array(
          '#type' => 'textfield',
          '#title' => t('id'),
          '#default_value' => $id,
          '#required' => TRUE,
          '#description' => t('The name of the parameter to be filtered against.')
        );

        $form['params'][$id]['label'] = array(
          '#type' => 'textfield',
          '#title' => t('label'),
          '#default_value' => $label,
          '#required' => FALSE,
          '#description' => t('A descriptive name of the parameter to be displayed on a form.')
        );

        $form['params'][$id]['require'] = array(
          '#type' => 'radios',
          '#title' => t('require'),
          '#default_value' => ($require) ? $require : "0",
          '#options' => array("1" => t('Yes'), "0" => t('No')),
          '#description' => t('Requires a value for this parameter to display the report. If there is not a default value, the user will be prompted to enter a value.')
        );

        $form['params'][$id]['value'] = array(
          '#type' => 'textfield',
          '#title' => 'default value',
          '#default_value' => $value,
          '#description' => t('The value of the parameter for the initial view of the report.')
        );

        $form['params'][$id]['desc'] = array(
          '#type' => 'textfield',
          '#title' => t('description'),
          '#default_value' => $desc,
          '#description' => t('Enter a helpful description about this parameter. This will display on the form when the user is prompted to enter a parameter.'),
        );

        $form['params'][$id]['data_source'] = array(
          '#type' => 'textfield',
          '#title' => t('Data Source'),
          '#default_value' => $data_source,
          '#autocomplete_path' => 'forena/data_block/autocomplete',
          '#description' => t('Data block to be used for list of values'),
        );

        $form['params'][$id]['data_field'] = array(
          '#type' => 'textfield',
          '#title' => t('Data Field'),
          '#default_value' => $data_field,
          '#description' => t('Column in data block to be used for ID'),
        );

         $form['params'][$id]['type'] = array(
          '#type' => 'select',
          '#title' => t('Input Control Type'),
          '#default_value' => $type,
          '#options' => array('textfield' => 'Text', 'radios' => 'Radios', 'select' => 'Select','multiselect' => 'Multi-Select', 'checkboxes' => 'Checkboxes'),
          '#description' => t('Enter a helpful description about this parameter. This will display on the form when the user is prompted to enter a parameter.'),
        );

        $form['params'][$id]['delete_ind'] = array(
          '#type' => 'checkbox',
          '#title' => 'Remove this parameter',
          '#description' => t('Check to remove this parameter, then save'),
        );
      }

      $form['submit'] = array(
       '#type' => 'submit',
       '#value' => 'Save',
      );
      return $form;
    }
  }
  else {
    drupal_not_found();
  }
}

function forena_admin_params_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $name = $values['rpt_name'];
  $params = $values['params'];

  $r = forena_get_report_editor($name);
  //add the new parms
  $r->setParameters($params);
  //Remove paramters to delete
  foreach ($params as $parm) {
    if ($parm['delete_ind']) {
      $r->removeParm($parm['id']);
    }
  }

  $new_report = $r->asXML();

  if (forena_save_report($name, $new_report, TRUE) == 1) {
    drupal_set_message(t('Your report has been saved'));
  }
}

/**
 * Auto complete function for categories
 * Checks access for security as well.
 *
 * @param $string = string to be matched against categories
 * @return An array containing all matching categories
 */
function forena_get_categories($string='') {
  $result = db_query("SELECT * FROM {forena_reports} where hidden=0 AND category LIKE :category ORDER BY category, title asc",
    array(':category' => $string . '%'));
  $categories = array();
  foreach ($result as $row)  {
      $access = TRUE;
      $cache = $row->cache;

      if ($cache) {
        $cache = unserialize($cache);

        // Check each callback function to see if we have an error.
        if ($cache['access']) foreach ($cache['access'] as $callback => $args) {
          if ($callback) foreach ($args as $arg) {
            $access = FALSE;
            if (function_exists($callback)) {
              $a = $callback($arg);
            }
            if ($a) $access = TRUE;
          }
          else {
            $access = TRUE;
          }
        }
      }
      if ($access && !$categories[$row->category]) {
        $categories[$row->category] = $row->category;
      }
    }
    return $categories;
}


