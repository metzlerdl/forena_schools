<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta http-equiv="Content-Type" content="text/html;charset=utf-8" />
  <title><?php print $title;  ?></title>
  <?php print $scripts;  ?>
  <?php print $css;  ?>
</head>
<body>
 <div class="page-wrap">
  <div id="h_container">
   <div id="header" class="container_12">

    <div id="header_text" class="grid_8">
    <h1><?php print $title?></h1>
    </div>
    <div id="logo" class="grid_2">
    <img alt="PedaGoggle" class="pedalogo" src="theme/pedagoggle.png"/>
    </div>
    <div id="primary-nav" class="grid_2">
    <ul class="nav hnav">
     <li><a href="<?php print $home?>">Home</a></li>
     <li><a href="<?php print $logout?>">Logout</a></li>
    </ul>
    </div>
  </div>
 </div>
 <div class="content container_12" >
  <?php print $content; ?>
 </div>
 <div class="push"></div>
</div>
<div class="footer ">
   <div class="container_12">
   <div class="grid_12">GPL</div>
   </div>
  </div>
</body>
</html>