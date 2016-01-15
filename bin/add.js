var a = split("1,,,22,333,,,4444,,,55555", ",");
Log(a.length);


for (var i = 0; i < a.length; ++i) {
    Log(a[i + ""]);
}

function fib(n) {
    if (n < 2) {
        return 1;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}

Log("Fib:");
for (var i = 0; i < 50; ++i) {
    Log(fib(i));
}

REPL();
