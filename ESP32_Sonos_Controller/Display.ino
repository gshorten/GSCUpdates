/*
   Display related functions
*/

void displayText(String lines[3], String Line4, int numLines) {
  // displays 3 rows of text, centered.
  const byte dispWidth = 128;                  // how many characters can be displayed
  const long charWidth = 6.5;                 // typical character width
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setBrightness(255);
  //display->setFont(ArialMT_Plain_16);       //alternative text, is really big
  //Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->setFont(Roboto_Condensed_12);
  //Heltec.display->setFont(DejaVu_Sans_12);
  int lineSpacing[] = {0, 16, 31};            //y position for starting pixel for each of 4 rows, based on the 10pt font
  int lineStart = 0;                          //x position for starting pixel
  Heltec.display->clear();                    // clear what is on the display

  for (int i = 0; i < numLines; i++) {
    // loop through text to be displayed, center short text, left align long text
    int lineLength = Heltec.display->getStringWidth(lines[i]);
    if (lineLength < dispWidth) {         //adjust start to center, only do this for first two lines of text
      lineStart = ((dispWidth - lineLength) / 2);
    }
    else {
      lineStart = 0;
    }
    if (i <= 1 ) {
      Heltec.display -> drawString(lineStart, lineSpacing[i], lines[i]);  //load the lines to the display buffer
    }
    else {
      // wrap the third line of text, typically for something that might be long, ie artist, title, but shorten it
      Heltec.display -> drawStringMaxWidth(lineStart, lineSpacing[i], dispWidth, lines[i]);
    }
  }
  Heltec.display->display();           // display the lines
}

void showStatus(int seconds) {
  // shows the staus display periodically
  static long lastTimeChecked = millis();     // last time we checked
  const long checkFreq = seconds * 1000;              // how often we check and  update the display
  if (millis() - lastTimeChecked > checkFreq && StatusDisplayOn == true && !g_ControlsActive) {
    // update the status display
    statusDisplay();
    lastTimeChecked = millis();
  }
}

void DisplayTimeout(int timeout) {    // timeout is in minutes
  /*Times out the status display, turns it off after a period of inactivity.  This saves the OLEDS
  */
  const long displayTimeout = timeout * 60000;         // time we let display stay on, in milliseconds
  //static long timeDisplayStarted = millis();
  long displayCheck = millis();
  if (g_SonosInfo.playState != "Playing" && g_ControlsActive == false) {
    // only time out if we are not playing and controls are not active
    if (displayCheck - g_TimeDisplayStarted >= displayTimeout) {
      Heltec.display->clear();        // clear buffer
      Heltec.display->display();      // display empty buffer , ie turn display off
      StatusDisplayOn = false;
    }
  }
  else {
    g_TimeDisplayStarted = millis();
  }
}

void statusDisplay() {
  /*
      Status of sonos s system, time, weather that is displayed when display is not showing other stuff,
      ie, not when the volume is changing or when we are showing a menu structure.
      There are several different displays.
      Check for g_UIStarted (millis) and only show the update if it's been 5 seconds or so from when the last UI action occured.
        This is to prevent the display updating when the volume is being changed or menu being selecteed.
  */

  if (StatusDisplayOn = true) {
    // Only update the status display if nothing else is using the display.
    // Must ensure that other functions using the display turn it back on when they are finished.
    // gets current volume, playstate, track info for updating the display

    String displayLines[3];
    displayLines[2].reserve(256);
    // get battery percent and current time
    int battPercent = batteryPercent();
    String hourMinutes = CurrentDT(CURR_TIME);

    // make first two lines of the display
    displayLines[0] = g_ActiveUnitName + " " + g_SonosInfo.playState + " Vol: " + g_SonosInfo.volume;
    displayLines[1] = String(g_Weather.currTemp) + "c  " + g_Weather.currShortDesc + " "
                      + hourMinutes + " b:" + battPercent + "%";
    if (g_TrackInfoAvailable == true) {
      // we have track information, so display it
      displayLines[2] = g_SonosInfo.creator;
      displayLines[2] += " : ";
      displayLines[2] += g_SonosInfo.title;
      displayLines[2] += " : ";
      displayLines[2] += g_SonosInfo.album;
    }
    else if (!g_TrackInfoAvailable) {
      // no track information
      displayLines[2] = "No track information is available for this source";
    }
    Serial.println(displayLines[0]);
    Serial.println(displayLines[1]);
    Serial.println(displayLines[2]);
    Serial.print("Track URI: "); Serial.println(g_SonosInfo.URI);
    Serial.print("Track Source: "); Serial.println(g_SonosInfo.source);
    Serial.println(" -------------------------------------------------------");
    Serial.println();
    displayText(displayLines);
  }
}

int batteryPercent() {
  // reads the battery voltage, returns an int 0 - 100 percent
  // battery voltage constants
  static long fullBattery = 3900;       // fully charged, divide by 1000 for mv
  const long EMPTY_BATTERY = 3100;      // fully discharged
  static long totalBattery = 0;
  static long avgBattery;
  const float BATT_ADJ = 2.25;        // to convert battery reading to a 3000 - 4200 range
  const int LOW_BATT = 20;
  const int avg = 5;                // number of readings to average
  static int readingNo = 1;
  static int battPercent = 50;

  Serial.println("Checking the Battery Voltage");
  adcStart(BATT_PIN);
  //while (adcBusy(BATT_PIN));
  long voltReading  =  analogRead(BATT_PIN) * BATT_ADJ ;
  Serial.printf("Battery power in GPIO 37: "); Serial.println(voltReading);
  adcEnd(BATT_PIN);
  if (readingNo <= avg) {
    totalBattery = totalBattery + voltReading;
    Serial.print("Total Battery: ");Serial.println(totalBattery);
    readingNo ++;
    Serial.print("Reading Number: ");Serial.println(readingNo);
  }
  else if ( readingNo > avg) {
    // calculate average battery level
    Serial.println("Calculating Battery Percent");
    avgBattery = totalBattery / avg;
    // check max battery level and reset if necessary
    if (fullBattery < avgBattery) {
      fullBattery = avgBattery ;
    }
    float battLevel = avgBattery - EMPTY_BATTERY;
    float battRange = fullBattery - EMPTY_BATTERY;
//    Serial.print("Battery Level is: ");Serial.println(battLevel);
//    Serial.print("Battery Range is: "); Serial.println(battRange);
    float battCalc = (battLevel / battRange);
//    Serial.print("Battery Calc is: "); Serial.println(battCalc);
    battPercent = battCalc * 100;
    totalBattery = 0;
    readingNo = 1 ;
  }
  if (battPercent < LOW_BATT ) {
    g_LowBattery = true;
  }
  else if (battPercent >= LOW_BATT) {
    g_LowBattery = false;
  }
//  Serial.print("Full Battery is: "); Serial.println(fullBattery);
//  Serial.print("Average Battery is: ");Serial.println(avgBattery);
//  Serial.print("Battery Percent: "); Serial.println(battPercent);
  
  return battPercent;
}

void pDInt(String label, int value) {
  // convienience method for printing debug messages
  Serial.print(label);
  Serial.println(value);
  Serial.println();
}

void pwrLED() {
  //blinks the power LED once per second, indicates that the unit is on.
  //LED is on when display is timed out, execept when power is low.
  // we only do this when the display is off.
  long blinkOn = 2000;
  long blinkOff = 500;
  static unsigned long blinkTimer = millis();
  static boolean ledOn = true;
  if (g_LowBattery == true) {
    // if battery is low turn on and off
    if (ledOn == true) {
      if ( millis() - blinkTimer > blinkOn) {
        ledOn = false;  // turn off
        ledcWrite(0, 0);
        blinkTimer = millis();
      }
    }
    else if (!ledOn) {
      if ( millis() - blinkTimer > blinkOff) {
        // turn on if display is timed out
        if (!StatusDisplayOn) {
          ledcWrite(0, 50);
          ledOn = true;
          blinkTimer = millis();
        }
      }
    }
  }
  else if (g_LowBattery == false) {
    if (StatusDisplayOn == true) {
      // turn off is status display is on
      ledOn = false;  // turn off
      ledcWrite(0, 0);
    }
    else if (StatusDisplayOn == false) {
      // turn on
      ledcWrite(0, 50);
    }
  }
}
