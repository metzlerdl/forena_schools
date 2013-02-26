<?php
class ForenaSchoolsValidation {
  static public function instance() { 
    static $o = ''; 
    if (!$o) { 
      $o = new ForenaSchoolsValidation(); 
    }
    return $o; 
  }
}