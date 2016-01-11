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

var a = [1,2,3];
a.append(234);
Log(a.length);
Log(a[3]);
var b = new Array();
b.append(123);
Log(b.length);

Log(fan.name);
Log(fan.age);
Log(fan.status);
