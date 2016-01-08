var b = 5;

var e;
e = true;

var a = {
    a: "123" + "asd",
    b: "345"
};

function d(n) {
    for (var i = 0; i < 3; i++) {
        print(i);
    }
    if (n == 1) {
        return n;
    } else {
        return b + d(n - 1);
    }
}

print(d);
