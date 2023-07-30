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
#define CYCLE_TIME 1 //delay in main loop in ms
int counter = 0; //used for checking main loop repetitions
int last_position = 100; //initialize fist comparison position for stationary check
int compare_position = 100; //initialize second comparison position for stationary check 
#define direction_offset 270 //If arrow and active LED on board donm't match up, given in degrees, should always be positive
int simon = 27;
const int simon_button = 5; // pick a suitable button; configure input pulldown and connect button between GND and chosen pin


int angle() {
  //function that measures the angle of the arrow and calculates the corresponding LED, which is returend
  float angle_measured = as5047p.readAngle() + direction_offset;
  if (angle_measured > 360){
    angle_measured = angle_measured - 360;
  }
  int angle_led = map(angle_measured, 0, 359, 49, 0);
  //Serial.println(angle_led);
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
  int lower_LED = current_field * 5;
  // turn on border, catch turning over at 49
  for (int i = 0; i < 6; i++){
    if(lower_LED + i > 49){
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
    // left neighbor -> Chase to the left

    break;
    case 1:
    case 6:
    //all
    fill_solid 	(leds, 150, CRGB::Red);
    FastLED.show();
    break;
    case 2:
    //left side
    fill_solid 	(leds, 25, CRGB::Red);
    FastLED.show();
    break;
    case 3:
    case 8:
    //you drink -> Flash current field
    fill_solid 	(leds, 150, CRGB::Black);
    FastLED.show();
    delay(500);
    highlight_field(current_field);
    FastLED.show();
    delay(500);
    break;
    case 4:
    case 9:
    //drink and spin again -> Flash field and do a rotating pattern with fields
    fill_solid 	(leds, 150, CRGB::Black);
    highlight_field(0);
    highlight_field(2);
    highlight_field(4);
    highlight_field(6);
    highlight_field(8);
    FastLED.show();
    delay(500);
    fill_solid 	(leds, 150, CRGB::Black);
    highlight_field(1);
    highlight_field(3);
    highlight_field(5);
    highlight_field(7);
    highlight_field(9);
    FastLED.show();
    delay(500);
    break;
    case 5:
    //simon drinks -> Highlight Simon's position
    fill_solid 	(leds, 150, CRGB::Black);
    for (int i = 0; i < 5; i++){
        leds[simon-2+i] = CRGB::Red;
    }
    FastLED.show();
    delay(500);
    break;
    case 7:
    //right side
    for (int i = 25; i < 50; i++){
      leds[i] = CRGB::Red;
    }
    leds[0] = CRGB::Red;
    FastLED.show();
    break;

  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(3000); // 3 second delay for recovery
   
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  // choose Simon
  pinMode(simon_button, INPUT_PULLUP);
  while(not digitalRead(simon_button)){
    fill_solid 	(leds, 150, CRGB::Black);
    int led = angle();
    leds[led] = CRGB::Red;
    FastLED.show();
    simon = led;
  }
  //confirm selected Simon
  leds[simon] = CRGB::Red;
  FastLED.show();
  delay(1000);
  leds[simon] = CRGB::Black;
  FastLED.show();
  delay(500);
  leds[simon] = CRGB::Red;
  delay(1000);
  //clear canvas
  fill_solid 	(leds, 150, CRGB::Black);
  FastLED.show();
  //keep dot static until first spin without starting a field animation
  int led = angle();
  leds[led] = CRGB::Red;
  FastLED.show();
  while (led == angle()){
    delay(10);
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
    fill_solid 	(leds, 150, CRGB::Black);
    FastLED.show();
    while (current_LED == angle()){
      play_field_animation(active_field);
    }
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