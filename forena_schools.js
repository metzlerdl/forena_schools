
(function ($) {

  function frxSchoolsAddGroup(context, settings) { 
    var chk="<th><input type='checkbox' checked='true'/></th>";
    $('.FrxSchoolsStudents thead tr').prepend(chk); 
    
    $('.FrxSchoolsStudents tbody tr').each(function() {
      var chk="<td><input type='checkbox' checked='true'/></td>";
      $(this).prepend(chk); 
    }); 
  }
  
  Drupal.behaviors.FrxSchoolsInit = {
    attach: function (context, settings) {
       frxSchoolsAddGroup(context, settings); 
    }
  };

})(jQuery);

