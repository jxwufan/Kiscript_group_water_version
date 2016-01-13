var a = split("1,,,22,333,,,4444,,,55555", ",");
Log(a.length);


for (var i = 0; i < a.length; ++i) {
    Log(a[i + ""]);
}
