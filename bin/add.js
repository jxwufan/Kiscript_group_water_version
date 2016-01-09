function a(n) {
    n + 0;
    if (n == 0) {
        return 1;
    } else {
        return a(n - 1) * n;
    }
}

a(3);