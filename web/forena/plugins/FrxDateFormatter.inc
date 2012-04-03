<?php
/**
 * @file
 * Handles common date formats. 
 *
 */
class FrxDateFormatter {
  public function format($value, $format_string) { 
    if (!$format_string) $format_string = 'F j,Y'; 
    if ($value)
      $date = format_date(strtotime($value), 'custom', $format_string);
      if ($date) $value = $date; 
  }
  
  public function format_string_help() {
    return 'Format  ISO formated date.  (e.g. "F j, Y, g:i a") Format string is passed to the PHP date_to_string.  See <a href="http://php.net/manual/en/function.date.php">PHP Documentation</a> for more info.';  
  }
}