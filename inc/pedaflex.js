/**
 * Close yourself.
 */
function flexClose(refreshParent) {
	  if (refreshParent) {
	    opener.location.href = opener.location.href;
	  }
      self.close();
}
/**
 * Set the title for a window.
 * @param newTitle
 */
function flexSetTitle(newTitle ) {
      document.title= newTitle;
}
 
/**
 * Determine if the hosting application is in a popup.
 * @returns {Boolean}
 */
function flexIsPopUp() {
      if (opener){
            return true;
      }
      else {
            return false;
      }
}

/**
 * Check fore flash and set cookie if we have the right version. 
 */
function flexSetCookie()
{
  var exdate=new Date();
  var value='true'; 
  var c_name = 'PEDAGOGGLE_FLEX'; 
  var exdays = 1; 
  var reqFlashVer = '10.2.0';
  if (swfobject.hasFlashPlayerVersion(reqFlashVer)) {
	  exdate.setDate(exdate.getDate() + exdays);
	  var c_value=escape(reqFlashVer) + ((exdays==null) ? "" : "; expires="+exdate.toUTCString());
	  document.cookie=c_name + "=" + c_value;
  }
}

flexSetCookie(); 
