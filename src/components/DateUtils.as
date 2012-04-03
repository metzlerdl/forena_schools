package components
{
	import mx.formatters.DateFormatter;
	
	public class DateUtils
	{
		private static var _df:DateFormatter = new DateFormatter;
		
		/**
		 * Format Strings
		 */
		public static const FMT_FULL_DATE_TIME:String = 'MM/DD/YYYY L:NN A';
		public static const	FMT_FULL_DATE:String = 	    'MM/DD/YYYY';
		public static const FMT_ISO_DATE_TIME:String =  'YYYY-MM-DD JJ:NN:SS';
		public static const FMT_ISO_DATE:String=        'YYYY-MM-DD';
		public static const FMT_ISO_TIME:String=        'JJ:NN:SS';
		public static const FMT_SHORT_DATE_TIME:String= 'MM/DD/YY L:NN A';
		public static const FMT_SHORT_DATE:String =     'MM/DD/YY';
		public static const FMT_SHORT_DATE_DAY:String=	'MM/DD/YY (EEE)';		
		public static const FMT_MINI_DATE_TIME:String=  'MM/DD L:NN A';
		public static const FMT_MINI_DATE:String =      'MM/DD';
		public static const FMT_MINI_DATE_DAY:String=	'MM/DD (EEE)';
		public static const FMT_MILITARY_TIME:String =  'JJ:NN';
		public static const FMT_FRIENDLY_DATE:String =  'DD MMM YYYY';
		public static const FMT_FRIENDLY_TIME:String =  'L:NN A';
		public static const FMT_FRIENDLY_DATE_TIME:String = 'DD MM YYYY L:NN A';
		
		/**
		 * Time facts
		 */ 
		public static const SECONDS_IN_HOUR:int		= 3600;
		public static const SECONDS_IN_DAY:int		= 86400;
		public static const MILISECONDS_IN_DAY:int  = 86400000;
		public static const MINUTES_IN_DAY:int		= 1440;
		
		public static const SHORT_MONTHS:Array = 
						['Jan','Feb','Mar','Apr',
						'May','Jun','Jul','Aug',
						'Sep','Oct','Nov','Dec'];
		public static const MONTHS:Array = ['January','February','March','April',
						'May','June','July','August','September','October',
						'November','December'];
		public static const DAYS:Array = ['', 'Sun','Mon','Tue','Wed','Thu','Fri','Sat'];
		public function DateUtils()
		{
			//Call static
		}
		public static function parseIso(value:String):Date
		{
		    var dateStr:String = value;
            dateStr = dateStr.replace(/-/g, "/");
            dateStr = dateStr.replace("T", " ");
            dateStr = dateStr.replace("Z", " GMT-0000");
            return new Date(Date.parse(dateStr));
		}
		public static function parseTime(value:String, guessAMPM:Boolean = true):Date
		{
			value = value.toUpperCase();
			var dt:Date = new Date(2000,0,1);
			var time:Object;
			var isMil:Boolean = false;
			//standard time regex
			var matches:Array;
			var reg:RegExp = /^(1[012]|[1-9])(:[0-5]\d)?(:[0-5]\d)?(\ ?[AaPp][Mm]?)?$/;
			matches = reg.exec(value);
			if(!matches) {
				//military time regex
				reg = /^(2[0-4]|1\d|0?\d)(:?[0-5]\d)?(:?[0-5]\d)?$/;
				isMil = true;
				matches = reg.exec(value);
			}
			if(!matches) {
				//could not parse
				return null;
			}
			time = {
				hours: Number(matches[1]),
				minutes: matches[2] ? Number(String(matches[2]).replace(':','')) : 0,
				seconds: matches[3] ? Number(String(matches[3]).replace(':','')) : 0,
				ampm: null
			};
			if(isMil) {
				//processing military format
				dt.setHours(time.hours, time.minutes, time.seconds);
			} else {
				//processing common format
				if(matches[4]) {
					//user indicated AM/PM
					if(String(matches[4]).indexOf('P') != -1) {
						//PM
						time.hours = time.hours == 12 ? 12 : time.hours + 12;
					} else if (time.hours == 12){
						time.hours = 0;
					}
				} else if (guessAMPM) {
					//will guess PM if <= 6
					time.hours = time.hours <= 6 ? time.hours + 12 : time.hours;
				}
			}
			dt.setHours(time.hours, time.minutes, time.seconds);
			return dt;
		}
		public static function format(value:Object, format:String = null):String
		{
			var ret:String = '';
			if(value == null) { 
				return ret;
			}
			_df.formatString = (format) ? format : DateUtils.FMT_ISO_DATE_TIME;
			ret = _df.format(value);
			if(ret.length == 0) {
				ret = _df.format(parseIso(value.toString()));
			}
			return ret;
		}
		public static function toHours(value:Object):Number
		{
			var d:Date;
			if(value is Date) {
				d = value as Date;
			} else {
				d = parseIso(value.toString());
			}
			if(!d) {
				return -1;
			} else {
				return d.getHours() + ((d.getMinutes() * 60 + d.getSeconds()) / 3600);
			}
		}
		public static function toMinutes(value:Object):Number
		{
			var d:Date;
			if(value is Date) {
				d = value as Date;
			} else {
				d = parseIso(value.toString());
			}
			if(!d) {
				return -1;
			} else {
				return d.getHours() * 60 + d.getMinutes() + (d.getSeconds() / 60);
			}
		}
		public static function toSeconds(value:Object):Number
		{
			var d:Date;
			if(value is Date) {
				d = value as Date;
			} else {
				d = parseIso(value.toString());
			}
			if(!d) {
				return -1;
			} else {
				return (d.getHours() * 3600) + (d.getMinutes() * 60) + d.getSeconds();
			}
		}
		public static function dayOfYear(dt:Date = null):int
		{
			if(!dt) {
				dt = new Date();
			}
			dt.setHours(0,0,0,0);
			var firstDay:Date = new Date(dt.getFullYear(), 0, 1);
			return Math.floor((dt.getTime() - firstDay.getTime()) / DateUtils.MILISECONDS_IN_DAY) + 1;
		}
		
		/**
		 * Rounds hours to the nearest tenth according to TESC payroll's roles
		 *  1-6  -> 0.1     37-42 -> 0.7
		 * 	7-12 -> 0.2  	43-48 -> 0.8
		 * 13-18 -> 0.3 ... 49-54 -> 0.9
		 * 19-24 -> 0.4		55-60 -> 1.0
		 */
		public static function toTenthHours(value:Object):Number
		{
			var hrs:Number = toHours(value);
			hrs = Math.ceil(hrs * 10) / 10.0;
			return hrs;
		}
	}
}