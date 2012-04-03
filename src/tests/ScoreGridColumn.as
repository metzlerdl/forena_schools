package tests {
	import mx.controls.AdvancedDataGrid;
	import mx.controls.advancedDataGridClasses.AdvancedDataGridColumn;
	
	import tests.ScoreRenderer;
	/**
	 * Custom Data grid column for use with score item renders to help facilitate binding 
	 * Measure
	 */
	public class ScoreGridColumn extends AdvancedDataGridColumn
	{
		public function ScoreGridColumn()
		{
			super();
			width = 50; 
			this.sortable=false; 
			this.sortCompareFunction = scoreSort; 
		    
	
		}	
		
		public function scoreSort(obj1:Object, obj2: Object):int { 
			
			var x1:XML = obj1 as XML; 
			var x2:XML = obj2 as XML; 
		    var s1:String = x1..measure.(@profile_sort==dataField).@score;	
			var s2:String = x2..measure.(@profile_sort==dataField).@score;
			var n1:Number = new Number(s1);
			var n2:Number = new Number(s2); 
			var result:int = (n1>n2)?-1:1; 
			return result;
		}
		
		
	}
	
	
}