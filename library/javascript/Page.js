/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-11
 ** Brief:	page
 ***********************************************************************/
Page = {};
//----------------------------------------------------------------------
// 获取指定数据,beginIndex:从0开始,(数据范围从beginIndex -> beginIndex + numPerPage)
Page.getData = function(arr, numPerPage, beginIndex) {
    if (!(arr instanceof Array)) {
        throw new Error("not support for arr type " + typeof(arr));
    }
    if (isNaN(numPerPage) || numPerPage <= 0) {
        throw new Error("not support for numPerPage");
    }
    if (isNaN(beginIndex) || beginIndex < 0) {
        throw new Error("not support for beginIndex");
    }
    var dataArr = [];
    var arrCount = arr.length;
    if (0 == arrCount || beginIndex < 0 || beginIndex >= arrCount) {
        return dataArr;
    }
    if (arrCount < numPerPage) {    // 不足一页
        beginIndex = 0;
    }
    var endIndex = beginIndex + numPerPage; // 结束索引号
    if (endIndex > arrCount) {  // 结束索引号 > 总个数
        endIndex = arrCount;
    }
    for (var i = beginIndex; i < endIndex; ++i) {
        dataArr.push(arr[i]);
    }
    return dataArr;
};
//----------------------------------------------------------------------
// 获取总页数,如:[1,2,3],[4,5,6],[7,8,9] => 3页
Page.getCountA = function(totalCount, numPerPage) {
    if (isNaN(totalCount) || totalCount < 0) {
        throw new Error("not support for totalCount");
    }
    if (isNaN(numPerPage) || numPerPage <= 0) {
        throw new Error("not support for numPerPage");
    }
    if (0 == totalCount) {
        return 0;
    }
    var div = Math.floor(totalCount/numPerPage);
    var rem = totalCount%numPerPage;
    if (0 == rem) {
        return div;
    }
    return div + 1;
};
//----------------------------------------------------------------------
// 获取指定页数据,currPage:从0开始,如:[1,2,3],[4,5,6],[7,8,9],第2页=>[4,5,6]
Page.getDataA = function(arr, numPerPage, currPage) {
    if (!(arr instanceof Array)) {
        throw new Error("not support for arr type " + typeof(arr));
    }
    if (isNaN(numPerPage) || numPerPage <= 0) {
        throw new Error("not support for numPerPage");
    }
    if (isNaN(currPage) || currPage < 0) {
        throw new Error("not support for currPage");
    }
    var dataArr = [];
    var arrCount = arr.length;
    var pageCount = this.getCountA(arrCount, numPerPage);
    if (0 == pageCount || currPage >= pageCount) {
        return dataArr;
    }
    var beginIndex = numPerPage*currPage;   // 开始索引号
    if (arrCount < numPerPage) {    // 不足一页
        beginIndex = 0;
    }
    var endIndex = beginIndex + numPerPage; // 结束索引号
    if (endIndex > arrCount) {  // 结束索引号 > 总个数
        endIndex = arrCount;
    }
    for (var i = beginIndex; i < endIndex; ++i) {
        dataArr.push(arr[i]);
    }
    return dataArr;
};
//----------------------------------------------------------------------
// 获取总页数,如:[1,2,3],[3,4,5],[5,6,7] => 3页
Page.getCountB = function(totalCount, numPerPage) {
    if (isNaN(totalCount) || totalCount < 0) {
        throw new Error("not support for totalCount");
    }
    if (isNaN(numPerPage) || numPerPage <= 0) {
        throw new Error("not support for numPerPage");
    }
    if (0 == totalCount) {
        return 0;
    }
    var div = Math.floor(totalCount/(numPerPage - 1));
    var rem = totalCount%(numPerPage - 1);
    if (1 >= rem) {
        return div;
    }
    return div + 1;
};
//----------------------------------------------------------------------
// 获取指定页数据,currPage:从0开始,如:[1,2,3],[3,4,5],[5,6,7],第2页=>[3,4,5]
Page.getDataB = function(arr, numPerPage, currPage) {
    if (!(arr instanceof Array)) {
        throw new Error("not support for arr type " + typeof(arr));
    }
    if (isNaN(numPerPage) || numPerPage <= 0) {
        throw new Error("not support for numPerPage");
    }
    if (isNaN(currPage) || currPage < 0) {
        throw new Error("not support for currPage");
    }
    var dataArr = [];
    var arrCount = arr.length;
    var pageCount = this.getCountB(arrCount, numPerPage);
    if (0 == pageCount || currPage >= pageCount) {
        return dataArr;
    }
    var beginIndex = (numPerPage - 1)*currPage; // 开始索引号
    if (arrCount < numPerPage) {    // 不足一页
        beginIndex = 0;
    }
    var endIndex = beginIndex + numPerPage; // 结束索引号
    if (endIndex > arrCount) {  // 结束索引号 > 总个数
        endIndex = arrCount;
    }
    for (var i = beginIndex; i < endIndex; ++i) {
        dataArr.push(arr[i]);
    }
    return dataArr;
};
//----------------------------------------------------------------------
// 创建页管理器,dataList:数据内容,numPerPage:每页显示几条;beginIndex:从0开始
Page.create = function(dataList, numPerPage, beginIndex) {
    var MoveType = {            // 移动类型
        MT_NONE: 0,                 // 无
        MT_ONE_PREV: 1,             // 向前移动一个
        MT_ONE_NEXT: 2,             // 向后移动一个
        MT_PAGE_PREV: 3,            // 向前移动一页(这里后一页的第一个是前一页的最后个)
        MT_PAGE_NEXT: 4,            // 向后移动一页(这里前一页的最后个是后一页的第一个)
        MT_PAGE_FIRST: 5,           // 移动到第一页
        MT_PAGE_LAST: 6             // 移动到最后页
    };
    var mDataList = [];         // 数据列表
    var mNumPerPage = 1;        // 每页显示几条
    var mBeginIndex = 0;        // 开始索引
    var page = {};              // 页管理器
    var self = this;
    //----------------------------------------------------------------------
    // 移动页面
    function movePage(moveType) {
        var dataCount = mDataList.length;
        if (0 == dataCount) {    // 数据为空
            return [];
        }
        if (dataCount < mNumPerPage) {  // 数据不足一页
            mBeginIndex = 0;
            return self.getData(mDataList, mNumPerPage, mBeginIndex);
        }
        if (mBeginIndex > dataCount - mNumPerPage) {    // 开始索引>最后页的开始索引
            mBeginIndex = dataCount - mNumPerPage;
            return self.getData(mDataList, mNumPerPage, mBeginIndex);
        }
	    if (MoveType.MT_ONE_PREV == moveType) {
		    if (mBeginIndex > 0) {
			    --mBeginIndex;
		    }
	    } else if (MoveType.MT_ONE_NEXT == moveType) {
		    if (mBeginIndex + mNumPerPage < dataCount) {
			    ++mBeginIndex;
		    }
	    } else if (MoveType.MT_PAGE_PREV == moveType) {
		    if (mBeginIndex - mNumPerPage > 0) {
			    mBeginIndex -= mNumPerPage - 1;
		    } else {
			    mBeginIndex = 0;
		    }
	    } else if (MoveType.MT_PAGE_NEXT == moveType) {
		    if (mBeginIndex + mNumPerPage*2 < dataCount) {
			    mBeginIndex += mNumPerPage - 1;
		    } else {
			    mBeginIndex = dataCount - mNumPerPage;
		    }
	    } else if (MoveType.MT_PAGE_FIRST == moveType) {
		    mBeginIndex = 0;
	    } else if (MoveType.MT_PAGE_LAST == moveType) {
		    mBeginIndex = dataCount - mNumPerPage;
	    }
        return self.getData(mDataList, mNumPerPage, mBeginIndex);
    }
    //----------------------------------------------------------------------
    // 重置
    page.reset = function(dataList, numPerPage, beginIndex) {
        if (!(dataList instanceof Array)) {
            throw new Error("not support for dataList type " + typeof(dataList));
        }
        numPerPage = isNaN(numPerPage) || numPerPage <= 0 ? 1 : numPerPage;
        beginIndex = isNaN(beginIndex) || beginIndex < 0 ? 0 : beginIndex;
        mDataList = dataList;
        mNumPerPage = numPerPage;
        mBeginIndex = beginIndex;
    };
    //----------------------------------------------------------------------
    // 是否可以前移
    page.isCanMovePrev = function() {
        return !(0 == mDataList.length || 0 == mBeginIndex);
    };
    //----------------------------------------------------------------------
    // 是否可以后移
    page.isCanMoveNext = function() {
        return !(0 == mDataList.length || mNumPerPage == mDataList.length || mBeginIndex >= mDataList.length - mNumPerPage);
    };
    //----------------------------------------------------------------------
    // 获取索引值
    page.getBeginIndex = function() {
        return mBeginIndex;
    };
    //----------------------------------------------------------------------
    // 获取每页条数
    page.getNumPerPage = function() {
        return mNumPerPage;
    };
    //----------------------------------------------------------------------
    // 获取当前页
    page.getCurrPage = function() {
        return movePage(MoveType.MT_NONE);
    };
    //----------------------------------------------------------------------
    // 向前移一个
    page.moveOnePrev = function() {
        return movePage(MoveType.MT_ONE_PREV);
    };
    //----------------------------------------------------------------------
    // 向后移一个
    page.moveOneNext = function() {
        return movePage(MoveType.MT_ONE_NEXT);
    };
    //----------------------------------------------------------------------
    // 向前移一页
    page.movePagePrev = function() {
        return movePage(MoveType.MT_PAGE_PREV);
    };
    //----------------------------------------------------------------------
    // 向后移一页
    page.movePageNext = function() {
        return movePage(MoveType.MT_PAGE_NEXT);
    };
    //----------------------------------------------------------------------
    // 移动到第一页
    page.movePageFirst = function() {
        return movePage(MoveType.MT_PAGE_FIRST);
    };
    //----------------------------------------------------------------------
    // 移动到最后页
    page.movePageLast = function() {
        return movePage(MoveType.MT_PAGE_LAST);
    };
    //----------------------------------------------------------------------
    // 移动到索引所在页面index:从0开始
    page.moveToIndex = function(index) {
        var num = Math.floor(index/mNumPerPage);
        mBeginIndex = mNumPerPage*num ;
        return movePage();
    };
    //----------------------------------------------------------------------
    page.reset(dataList, numPerPage, beginIndex);
    return page;
};
//----------------------------------------------------------------------