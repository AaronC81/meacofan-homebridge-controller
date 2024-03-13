$fn=50;

INFINITY = 99999;
INFINITY_DIV2 = INFINITY / 2;

board_depth = 1.6;
board_width = 36;
board_height = 32;
corner_radius = 4.06;

module board_outline() {
    translate([corner_radius, corner_radius])
    minkowski() {
        circle(r=corner_radius);
        square([
            board_width - corner_radius * 2,
            board_height - corner_radius * 2,
        ]);
    }
}

module mounting_post() {
    m3_screw_hole_inner_diameter = 2.6;
    post_diameter = m3_screw_hole_inner_diameter + 3;
    
    difference() {
        circle(d=post_diameter);
        circle(d=m3_screw_hole_inner_diameter);
    }
}

module case() {
    case_border_width = 2;
    case_bottom_depth = 2;
    
    case_mounting_height = 7;
    
    module case_outline() {
        offset(case_border_width)
        board_outline();
    }
    module case_border() {
        difference() {
            case_outline();
            board_outline();
        }
    }

    // Base
    linear_extrude(case_bottom_depth)
    case_outline();
    
    translate([0, 0, case_bottom_depth]) {
        // Border around board
        linear_extrude(case_mounting_height)
        case_border();
        
        // Mounting posts
        linear_extrude(case_mounting_height - board_depth) {
            for (x = [corner_radius, board_width - corner_radius]) {
                for (y = [corner_radius, board_height - corner_radius]) {
                    translate([x, y])
                    mounting_post();
                }
            }
        }
    }
    
    // Raise one side of the border up higher, and cut an angled "chute" in it for the LED
    led_support_height = 70;
    led_diameter = 5;
    led_chute_border = 1.5;
    led_chute_length = 4;
    led_chute_angle = 120;
    module led_chute(add_length=0) {
        rotate([led_chute_angle, 0, 0])
        linear_extrude(led_chute_length + add_length)
        difference() {
            circle(d=led_diameter + led_chute_border);
            circle(d=led_diameter);
        }
    }
    
    translate([0, 0, case_bottom_depth + case_mounting_height])
    union() {
        difference() {
            linear_extrude(led_support_height)
            intersection() {
                case_border();

                translate([-INFINITY_DIV2, -case_border_width, 0])
                square([INFINITY, 10]);
            }
            
            translate([board_width / 2, led_chute_length/2.5, led_support_height - led_diameter])
            hull() led_chute(INFINITY);
        }
        
        translate([board_width / 2, led_chute_length/2.5, led_support_height - led_diameter])
        led_chute();
    }
}

case();
