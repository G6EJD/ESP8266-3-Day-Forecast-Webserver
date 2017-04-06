//########################   Weather Display  #############################
/*
 * Receives and displays the weather forecast from the Weather Underground and then displays using a 
 * JSON decoder wx data to display on a web page using a webserver.
 * Weather data received via WiFi connection to Weather Underground Servers and using their 'Forecast' API and data
 * is decoded using Copyright Benoit Blanchon's (c) 2014-2017 excellent JSON library.
 * This MIT License (MIT) is copyright (c) 2017 by David Bird and permission is hereby granted, free of charge, to
 * any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, but not to sub-license and/or 
 * to sell copies of the Software or to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 *   See more at http://dsbird.org.uk
*/
//################# LIBRARIES ################
String version = "3.0";       // Version of this program

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <WiFiManager.h>     // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include <DNSServer.h>
//################ VARIABLES ################

//------ NETWORK VARIABLES---------
// Use your own API key by signing up for a free developer account at http://www.wunderground.com/weather/api/
int    port          = 5000;
String API_key       = "2--------------7";            // See: http://www.wunderground.com/weather/api/d/docs (change here with your KEY)
String City          = "Lyneham";                     // Your home city
String Country       = "UK";                          // Your country   
String Conditions    = "conditions";                  // See: http://www.wunderground.com/weather/api/d/docs?d=data/index&MR=1
char   wxserver[]    = "api.wunderground.com";        // Address for WeatherUnderGround
unsigned long        lastConnectionTime = 0;          // Last time you connected to the server, in milliseconds
const unsigned long  postingInterval = 30L*60L*1000L; // Delay between updates, in milliseconds, WU allows 500 requests per-day maximum, set to every 30-mins or 48/day
String SITE_WIDTH =  "900";
String icon_set   =  "k";
String Units      =  "X"; // Default use either M for Metric, X for Mixed and I for Imperial

//################ PROGRAM VARIABLES and OBJECTS ################
// Conditions
String webpage, city, country, date_time, observationtime,
       DWDay0, DMon0, DDateDa0, DDateMo0, DDateYr0, Dicon_url0, DHtemp0, DLtemp0, DHumi0, Dpop0, DRain0, DW_mph0, DW_dir0, DW_dir_deg0, 
       DWDay1, DMon1, DDateDa1, DDateMo1, DDateYr1, Dicon_url1, DHtemp1, DLtemp1, DHumi1, DPop1, DRain1, DW_mph1, DW_dir1, DW_dir_deg1, 
       DWDay2, DMon2, DDateDa2, DDateMo2, DDateYr2, Dicon_url2, DHtemp2, DLtemp2, DHumi2, DPop2, DRain2, DW_mph2, DW_dir2, DW_dir_deg2, 
       DWDay3, DMon3, DDateDa3, DDateMo3, DDateYr3, Dicon_url3, DHtemp3, DLtemp3, DHumi3, DPop3, DRain3, DW_mph3, DW_dir3, DW_dir_deg3,
       DWDay4, DMon4, DDateDa4, DDateMo4, DDateYr4, Dicon_url4, DHtemp4, DLtemp4, DHumi4, DPop4, DRain4, DW_mph4, DW_dir4, DW_dir_deg4;

// Astronomy
String  DphaseofMoon, Sunrise, Sunset, moon_url;
       
ESP8266WebServer server(port); // Start server on port 80 (default for a web-browser, change to your requirements, e.g. 8080 if your Router uses port 80
                               // To access server from the outside of a WiFi network e.g. ESP8266WebServer server(8266) add a rule on your Router that forwards a
                               // connection request to http://your_network_ip_address:8266 to port 8266 and view your ESP server from anywhere.
                               // Example http://yourhome.ip:8266 will be directed to http://192.168.0.40:8266 or whatever IP address your router gives to this server

WiFiManager wifiManager;
  
void setup() {
  Serial.begin(115200); // initialize serial communications
  WiFiManager wifiManager;
  // New OOB ESP8266 has no Wi-Fi credentials so will connect and not need the next command to be uncommented and compiled in, a used one with incorrect credentials will
  // so restart the ESP8266 and connect your PC to the wireless access point called 'ESP8266_AP' or whatever you call it below in ""
  // wifiManager.resetSettings(); // Command to be included if needed, then connect to http://192.168.4.1/ and follow instructions to make the WiFi connection
  // Set a timeout until configuration is turned off, useful to retry or go to sleep in n-seconds
  wifiManager.setTimeout(180);
  //fetches ssid and password and tries to connect, if connections succeeds it starts an access point with the name called "ESP8266_AP" and waits in a blocking loop for configuration
  if(!wifiManager.autoConnect("ESP8266_AP")) {
    Serial.println(F("failed to connect and timeout occurred"));
    delay(6000);
    ESP.reset(); //reset and try again
    delay(180000);
  }
  // At this stage the WiFi manager will have successfully connected to a network, or if not will try again in 180-seconds
  Serial.println(F("WiFi connected..."));
  //----------------------------------------------------------------------
  server.begin(); Serial.println(F("Webserver started...")); // Start the webserver
  Serial.print(F("Use this URL to connect: http://")); Serial.println(WiFi.localIP().toString()+"/");// Print the IP address
  server.on("/", SystemSetup);
  server.on("/homepage", homepage);
  lastConnectionTime = millis();
  delay(10);
  obtain_forecast("forecast");
}

void loop() {
  server.handleClient();
  if (millis() - lastConnectionTime > postingInterval) {
    obtain_forecast("forecast");
    lastConnectionTime = millis();
  }
}

void homepage(){
  append_page_header(); 
  webpage += F("<hr>");
  webpage += F("<table class='style7'>");
  webpage += F("      <tr>");
  webpage += "          <th>"+DWDay1+"</th>";
  webpage += "          <th>"+DWDay2+"</th>";
  webpage += "          <th>"+DWDay3+"</th>";
  webpage += "          <th>"+DWDay4+"</th>";
  webpage += F("      </tr>");
  webpage += F("      <tr>");
  webpage += F("        <td>");
  webpage += "            "+DDateDa1+"-"+DMon1+"-"+DDateYr1;
  webpage += F("        </td>");
  webpage += F("        <td>");
  webpage += "            "+DDateDa2+"-"+DMon2+"-"+DDateYr2;
  webpage += F("        </td>");
  webpage += F("        <td>");
  webpage += "            "+DDateDa3+"-"+DMon2+"-"+DDateYr3;
  webpage += F("        </td>");
  webpage += F("        <td>");
  webpage += "            "+DDateDa4+"-"+DMon3+"-"+DDateYr4;
  webpage += F("        </td>");
  webpage += F("      </tr>");
  webpage += F("      <tr>");
  webpage += "          <td> <img class='imgdisplay' alt='wx' height='70' src='"+Dicon_url1+"' width='70'/></td>";
  webpage += "          <td> <img class='imgdisplay' alt='wx' height='70' src='"+Dicon_url2+"' width='70'/></td>";
  webpage += "          <td> <img class='imgdisplay' alt='wx' height='70' src='"+Dicon_url3+"' width='70'/></td>";
  webpage += "          <td> <img class='imgdisplay' alt='wx' height='70' src='"+Dicon_url4+"' width='70'/></td>";
  webpage += F("      </tr>");
  webpage += F("      <tr>");  
  webpage += F("        <td class='style8'>Max - Min - Hum</td>");
  webpage += F("        <td class='style8'>Max - Min - Hum</td>");
  webpage += F("        <td class='style8'>Max - Min - Hum</td>");
  webpage += F("        <td class='style8'>Max - Min - Hum</td>");
  webpage += F("      </tr>");
  webpage += F("      <tr>");
  webpage += F("        <td class='style9'>");
  webpage += "            &nbsp"+DHtemp1+"&deg;&nbsp;&nbsp;&nbsp;&nbsp"+DLtemp1+"&deg;&nbsp;&nbsp;&nbsp;&nbsp"+DHumi1+"%</td>";
  webpage += F("        </td>");
  webpage += F("        <td class='style9'>");
  webpage += "            &nbsp"+DHtemp2+"&deg;&nbsp;&nbsp;&nbsp;&nbsp"+DLtemp2+"&deg;&nbsp;&nbsp;&nbsp;&nbsp"+DHumi2+"%</td>";
  webpage += F("        </td>");
  webpage += F("        <td class='style9'>");
  webpage += "            &nbsp"+DHtemp3+"&deg;&nbsp;&nbsp;&nbsp;&nbsp"+DLtemp3+"&deg;&nbsp;&nbsp;&nbsp;&nbsp"+DHumi3+"%</td>";
  webpage += F("        </td>");
  webpage += F("        <td class='style9'>");
  webpage += "            &nbsp"+DHtemp4+"&deg;&nbsp;&nbsp;&nbsp;&nbsp"+DLtemp4+"&deg;&nbsp;&nbsp;&nbsp;&nbsp"+DHumi4+"%</td>";
  webpage += F("        </td>");
  webpage += F("      </tr>");
  webpage += F("      <tr>");
  webpage += F("        <td class='style9'>");
  webpage += "            "+winddirsymbol(&DW_dir1) + " @ " + DW_mph1 + "mph";
  webpage += F("        </td>");
  webpage += F("        <td class='style9'>");
  webpage += "            "+winddirsymbol(&DW_dir2) + " @ " + DW_mph2 + "mph";
  webpage += F("        </td>");
  webpage += F("        <td class='style9'>");
  webpage += "            "+winddirsymbol(&DW_dir3) + " @ " + DW_mph3 + "mph";
  webpage += F("        </td>");
  webpage += F("        <td class='style9'>");
  webpage += "            "+winddirsymbol(&DW_dir4) + " @ " + DW_mph4 + "mph";
  webpage += F("        </td>");
  webpage += F("      </tr>");
  webpage += F("      <tr>");
  webpage += F("        <td class='style8'>");
  webpage += "            "+DPop1+"% chance of "+(DRain1=="0"?"rain":DRain1+"mm rain");
  webpage += F("        </td>");
  webpage += F("        <td class='style8'>");
  webpage += "            "+DPop2+"% chance of "+(DRain2=="0"?"rain":DRain2+"mm rain");
  webpage += F("        </td>");
  webpage += F("        <td class='style8'>");
  webpage += "            "+DPop3+"% chance of "+(DRain3=="0"?"rain":DRain3+"mm rain");
  webpage += F("        </td>");
  webpage += F("        <td class='style8'>");
  webpage += "            "+DPop4+"% chance of "+(DRain4=="0"?"rain":DRain4+"mm rain");
  webpage += F("        </td>");
  webpage += F("      </tr>");
  webpage += F("    </table>");
  webpage += F("<br>");
  append_page_footer();
  server.send(200, "text/html", webpage);
}

void obtain_forecast (String forecast_type) {
  static char RxBuf[8704];
  String request;
  request  = "GET /api/" + API_key + "/"+ forecast_type + "/q/" + Country + "/" + City + ".json HTTP/1.1\r\n";
  request += F("User-Agent: Weather Webserver v");
  request += version;
  request += F("\r\n");
  request += F("Accept: */*\r\n");
  request += "Host: " + String(wxserver) + "\r\n";
  request += F("Connection: close\r\n");
  request += F("\r\n");
  Serial.println(request);
  Serial.print(F("Connecting to ")); Serial.println(wxserver);
  WiFiClient httpclient;
  if (!httpclient.connect(wxserver, 80)) {
    Serial.println(F("connection failed"));
    httpclient.flush();
    httpclient.stop();
    return;
  }
  Serial.print(request);
  httpclient.print(request); //send the http request to the server
  httpclient.flush();
  // Collect http response headers and content from Weather Underground, discarding HTTP headers, the content is JSON formatted and will be returned in RxBuf.
  int    respLen = 0;
  bool   skip_headers = true;
  String rx_line;
  while (httpclient.connected() || httpclient.available()) {
    if (skip_headers) {
      rx_line = httpclient.readStringUntil('\n'); //Serial.println(rx_line);
      if (rx_line.length() <= 1) { // a blank line denotes end of headers
        skip_headers = false;
      }
    }
    else {
      int bytesIn;
      bytesIn = httpclient.read((uint8_t *)&RxBuf[respLen], sizeof(RxBuf) - respLen);
      //Serial.print(F("bytesIn ")); Serial.println(bytesIn);
      if (bytesIn > 0) {
        respLen += bytesIn;
        if (respLen > sizeof(RxBuf)) respLen = sizeof(RxBuf);
      }
      else if (bytesIn < 0) {
        Serial.print(F("read error "));
        Serial.println(bytesIn);
      }
    }
    delay(1);
  }
  httpclient.stop();

  if (respLen >= sizeof(RxBuf)) {
    Serial.print(F("RxBuf overflow "));
    Serial.println(respLen);
    delay(1000);
    return;
  }
  RxBuf[respLen++] = '\0'; // Terminate the C string
  Serial.print(F("respLen ")); Serial.println(respLen); //Serial.println(RxBuf);
  if (forecast_type == "forecast"){
    if (showWeather_forecast(RxBuf))   delay(1000); 
  }
}

bool showWeather_forecast(char *json) {
  DynamicJsonBuffer jsonBuffer(8704);
  char *jsonstart = strchr(json, '{');
  //Serial.print(F("jsonstart ")); Serial.println(jsonstart);
  if (jsonstart == NULL) {
    Serial.println(F("JSON data missing"));
    return false;
  }
  json = jsonstart;

  // Parse JSON
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
    return false;
  }

  JsonObject& forecast = root["forecast"]["simpleforecast"];
  String Temp_mon   = forecast["forecastday"][0]["date"]["monthname"];
  String Mon1       = forecast["forecastday"][0]["date"]["monthname_short"]; DMon1       = Mon1;
  int DateYr1       = forecast["forecastday"][0]["date"]["year"];            DDateYr1    = String(DateYr1).substring(2);
  int DateDa1       = forecast["forecastday"][0]["date"]["day"];             DDateDa1    = DateDa1<10?"0"+String(DateDa1):String(DateDa1);
  observationtime   = "from " + String(DDateDa1) + " " + Temp_mon + ", " + DateYr1;
  String WDay1      = forecast["forecastday"][0]["date"]["weekday_short"];   DWDay1      = WDay1;
  int Htemp1        = forecast["forecastday"][0]["high"]["celsius"];         DHtemp1     = String(Htemp1);
  int Ltemp1        = forecast["forecastday"][0]["low"]["celsius"];          DLtemp1     = String(Ltemp1);
  String icon_url1  = forecast["forecastday"][0]["icon_url"];           
  Dicon_url1        = icon_url1.substring(0,icon_url1.indexOf("/i/c/")+5) + icon_set + icon_url1.substring(icon_url1.indexOf("/i/c/")+6);
  String pop1       = forecast["forecastday"][0]["pop"];                     DPop1       = String(pop1);
  int rain1         = forecast["forecastday"][0]["qpf_allday"]["mm"];        DRain1      = String(rain1);
  int w_mph1        = forecast["forecastday"][0]["avewind"]["mph"];          DW_mph1     = String(w_mph1);
  String w_dir1     = forecast["forecastday"][0]["avewind"]["dir"];          DW_dir1     = String(w_dir1);
  String w_dir_deg1 = forecast["forecastday"][0]["avewind"]["degrees"];      DW_dir_deg1 = String(w_dir_deg1);
  int humi1         = forecast["forecastday"][0]["avehumidity"];             DHumi1      = String(humi1);

  String WDay2      = forecast["forecastday"][1]["date"]["weekday_short"];   DWDay2      = WDay2;
  String Mon2       = forecast["forecastday"][1]["date"]["monthname_short"]; DMon2       = Mon2;
  int DateDa2       = forecast["forecastday"][1]["date"]["day"];             DDateDa2    = DateDa2<10?"0"+String(DateDa2):String(DateDa2);
  int DateYr2       = forecast["forecastday"][1]["date"]["year"];            DDateYr2    = String(DateYr2).substring(2);
  int Htemp2        = forecast["forecastday"][1]["high"]["celsius"];         DHtemp2     = String(Htemp2);
  int Ltemp2        = forecast["forecastday"][1]["low"]["celsius"];          DLtemp2     = String(Ltemp2);
  String icon_url2  = forecast["forecastday"][1]["icon_url"];           
  Dicon_url2        = icon_url2.substring(0,icon_url2.indexOf("/i/c/")+5) + icon_set + icon_url2.substring(icon_url2.indexOf("/i/c/")+6);
  String pop2       = forecast["forecastday"][1]["pop"];                     DPop2       = String(pop2);
  int rain2         = forecast["forecastday"][1]["qpf_allday"]["mm"];        DRain2      = String(rain2);
  int w_mph2        = forecast["forecastday"][1]["avewind"]["mph"];          DW_mph2     = String(w_mph2);
  String w_dir2     = forecast["forecastday"][1]["avewind"]["dir"];          DW_dir2     = String(w_dir2);
  String w_dir_deg2 = forecast["forecastday"][1]["avewind"]["degrees"];      DW_dir_deg2 = String(w_dir_deg2);
  int humi2         = forecast["forecastday"][1]["avehumidity"];             DHumi2      = String(humi2);

  String WDay3      = forecast["forecastday"][2]["date"]["weekday_short"];   DWDay3      = WDay3;
  String Mon3       = forecast["forecastday"][2]["date"]["monthname_short"]; DMon3       = Mon3;
  int DateDa3       = forecast["forecastday"][2]["date"]["day"];             DDateDa3    = DateDa3<10?"0"+String(DateDa3):String(DateDa3);
  int DateYr3       = forecast["forecastday"][2]["date"]["year"];            DDateYr3    = String(DateYr3).substring(2);
  int Htemp3        = forecast["forecastday"][2]["high"]["celsius"];         DHtemp3     = String(Htemp3);
  int Ltemp3        = forecast["forecastday"][2]["low"]["celsius"];          DLtemp3     = String(Ltemp3);
  String icon_url3  = forecast["forecastday"][2]["icon_url"];           
  Dicon_url3        = icon_url3.substring(0,icon_url3.indexOf("/i/c/")+5) + icon_set + icon_url3.substring(icon_url3.indexOf("/i/c/")+6);
  String pop3       = forecast["forecastday"][2]["pop"];                     DPop3       = String(pop3);
  int rain3         = forecast["forecastday"][2]["qpf_allday"]["mm"];        DRain3      = String(rain3);
  int w_mph3        = forecast["forecastday"][2]["avewind"]["mph"];          DW_mph3     = String(w_mph3);
  String w_dir3     = forecast["forecastday"][2]["avewind"]["dir"];          DW_dir3     = String(w_dir3);
  String w_dir_deg3 = forecast["forecastday"][2]["avewind"]["degrees"];      DW_dir_deg3 = String(w_dir_deg3);
  int humi3         = forecast["forecastday"][2]["avehumidity"];             DHumi3      = String(humi3);

  String WDay4      = forecast["forecastday"][3]["date"]["weekday_short"];   DWDay4      = WDay4;
  String Mon4       = forecast["forecastday"][3]["date"]["monthname_short"]; DMon4       = Mon4;
  int DateDa4       = forecast["forecastday"][3]["date"]["day"];             DDateDa4    = DateDa4<10?"0"+String(DateDa4):String(DateDa4);
  int DateYr4       = forecast["forecastday"][3]["date"]["year"];            DDateYr4    = String(DateYr4).substring(2);
  int Htemp4        = forecast["forecastday"][3]["high"]["celsius"];         DHtemp4     = String(Htemp4);
  int Ltemp4        = forecast["forecastday"][3]["low"]["celsius"];          DLtemp4     = String(Ltemp4);
  String icon_url4  = forecast["forecastday"][3]["icon_url"];           
  Dicon_url4        = icon_url4.substring(0,icon_url4.indexOf("/i/c/")+5) + icon_set + icon_url4.substring(icon_url4.indexOf("/i/c/")+6);
  String pop4       = forecast["forecastday"][3]["pop"];                     DPop4       = String(pop4);
  int rain4         = forecast["forecastday"][3]["qpf_allday"]["mm"];        DRain4      = String(rain4);
  int w_mph4        = forecast["forecastday"][3]["avewind"]["mph"];          DW_mph4     = String(w_mph4);
  String w_dir4     = forecast["forecastday"][3]["avewind"]["dir"];          DW_dir4     = String(w_dir4);
  String w_dir_deg4 = forecast["forecastday"][3]["avewind"]["degrees"];      DW_dir_deg4 = String(w_dir_deg4);
  int humi4         = forecast["forecastday"][3]["avehumidity"];             DHumi4      = String(humi4);

//  JsonObject& current = root["forecast"]["simpleforecast"];
//  int DateDa = current["forecastday"][0]["date"]["day"];      Serial.print(String(DateDa)+"/");
//  int DateMo = current["forecastday"][0]["date"]["month"];    Serial.print((DateMo<10?"0":"")+String(DateMo)+"/");
//  int DateYr = current["forecastday"][0]["date"]["year"];     Serial.print(String(DateYr)+" ");
//  int Htemp  = current["forecastday"][0]["high"]["celsius"];  Serial.print("High temp :"+String(Htemp));
//  int Ltemp  = current["forecastday"][0]["low"]["celsius"];   Serial.println(" Low temp :"+String(Ltemp));
  return true;
}

String winddirsymbol(String *wdir){
  if (*wdir == "N" || *wdir == "North") *wdir = "&darr; N "; // The API sometimes returns North rather than N
  if (*wdir == "NNE")                   *wdir = "&swarr; NNE ";
  if (*wdir == "NE")                    *wdir = "&swarr; NE ";
  if (*wdir == "ENE")                   *wdir = "&swarr; ENE ";

  if (*wdir == "E" || *wdir == "East")  *wdir = "&larr; E ";
  if (*wdir == "ESE")                   *wdir = "&swarr; ESE ";
  if (*wdir == "SE")                    *wdir = "&swarr; SE ";
  if (*wdir == "SSE")                   *wdir = "&swarr; SSE ";

  if (*wdir == "S" || *wdir == "South") *wdir = "&uarr; S ";
  if (*wdir == "SSW")                   *wdir = "&nearr; SSW ";
  if (*wdir == "SW")                    *wdir = "&nearr; SW ";
  if (*wdir == "WSW")                   *wdir = "&nearr; WSW ";

  if (*wdir == "W" || *wdir == "West")  *wdir = "&rarr; W ";
  if (*wdir == "WNW")                   *wdir = "&searr; WNW ";
  if (*wdir == "NW")                    *wdir = "&searr; NW ";
  if (*wdir == "NNW")                   *wdir = "&searr; NNW ";
  return *wdir;
}

void append_page_header() {
  webpage  = F("<!DOCTYPE html><html lang='en'><head>"); // Change lauguage (en) as required
  webpage += F("<meta http-equiv='refresh' content='60'/>"); // 60-sec refresh time
  webpage += F("<meta http-equiv='content-type' content='text/html; charset=UTF-8'/>");
  webpage += F("<title>Weather Forecast</title><style>");
  webpage += F("body {width:");
  webpage += SITE_WIDTH;
  webpage += F("px;margin:0 auto;font-family:verdana, sans-serif;font-size:14px;text-align:center;color:#cc66ff;background-color:#F7F2Fd;}");
  webpage += F("ul{list-style-type:none;margin:0;padding:0;overflow:hidden;background-color:#D8BFD8;font-size:12px;}");
  webpage += F("li{float:left;border-right:1px solid #bbb;}.last-child {border-right:none;}");
  webpage += F("li a{display: block;padding:2px 10px;text-decoration:none;}");
  webpage += F("li a:hover{background-color:#FFFFFF;}");
  webpage += F("section {font-size:14px;}");
  webpage += F("p {background-color:#E3D1E2;}");
  webpage += F("div.header,div.footer{padding:0.5em;font-size:10px;color:white;background-color:gray;clear:left;}");
  webpage += F("h1{background-color:#D8BFD8;font-size:16px;}");
  webpage += F("h2{color:#9370DB;font-size:14px;}");
  webpage += F("h3{color:#9370DB;font-size:12px;}");
  webpage += F("table{font-family:arial,sans-serif;font-size:14px;border-collapse:collapse;width:100%;height:100%}");
  webpage += F("td {border:0px solid black;text-align:center;padding:2px;}");
  webpage += F("th {border:0px solid black;text-align:center;padding:2px;font-size:20px;}");
  webpage += F(".style1 {text-align:center;font-size:54px;background-color:#D8BFD8;}");
  webpage += F(".style2 {text-align:center;font-size:14px;background-color:#ADD8E6;color:#0066ff;}");
  webpage += F(".style3 {text-align:center;font-size:60px;background-color:#FFE4B5;}");
  webpage += F(".style4 {text-align:center;font-size:20px;background-color:#FFE4B5;}");
  webpage += F(".style5 {text-align:center;font-size:18px;background-color:#D8BFD8;}");
  webpage += F(".style6 td {border:0px solid black;text-align:right;padding:0px;font-size:13px;background-color:#B0C4DE;color:#0066ff;}");
  webpage += F(".style7 {text-align:center;font-size:14px;background-color:#F7F2Fd;}");
  webpage += F(".style8 {text-align:center;font-size:12px;border:0px solid black;padding:2px;color:#990099;}");
  webpage += F(".style9 {text-align:center;font-size:14px;color:blue;}");
  webpage += F("img.imgdisplay {display:block;margin-left:auto;margin-right:auto;}");
  webpage += F("sup {vertical-align:super;font-size:smaller;}");
  webpage += F("</style></head><body><h1>Weather Server ");
  webpage += version+"</h1>";
  webpage += F("<h2>Weather Forecast for ");
  webpage += City;
  webpage += F(" ");
  webpage += observationtime;
  webpage += F("</h2>");
}

void append_page_footer(){ // Saves repeating many lines of code for HTML page footers
  webpage += F("<ul><li><a href='/homepage'>Home Page</a></li>");
  webpage += F("<li><a href='/'>System Setup</a></li></ul>");
  webpage += F("<div class='footer'>&copy; Weather Underground Data and Icons 2017 <img src = 'https://icons.wxug.com/logos/JPG/wundergroundLogo_4c_horz.jpg' alt = 'not found' width='75' height='15'/><br>");
  webpage += "&copy;"+String(char(byte(0x40>>1)))+String(char(byte(0x88>>1)))+String(char(byte(0x5c>>1)))+String(char(byte(0x98>>1)))+String(char(byte(0x5c>>1)));
  webpage += String(char((0x84>>1)))+String(char(byte(0xd2>>1)))+String(char(0xe4>>1))+String(char(0xc8>>1))+String(char(byte(0x40>>1)));
  webpage += String(char(byte(0x64/2)))+String(char(byte(0x60>>1)))+String(char(byte(0x62>>1)))+String(char(0x6e>>1))+"</div>";
  webpage += F("</body></html>");
}

void SystemSetup() {
  String previous_city     = City;
  String previous_country  = Country;
  String previous_icon_set = icon_set;
  String previous_units    = Units;
  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_page_header();
  String IPaddress = WiFi.localIP().toString()+":"+String(port)+"/";
  webpage += F("<h3>System Setup, if required enter values then Enter</h3>");
  webpage += "<form action=\"http://"+IPaddress+"\" method=\"POST\">";
  webpage += F("City<br><input type='text' name='wu_city' value='");
  webpage += City;
  webpage += F("'><br>");
  webpage += F("Country<br><input  type='text' name='wu_country' value='");
  webpage += Country;
  webpage += F("'><br>");
  webpage += F("<input type='checkbox' name='iconset' value='a'>Icon-set A ");
  webpage += F("<input type='checkbox' name='iconset' value='b'>Icon-set B ");
  webpage += F("<input type='checkbox' name='iconset' value='c'>Icon-set C ");
  webpage += F("<input type='checkbox' name='iconset' value='d'>Icon-set D ");
  webpage += F("<input type='checkbox' name='iconset' value='e'>Icon-set E ");
  webpage += F("<input type='checkbox' name='iconset' value='f'>Icon-set F ");
  webpage += F("<input type='checkbox' name='iconset' value='g'>Icon-set G ");
  webpage += F("<input type='checkbox' name='iconset' value='h'>Icon-set H ");
  webpage += F("<input type='checkbox' name='iconset' value='i'>Icon-set I ");
  webpage += F("<input type='checkbox' name='iconset' value='j'>Icon-set J ");
  webpage += F("<input type='checkbox' name='iconset' value='k'>Icon-set K <br><br>");
  webpage += F("<input type='checkbox' name='units'   value='M'>Metric Units ");
  webpage += F("<input type='checkbox' name='units'   value='X'>Mixed Units ");
  webpage += F("<input type='checkbox' name='units'   value='I'>Imperial Units<br><br>");
  webpage += F("<input type='submit' value='Enter'><br><br>");
  webpage += F("</form>");
  append_page_footer();
  server.send(200, "text/html", webpage); // Send a response to the client to enter their inputs, if needed, Enter=defaults
  if (server.args() > 0 ) { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      String Argument_Name   = server.argName(i);
      String client_response = server.arg(i);
      if (Argument_Name == "wu_city")    City    = client_response;
      if (Argument_Name == "wu_country") Country = client_response;
      if (Argument_Name == "iconset") {
        if (client_response.length() > 1) icon_set = "k"; else icon_set = client_response; // Checking for more than one check-box selected too
      }
      if (Argument_Name == "units") {
        if (client_response.length() > 1) Units = "X"; Units = client_response; // Checking for more than one check-box selected too
      }
    }
  }
  if (previous_city != City || previous_country != Country || previous_icon_set != icon_set || previous_units != Units) { // reduces calls to WU
    obtain_forecast("conditions");
    obtain_forecast("forecast");
    obtain_forecast("astronomy");
  }
}

// <img src='https://icons.wxug.com/logos/JPG/wundergroundLogo_4c.jpg' width='35' height='20'/>";
// https://icons.wxug.com/logos/JPG/wundergroundLogo_4c.jpg

/* Response from WeatherUnderground for 'Conditions' query
{
  "response": {
  "version":"0.1",
  "termsofService":"http://www.wunderground.com/weather/api/d/terms.html",
  "features": {
  "conditions": 1
  }
  }
  , "current_observation": {
    "image": {
    "url":"http://icons.wxug.com/graphics/wu2/logo_130x80.png",
    "title":"Weather Underground",
    "link":"http://www.wunderground.com"
    },
    "display_location": {
    "full":"Melksham, United Kingdom",
    "city":"Melksham",
    "state":"WIL",
    "state_name":"United Kingdom",
    "country":"UK",
    "country_iso3166":"GB",
    "zip":"00000",
    "magic":"58",
    "wmo":"03740",
    "latitude":"51.36999893",
    "longitude":"-2.14000010",
    "elevation":"64.0"
    },
    "observation_location": {
    "full":"Melksham, Melksham, ",
    "city":"Melksham, Melksham",
    "state":"",
    "country":"GB",
    "country_iso3166":"GB",
    "latitude":"51.364689",
    "longitude":"-2.129354",
    "elevation":"131 ft"
    },
    "estimated": {
    },
    "station_id":"IMELKSHA2",
    "observation_time":"Last Updated on January 7, 12:45 PM GMT",
    "observation_time_rfc822":"Sat, 07 Jan 2017 12:45:13 +0000",
    "observation_epoch":"1483793113",
    "local_time_rfc822":"Sat, 07 Jan 2017 12:48:35 +0000",
    "local_epoch":"1483793315",
    "local_tz_short":"GMT",
    "local_tz_long":"Europe/London",
    "local_tz_offset":"+0000",
    "weather":"Overcast",
    "temperature_string":"51.4 F (10.8 C)",
    "temp_f":51.4,
    "temp_c":10.8,
    "relative_humidity":"100%",
    "wind_string":"Calm",
    "wind_dir":"WSW",
    "wind_degrees":248,
    "wind_mph":0.0,
    "wind_gust_mph":"1.0",
    "wind_kph":0,
    "wind_gust_kph":"1.6",
    "pressure_mb":"1033",
    "pressure_in":"30.50",
    "pressure_trend":"+",
    "dewpoint_string":"51 F (11 C)",
    "dewpoint_f":51,
    "dewpoint_c":11,
    "heat_index_string":"NA",
    "heat_index_f":"NA",
    "heat_index_c":"NA",
    "windchill_string":"NA",
    "windchill_f":"NA",
    "windchill_c":"NA",
    "feelslike_string":"51.4 F (10.8 C)",
    "feelslike_f":"51.4",
    "feelslike_c":"10.8",
    "visibility_mi":"4",
    "visibility_km":"6",
    "solarradiation":"--",
    "UV":"-1",
    "precip_1hr_string":"0.00 in ( 0 mm)",
    "precip_1hr_in":"0.00",
    "precip_1hr_metric":" 0",
    "precip_today_string":"0.02 in (1 mm)",
    "precip_today_in":"0.02",
    "precip_today_metric":"1",
    "icon":"cloudy",
    "icon_url":"http://icons.wxug.com/i/c/k/cloudy.gif",
    "forecast_url":"http://www.wunderground.com/global/stations/03740.html",
    "history_url":"http://www.wunderground.com/weatherstation/WXDailyHistory.asp?ID=IMELKSHA2",
    "ob_url":"http://www.wunderground.com/cgi-bin/findweather/getForecast?query=51.364689,-2.129354",
    "nowcast":""
  }
}

{
  "response": {
  "version":"0.1",
  "termsofService":"http://www.wunderground.com/weather/api/d/terms.html",
  "features": {
  "astronomy": 1
  }
  }
    , "moon_phase": {
    "percentIlluminated":"94",
    "ageOfMoon":"17",
    "phaseofMoon":"Waning Gibbous",
    "hemisphere":"North",
    "current_time": {
    "hour":"14",
    "minute":"48"
    },
    "sunrise": {
    "hour":"8",
    "minute":"07"
    },
    "sunset": {
    "hour":"16",
    "minute":"28"
    },
    "moonrise": {
    "hour":"19",
    "minute":"15"
    },
    "moonset": {
    "hour":"9",
    "minute":"08"
    }
  },
  "sun_phase": {
    "sunrise": {
    "hour":"8",
    "minute":"07"
    },
    "sunset": {
    "hour":"16",
    "minute":"28"
    }
  }
}

{
  "response": {
  "version":"0.1",
  "termsofService":"http://www.wunderground.com/weather/api/d/terms.html",
  "features": {
  "forecast": 1
  }
  }
    ,
  "forecast":{
    "txt_forecast": {
    "date":"9:54 AM GMT",
    "forecastday": [
    {
    "period":0,
    "icon":"rain",
    "icon_url":"http://icons.wxug.com/i/c/k/rain.gif",
    "title":"Monday",
    "fcttext":"Cloudy skies with periods of rain this afternoon. High 46F. Winds SSE at 10 to 20 mph. Chance of rain 80%.",
    "fcttext_metric":"Becoming cloudy with occasional rain during the afternoon. High 8C. Winds SSE at 15 to 30 km/h. Chance of rain 80%.",
    "pop":"80"
    }
    ,
    {
    "period":1,
    "icon":"nt_rain",
    "icon_url":"http://icons.wxug.com/i/c/k/nt_rain.gif",
    "title":"Monday Night",
    "fcttext":"Rain ending this evening. Partial clearing overnight. Low 38F. SSE winds shifting to WSW at 10 to 20 mph. Chance of rain 80%.",
    "fcttext_metric":"Rain early. Decreasing clouds overnight. Low 4C. SSE winds shifting to WSW at 15 to 30 km/h. Chance of rain 80%.",
    "pop":"80"
    }
    ,
    {
    "period":2,
    "icon":"chancerain",
    "icon_url":"http://icons.wxug.com/i/c/k/chancerain.gif",
    "title":"Tuesday",
    "fcttext":"Sunny skies during the morning hours will give way to occasional showers in the afternoon. High 48F. Winds WSW at 5 to 10 mph. Chance of rain 40%.",
    "fcttext_metric":"Sunny skies during the morning hours will give way to occasional showers in the afternoon. High 9C. Winds WSW at 10 to 15 km/h. Chance of rain 40%.",
    "pop":"40"
    }
    ,
    {
    "period":3,
    "icon":"nt_chancerain",
    "icon_url":"http://icons.wxug.com/i/c/k/nt_chancerain.gif",
    "title":"Tuesday Night",
    "fcttext":"A shower or two possible early with partly cloudy skies later at night. Scattered frost possible. Low 32F. Winds NNW at 5 to 10 mph. Chance of rain 40%.",
    "fcttext_metric":"Rain showers early with clearing later at night. Scattered frost possible. Low around 0C. Winds NNW at 10 to 15 km/h. Chance of rain 40%.",
    "pop":"40"
    }
    ,
    {
    "period":4,
    "icon":"mostlycloudy",
    "icon_url":"http://icons.wxug.com/i/c/k/mostlycloudy.gif",
    "title":"Wednesday",
    "fcttext":"A mix of clouds and sun early, then becoming cloudy later in the day. High 43F. Winds NNE at 5 to 10 mph.",
    "fcttext_metric":"Partly to mostly cloudy. High 6C. Winds NNE at 10 to 15 km/h.",
    "pop":"20"
    }
    ,
    {
    "period":5,
    "icon":"nt_partlycloudy",
    "icon_url":"http://icons.wxug.com/i/c/k/nt_partlycloudy.gif",
    "title":"Wednesday Night",
    "fcttext":"Cloudy early, becoming mostly clear after midnight. Low 28F. Winds light and variable.",
    "fcttext_metric":"Cloudy skies early, followed by partial clearing. Low -2C. Winds light and variable.",
    "pop":"10"
    }
    ,
    {
    "period":6,
    "icon":"mostlycloudy",
    "icon_url":"http://icons.wxug.com/i/c/k/mostlycloudy.gif",
    "title":"Thursday",
    "fcttext":"A mix of clouds and sun in the morning followed by cloudy skies during the afternoon. High 39F. Winds E at 5 to 10 mph.",
    "fcttext_metric":"Partly cloudy skies in the morning will give way to cloudy skies during the afternoon. High 4C. Winds E at 10 to 15 km/h.",
    "pop":"10"
    }
    ,
    {
    "period":7,
    "icon":"nt_mostlycloudy",
    "icon_url":"http://icons.wxug.com/i/c/k/nt_mostlycloudy.gif",
    "title":"Thursday Night",
    "fcttext":"Mainly cloudy. Low 31F. Winds ENE at 5 to 10 mph.",
    "fcttext_metric":"Mostly cloudy. Low near 0C. Winds ENE at 10 to 15 km/h.",
    "pop":"10"
    }
    ]
    },
    "simpleforecast": {
    "forecastday": [
    {"date":{
  "epoch":"1486407600",
  "pretty":"7:00 PM GMT on February 06, 2017",
  "day":6,
  "month":2,
  "year":2017,
  "yday":36,
  "hour":19,
  "min":"00",
  "sec":0,
  "isdst":"0",
  "monthname":"February",
  "monthname_short":"Feb",
  "weekday_short":"Mon",
  "weekday":"Monday",
  "ampm":"PM",
  "tz_short":"GMT",
  "tz_long":"Europe/London"
},
    "period":1,
    "high": {
    "fahrenheit":"46",
    "celsius":"8"
    },
    "low": {
    "fahrenheit":"38",
    "celsius":"3"
    },
    "conditions":"Rain",
    "icon":"rain",
    "icon_url":"http://icons.wxug.com/i/c/k/rain.gif",
    "skyicon":"",
    "pop":80,
    "qpf_allday": {
    "in": 0.20,
    "mm": 5
    },
    "qpf_day": {
    "in": 0.10,
    "mm": 3
    },
    "qpf_night": {
    "in": 0.10,
    "mm": 3
    },
    "snow_allday": {
    "in": 0.0,
    "cm": 0.0
    },
    "snow_day": {
    "in": 0.0,
    "cm": 0.0
    },
    "snow_night": {
    "in": 0.0,
    "cm": 0.0
    },
    "maxwind": {
    "mph": 20,
    "kph": 32,
    "dir": "SSE",
    "degrees": 150
    },
    "avewind": {
    "mph": 15,
    "kph": 24,
    "dir": "SSE",
    "degrees": 150
    },
    "avehumidity": 83,
    "maxhumidity": 0,
    "minhumidity": 0
    }
    ,
    {"date":{
  "epoch":"1486494000",
  "pretty":"7:00 PM GMT on February 07, 2017",
  "day":7,
  "month":2,
  "year":2017,
  "yday":37,
  "hour":19,
  "min":"00",
  "sec":0,
  "isdst":"0",
  "monthname":"February",
  "monthname_short":"Feb",
  "weekday_short":"Tue",
  "weekday":"Tuesday",
  "ampm":"PM",
  "tz_short":"GMT",
  "tz_long":"Europe/London"
},
    "period":2,
    "high": {
    "fahrenheit":"48",
    "celsius":"9"
    },
    "low": {
    "fahrenheit":"32",
    "celsius":"0"
    },
    "conditions":"Chance of Rain",
    "icon":"chancerain",
    "icon_url":"http://icons.wxug.com/i/c/k/chancerain.gif",
    "skyicon":"",
    "pop":40,
    "qpf_allday": {
    "in": 0.04,
    "mm": 1
    },
    "qpf_day": {
    "in": 0.03,
    "mm": 1
    },
    "qpf_night": {
    "in": 0.01,
    "mm": 0
    },
    "snow_allday": {
    "in": 0.0,
    "cm": 0.0
    },
    "snow_day": {
    "in": 0.0,
    "cm": 0.0
    },
    "snow_night": {
    "in": 0.0,
    "cm": 0.0
    },
    "maxwind": {
    "mph": 10,
    "kph": 16,
    "dir": "WSW",
    "degrees": 252
    },
    "avewind": {
    "mph": 9,
    "kph": 14,
    "dir": "WSW",
    "degrees": 252
    },
    "avehumidity": 84,
    "maxhumidity": 0,
    "minhumidity": 0
    }
    ,
    {"date":{
  "epoch":"1486580400",
  "pretty":"7:00 PM GMT on February 08, 2017",
  "day":8,
  "month":2,
  "year":2017,
  "yday":38,
  "hour":19,
  "min":"00",
  "sec":0,
  "isdst":"0",
  "monthname":"February",
  "monthname_short":"Feb",
  "weekday_short":"Wed",
  "weekday":"Wednesday",
  "ampm":"PM",
  "tz_short":"GMT",
  "tz_long":"Europe/London"
},
    "period":3,
    "high": {
    "fahrenheit":"43",
    "celsius":"6"
    },
    "low": {
    "fahrenheit":"28",
    "celsius":"-2"
    },
    "conditions":"Mostly Cloudy",
    "icon":"mostlycloudy",
    "icon_url":"http://icons.wxug.com/i/c/k/mostlycloudy.gif",
    "skyicon":"",
    "pop":20,
    "qpf_allday": {
    "in": 0.00,
    "mm": 0
    },
    "qpf_day": {
    "in": 0.00,
    "mm": 0
    },
    "qpf_night": {
    "in": 0.00,
    "mm": 0
    },
    "snow_allday": {
    "in": 0.0,
    "cm": 0.0
    },
    "snow_day": {
    "in": 0.0,
    "cm": 0.0
    },
    "snow_night": {
    "in": 0.0,
    "cm": 0.0
    },
    "maxwind": {
    "mph": 10,
    "kph": 16,
    "dir": "NNE",
    "degrees": 30
    },
    "avewind": {
    "mph": 7,
    "kph": 11,
    "dir": "NNE",
    "degrees": 30
    },
    "avehumidity": 88,
    "maxhumidity": 0,
    "minhumidity": 0
    }
    ,
    {"date":{
  "epoch":"1486666800",
  "pretty":"7:00 PM GMT on February 09, 2017",
  "day":9,
  "month":2,
  "year":2017,
  "yday":39,
  "hour":19,
  "min":"00",
  "sec":0,
  "isdst":"0",
  "monthname":"February",
  "monthname_short":"Feb",
  "weekday_short":"Thu",
  "weekday":"Thursday",
  "ampm":"PM",
  "tz_short":"GMT",
  "tz_long":"Europe/London"
},
    "period":4,
    "high": {
    "fahrenheit":"39",
    "celsius":"4"
    },
    "low": {
    "fahrenheit":"31",
    "celsius":"-1"
    },
    "conditions":"Mostly Cloudy",
    "icon":"mostlycloudy",
    "icon_url":"http://icons.wxug.com/i/c/k/mostlycloudy.gif",
    "skyicon":"",
    "pop":10,
    "qpf_allday": {
    "in": 0.00,
    "mm": 0
    },
    "qpf_day": {
    "in": 0.00,
    "mm": 0
    },
    "qpf_night": {
    "in": 0.00,
    "mm": 0
    },
    "snow_allday": {
    "in": 0.0,
    "cm": 0.0
    },
    "snow_day": {
    "in": 0.0,
    "cm": 0.0
    },
    "snow_night": {
    "in": 0.0,
    "cm": 0.0
    },
    "maxwind": {
    "mph": 10,
    "kph": 16,
    "dir": "E",
    "degrees": 89
    },
    "avewind": {
    "mph": 9,
    "kph": 14,
    "dir": "E",
    "degrees": 89
    },
    "avehumidity": 79,
    "maxhumidity": 0,
    "minhumidity": 0
    }
    ]
    }
  }
}


*/
