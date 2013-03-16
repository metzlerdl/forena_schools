(function ($) {

  Drupal.behaviors.forenaSchools = {
    attach: function (context, settings) {
      var oTable = jQuery('table.pedagoggle-scores').dataTable({
        "bPaginate": false, 
        "bSort": true,
        "sScrollX": "100%",
        "sScrollXInner": "100%",
        "bScrollCollapse": false,
        "sScrollY": "25em",
        "fnDrawCallback": function ( oSettings ) {
          /* Need to redo the counters if filtered or sorted */
          if ( oSettings.bSorted || oSettings.bFiltered ) {
            for ( var i=0, iLen=oSettings.aiDisplay.length ; i<iLen ; i++ ) {
              this.fnUpdate( i+1, oSettings.aiDisplay[i], 0, false, false );
            }
          }
        }
      }); 
      if (oTable) { 
        new FixedColumns( oTable, {"iLeftColumns": 1, "iLeftWidth": 200 });
      }
   }
  };
})(jQuery);
