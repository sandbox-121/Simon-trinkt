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
#define BRIGHTNESS          19
#define FRAMES_PER_SECOND  120

//program specific setup
#define CYCLE_TIME 1          //delay in main loop in ms
int counter = 0;              //used for checking main loop repetitions
int last_position = 0;        //initialize fist comparison position for stationary check
int compare_position = 100;   //initialize second comparison position for stationary check 

#define direction_offset 270  //If arrow and active LED on board donm't match up, given in degrees, should always be positive
int simon = 27;
const int simon_button = 5;   // pick a suitable button; configure input pulldown and connect button between GND and chosen pin for all of these
const int color_button = 6;
const int third_button = 7;

//setting up variables for timing / parallelization
unsigned long triggerTime = 0;
unsigned long delayBeforeAnimation = 300;

//set up color palette and variable for current color
CRGBPalette16 myPalette;
int colorindex = 16;

void SetupPalette(){
myPalette = RainbowColors_p;
}

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
  //function that checks whether the current position is the same as it was a few ms ago
  counter = counter + 1;
  if(counter > 20/CYCLE_TIME && (latest_position != compare_position || latest_position != last_position)){
    counter = 0;
    compare_position = last_position;
    last_position = latest_position;
    triggerTime = millis();
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
      leds[line_leds[i]] = ColorFromPalette(myPalette, colorindex);
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
      leds[0] = ColorFromPalette(myPalette, colorindex);
    }
    else{
    leds[lower_LED + i] = ColorFromPalette(myPalette, colorindex);
    }
  }
  //draw lower line
  draw_line(lower_LED);
  //draw upper line
  draw_line(lower_LED + 5);
}

void play_field_animation(int current_field, int animationStep){

  switch (current_field){
    case 0:
    // left neighbor -> Chase to the left
      leds[49-(animationStep%50)] = ColorFromPalette(myPalette, colorindex);
      delay(20);
    break;
    case 1:
    case 6:
    //all
    fill_solid 	(leds, 150, ColorFromPalette(myPalette, colorindex));
    break;
    case 2:
    //left side
    fill_solid 	(leds, 25, ColorFromPalette(myPalette, colorindex));
    break;
    case 3:
    case 8:
    //you drink -> Flash current field
    if ((millis()-triggerTime)%(4*delayBeforeAnimation)>600){
    fill_solid 	(leds, 150, CRGB::Black);
    }
    else{
    highlight_field(current_field);
    }
    break;
    case 4:
    case 9:
    //drink and spin again -> Flash field and do a rotating pattern with fields
    if ((millis()-triggerTime)%(4*delayBeforeAnimation)>600){
    fill_solid 	(leds, 150, CRGB::Black);
    highlight_field(0);
    highlight_field(2);
    highlight_field(4);
    highlight_field(6);
    highlight_field(8);
    }
    else{
    fill_solid 	(leds, 150, CRGB::Black);
    highlight_field(1);
    highlight_field(3);
    highlight_field(5);
    highlight_field(7);
    highlight_field(9);
    }
    break;
    case 5:
    //simon drinks -> Highlight Simon's position
    fill_solid 	(leds, 150, CRGB::Black);
    for (int i = 0; i < 5; i++){
        leds[simon-2+i] = ColorFromPalette(myPalette, colorindex);
    }
    FastLED.show();
    delay(500);
    break;
    case 7:
    //right side
    for (int i = 25; i < 50; i++){
      leds[i] = ColorFromPalette(myPalette, colorindex);
    }
    leds[0] = ColorFromPalette(myPalette, colorindex);
    break;

  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(3000); // 3 second delay for recovery
  SetupPalette();
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  // Set up pins
  pinMode(simon_button, INPUT_PULLUP);
  pinMode(color_button, INPUT_PULLUP);
  pinMode(third_button, INPUT_PULLUP);

  // choose Simon
  while(not digitalRead(simon_button)){
    fill_solid 	(leds, 150, CRGB::Black);
    int led = angle();
    leds[led] = ColorFromPalette(myPalette, colorindex);
    FastLED.show();
    simon = led;
  }
  //confirm selected Simon
  leds[simon] = ColorFromPalette(myPalette, colorindex);
  FastLED.show();
  delay(1000);
  leds[simon] = CRGB::Black;
  FastLED.show();
  delay(500);
  leds[simon] = ColorFromPalette(myPalette, colorindex);
  delay(1000);
  //clear canvas
  fill_solid 	(leds, 150, CRGB::Black);
  FastLED.show();
  //keep dot static until first spin without starting a field animation
  int led = angle();
  leds[led] = ColorFromPalette(myPalette, colorindex);
  FastLED.show();
  while (led == angle()){
    delay(10);
  }
}

void loop() {
  //find current position
  int current_LED = angle();
  bool stationaryState = stationary(current_LED);

  //Set every LED to black
  if(not stationaryState){
  fill_solid 	(leds, 150, CRGB::Black);
  }

  //TESTING
  if (not digitalRead(color_button)){
    colorindex = colorindex + 1 % 256;
    delay(5);
  }

  if(stationaryState){
    int active_field = identify_field(current_LED);
    
    if((millis() - delayBeforeAnimation) > triggerTime){
      fill_solid 	(leds, 150, CRGB::Black);
      play_field_animation(active_field, counter);
      
    
    }
    else{
      highlight_field(active_field);
      FastLED.show();
    }
  }
  else{
    //set current LED
    leds[current_LED] = ColorFromPalette(myPalette, colorindex);

    //check if current LED is on a line and turn on line
    if (current_LED % 5 == 0) {
      draw_line(current_LED);
    }
  }
  
  //show canvas
  FastLED.show();
  delay(CYCLE_TIME);
}