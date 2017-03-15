/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-18
 ** Brief:	system time
 ***********************************************************************/
STime = {};
//----------------------------------------------------------------------
// revise time(seconds from 1970-1-1), e.g. 1441712924
STime.revise = function(reviseSeconds) {
	// calculate the time lag, milliseconds
	var reviseTime = parseInt(reviseSeconds);
	if (!isNaN(reviseTime) && reviseTime > 0) {
		if (reviseTime == this.mReviseTime) {
			return;
		}
		this.mReviseTime = reviseTime;
		this.mDeltaTime = Date.now() - (reviseTime * 1000);
	}
};
//----------------------------------------------------------------------
// get revise time(seconds from 1970-1-1)
STime.getReviseTime = function() {
	return isNaN(this.mReviseTime) ? 0 : this.mReviseTime;
};
//----------------------------------------------------------------------
// get now time(milliseconds from 1970-1-1)
STime.getTime = function() {
	return isNaN(this.mDeltaTime) ? Date.now() : Date.now() - this.mDeltaTime;
};
//----------------------------------------------------------------------
// get now date
STime.getDate = function() {
	return isNaN(this.mDeltaTime) ? new Date() : new Date(Date.now() - this.mDeltaTime);
};
//----------------------------------------------------------------------
// get now date(custom)
STime.getDateEx = function() {
	var date = this.getDate();
	return {
		year: date.getFullYear(),						// >= 1970
		month: date.getMonth() + 1,						// 1-12
		day: date.getDate(),							// 1-31
		wday: 0 == date.getDay() ? 7 : date.getDay(),	// 1-7
		hour: date.getHours(),							// 0-23
		minute: date.getMinutes(),						// 0-59
		seconds: date.getSeconds(),						// 0-59
		milliseconds: date.getMilliseconds()			// 0-999
	};
};
//----------------------------------------------------------------------
// get date order(17)=year(4)+month(2)+day(2)+hour(2)+minute(2)+seconds(2)+milliseconds(3), e.g. "20151118103438714"
STime.getDateOrder = function() {
    function np(num, bit) {
        var len = num.toString().length;
        while (len++ < bit) {
            num = '0' + num;
        }
        return num.toString();
    }
    var dateEx = this.getDateEx();
    var dateOrder = np(dateEx.year, 4) + np(dateEx.month, 2) + np(dateEx.day, 2);
    dateOrder += np(dateEx.hour, 2) + np(dateEx.minute, 2) + np(dateEx.seconds, 2);
    dateOrder += np(dateEx.milliseconds, 3);
    return dateOrder;
};
//----------------------------------------------------------------------