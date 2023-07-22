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
int compare_position = 100;

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
  //function that checks whether the current position is the same as it was 200-400 ms ago
  counter = counter + 1;
  if(counter > 200/CYCLE_TIME){
    counter = 0;
    compare_position = last_position;
    last_position = latest_position;
  }
  //check if last two recorded positions are identical to current position. If only comparing to one it can lead to issues with very fast/slow spin resonating positions
  if(latest_position == compare_position && latest_position == last_position){
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

int identify_field(int current_LED){
  int field = current_LED / 5;
  return field;
}

void highlight_field(int current_field){
  //highlight current slice of the field (all 6 LEDs on the outer circle and both border lines)
  // calculate lower bound of slice
  int lower_LED = current_field *5;
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

void play_field_animation(int current_field){
  switch (current_field){
    case 0:
    // left neighbor

    break;
    case 1:
    case 6:
    //all
    fill_solid 	(leds, 150, CRGB::Red);
    break;
    case 2:
    //left side
    fill_solid 	(leds, 25, CRGB::Red);
    break;
    case 3:
    case 8:
    //you drink
    
    break;
    case 4:
    case 9:
    //drink and spin again
    
    break;
    case 5:
    //simon drinks

    break;
    case 7:
    //right side
    break;

  }
}

void loop() {
  //find current position
  int current_LED = angle(); 

    //Set every LED to black
  if(not stationary(current_LED)){
  fill_solid 	(leds, 150, CRGB::Black);
  }

  if(stationary(current_LED)){
    int active_field = identify_field(current_LED);
    highlight_field(active_field);
    FastLED.show();
    delay(300);
    play_field_animation(active_field);
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