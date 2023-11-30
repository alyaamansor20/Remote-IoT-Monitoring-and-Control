#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

void LCD_Init();

// Sensor Handlers
void Smoke_Sensor_Handler();
void Temp_Sensor_Handler();
void Motion_Sensor_Handler();

// ESP
void ESP_Init();
void ESP_Handler();
String ESP_Send(String command, const int timeout, boolean debug);
String Web_Server();

// ESP8266
#define rxPin 12
#define txPin 13
SoftwareSerial esp8266(rxPin,txPin);
#define serialCommunicationSpeed 9600
#define DEBUG true

// Sensors
#define buzzerPin 2
#define smokePin A5
#define ledPin 4
#define motionPin A4
#define tempPin A0


// LCD
#define rs 6
#define enable 7
#define d4 8
#define d5 9
#define d6 10
#define d7 11
LiquidCrystal lcd(rs, enable, d4, d5, d6, d7);


#define TEMP_CRITICAL 50
#define SMOKE_CRITICAL 200

int smokeValue = 0;
float tempvalue = 0;
bool motionDetected = 0;

void Smoke_Sensor_Handler(){
  smokeValue = analogRead(smokePin);
  Serial.print("Smoke Detected: ");
  Serial.println(smokeValue);
  
  digitalWrite(buzzerPin, smokeValue > SMOKE_CRITICAL);
}

void Temp_Sensor_Handler(){
  tempvalue = (analogRead(tempPin) * 4.88); /* Convert adc value to equivalent voltage */
  tempvalue = (tempvalue/10); /* LM35 gives output of 10mv/°C */
  Serial.print("Temperature = ");
  Serial.print(tempvalue);
  Serial.print(" Degree Celsius\n");
  
  // Print Temp
  lcd.clear();
  lcd.print("Temp: ");
  lcd.print(tempvalue);
}

void Motion_Sensor_Handler(){
  Serial.print("Motion Detected: ");
  motionDetected = digitalRead(motionPin);
  Serial.println(motionDetected);
  
  // LEd on/off
  digitalWrite(ledPin, motionDetected);
}

void ESP_Init()
{
  // set the data rate for the SoftwareSerial port
  esp8266.begin(serialCommunicationSpeed);
  
  // Reset module
  ESP_Send("AT+RST\r\n", 2000, DEBUG);

  // Connect to WIFI
  ESP_Send("AT+CWJAP=\"Kirlos\",\"helloworld\"\r\n", 2000, DEBUG);
  delay (3000);
  
  // Set mode to connect to WIFI
  ESP_Send("AT+CWMODE=1\r\n", 1500, DEBUG);
  delay (1500);
  
  // Get IP Address
  ESP_Send("AT+CIFSR\r\n", 1500, DEBUG);
  delay (1500);
  
  // Set for multiple connections
  ESP_Send("AT+CIPMUX=1\r\n", 1500, DEBUG);
  delay (1500);
  
  // Set as a server on port 80
  ESP_Send("AT+CIPSERVER=1,80\r\n", 1500, DEBUG);
}

void LCD_Init(){
  lcd.begin(16,2);
  lcd.setCursor(0,0);
}

void setup()
{
  // Set data rate for arduino
  Serial.begin(serialCommunicationSpeed);

  ESP_Init();
  LCD_Init();

  // Sensors modes
  pinMode(smokePin, INPUT);
  pinMode(tempPin, INPUT);
  pinMode(motionPin, INPUT);
  
  // blalblalal modes
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  //pinMode(switchPin, INPUT);
}

void loop()
{
  Temp_Sensor_Handler();
  Smoke_Sensor_Handler();
  Motion_Sensor_Handler();
  ESP_Handler();
  
  delay(1000);
}


void ESP_Handler(){
  // returns the number of bytes available for reading (in buffer)
  if(esp8266.available())
  {
    if(esp8266.find("+IPD,"))
    {
      delay(1000);
      int connectionId = esp8266.read() - '0';
      String cipSend = "AT+CIPSEND=";
      cipSend += connectionId;
      cipSend += ",";
      String webpage = Web_Server();
      cipSend += webpage.length();
      cipSend += "\r\n";
      
      ESP_Send(cipSend,1000,DEBUG);
      ESP_Send(webpage,1000,DEBUG);
      
      String closeCommand = "AT+CIPCLOSE=";
      closeCommand+=connectionId; // append connection id
      closeCommand+="\r\n";
      ESP_Send(closeCommand,3000,DEBUG);
    }
  }
}

String Web_Server(){
  String webServer = "<!DOCTYPE HTML><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.7.2/css/all.css\" integrity=\"sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr\" crossorigin=\"anonymous\"><style>html{font-family: Arial;display: inline-block;margin: 0px auto;text-align: center;}h2 { font-size: 3.0rem; }p { font-size: 3.0rem; }.units { font-size: 1.2rem; }.dht-labels{font-size: 1.5rem;vertical-align:middle;}.button{height: 70px;padding: 10;background: #059e8a;border: black;font-family: Arial;font-size: 30px;font-weight: 500;}.button:hover{background: #008168;}</style></head><body><h2>ESP8266 DHT Server</h2><p><i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;\"></i><span class=\"dht-labels\">Temperature</span><span id=\"temperature\">%TEMP%</span><sup class=\"units\">&deg;C</sup></p><p><i class=\"fas fa-joint\" style=\"color:#black;\"></i><span class=\"dht-labels\">Smoke</span><span id=\"smoke\">%SMOKE%</span></p><p><i class=\"fab fa-jenkins\" style=\"color:#black;\"></i><span class=\"dht-labels\">Motion Detected</span><span id=\"yes_or_no\">%YN%</span></p><button type=\"button\" class=\"button\" value=\"LedOff\"><span class=\"button_text\">Led</span><span id=\"off_or_on\">%off_or_on%</span></button></body><script>";

  return webServer;
}

String ESP_Send(String data, const int timeout, boolean debug)
{
    String response = "";
    esp8266.print(data);
    long int time = millis();
    while( (time+timeout) > millis())
    {
    while(esp8266.available())
    {
      char c = esp8266.read();
      response+=c;
    }
    }
    if(debug)
    {
    Serial.print(response);
    }
    return response;
}
