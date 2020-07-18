$fn = 300;

linear_extrude(1)
difference(){
    l1 = 8;
    l = 96.2 + l1*2;
    r = 5;
    translate([l/2-l1, 0, 0]) 
    minkowski() {
        square(size=[l-r, 10-r], center=true);
        circle(d=r);
    }
    circle(d=2.8);
    translate([96.2, 0, 0]) 
    circle(d=2.8);
}
