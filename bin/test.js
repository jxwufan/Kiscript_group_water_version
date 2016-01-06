var b = 5;

var a = {
    a: "123",
    b: "345"
};

function d(n) {
    if (n == 1) {
        return n;
    } else {
        return b + d(n - 1);
    }
}

print(d);
