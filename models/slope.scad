module make_electromagnet(padding=0){
    color("blue")
    cylinder(d=25+padding, h=20+padding, center=true);
}

module make_inductive_sensor(padding=0, extra_length=0){
    color("purple")
    cube([16+padding, 30.1+padding+extra_length, 4.2+padding], center=true);
    rotate([90,0,0])
    cylinder(d=2, h=100, center=true, $fn=50);
}

module make_post(w=9.7, h=50, padding=0){
    cube([w+padding, w+padding, h+padding], center=true);
}

module make_ball(d=10){
    sphere(d=10, $fn=100);
}

module torus(r1=1, r2=2, angle=360, endstops=0, $fn=50){
    if(angle < 360){
        intersection(){
            rotate_extrude(convexity=10, $fn=$fn)
            translate([r2, 0, 0])
            circle(r=r1, $fn=$fn);
            
            color("blue")
            wedge(h=r1*3, r=r2*2, a=angle);
        }
    }else{
        rotate_extrude(convexity=10, $fn=$fn)
        translate([r2, 0, 0])
        circle(r=r1, $fn=$fn);
    }
    
    if(endstops && angle < 360){
        rotate([0,0,angle/2])
        translate([0,r2,0])
        sphere(r=r1);
        
        rotate([0,0,-angle/2])
        translate([0,r2,0])
        sphere(r=r1);
    }
}

module make_slope(){

    slope_d = 500;
    thickness = 10;
    width = 50;
    box_width = 180;
    post_w = 9.7;
    sensor_extra_length = 7;

    inner_slope_d = slope_d-thickness*2;

    difference(){
        intersection(){

            // slope
            color("red")
            translate([0,0,slope_d/2])
            rotate([90,0,0]){
                difference(){
                    cylinder(d=slope_d, h=width, center=true, $fn=300);
                    cylinder(d=inner_slope_d, h=width*2, center=true, $fn=300);
                }
            }

            // bounding box
            translate([0,0,0])
            cube([box_width, 50+1, 50], center=true);
        }

        // post cutouts
        for(i=[-1:2:1]) for(j=[-1:2:1])
        translate([(box_width/2-post_w/2-1.5)*i, (width/2-post_w/2-1.5)*j, -5])
        color("green")
        make_post(padding=0.5);
        
        // ball cutout
        translate([0,0,slope_d/2])
        rotate([0,-21.25,0])
        translate([0,0,-slope_d/2+12])
        //translate([0,-5,0])
        make_ball();
            
        // ball track
        translate([0,0,slope_d/2])
        rotate([90,0,0])
        torus(r1=1, r2=inner_slope_d/2, $fn=200);

        // magnet cutout
        translate([0,0,-2])
        //translate([0,-25,0])
        make_electromagnet(padding=0.5);

        // magnet mount holes
        for(i=[-1:2:1])
        translate([0,20*i,0])
        cylinder(d=2, h=10, center=true, $fn=50);

        translate([0,-2.5,0]){

            // inner sensors
            for(i=[-1:2:1])
            translate([0,0,slope_d/2])
            rotate([0,i*7,0])
            translate([0,-(30-23)-0,-slope_d/2+8-3])
            make_inductive_sensor(padding=0.5, extra_length=sensor_extra_length);

            // outer sensors
            for(i=[-1:2:1])
            translate([0,0,slope_d/2])
            rotate([0,i*13,0])
            translate([0,-(30-23)-0,-slope_d/2+8-3])
            make_inductive_sensor(padding=0.5, extra_length=sensor_extra_length);

        }

    }
}

module make_ring(){
    difference(){
        cylinder(d=30-0.5, h=10, center=true, $fn=100);
        cylinder(d=25+0.5, h=10*2, center=true, $fn=100);
        translate([0,100/2,0])
        cube([2,100,100], center=true);
    }
}

if(0)
make_ring();

if(1)
rotate([0,0,45])rotate([-90,0,0])
make_slope();

// pads to help bed adhesion
color("blue")
for(i=[0:1])
mirror([i,i,0])
translate([44.5,74,-50/2+0.2/2])
cube([15,15,0.2], center=true);

if(0)
translate([0,0,-2])
//translate([0,-25,0])
make_electromagnet(padding=0);

if(0)
translate([-19,-50,8])
sphere(r=5/2, center=true, $fn=50);

if(0)
translate([0,0,slope_d/2])
rotate([90,0,0])
cylinder(d=inner_slope_d, h=10, center=true, $fn=100);

