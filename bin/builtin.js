/**
 * 
 * Created by Fan on 1/10/16.
 */

function Object() {
}

Object.create = function(proto) {
    var new_obj = new Object();
    new_obj.__proto__ = proto;
}

function Function() {

}

Function.prototype.call = function(init_this) {
    var tmp = init_this.constructor;
    init_this.constructor = this.constructor()
    init_this.constructor();
    init_this.constructor = tmp;
}

function Array() {
    this.length = 0;
}

Array.prototype.append = function(item) {
    this[this.length++] = item;
};

