package components
{
	import components.DateUtils;
	
	import mx.validators.StringValidator;
	import mx.validators.ValidationResult;
	import mx.validators.Validator; 
	
	public class DateRangeValidator extends StringValidator
	{


		[Bindable] public var fieldLabel:String; 	
		[Bindable] public var start_date:String; 
		[Bindable] public var end_date:String; 

		
		 override protected function doValidation(value:Object):Array {
			var r:Array = new Array(); 

			r = super.doValidation(value); 
			// Now we need to check for the existence of the data  in the class

			if (DateUtils.format(value, DateUtils.FMT_ISO_DATE) < start_date || value > end_date ) { 
			  var msg:String = "Specify a date between " + start_date + " and " + end_date; 
		      r.push(new ValidationResult(true, null, "dateOutOfRange",msg));
			}
			return r; 
		}
			
	}
}