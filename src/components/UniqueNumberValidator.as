package components
{
	import mx.validators.NumberValidator;
	import mx.validators.StringValidator;
	import mx.validators.ValidationResult;
	import mx.validators.Validator;
	
	public class UniqueNumberValidator extends NumberValidator
	{
		[Bindable] public var dataSource:XMLList; 
		[Bindable] public var dataField:String; 
		[Bindable] public var fieldLabel:String; 
		
		public function UniqueNumberValidator()
		{
			super();
		}
		
		 override protected function doValidation(value:Object):Array {
			var r:Array = new Array(); 
			var xist:Boolean = false; 

			r = super.doValidation(value); 
			// Now we need to check for the existence of the data  in the class
		    for each (var x:XML in dataSource) { 
		      if (x[dataField]==value.toString()) { 
				 xist = true; 
			  }
			}
			if (xist) { 
		      r.push(new ValidationResult(true, null, "fieldExists",fieldLabel + value.toString() +" allready defined"));
			}
			return r; 
		}
			
	}
}