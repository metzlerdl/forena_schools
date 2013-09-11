package services
{
	import components.ProgressWindow;
	
	import events.FlexXMLEvent;
	
	import flash.events.Event;
	
	import mx.controls.Alert;
	import mx.core.FlexGlobals;
	import mx.core.UIComponent;
	import mx.events.FlexEvent;
	import mx.managers.PopUpManager;
	import mx.rpc.events.ResultEvent;
	import mx.rpc.http.HTTPService;
	import mx.utils.ObjectUtil; 
		
	[Event (name="flexXMLChange", type="events.FlexXMLEvent") ]
    [Deprecated (replacement="DataService")]
	public class RestXMLService extends UIComponent {
		private var _service:HTTPService; 
		private var _xml:XML; 
		private var _lastXML:String; 
		public var defaultService:String; 
		public var defaultMethod:String;
		public var preload:Boolean; 
		public var debug:Boolean = false; 
		public var progressPopup:ProgressWindow = new ProgressWindow(); 
		private var progressUp:Boolean = false; 
		[Bindable]public var progressMessage:String; 
		[Bindable]public var showProgress:Boolean = false; 
		[Bindable]public var isChanged:Boolean = false; 
		
		public function RestXMLService()
		{
			super();
			_service = new HTTPService(); 
			_service.resultFormat="e4x"; 
			_service.method="POST"; 
			_service.addEventListener(ResultEvent.RESULT,this.dataHandler); 	
			addEventListener(FlexEvent.CREATION_COMPLETE,onCreationComplete); 
		}
		
		private function onCreationComplete(event:FlexEvent):void {
			if (!debug && FlexGlobals.topLevelApplication.parameters.debug=='true')  {
				debug=true; 
			}
			if (defaultMethod && defaultService && preload) 
			  this.send(); 
		}
		
		// Bindable xml property fires an FlexXML.CHANGE event to notify other controls of the data changing. 
		[Bindable]
		public function get xml():XML { 
			return _xml; 
		}
		
		public function set xml(value:XML):void { 
			_xml =  value;
			if (value) _lastXML = value.toXMLString(); 
			isChanged = false; 
			var e:Event = new FlexXMLEvent(FlexXMLEvent.CHANGE,_xml); 
			this.dispatchEvent(e); 
		}
		
		/*
		* Update the modified flag. You might need to use this to test to see if data has changed. 
		*/ 
		public function checkModified():Boolean {
			if (xml && xml.toString() == _lastXML) {
				isChanged = false; 
			}
			else {
				isChanged = true; 
			}
			return isChanged; 
		}
		
		
	
		
		// Send web service call adding default method and service if set for the object and not in the request. 
		public function send(request:Object = null):void { 
		   // Make sure we've got a dataService. 
		   config(); 
		   // If we're supposed to show progress, then popup the window; 
		   if (showProgress)  {
		   	    if (progressMessage) {
		   	    	progressPopup.message=progressMessage; 
		   	    }
		   	    PopUpManager.addPopUp(progressPopup,this,true); 
		     	PopUpManager.centerPopUp(progressPopup); 
		     	progressUp = true; 
		   }
		   
	       if (request==null) request =  ObjectUtil.copy(FlexGlobals.topLevelApplication.parameters); 
	       if (!request["service"] && defaultService) {
	       	  request["service"] = defaultService; 
	       }
	       if (!request["method"] && defaultMethod) { 
	       	 request["method"] = defaultMethod; 
	       }
		   if (debug) {
			   trace(id + "Sending"); 
			   for (var prop:String in request) {
			   	trace( prop + ':' + String(request[prop])); 
			   } 
		   	
		   } 
	       _service.send(request); 
		}
		
		/*
		 * Provide property so that we can see the service url. 
		 */ 
		public function get url():String { 
			config(); 
			return _service.url; 
		}
		
		private function config():void { 
			if (!_service.url) { 
				if (debug) {
				  _service.url=FlexGlobals.topLevelApplication.parameters.dataService + '?debug=true'; 
				}
				else {
				  _service.url=FlexGlobals.topLevelApplication.parameters.dataService; 
				}
			}
		}
		
		private function dataHandler(event:ResultEvent):void { 
			var x:XML = XML(event.result); 
			var tag:String; 
			var msg:String; 
			if (progressUp) {
				PopUpManager.removePopUp(progressPopup); 
			}
			if (event.result && x) { 
			  tag = x.name().toString();
			  msg = x.attribute('message');   
			}
			if (debug) {
    			trace(this.id + "Recieved"); 
    			if (x) trace(x.toXMLString()); 
			}
			
			if (msg) {
				Alert.show(msg); 
			}
			
			switch(tag) { 
				case "pre": 
				case "error":
				case "PRE": 
				case "ERROR": 
				case "message": 
				  Alert.show(x.text()); 
				  break; 
				default: 
				  xml = x; 
			}
		}
		
	}
}