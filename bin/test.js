var b = 5;

function d(n) {
    if (n == 1) {
        return n;
    } else {
        return b + d(n - 1);
    }
}

print(d);
