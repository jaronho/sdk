/***********************************************************************
** Author:	jaron.ho
** Date:	2015-08-06
** Brief:	event define
***********************************************************************/
function EventDef_enum(tb) {
	var enumTb = {};
	for (var i = 0, len = tb.length; i < len; ++i) {
		if (enumTb.hasOwnProperty(tb[i])) {
			throw new Error("duplicate event: " + tb[i] + " in EventDef");
		}
		enumTb[tb[i]] = i;
	}
	return enumTb;
}
//----------------------------------------------------------------------
var EventDef = [
	"ED_LOGIN"				// 登录
];
//----------------------------------------------------------------------
EventDef = EventDef_enum(EventDef);
//----------------------------------------------------------------------