#include "HX711.h"

// HX711 Pins
#define LOADCELL_DOUT_PIN  3
#define LOADCELL_SCK_PIN   2

// Relay Pin
#define RELAY_PIN 7

// Create HX711 object
HX711 scale;

// Configuration variables
float calibration_factor = -7050; // Adjust this value during calibration
float weight_threshold = 10.0;     // Minimum weight change in grams to consider milking active
unsigned long check_interval = 5000; // Check every 5 seconds (5000ms)
int stable_count_threshold = 3;    // Number of stable readings before stopping

// Working variables
float previous_weight = 0;
float current_weight = 0;
int stable_count = 0;
unsigned long last_check_time = 0;
bool machine_running = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Automated Milking Machine Controller (Relay + LED Demo)");
  Serial.println("======================================================");
  
  // Initialize relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Relay OFF initially
  
  // Initialize HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  Serial.println("Initializing load cell...");
  delay(2000);
  
  if (scale.is_ready()) {
    scale.set_scale(calibration_factor);
    scale.tare(); // Reset the scale to 0
    Serial.println("Load cell ready!");
    Serial.println("Place empty container and it will auto-tare in 3 seconds...");
    delay(3000);
    scale.tare();
    Serial.println("Tared! Ready to start milking.");
    Serial.println();
  } else {
    Serial.println("HX711 not found. Check wiring!");
    while (1); // Stop here if load cell not found
  }
  
  // Start the machine
  startMachine();
}

void loop() {
  // Check if it's time to measure weight
  if (millis() - last_check_time >= check_interval) {
    last_check_time = millis();
    
    // Read current weight
    if (scale.is_ready()) {
      current_weight = scale.get_units(10); // Average of 10 readings
      
      // Calculate weight change
      float weight_change = abs(current_weight - previous_weight);
      
      // Display current status
      Serial.print("Current Weight: ");
      Serial.print(current_weight, 1);
      Serial.print(" g | Weight Change: ");
      Serial.print(weight_change, 1);
      Serial.print(" g | Status: ");
      
      // Check if weight is changing (milking is active)
      if (weight_change >= weight_threshold) {
        stable_count = 0; // Reset stable count
        Serial.println("MILKING ACTIVE");
        
        // Ensure machine is running
        if (!machine_running) {
          startMachine();
        }
      } else {
        stable_count++;
        Serial.print("STABLE (");
        Serial.print(stable_count);
        Serial.print("/");
        Serial.print(stable_count_threshold);
        Serial.println(")");
        
        // If weight has been stable for threshold count, stop machine
        if (stable_count >= stable_count_threshold && machine_running) {
          stopMachine();
        }
      }
      
      // Update previous weight
      previous_weight = current_weight;
      
    } else {
      Serial.println("Load cell ");
    }
    
    Serial.println("---");
  }
}

void startMachine() {
  digitalWrite(RELAY_PIN, HIGH); // Turn ON relay (LED lights up)
  machine_running = true;
  Serial.println(">>> RELAY ON - LED/MACHINE STARTED <<<");
  Serial.println();
}

void stopMachine() {
  digitalWrite(RELAY_PIN, LOW); // Turn OFF relay (LED turns off)
  machine_running = false;
  Serial.println();
  Serial.println(">>> RELAY OFF - LED/MACHINE STOPPED - MILKING COMPLETE <<<");
  Serial.print("Total milk collected: ");
  Serial.print(current_weight, 1);
  Serial.println(" g");
  Serial.println();
  Serial.println("Remove container and press RESET to start again");
  Serial.println("====================================");
  
  // Optionally, you can add a delay or wait for manual reset
  while(1); // Stop program - requires manual reset to restart
}