package services
{
	import flash.net.URLRequest;
	
	import mx.core.UIComponent;
	import mx.core.FlexGlobals; 
    [Deprecated(replacement="FlexApp")]
	public class Navigator extends UIComponent
	{
		private var _baseURL:String   // Base url for the iste if not present
        public var app:Object; 		
		public function set baseURL(b:String):void {
			_baseURL = trimSlashes(b); 
		}
		
		private function trimSlashes(url:String):String {
           var char:String = '/';
           if (url.charAt(url.length - 1) == char) {
             url = trimSlashes(url.substring(0, url.length - 1));
           }
        return url;
		}

		
		public function navigateTo(url:String, target:String):void {
			// Default in the base path.  
			if (!_baseURL && FlexGlobals.topLevelApplication.parameters.basePath) {
				baseURL = FlexGlobals.topLevelApplication.parameters.basePath; 
			}
			
			if (url.search(":") == -1) {
				url = _baseURL + '/' + url; 
			}
			var request:URLRequest = new URLRequest(url);
			
			request.method="GET"; 
			flash.net.navigateToURL(request,target); 
		}
		
		public function popUp(url:String):void {
			navigateTo(url,"_blank"); 
		}
		
		public function goto(url:String):void { 
			navigateTo(url,"_top"); 
		}
	}
}