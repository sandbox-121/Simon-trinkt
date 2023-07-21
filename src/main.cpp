#include <Arduino.h>
#include <fastled.h>
#include <AS5X47.h>

//Rotary encoder lib setup
#define slaveSelectPin 10
AS5X47 as5047p(slaveSelectPin);

//FastLED lib setup
#define DATA_PIN 3
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    150  
CRGB leds[NUM_LEDS];
#define BRIGHTNESS          25
#define FRAMES_PER_SECOND  120

//program specific setup
#define CYCLE_TIME 1
int counter = 0;
int last_position = 100;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

int angle() {
  //function that measures the angle of the arrow and calculates the corresponding LED, which is returend
  float angle_measured = as5047p.readAngle();
  int angle_led = map(angle_measured, 0 , 359, 49, 0);
    Serial.println(angle_led);
  return angle_led;
}

bool stationary(int latest_position){
  //function that checks whether the current position is the same as it was 100 ms ago
  counter = counter + 1;
  if(counter > 100/CYCLE_TIME){
    counter = 0;
    last_position = latest_position;
  }
  if(latest_position == last_position){
   return true;
  } 
  else{
    return false;
  }
}


void draw_line(int head_led){
    int line_leds[9];
    if (head_led == 50){
      head_led = 0;
    }
    for (int i = 0; i < 10; i++){
      line_leds[i] = (head_led + 1) / 5  * 10 + 50 + i;
      leds[line_leds[i]] = CRGB::Red;
    }
}


void highlight_field(int current_LED){
  //highlight current slice of the field (all 6 LEDs on the outer circle and both border lines)
  // calculate lower bound of slice
  int lower_LED = current_LED / 5 *5;
  // turn on border, catch turning over at 49
  for (int i = 0; i < 6; i++){
    if(lower_LED+i >49){
      leds[0] = CRGB::Red;
    }
    else{
    leds[lower_LED + i] = CRGB::Red;
    }
  }
  //draw lower line
  draw_line(lower_LED);
  //draw upper line
  draw_line(lower_LED + 5);
}

void loop() {
  //Set every LED to black
  fill_solid 	(leds, 150, CRGB::Black);

  //find current position
  int current_LED = angle(); 

  if(stationary(current_LED)){
    highlight_field(current_LED);
  }
  else{
    //set current LED
    leds[current_LED] = CRGB::Red;

    //check if current LED is on a line and turn on line
    int div_line = current_LED % 5;
    if (div_line == 0) {
      draw_line(current_LED);
    }
  }
  

  //show canvas
  FastLED.show();
  delay(CYCLE_TIME);

}