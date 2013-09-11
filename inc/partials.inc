<?php
/**
* Render a partial template from an array.
* @param $info an two dimensional array to iterate through for partial
* @param $template the template to execute for the partial.
*/
function render_parital($template,$vars='',$extract=true) {
  if (is_array($item) && $extract==true)  {
    extract($item);
  }
  include("../theme/".$template.".tpl.php");
}
/**
* Render a partial template from an array.
* @param mixed $collectin an array to itterate through as the partical
* @param string $template the template to execute for the partial.
*/
function render_partial_iterate($template,$collection='',$extract=true) {
  foreach ($collection as $item)  {
    render_partial($template,$item,$name,$extract);
  }
}