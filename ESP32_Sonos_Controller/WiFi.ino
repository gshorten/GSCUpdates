/*
   Functions to set and save the wifi ssid and password to Eprom (so it's not hard coded)
*/

/////////////////////////////////////////////////////////////////////////////

boolean initWiFi() {
  //connects to the wifi network
  boolean wifiConnected = false;
  // attempt to connect using saved values for ssid and pwd
  WiFi.begin(conf.values[0].c_str(), conf.values[1].c_str());
  int wifiAttempts = 0;
  int maxAttempts = 10;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < maxAttempts) {   //loop until wifi is connected OR max attempts
    wifiAttempts ++;
    delay(1000);
    Serial.print(".");
  }
  // if we are connected return true
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    g_State = OPERATING;
  }
  else if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    Serial.println("************ Going into Setup Mode ****************");
    g_State = SETUP;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(conf.getApName(), "", 1);
  }
  return wifiConnected;
}
