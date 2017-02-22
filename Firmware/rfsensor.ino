/*
 Copyright (c) 2017 sigmaIQ.  All right reserved.

 This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
*/

#include <RFduinoBLE.h>

int count                 = 0;           // The number of iterations through the main loop before the display is updated count*100mS.
int updateinterval        = 1000;        // screen refresh rate
int loopdelay             = 100;          // main loop delay mS

// Pin Assignments
int powerSensor           = 6;//0;//PC0;           // Pin 23 - from RF Detector Chip - Analog Input


int maxCount              =10;


//Buffer Array for RF Measurements - we collect measurements every loopdelay=100mS and then average them over the updateinterval (which is the display rate)
int rfmeasurements[10];
    
// Last Measurements of RF Power taken
int adcPower              = 0;

void clearMeasurements() {
  for (int i = 0; i <= maxCount; i++) {
    rfmeasurements[i] = 1023;
  }
}


int adc2voltage(int adc) {

  int v = trunc((adc * (3.3/1023.0))*1000);
  return v;
}

void setup() {

   Serial.begin(9600);
   delay(500);

 // analogReference(EXTERNAL);
//  analogReference(VBG); // Sets the reference 1.2V band gap
//  analogSelection(VDD_1_3_PS);

   pinMode(powerSensor, INPUT);


   RFduinoBLE.advertisementData = "adc";
   RFduinoBLE.txPowerLevel = 4;//-20;

   RFduinoBLE.customUUID = "d2be6e7a-a915-4b02-a65c-787bae61d97f";
   RFduinoBLE.deviceName = "RF Power Meter";

   // start the BLE stack
   RFduinoBLE.begin();

   clearMeasurements();
  
   count        =0;
}

int averageMeasurements(int c) {

 // char result[4];
  int pwr = 0; 
  
 
  if (c > 0) {

    //Total up all the measurements
    for (int i = 1; i <= c; i++) {
      pwr = pwr + rfmeasurements[i];
    }
      if (pwr > 0) { // PWR is the raw voltage from the ADC

         // Calculate the Mean
         pwr = pwr / count;         // the pwr is still in units of ADC  
         pwr = adc2voltage(pwr);    // adjust ADC output with the Reference Voltage 
  }
}
return pwr;
}


void measureRF() {

  // Sample the RF Power from the ADC
  adcPower = analogRead(powerSensor);

 // Serial.println(adcPower);

  
  if (count>maxCount) {count=0;} //lets make sure we haven't stepped past the end of the array
  count++;
  
  rfmeasurements[count] = adcPower;
 
  if (count >= (updateinterval / loopdelay))
  {

    int currentADC=averageMeasurements(count);
    count = 0;  //reset the rfmeasurements counter

    Serial.println(currentADC);
    RFduinoBLE.sendInt(currentADC);
    
  }
}


void loop() {

  measureRF();
  delay(loopdelay);
}
