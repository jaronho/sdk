/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-14
 ** Brief:	define type of class
 ***********************************************************************/
//----------------------------------------------------------------------
function setobjectclone(obj) {
    function copyObj(obj) {
        if (undefined == obj || null == obj || 'object' != typeof(obj) || obj instanceof HTMLElement) {
            return obj;
        }
        var newObj = obj.constructor ? new obj.constructor : {};
        for (var key in obj) {
            if (obj.hasOwnProperty(key)) {
                newObj[key] = copyObj(obj[key]);
            } else {
                newObj[key] = copyObj(obj[key]);
            }
        }
        return newObj;
    }
    return copyObj(obj);
}
//----------------------------------------------------------------------
// define getter setter, proto:Object, prop:String, getter:function, setter:function, getterName:String, setterName:String
function definegettersetter(proto, prop, getter, setter, getterName, setterName) {
    if (proto.__defineGetter__) {
        getter && proto.__defineGetter__(prop, getter);
        setter && proto.__defineSetter__(prop, setter);
    } else if (Object.defineProperty) {
        var desc = {enumerable: false, configurable: true};
        getter && (desc.get = getter);
        setter && (desc.set = setter);
        Object.defineProperty(proto, prop, desc);
    } else {
        throw new Error("browser does not support getters");
    }
    if (!getterName && !setterName) {
        // lookup getter/setter function
        var hasGetter = (null != getter), hasSetter = (undefined != setter), props = Object.getOwnPropertyNames(proto);
        for (var i = 0; i < props.length; i++) {
            var name = props[i];
            if ((proto.__lookupGetter__ ? proto.__lookupGetter__(name) : Object.getOwnPropertyDescriptor(proto, name)) || 'function' != typeof(proto[name])) {
                continue;
            }
            var func = proto[name];
            if (hasGetter && func == getter) {
                getterName = name;
                if (!hasSetter || setterName) {
                    break;
                }
            }
            if (hasSetter && func == setter) {
                setterName = name;
                if (!hasGetter || getterName) {
                    break;
                }
            }
        }
    }
    // found getter/setter
    var ctor = proto.constructor;
    if (getterName) {
        if (!ctor.__getters__) {
            ctor.__getters__ = {};
        }
        ctor.__getters__[getterName] = prop;
    }
    if (setterName) {
        if (!ctor.__setters__) {
            ctor.__setters__ = {};
        }
        ctor.__setters__[setterName] = prop;
    }
}
//----------------------------------------------------------------------
var mClassManager = {
    id: (0|(Math.random()*998)),
    instanceId: (0|(Math.random()*998)),
    getNewID: function() {
        return this.id++;
    },
    getNewInstanceId: function() {
        return this.instanceId++;
    }
};
//----------------------------------------------------------------------
// based on John Resig's Simple JavaScript Inheritance http://ejohn.org/blog/simple-javascript-inheritance/
Class=(function() {
    var fnTest = /\b_super\b/;
    // the base class implementation (does nothing)
    var Superclass = function(){};
    //----------------------------------------------------------------------
    // create a new class that inherits from this class, props:object, return:function
    Superclass.extend = function(props) {
        var _super = this.prototype;
        // instantiate a base class (but only create the instance, don't run the init constructor)
        var prototype = Object.create(_super);
        var classId = mClassManager.getNewID();
        mClassManager[classId] = _super;
        // copy the properties over onto the new prototype
        var desc = {writable: true, enumerable: false, configurable: true};
	    prototype.__instanceId = null;
	    // the dummy class constructor
	    function Subclass() {
		    this.__instanceId = mClassManager.getNewInstanceId();
		    // all construction is actually done in the init method
		    if ('function' == typeof(this.ctor)) {
                this.ctor.apply(this, arguments);
            }
	    }
        Subclass.id = classId;
	    desc.value = classId;
	    Object.defineProperty(prototype, '__pid', desc);
	    // populate our constructed prototype object
        Subclass.prototype = prototype;
	    // enforce the constructor to be what we expect
	    desc.value = Subclass;
	    Object.defineProperty(Subclass.prototype, 'constructor', desc);
	    // copy getter/setter
	    this.__getters__ && (Subclass.__getters__ = setobjectclone(this.__getters__));
	    this.__setters__ && (Subclass.__setters__ = setobjectclone(this.__setters__));
        for (var idx = 0, li = arguments.length; idx < li; ++idx) {
            var prop = arguments[idx];
            for (var name in prop) {
                var isFunc = ('function' == typeof(prop[name]));
                var override = ('function' == typeof(_super[name]));
                var hasSuperCall = fnTest.test(prop[name]);
                if (isFunc && override && hasSuperCall) {
                    desc.value = (function(name, fn) {
                        return function() {
                            var tmp = this._super;
                            // add a new ._super() method that is the same method but on the super-class
                            this._super = _super[name];
                            // the method only need to be bound temporarily, so we remove it when we're done executing
                            var ret = fn.apply(this, arguments);
                            this._super = tmp;
                            return ret;
                        };
                    })(name, prop[name]);
                    Object.defineProperty(prototype, name, desc);
                } else if (isFunc) {
                    desc.value = prop[name];
                    Object.defineProperty(prototype, name, desc);
                } else {
                    prototype[name] = prop[name];
                }
                if (isFunc) {
                    // override registered getter/setter
                    var getter, setter, propertyName;
                    if (this.__getters__ && this.__getters__[name]) {
                        propertyName = this.__getters__[name];
                        for (var i in this.__setters__) {
                            if (this.__setters__[i] == propertyName) {
                                setter = i;
                                break;
                            }
                        }
                        definegettersetter(prototype, propertyName, prop[name], prop[setter] ? prop[setter] : prototype[setter], name, setter);
                    }
                    if (this.__setters__ && this.__setters__[name]) {
                        propertyName = this.__setters__[name];
                        for (var j in this.__getters__) {
                            if (this.__getters__[j] == propertyName) {
                                getter = j;
                                break;
                            }
                        }
                        definegettersetter(prototype, propertyName, prop[getter] ? prop[getter] : prototype[getter], prop[name], getter, name);
                    }
                }
            }
        }
        // and make this class extendable
        Subclass.extend = Class.extend;
        // add implementation method
        Subclass.implement = function(prop) {
            for (var name in prop) {
                prototype[name] = prop[name];
            }
        };
        return Subclass;
    };
    return Superclass;
})();
//----------------------------------------------------------------------