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
        //new FixedColumns( oTable, {"iLeftColumns": 1, "iLeftWidth": 200 });
      }
   }
  };


  Drupal.behaviors.peninsulaFeeds = {
    attach: function(context, settings) {
      console.log("well here we are!");
      $("#websiteRSS-container").rss(
          "http://www.psd401.net/index.php?format=feed&amp;type=rss",
          {
              limit: 2,
              layoutTemplate: "{entries}",
              entryTemplate: '<p><a href="{url}">{title}</a> {shortBody}</p><p class="sub">{date}</p>'
          }
      );

      $("#techblogRSS-container").rss(
          "http://techblog.psd401.net/?feed=rss2",
          {
              limit: 2,
              layoutTemplate: "{entries}",
              entryTemplate: '<p><a href="{url}">{title}</a> {shortBody}</p><p class="sub">{date}</p>'
          }
      );

      $("#supRSS-container").rss(
          "https://script.google.com/a/macros/krishagel.com/s/AKfycbxIvJLmswNQRctRQ3UrQqwad_1sOx_SIH5VrSugxXYU-ot2hsc/exec?action=timeline&q=psdsup",
          {
              limit: 2,
              layoutTemplate: "{entries}",
              entryTemplate: '<p><a href="{url}">{title}</a></p><p class="sub">{date}</p>'
          }
      );

    }
  };
})(jQuery);
