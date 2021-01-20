
/***** FILE NovalithICLite_Shield_Starting_Software.ino  ***********
*
* Revison: 001.00
*
* AUTHOR   :     Dusica Radovic
* (c)      :     Copyright by Infineon
*
* USE      :     NovalithIC Lite Arduino Shield (Half-bridge x 2 Arduino Shield)-> BTN7030-1EPA  
*                Infineon XMC 1100 Bootkit 
*                or Arduino UNO µC board   
*
* COMPILE  :     with compiler from Arduino version 1.8.12
*
* LINK     :     Arduino development environment
*
* DESCRIPE :     NovalithIC Lite Arduino Shield 
*                The board is equiped with 2 BTN7030-1EPA device, each of which can drive one motor in Half-bridge configuration
*                or they can be programed to work in Full-bridge configuration
*                Diagnostic not included
* 
* BTN7030-1EPA:   Package: PG-TSDSO-14
*                 Nominal load current (TA = 85 °C)           IL(NOM)            =    7 A
*                 Minimum overload detection current          IL(OVL0)_150(HS)   =   14 A
*                 Typical current sense ratio at IL = IL(NOM) kILIS              =   4300
*                 VBAT                                                           = max 28V
*                 
* 
***********************************************************************/
// -------------------------------------------------------------------------------------------
// Pin declaration for Arduino UNO
// -------------------------------------------------------------------------------------------
// IC1:
#define EN1  10     //Enable pin
#define IN1  3      //Input pin
#define DEN1  12    //Diagnostic enable pin
#define IS1  A1     //Sense pin
#define IS1_ADC  0  //Sense pin to ADC
// IC2:
#define EN2  13
#define IN2  11
#define DEN2  9
#define IS2  A0
#define IS2_ADC  0

#define OLOFF_PU  8 // Diagnostic in open load

/**************** Diagnositc signals in A, V, Ohm, ms ****************/
#define K_ILIS  4400
#define R_SENSE  1200       // Ohms
#define I_IS_FAULT  5.5     // mA
#define V_IS_FAULT  6.6     // V_IS_FAULT is R_SENSE x I_IS_FAULT
#define I_IS_OLOFF  2.5     // mA
#define V_IS_OLOFF  3       // V_IS_OLOFF is R_SENSE x I_IS_OLOFF
#define V_DS_OLOFF  1.8     // V
#define T_IS_OLOFF_D  0.07  // timings in ms
  
/*************************** Error states ***************************/
bool open_load_off_state = false;
bool short_to_bat_off_state = false;
bool short_to_gnd_off_state = false;

bool open_load = false;
bool short_to_bat = false;
bool short_to_gnd = false;

/*************************** Global Variables ***************************/
int option;
bool dataAvailable=false;
String inputString="";

// -------------------------------------------------------------------------------------------
// Serial function declaration - serial port can be used (CTRL+SHIFT+M) to control the devices
// -------------------------------------------------------------------------------------------
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n')
      dataAvailable = true;      
  }
}

void getDataUART(){
if (dataAvailable){
    option=inputString.toInt();
    Serial.print(inputString);
    switch(option){
      case 1: 
        digitalWrite(EN1, HIGH); 
        break;
      case 2:
        digitalWrite(EN1, LOW);
        break;
      case 3: 
        digitalWrite(IN1, HIGH);
        break;
      case 4:
        digitalWrite(IN1, LOW);
        break;
      case 5:
        digitalWrite(EN2, HIGH);
        break;
      case 6:
        digitalWrite(EN2, LOW);
        break;
      case 7:
        digitalWrite(IN2, HIGH); 
        break;
      case 8:
        digitalWrite(IN2, LOW);
        break;
      case 9:
        digitalWrite(OLOFF_PU, HIGH);
        break;
      case 0:
        digitalWrite(OLOFF_PU, LOW);
        break;
    }
    inputString = ""; 
    dataAvailable = false;
  }
}

// -------------------------------------------------------------------------------------------
// Sense functions declaration - reading out the output current value mirrored on ISx pin
// -------------------------------------------------------------------------------------------
  float readUIs1(){
    float is1;
    is1 = analogRead(IS1);
    // convert to Volt, Arduino UNO has a resolution of 4.9mV per point (5V/1024points)
    is1 = is1 * 0.0049;
    return is1;
  }

  float readUIs2(){
    float is2;
    is2 = analogRead(IS2);
    // convert to Volt, Arduino UNO has a resolution of 4.9mV per point (5V/1024points)
    is2 = is2 * 0.0049;
    return is2;
  }

  float readIIs1(){
    float is1;
    is1 = analogRead(IS1);
    // convert to Amp, Arduino UNO has a resolution of 4.9mV per point (5V/1024points)
    is1 = is1 * 0.0049 * (K_ILIS/R_SENSE);
    return is1;
  }

  float readIIs2(){
    float is2;
    is2 = analogRead(IS2);
    // convert to Amp Arduino UNO has a resolution of 4.9mV per point (5V/1024points)
    is2 = is2 * 0.0049 * (K_ILIS/R_SENSE);
    return is2;
  }


// -------------------------------------------------------------------------------------------
// Mode setting functions (Enable, diagnostic enable and input pin setting to LOW/HIGH)
// -------------------------------------------------------------------------------------------
  void allSimpleStop(){
    digitalWrite(EN1, LOW);
    digitalWrite(EN2, LOW);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  void allGoToActiveModeWithDiagnostic(){
    digitalWrite(EN1, HIGH);
    digitalWrite(EN2, HIGH);
    digitalWrite(DEN1, HIGH);
    digitalWrite(DEN2, HIGH);
  }

  void allGoToActiveModeWithoutDiagnostic(){
    digitalWrite(EN1, HIGH);
    digitalWrite(EN2, HIGH);
    digitalWrite(DEN1, LOW);
    digitalWrite(DEN2, LOW);
  }

  void allGoToSleepMode(){
    digitalWrite(EN1, LOW);
    digitalWrite(EN2, LOW);
    digitalWrite(DEN1, LOW);
    digitalWrite(DEN2, LOW);
  }


// -------------------------------------------------------------------------------------------
// H-Bridge configuration setting and driving the load
// -------------------------------------------------------------------------------------------

// Current from out1->out2: negative
  void ic1HSfwLSactive(float on_time_ms, float off_time_ms){//High Side FreeWheel, Low Side ON
    //digitalWrite(DEN1, HIGH);
    digitalWrite(EN1, HIGH);
    digitalWrite(EN2, HIGH);
    //start rotation
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    delay(on_time_ms);
    //stop rotation
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
    delay(off_time_ms);
    //digitalWrite(DEN1, LOW);
  }

// Current from out1->out2: positive
  void ic1HSactiveLSfw(float on_time_ms, float off_time_ms){//High Side ON, Low Side FreeWheel
    digitalWrite(DEN1, HIGH);
    digitalWrite(EN1, HIGH);
    digitalWrite(EN2, HIGH);
    //start rotation
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    delay(on_time_ms);
    //stop rotation
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    delay(off_time_ms);
    digitalWrite(DEN1, LOW);
  }

// Current from out1->out2: positive
  void ic2HSfwLSactive(float on_time_ms, float off_time_ms){//
    digitalWrite(DEN2, HIGH);
    digitalWrite(EN1, HIGH);
    digitalWrite(EN2, HIGH);
    //start rotation
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    delay(on_time_ms);
    //stop rotation
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
    delay(off_time_ms);
    //digitalWrite(DEN2, LOW);
    //digitalWrite(EN2, LOW);
    delay(off_time_ms);
  }

// Current from out1->out2: negative
  void ic2HSactiveLSfw(float on_time_ms, float off_time_ms){
    digitalWrite(DEN2, HIGH);
    digitalWrite(EN1, HIGH);
    digitalWrite(EN2, HIGH);
    //start rotation
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    delay(on_time_ms);
    //stop rotation
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    delay(off_time_ms);
    //digitalWrite(DEN2, LOW);
    //digitalWrite(EN2, LOW);
    delay(off_time_ms);
  }


// -------------------------------------------------------------------------------------------
// Diagnostic function: Open Load in OFF, Short to VBAT in OFF, Short to GND in OFF
// -------------------------------------------------------------------------------------------

  // returns true if OL or other error is detected in off state
  // returns false if NO OL or other error is detected in off state
  
  bool OpenLoadDetectInOffStatePnpAtOut1(){
    float is1_pnp_off;
    float is1_pnp_on;
    float is2_pnp_off;
    float is2_pnp_on;

    if(EN1==HIGH || EN2==HIGH){
      Serial.print("\nError ic2OpenLoadDetectInOffState: IC1 or IC2 not in OFF state. Set EN1 and EN2 to zero!");
      return true;
    }

    digitalWrite(OLOFF_PU, LOW);
    delay(T_IS_OLOFF_D);
    is1_pnp_off = readUIs1();
    is2_pnp_off = readUIs2();
    digitalWrite(OLOFF_PU, HIGH);
    delay(T_IS_OLOFF_D);
    is1_pnp_on = readUIs1();
    is2_pnp_on = readUIs2();
    digitalWrite(OLOFF_PU, LOW);
    delay(T_IS_OLOFF_D);
    
    if((is1_pnp_off<V_IS_OLOFF) && (is2_pnp_off<V_IS_OLOFF) && (is1_pnp_on>V_IS_OLOFF) && (is2_pnp_on>V_IS_OLOFF)){
      open_load = false;
      short_to_bat = false;
      short_to_gnd = false;
      return false;
    } else if ((is1_pnp_off<V_IS_OLOFF) && (is2_pnp_off<V_IS_OLOFF) && (is1_pnp_on>V_IS_OLOFF) && (is2_pnp_on<V_IS_OLOFF)){
      Serial.print("\nError: Open Load in off state");
      open_load = true;
      short_to_bat = false;
      short_to_gnd = false;
      return true;
    } else if ((is1_pnp_off>V_IS_OLOFF) && (is2_pnp_off>V_IS_OLOFF) && (is1_pnp_on>V_IS_OLOFF) && (is2_pnp_on>V_IS_OLOFF)){
      Serial.print("\nError: Short to Battery in off state");
      open_load = false;
      short_to_bat = true;
      short_to_gnd = false;
      return true;
    } else if ((is1_pnp_off<V_IS_OLOFF) && (is2_pnp_off<V_IS_OLOFF) && (is1_pnp_on<V_IS_OLOFF) && (is2_pnp_on<V_IS_OLOFF)){
      Serial.println("\nError: Short to GND in off state, check it out dude!");
      open_load = false;
      short_to_bat = false;
      short_to_gnd = true;
      return true;
    } else {
      Serial.print("\nError: Open Load and short to Battery or GND in off state");
      open_load = true;
      short_to_bat = true;
      short_to_gnd = true;
      return true;
    }
  }  


// -------------------------------------------------------------------------------------------
// Half-bridge functions to drive a motor with switching directions
// -------------------------------------------------------------------------------------------

 //a motor has to be connected between the corresponding VOUT and Shield's GND 
 void switchDirectionHSfw(float on_time_ms, float off_time_ms, int num_on_off_cycles){
  int i = 0;
  for(i=0;i<num_on_off_cycles;i++){
    ic1HSfwLSactive(on_time_ms, off_time_ms);
    ic2HSfwLSactive(on_time_ms, off_time_ms);
  }
 }
 
 //a motor has to be connected between the corresponding VOUT and Shield's VBAT 
 void switchDirectionLSfw(float on_time_ms, float off_time_ms, int num_on_off_cycles){
  int i = 0;
  for(i=0;i<num_on_off_cycles;i++){
    ic2HSactiveLSfw(on_time_ms, off_time_ms);
    ic1HSactiveLSfw(on_time_ms, off_time_ms);
  }
 }  
  
void setup() {
  // Pin initialization
  pinMode(EN1, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(DEN1, OUTPUT);
  pinMode(EN2, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(DEN2, OUTPUT);
  pinMode(OLOFF_PU, OUTPUT);
  pinMode(IS1, INPUT);

  // initial condition: OFF with diagnostic ( = stand by mode)
  digitalWrite(EN1, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(DEN1, HIGH);
  digitalWrite(EN2, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(DEN2, HIGH);
  // initial OL detect: OFF
  digitalWrite(OLOFF_PU, LOW);

  // Serial initialization
  Serial.begin(9600);

}

void loop() {
  //call the function you want to execute
  switchDirectionLSfw(500,500,1);

}
