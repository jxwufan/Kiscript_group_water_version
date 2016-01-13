function Object() {
}

Object.create = function(proto) {
    var new_obj = new this();
    new_obj.__proto__ = proto;
    return new_obj;
};

var undefined = Object.fm;

function Function() {
}

Function.call = function(init_this) {
    var tmp = init_this.constructor;
    init_this.constructor = this;
    init_this.constructor(arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
    init_this.constructor = tmp;
};

function Array(length) {
    if (length == undefined) {
        this.length = 0;
    } else {
        this.length = length;
    }
}

Array.prototype.append = function(item) {
    this[this.length++] = item;
};

function substr(str, start, end) {
    var ret = "";
    for (var i = start; i < end; i++) {
        ret += str[i];
    }
    return ret;
}

function split(str, spliter) {
    var last = 0;
    var result = new Array(0);
    for (var i = 0; i < str.length; ++i) {
        if (str[i] == spliter) {
            if (last != i) {
                result.append(substr(str, last, i));
            }
            last = i + 1;
        }
    }
    if (last != str.length) {
        result.append(substr(str, last, str.length));
    }
    return result;
}

