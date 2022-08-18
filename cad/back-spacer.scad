$fn = 300;

linear_extrude(10)
difference() {
    circle(d=16);
    circle(d=10);
}

linear_extrude(1.2)
difference() {
    circle(d=16);
    circle(d=2.3);
}

*linear_extrude(10)
difference() {
    translate([-8, 0, 0]) 
    square(size=[16, 9.65]);
    circle(d=10);
}

linear_extrude(1.6+1.2)
difference() {
    circle(d=10);
    circle(d=4.6, $fn=6);
}