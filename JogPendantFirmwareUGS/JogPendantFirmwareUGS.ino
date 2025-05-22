/*  Author:   Seth Victus
    Date:     22-05-2025
    Version:  0.2
    Note: Firmware built for an 18 button CNC jogging device. This converts the button matrix into XInput data 
    that can be used in combination with Universal GCode Sender. The button matrix layout was very strange and 
    was manually reverse engineerd which is why the firmware uses a pin pair concept as pure matrix scanning 
    would result in a lot of unneeded readings of non existing buttons.
*/

//https://github.com/dmadison/ArduinoXInput --> Follow instructions on github before flashing!
#include <XInput.h>

const byte portSize = 15; //amount of pins declared
const byte portPairSize = 18; //amount of pin combinations that are relevant buttons

const byte initPorts[] = {2,3,4,5,6,7,8,9,10,14,15,18,19,20,21}; //List of pins to initialize
const String buttonNames[] = {  "NW","Y+","NE",
                                "X-","C","X+",
                                "SW","Y-","SE",
                                
                                "X","Y","Z",
                                "F1","Ur","Z+",
                                "Back","Ul","Z-"
                                };
                                //Names of the buttons mapped to declared pin combinations, only used for debugging via serial
byte portPairs[][3] = { {2,21},{2,6},{2,20},
                        {3,21},{3,5},{3,20},
                        {4,21},{4,6},{4,20},
                        
                        {7,15},{7,10},{3,18},
                        {2,15},{20,19},{8,20},
                        {10,14},{6,19},{4,9}                        
                        };
                        //Array containing the pin combination for a relevant button, [0] = LOW, [1] = Pullup INPUT, [2] = current button state, 1 = pressed, 0 = released
void setup() {
  Serial.begin(9600); //Only needed for debugging
  for (int i = 0; i < portSize; i++)
  {
    pinMode(initPorts[i],INPUT_PULLUP);
  }

  XInput.setAutoSend(false);
  XInput.setJoystickRange(-1000, 1000);
  XInput.begin();
}

void loop() {
  for(int i = 0; i < portPairSize; i++){
    byte oldState = portPairs[i][2]; //old state readout to make sure we only trigger on state changes
    pinMode(portPairs[i][0],OUTPUT);
    pinMode(portPairs[i][1],INPUT_PULLUP);
    digitalWrite(portPairs[i][0],LOW);
    if(digitalRead(portPairs[i][1]) == LOW){
      portPairs[i][2] = 1;
    } else {
      portPairs[i][2] = 0;
    }
    pinMode(portPairs[i][0],INPUT_PULLUP);
    pinMode(portPairs[i][1],INPUT_PULLUP);

    //DEBUG ONLY
    if(oldState != portPairs[i][2]){
      if(portPairs[i][2] == 1){
        Serial.println(buttonNames[i] + " pressed");
      } else {
        Serial.println(buttonNames[i] + " released");
      }
    }
    //END DEBUG ONLY

    //Prepare XInput report for sending

    XInput.setButton(BUTTON_A, portPairs[4][2]); //Centre Button
    //X Y Z buttons
    XInput.setButton(BUTTON_X, portPairs[9][2]);
    XInput.setButton(BUTTON_Y, portPairs[10][2]);
    XInput.setButton(BUTTON_B, portPairs[11][2]);
    //F1 / Ur 
    XInput.setButton(BUTTON_START, portPairs[12][2]);
    XInput.setButton(BUTTON_L3, portPairs[13][2]);
    //Back / Ul
    XInput.setButton(BUTTON_BACK, portPairs[15][2]);
    XInput.setButton(BUTTON_R3, portPairs[16][2]);


    //Z+ / Z-
    if(portPairs[14][2] != portPairs[17][2]){ // check if not both 0 or 1
      if(portPairs[14][2] == 1){
        XInput.setJoystickX(JOY_LEFT, 1000);
      } else {
        XInput.setJoystickX(JOY_LEFT, -1000);
      }
    } else if (portPairs[14][2] == 0){ // check if both are 0, if so return axis to 0
      XInput.setJoystickX(JOY_LEFT, 0);
    }

    // X / Y Axis
    if((portPairs[1][2] + portPairs[7][2] + portPairs[3][2] + portPairs[5][2] + portPairs[0][2] + portPairs[2][2] + portPairs[6][2] + portPairs[8][2]) == 1){ //check if only 1 button of the directional keys is pressed
      if(portPairs[0][2] == 1){ XInput.setJoystick(JOY_RIGHT,-1000,1000);}         //NW
      else if(portPairs[1][2] == 1){ XInput.setJoystick(JOY_RIGHT,0,1000);}        //N
      else if(portPairs[2][2] == 1){ XInput.setJoystick(JOY_RIGHT,1000,1000);}     //NE
      else if(portPairs[3][2] == 1){ XInput.setJoystick(JOY_RIGHT,-1000,0);}       //W
      else if(portPairs[5][2] == 1){ XInput.setJoystick(JOY_RIGHT,1000,0);}        //E
      else if(portPairs[6][2] == 1){ XInput.setJoystick(JOY_RIGHT,-1000,-1000);}   //SW
      else if(portPairs[7][2] == 1){ XInput.setJoystick(JOY_RIGHT,0,-1000);}       //S
      else if(portPairs[8][2] == 1){ XInput.setJoystick(JOY_RIGHT,1000,-1000);}    //SE
    } else {
      XInput.setJoystick(JOY_RIGHT,0,0); //Reset to 0,0
    }

    XInput.send();
  }
  
}
