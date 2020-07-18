$fn = 300;

linear_extrude(10)
difference() {
    circle(d=16);
    circle(d=10);
}
linear_extrude(2)
difference() {
    circle(d=16);
    circle(d=3);
}