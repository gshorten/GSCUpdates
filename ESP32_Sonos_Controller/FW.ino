/*
   Firmware update functions
   Revisions
   1.2  First working OTA
   1.3  added messages on display when OTA updating
   2.0  major update, moved all files to git repository
   2.1  increased time between wifi connect attempts to 1 second, 10 max attempts
   2.2  minor tweaks
   2.3  tweaking startup displays, sequence
   2.4  fixed weather update splash
   2.5  save and retrieve firmware version for startup display
   2.6  tweaking saving fw version
   3.0  changed file names for binaries
   3.1  tweaked saving of firmware version
   3.2  tweaked saving of firmware
   3.3  tweaks
   3.4  changed splash
   3.5  put flag in to stop weather updating if button is clicked.
   3.6  changed flags used to indicate controls in use
   3.7  code cleanup
   3.8  changed battery settings
   3.9  tweaked battery settings
   3.10 code cleanup
   3.11 code cleanup
   3.12 fix display timeout
   3.13 code cleanup
   3.14 debug button click
   3.15 still debugging button click
   4.0  change to how track title, artist, creator is retrieved and displayed
   4.1  tweaked button presses
   4.2  display timeout stopped while unit is paused and encoder is turned
   4.3 fixed display timeout
   4.4 tweaked LED
   4.5 fixed tweaked LED
   4.6 added volume bar
   4.7 tweaked volume bar
   4.8 tweaked power LED
   4.9 tweaked power LED
   4.11 tweaked startup
   4.12 tweaked startup
   5.0 added reset function
   5.1 added ip check for sonos units, units that do not respond to a ping are not added
   5.2 shortened ip check to 2 pings, tweaked pause play for faster response
   5.3 numerous small bugfixes, optimizations. added battery calibration feature
   5.4 fixed battery reading
   5.5 fixed battery reading, again
   5.6 tweaked startup display to make more informative, fixed batt display, auto calibrate max 
   5.7 increased wifi search attempts and time between attempts
   5.8 fixed bug where hangs if last used unit cannot be found
   5.9 tweaked low battery setting
   5.10 various code optimizations
   5.11 fix version
*/

void firmwareUpdate(void) {
  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      {
        Serial.println("HTTP_UPDATE_NO_UPDATES");
      }
      break;

    case HTTP_UPDATE_OK:
      {
        Serial.println("HTTP_UPDATE_OK");
        String splash[3] = {"Firmware Updated", "", ""};
        displayText(splash);
      }
      break;
  }
}

int FirmwareVersionCheck(void) {
  String payload;
  int httpCode;
  String fwurl = "";
  fwurl += URL_fw_Version;
  fwurl += "?";
  fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClientSecure * client = new WiFiClientSecure;

  if (client)
  {
    client -> setCACert(rootCACertificate);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
    HTTPClient https;

    if (https.begin( * client, fwurl))
    { // HTTPS
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        Serial.print("error in downloading version file:");
        Serial.println(httpCode);
      }
      https.end();
    }
    delete client;
  }

  if (httpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    }
    else
    {
      Serial.println(payload);
      Serial.println("New firmware detected");
      String splash[3];
      splash[0] = "New Firmware found";
      splash[1] = "Please Wait";
      splash[2] = "Updating to latest Firmware, version: ";
      splash[2] +=  payload;
      displayText(splash);
      return 1;
    }
  }
  return 0;
}
