package components {
	import mx.controls.dataGridClasses.DataGridItemRenderer;
	import mx.controls.DataGrid;
	import mx.controls.dataGridClasses.DataGridItemRenderer;	
	public class MxMeasureLabelRenderer extends DataGridItemRenderer
	{
		public function MxMeasureLabelRenderer()
		{
			super();
		}
					
		override public function validateNow():void {
		  if (this.listData) {
			this.setStyle('fontWeight', (data.@id==data.@parent || data.@parent=='' || data.@parent.length()==0) ? 'bold' : 'normal'); 
		  }
	      super.validateNow();	
	    }
	}
}