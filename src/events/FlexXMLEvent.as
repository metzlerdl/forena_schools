package events
{
	import flash.events.Event;
	
	public class FlexXMLEvent extends Event
	{
		public static var CHANGE:String = "flexXMLChange"; 
		public var xml:XML; 
		
		//* Constructor
		public function FlexXMLEvent(type:String,xmlValue:XML,bubbles:Boolean=false,cancelable:Boolean = false)
		{
			 super(type, bubbles, cancelable);
			 xml = xmlValue; 
		}
		
		 //* Handles event bubbles passing the xml data
		public override function clone():Event {
			return new FlexXMLEvent(type,xml,bubbles,cancelable); 
		}

	}
}