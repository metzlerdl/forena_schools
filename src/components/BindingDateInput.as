package components
{
	[IconFile("BindingDateInput.png")]
	
	import flash.events.Event;
	import mx.controls.DateField;	
	import components.DateUtils;
	
	// Dispatched when value in bindingObject[bindingField] is changed
	[Event (name="valueChange", type="mx.events.FlexEvent")]

	public class BindingDateInput extends DateField
	{
		[IconFile("BindingDateField.png")]
		private var _bindingObject:Object;
		private var _bindingField:String;
		private var _disableBinding:Boolean = false;
		
		public function BindingDateInput()
		{
			super();
			addEventListener(Event.CHANGE, changeHandler);
		}
		
		[Bindable]
		[Inspectable(category="Data")]
		public function get bindingObject():Object
		{
			return _bindingObject
		}
		public function set bindingObject(item:Object):void
		{
			if(_disableBinding) {
				_bindingObject = null;
				trace('warning: binding object set when bidirectional binding is disabled');
				return; 
			}
			_bindingObject = item;
			if (item && bindingField) {
				if (item.hasOwnProperty(bindingField)) {
					selectedDate = DateUtils.parseIso(item[bindingField]);
				} else {
					selectedDate = null;
				}
			}
		}
		
		/**
		 * @public
		 * Specifies property of bindingObject to bind to
		 */
		[Inspectable(category="Data")]
		public function set bindingField(value:String):void
		{
			if(value && value.length > 0) {
				_bindingField = value;
				if(bindingObject) {
					if(bindingObject.hasOwnProperty(value)){
						this.textInput.text = bindingObject[bindingField];
					} else {
						this.textInput.text = '';
					}
				}
			}
		}
		public function get bindingField():String
		{
			return _bindingField;	
		}
		
		public function changeHandler(event:Event):void
		{
			if(bindingObject && bindingField) {
				bindingObject[bindingField] = DateUtils.format(this.selectedDate, DateUtils.FMT_ISO_DATE);
			}
		}
	}
}