package tests {
	import mx.controls.DataGrid;
	import mx.controls.TextInput;
	import mx.controls.dataGridClasses.DataGridItemRenderer;
	import mx.controls.dataGridClasses.DataGridListData;
	
	public class ScoreEditor extends TextInput
	{
		public function ScoreEditor()
		{
			super();
			width = 60; 
			setStyle("borderStyle", "solid");	
            editable = true; 
		}
			
		public var scoreXML:XML; 
		// Override the set method for the data property.
		// @TODO: Replace this with one that gets score data
		override public function set data(value:Object):void {
			super.data = value;
	
			if (value != null)
			{
				var x:XML = XML(value); 
				var measure_id:String; 
				var dataField:String = DataGridListData(listData).dataField;
				var score:XML = x..measure.(@measure_id==dataField)[0];
				scoreXML = score; 
				if (score && score.@score) {
					text = score.@score; 
				}
				else {
					text = ""; 
				}
				
				// Set style to indicate the normal range for the test. 
				if(text == "") {
					setStyle("contentBackgroundColor", 0xFFFFFF); 
					setStyle("borderColor", 0xEEEEEE); 
				}
				else if(score.@l_4.toString() && (Number(text) >= Number(score.@l_4)))
				{
					setStyle("contentBackgroundColor", 0xB5C0E3);
					setStyle("borderColor", 0xB5C0E3); 
				}
				else if (score.@l_3.toString() && (Number(text) >= Number(score.@l_3))) { 
					setStyle("contentBackgroundColor", 0xBBE3B5);
					setStyle("borderColor", 0xBBE3B5); 
				}
				else if (score.@l_2.toString() && (Number(text) >= Number(score.@l_2))){
					setStyle("contentBackgroundColor", 0xE3E288); 
					setStyle("borderColor", 0xE3E288); 
				} else { 
					setStyle("contentBackgroundColor", 0xE3B5B5); 
					setStyle("borderColor", 0xE3B5B5); 
				}
			}
				
			else
			{
				// If value is null, clear text.
				text= "";
				setStyle("contentBackgroundColor", 0xEEEEEE); 
				setStyle("borderColor", 0xEEEEEE); 	
			}
			
			super.invalidateDisplayList();
		}
	}
	

}