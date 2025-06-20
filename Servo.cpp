#include <ESP32Servo.h>  // Include the ESP32 Servo library

// Create Servo objects for X and Y axes
Servo servo_x;  // X-axis servo (left/right movement)
Servo servo_y;  // Y-axis servo (up/down movement)

// Define pin connections
const int LASER_PIN = 18;     // Laser module connected to GPIO18
const int SERVO_X_PIN = 12;   // X-axis servo control pin
const int SERVO_Y_PIN = 14;   // Y-axis servo control pin

// Define angle limits for the servos (calibrated for your setup)
const int X_MIN_ANGLE = 40;   // Left-most angle for X servo
const int X_MAX_ANGLE = 160;  // Right-most angle for X servo
const int Y_MIN_ANGLE = 40;   // Bottom-most angle for Y servo
const int Y_MAX_ANGLE = 160;  // Top-most angle for Y servo

// Define movement timing
const int MOVE_DELAY = 3000;  // Pause duration at each point in milliseconds
const int STEP_DELAY = 15;    // Delay between steps (not used in this version but useful for smooth movement)

// Setup function runs once when the ESP32 starts
void setup() {
  Serial.begin(115200);  // Initialize serial communication for debugging

  pinMode(LASER_PIN, OUTPUT);  // Set laser pin as output
  Serial.println("Laser Control Ready");

  // Attach the servo motors to their respective pins
  if (!servo_x.attach(SERVO_X_PIN)) {
    Serial.println("Failed to attach X servo");
  }
  if (!servo_y.attach(SERVO_Y_PIN)) {
    Serial.println("Failed to attach Y servo");
  }

  delay(500);  // Delay to allow servos to initialize
  Serial.println("Servo Control Ready");

  // Move servos to center position at startup
  moveToCenter();
}

// Function to move servos to a specific normalized position (-1.0 to 1.0)
void moveToPosition(float x, float y) {
  // Convert normalized position to servo angles using map
  int target_x = map(x * 100, -100, 100, X_MIN_ANGLE, X_MAX_ANGLE);
  int target_y = map(y * 100, -100, 100, Y_MIN_ANGLE, Y_MAX_ANGLE);

  // Constrain angles within allowed servo limits
  target_x = constrain(target_x, X_MIN_ANGLE, X_MAX_ANGLE);
  target_y = constrain(target_y, Y_MIN_ANGLE, Y_MAX_ANGLE);

  // Move the servos to calculated angles
  servo_x.write(target_x);
  servo_y.write(target_y);

  // Print movement info for debugging
  Serial.print("Moving to X:");
  Serial.print(x);
  Serial.print(" Y:");
  Serial.print(y);
  Serial.print(" (Angles X:");
  Serial.print(target_x);
  Serial.print(" Y:");
  Serial.print(target_y);
  Serial.println(")");

  delay(MOVE_DELAY);  // Wait at the new position
}

// Move both servos to center position (0,0 in normalized coordinates)
void moveToCenter() {
  moveToPosition(0.0, 0.0);
}

/*
// Example shape points (commented out version)
// IMT Atlantique Logo points, normalized in range -1.0 to 1.0
float points[][2] = {
  ...
};
*/

// Define shape points for drawing (triangle pointing down)
float points[][2] = {
  {0, -0.25},
  {1.0/8, 0},
  {-1.0/8, 0},
  {0, -0.25}
};

// Loop function runs repeatedly
void loop() {
  // Turn on the laser
  digitalWrite(LASER_PIN, HIGH);
  Serial.println("Laser ON");
  delay(1000);  // Keep laser on for 1 second before starting motion

  // Calculate number of points in the shape
  int numPoints = sizeof(points) / sizeof(points[0]);

  Serial.println("Starting movement sequence");

  // Iterate through all points and move the laser to each one
  for (int i = 0; i < numPoints; i++) {
    Serial.print("Moving to point ");
    Serial.print(i + 1);
    Serial.print(" of ");
    Serial.println(numPoints);

    // Move to the current point
    moveToPosition(points[i][0], points[i][1]);

    // Print current servo angles for verification
    Serial.print("Current position - X:");
    Serial.print(servo_x.read());
    Serial.print(" Y:");
    Serial.print(servo_y.read());
    Serial.println("\n");
  }

  Serial.println("Sequence completed. Restarting...");
  delay(1000);  // Short delay before restarting the loop
}
