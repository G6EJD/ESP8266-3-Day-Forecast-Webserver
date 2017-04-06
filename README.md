# ESP8266-3-Day-Forecast-Webserver
An ESP8266 provides a webserver and single page 3-day weather forecast after querying the
Weather Underground server using their Forecast API, data is decoded using the JSON library, then HTML codes are generated
to be displayed on a webpage.

Connect to the server to its designated IP address (provided by your Router) and use port 5000, although the port can be changed.
Example http://192.68.0.51:5000/
Enter your city and country details, then click on Homepage

Alternatively change the port to 80 and connect like this:
http:/192.168.0.51/homepage
