(function ($) {

  Drupal.behaviors.graphSkin = {
    attach: function (context, settings) {
      	console.log("hello world");
    	$('use').hover(function(){
    		var matrix = $(this)[0].getCTM();
    		matrix.a = 2;

    		var s = "matrix(" + matrix.a + "," + matrix.b + "," + matrix.c + "," + matrix.d + "," + matrix.e + "," + matrix.f + ")";
            $(this)[0].css("transform", s);

    	}, function(){

    	});
    }
  };

})(jQuery);