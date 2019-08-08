/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2018-08-21
 ** Brief:  资源模块
 ***********************************************************************/
.pragma library
Qt.include("ResourceImageList.js")

if (!Array.indexOf) {
    Array.prototype.indexOf = function(obj) {
        for (var i = 0; i < this.length; ++i) {
            if (obj === this[i]) {
                return i;
            }
        }
        return -1;
    }
}

/* 字体大小 */
function fontSize(size, factor) {
    return size + ('number' === typeof(factor) ? factor : 0);
}

/* 图片路径 */
function urlimg(name, sizeType) {
    var fullPath = "";
    if (1 === sizeType) {
        fullPath = "image/middle/" + name;
    } else if (2 === sizeType) {
        fullPath = "image/large/" + name;
    } else {
        fullPath = "image/default/" + name;
    }
    if ( fullPath.length > 0 && resource_image_list.indexOf(fullPath) < 0) {
        console.log("can't find image file: \"" + fullPath + "\"");
    }
    return "qrc:/" + fullPath;
}

/* qml路径 */
function urlqml(name) {
    return "qrc:/qml/" + name;
}

/* 着色器路径 */
function urlshader(name) {
    return "qrc:/shader/" + name;
}

/* 模型路径 */
function urlobj(name) {
    return "qrc:/obj/" + name;
}
