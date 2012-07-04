function ForenaSchools(context) { 
	var oTable = jQuery('table.pedagoggle-scores').dataTable({
		"bPaginate": false, 
		"bSort": true,
		"sScrollX": "100%",
	}); 
	new FixedColumns( oTable, {"iLeftColumns": 2,
			"iLeftWidth": 250 });
}