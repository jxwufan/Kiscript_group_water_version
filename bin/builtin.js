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

function Array() {

}

Array.prototype.append(item) {
    this[this.length++] = item;
}

function Function() {

}

Function.prototype.call = function(init_this) {
    init_this.constructor = this.constructor()
}