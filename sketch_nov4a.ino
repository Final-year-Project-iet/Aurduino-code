#include <string.h>

// Pin definitions
const int trigPin = 9;
const int echoPin = 8;

// Constants
const int TIMEOUT = 25000;  // Maximum microseconds to wait for sensor
const int MIN_DISTANCE = 2; // Minimum reliable distance in cm
const int MAX_DISTANCE = 400; // Maximum reliable distance in cm
const int DETECTION_THRESHOLD = 40; // Distance threshold for object detection in cm

// Variables to store measurements
long duration;
int distance;
float smoothedDistance;  // For moving average
const int numReadings = 5;  // Number of readings to average
float readings[5];  // Array to store readings
int readIndex = 0;  // Current position in array

bool isArmed = false;
String cmd = "";

void setup() {
  // Set up serial communication
  Serial.begin(9600);
  
  // Configure the sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Initialize readings array
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
  
  Serial.println("Ultrasonic Sensor System Started");
}

int measureDistance() {
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echoPin with timeout
  duration = pulseIn(echoPin, HIGH, TIMEOUT);
  
  // Check for timeout or invalid readings
  if (duration == 0) {
    Serial.println("Error: Sensor timeout");
    return -1;
  }
  
  // Calculate distance in centimeters
  int dist = duration * 0.034 / 2;
  
  // Validate reading
  if (dist < MIN_DISTANCE || dist > MAX_DISTANCE) {
    Serial.println("Error: Distance out of range");
    return -1;
  }
  
  return dist;
}

float smoothDistance(int currentDistance) {
  // Add current reading to array
  readings[readIndex] = currentDistance;
  readIndex = (readIndex + 1) % numReadings;
  
  // Calculate average
  float total = 0;
  for (int i = 0; i < numReadings; i++) {
    total += readings[i];
  }
  return total / numReadings;
}

void loop() {
  // Read commands from Node.js
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      processSerialCommand(cmd);
      cmd = "";  // Reset command
    } else {
      cmd += c;
    }
  }

  // Take measurement
  distance = measureDistance();
  
  if (distance != -1) {  // Valid measurement
    // Apply smoothing
    smoothedDistance = smoothDistance(distance);
    
    // Print results
    Serial.print("Raw Distance: ");
    Serial.print(distance);
    Serial.print(" cm, Smoothed: ");
    Serial.print(smoothedDistance);
    Serial.print(" cm - ");
    
    if (isArmed) {
      if (smoothedDistance < DETECTION_THRESHOLD) {
        Serial.println("Object Detected!");
      } else {
        Serial.println("No Object");
      }
    } else {
      Serial.println("System is Unarmed");
    }
  }
  
  // Add a short delay between measurements
  delay(100);
}

void processSerialCommand(String command) {
   Serial.println("command");

    Serial.println(command);
  if (command == "ARM") {
    isArmed = true;
    Serial.println("STATUS_UPDATE:SYSTEM ARMED");
  } else if (command == "DISARM") {
    isArmed = false;
    Serial.println("STATUS_UPDATE:SYSTEM DISARMED");
  }
}