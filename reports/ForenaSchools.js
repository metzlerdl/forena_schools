(function ($) {

  Drupal.behaviors.forenaSchools = {
    attach: function (context, settings) {
      var oTable = jQuery('table.pedagoggle-scores').dataTable({
        "bPaginate": false, 
        "bSort": true,
        "sScrollX": "100%",
        "sScrollY": "500",
        "bScrollCollapse": true
      }); 
      if (oTable) { 
        new FixedColumns( oTable, {"iLeftColumns": 1, "iLeftWidth": 200 });
      }
   }
  };
})(jQuery);
