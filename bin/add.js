function People(name, age) {
    this.name = name;
    this.age = age;
}

function Student(name, age, status) {
    People.call(this, name, age);
    this.status = status;
}

var fan = new Student("Fan Wu", 20, "Gua ke");

fan.name;
fan.age;
fan.status;

Log(fan.name);

var a = "";
for (var i = 0; i < 10; i++) {
    a = a + "*";
    Log(a);
}