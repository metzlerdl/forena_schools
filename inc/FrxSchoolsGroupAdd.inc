<?php
class FrxSchoolsGroupAdd extends FrxRenderer {
  public function render() {
    $output = '';
    if (!forena_schools_access('bldg_admin')) {
      return '';
    }

    $ids = array();
    $data = Frx::Data()->getContext('parm');
    $data['group_type'] = 'intervention';
    $bldg_id = @$data['bldg_id'];
    $xml = Frx::Data()->currentContext();
    $groups = array('' => 'New Group');
    if (!@$data['max_grade_level']) {
      $min_grade=$max_grade=$data['grade_level'];
    }
    else {
      $min_grade = @$data['grade_level'];
      $max_grade = @$data['max_grade_level'];
    }
    $user_groups = ForenaSchoolsGroup::instance()->userGroups('intervention', @$bldg_id, $min_grade, $max_grade);
    if ($user_groups) $groups = array_merge($groups, $user_groups);
    if ($xml && is_object($xml)) {
      $rows = $xml->xpath('//student_id');
      foreach ($rows as $node) {
        $ids[(string)$node] = array('student_id' => (string)$node);
      }
    }
    if ($bldg_id ) {
      $path = drupal_get_path('module', 'forena_schools');
      drupal_add_js($path . '/forena_schools.js');
      $form = drupal_get_form('forena_schools_group_add_js', $groups, $ids, $data);
      $output = drupal_render($form);
    }
    return $output;
  }
}