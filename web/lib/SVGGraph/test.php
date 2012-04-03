<?php
 require_once 'SVGGraph.php';
 $graph = new SVGGraph(640, 480);
 $graph->colours = array('red','green','blue');
 $graph->Values(100, 200, 150);
 $graph->Links('/Tom/', '/Dick/', '/Harry/');
 $graph->Render('BarGraph');