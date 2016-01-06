var b = 5;
func a() {
	if (b == 1) {
		return b;
	} else {
		var c = b;
		b = b - 1;
		return c + a();
	}
}

a();
