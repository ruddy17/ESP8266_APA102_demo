#include <SPI.h>
#include <ESP8266WiFi.h>
#include <math.h>

const char* ssid = "your_ssid";
const char* password = "your_pass";

const int LED_COUNT = 300;

WiFiServer server(80);

void setup()
{
  SPI.begin();
  SPI.setFrequency(10000000);
  LED_Off();

  Serial.begin(115200);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

uint8_t r, g, b; // Red Green Blue
float h = 0.1, l=0, temp; // Hue, Luminosity
int value = LOW; // LEDs Enable

void loop()
{
  // Start rainbow if required
  if (value == HIGH)
  {
    LED_Rainbow();
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }

  // Wait until the client sends some data
  while (!client.available())
  {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  client.flush();

  // Match the request
  if (request.indexOf("/LED=ON") != -1)
  {
    value = HIGH;
  }
  if (request.indexOf("/LED=OFF") != -1)
  {
    value = LOW;
    LED_Off();
  }

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.println("<br><br>");
  client.println("<input type=button onclick=\"location.href='/LED=ON'\" value=ON style=\"width:50%; height:500px; font-size:80px; border-radius: 0; -webkit-appearance:none; -webkit-border-radius:0;\">");
  client.println("<input type=button onclick=\"location.href='/LED=OFF'\" value=OFF style=\"width:49%; height:500px; font-size:80px; border-radius: 0; -webkit-appearance:none; -webkit-border-radius:0;\">");
  client.println("</html>");
  delay(100);
}

void LED_Rainbow()
{
  APA_Start();

  h += 1.0; // change this to control rainbow speed
  if (h > 359.0) h = 0.1; // Hue loop

  if(l < 1.0) l+=0.001; // Smooth ramp up

  for (int i = 0; i < LED_COUNT; i++)
  {
    temp = h + i; // gradient
    while (temp > 360.0) temp -= 359.0;
    HSV_to_RGB(temp, 1.0, l, &r, &g, &b);
    APA_LED(31, r, g, b);
  }
  delay(5); // change this to control rainbow speed
  APA_Stop();
}

void LED_Off()
{
  APA_Start();
  for (int i = 0; i < 300; i++)
  {
    APA_LED(0, 0, 0, 0);
  }
  APA_Stop();

  l = 0.0;
}

void APA_Start()
{
  SPI.transfer(0);
  SPI.transfer(0);
  SPI.transfer(0);
  SPI.transfer(0);
}

void APA_Stop()
{
  for (int i = 0; i < (1 + LED_COUNT / 32) * 4; i++) SPI.transfer(0xFF); // one stop frame for every 32 leds
}

void APA_LED(uint8_t L, uint8_t R, uint8_t G, uint8_t B)
{
  SPI.transfer(0xE0 | L);
  SPI.transfer(B);
  SPI.transfer(G);
  SPI.transfer(R);
}

void HSV_to_RGB(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
  float f, p, q, t;
  int i;
  h = _min(360.0, h);
  h = _max(0.0, h);
  s = _min(1.0, s);
  s = _max(0.0, s);
  v = _min(1.0, v);
  v = _max(0.0, v);

  if (s == 0) {
    // Achromatic (grey)
    *r = *g = *b = round(v * 255);
    return;
  }

  h /= 60.0; // sector 0 to 5
  i = floor(h);
  f = h - i; // factorial part of h
  p = v * (1.0 - s);
  q = v * (1.0 - s * f);
  t = v * (1.0 - s * (1.0 - f));
  switch (i) {
    case 0:
      *r = round(255 * v);
      *g = round(255 * t);
      *b = round(255 * p);
      break;
    case 1:
      *r = round(255 * q);
      *g = round(255 * v);
      *b = round(255 * p);
      break;
    case 2:
      *r = round(255 * p);
      *g = round(255 * v);
      *b = round(255 * t);
      break;
    case 3:
      *r = round(255 * p);
      *g = round(255 * q);
      *b = round(255 * v);
      break;
    case 4:
      *r = round(255 * t);
      *g = round(255 * p);
      *b = round(255 * v);
      break;
    default: // case 5:
      *r = round(255 * v);
      *g = round(255 * p);
      *b = round(255 * q);
  }
}


