var b = {};
var a = 0;
b.a = function(a, b, c) {
    var a = 5;

    return function() {return a;};
};

var d;
d = b.a(1,2,3);
//d();
d = 123;
