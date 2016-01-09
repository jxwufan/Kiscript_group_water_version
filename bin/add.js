function a(b) {
    this.c = b;
}

a.prototype = {e:123};
d = new a(10);
d.e + 0;
