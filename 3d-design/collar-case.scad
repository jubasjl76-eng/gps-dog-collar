/**
 * GPS Dog Collar - 3D Enclosure Design
 * OpenSCAD script
 * 
 * Features:
 * - Waterproof design
 * - Shock resistant
 * - Comfortable for dogs
 * - USB-C charging
 * - LED indicator window
 * - GPS antenna clearance
 */

// ============== PARAMETERS ==============
COLLAR_WIDTH = 40;     // mm
COLLAR_LENGTH = 80;    // mm  
COLLAR_HEIGHT = 18;    // mm
WALL_THICKNESS = 3;    // mm

// Component dimensions
ESP32_SIZE = [55, 35, 3];
GPS_SIZE = [25, 25, 8];
BATTERY_SIZE = [50, 30, 8];

// ============== MAIN ENCLOSURE ==============
module main_body() {
  difference() {
    // Outer shell with rounded corners
    hull() {
      translate([COLLAR_LENGTH/2 - 10, COLLAR_WIDTH/2 - 10, 0]) 
        cylinder(r=10, h=COLLAR_HEIGHT, center=true);
      translate([COLLAR_LENGTH/2 - 10, -COLLAR_WIDTH/2 + 10, 0]) 
        cylinder(r=10, h=COLLAR_HEIGHT, center=true);
      translate([-COLLAR_LENGTH/2 + 10, COLLAR_WIDTH/2 - 10, 0]) 
        cylinder(r=10, h=COLLAR_HEIGHT, center=true);
      translate([-COLLAR_LENGTH/2 + 10, -COLLAR_WIDTH/2 + 10, 0]) 
        cylinder(r=10, h=COLLAR_HEIGHT, center=true);
    }
    
    // Inner cavity
    translate([0, 0, WALL_THICKNESS]) {
      hull() {
        translate([COLLAR_LENGTH/2 - 15, COLLAR_WIDTH/2 - 15, 0]) 
          cylinder(r=8, h=COLLAR_HEIGHT, center=true);
        translate([COLLAR_LENGTH/2 - 15, -COLLAR_WIDTH/2 + 15, 0]) 
          cylinder(r=8, h=COLLAR_HEIGHT, center=true);
        translate([-COLLAR_LENGTH/2 + 15, COLLAR_WIDTH/2 - 15, 0]) 
          cylinder(r=8, h=COLLAR_HEIGHT, center=true);
        translate([-COLLAR_LENGTH/2 + 15, -COLLAR_WIDTH/2 + 15, 0]) 
          cylinder(r=8, h=COLLAR_HEIGHT, center=true);
      }
    }
    
    // Lid screws (4 corners)
    for (x = [-COLLAR_LENGTH/2 + 8, COLLAR_LENGTH/2 - 8]) {
      for (y = [-COLLAR_WIDTH/2 + 8, COLLAR_WIDTH/2 - 8]) {
        translate([x, y, COLLAR_HEIGHT/2]) cylinder(r=1.5, h=2, center=true);
      }
    }
  }
}

// ============== LID ==============
module lid() {
  difference() {
    hull() {
      translate([COLLAR_LENGTH/2 - 10, COLLAR_WIDTH/2 - 10, 0]) 
        cylinder(r=10, h=WALL_THICKNESS, center=true);
      translate([COLLAR_LENGTH/2 - 10, -COLLAR_WIDTH/2 + 10, 0]) 
        cylinder(r=10, h=WALL_THICKNESS, center=true);
      translate([-COLLAR_LENGTH/2 + 10, COLLAR_WIDTH/2 - 10, 0]) 
        cylinder(r=10, h=WALL_THICKNESS, center=true);
      translate([-COLLAR_LENGTH/2 + 10, -COLLAR_WIDTH/2 + 10, 0]) 
        cylinder(r=10, h=WALL_THICKNESS, center=true);
    }
    
    // LED indicator window
    translate([COLLAR_LENGTH/2 - 15, 0, 0]) {
      cylinder(r=4, h=WALL_THICKNESS + 1, center=true);
    }
    
    // Reset button hole
    translate([-COLLAR_LENGTH/2 + 15, COLLAR_WIDTH/2 - 12, 0]) {
      cylinder(r=3, h=WALL_THICKNESS + 1, center=true);
    }
  }
}

// ============== USB-C CHARGE PORT ==============
module charge_port() {
  // USB-C receptacle cutout
  translate([0, -COLLAR_WIDTH/2 + 5, COLLAR_HEIGHT/2]) {
    cube([10, 4, 3], center=true);
  }
}

// ============== GPS ANTENNA AREA ==============
module gps_antenna_area() {
  // Top area for GPS antenna - thin wall for signal
  translate([0, 0, COLLAR_HEIGHT/2 - 1]) {
    cylinder(r=12, h=2, center=true);
  }
}

// ============== BATTERY COMPARTMENT ==============
module battery_compartment() {
  translate([0, 10, 0]) {
    cube([BATTERY_SIZE[0] + 2, BATTERY_SIZE[1] + 2, BATTERY_SIZE[2]], center=true);
  }
}

// ============== STRAP MOUNTS ==============
module strap_mounts() {
  // Strap attachment points on sides
  for (x = [-COLLAR_LENGTH/2 + 15, COLLAR_LENGTH/2 - 15]) {
    translate([x, COLLAR_WIDTH/2 + 2, 0]) {
      cube([8, 6, 12]);
    }
    translate([x, -COLLAR_WIDTH/2 - 2, 0]) {
      cube([8, 6, 12]);
    }
  }
}

// ============== ASSEMBLY ==============
module collar_assembly() {
  // Main body
  color([0.2, 0.2, 0.2]) main_body();
  
  // Charge port
  color([0.3, 0.3, 0.3]) charge_port();
  
  // Strap mounts
  color([0.25, 0.25, 0.25]) strap_mounts();
  
  // GPS antenna area
  color([0.15, 0.15, 0.15, 0.7]) gps_antenna_area();
  
  // Lid (separate part)
  color([0.3, 0.3, 0.3]) translate([0, 0, COLLAR_HEIGHT/2 + WALL_THICKNESS/2]) {
    lid();
  }
}

// ============== PRINT LAYOUT ==============
module print_parts() {
  // Main body (bottom)
  translate([0, 0, 0]) main_body();
  
  // Lid (top)
  translate([0, COLLAR_WIDTH + 20, 0]) lid();
  
  // Strap mounts
  translate([COLLAR_LENGTH + 15, 0, 0]) strap_mounts();
}

// Render the assembly
collar_assembly();

// For 3D printing, uncomment:
// print_parts();

// ============== DIMENSIONS ==============
echo("=== GPS Dog Collar Dimensions ===");
echo(str("Length: ", COLLAR_LENGTH, "mm"));
echo(str("Width: ", COLLAR_WIDTH, "mm"));
echo(str("Height: ", COLLAR_HEIGHT, "mm"));
echo(str("Wall Thickness: ", WALL_THICKNESS, "mm"));
echo("");
echo("=== Component Clearance ===");
echo(str("ESP32: ", ESP32_SIZE[0], "x", ESP32_SIZE[1], "mm"));
echo(str("GPS Module: ", GPS_SIZE[0], "x", GPS_SIZE[1], "mm"));
echo(str("Battery: ", BATTERY_SIZE[0], "x", BATTERY_SIZE[1], "x", BATTERY_SIZE[2], "mm"));
