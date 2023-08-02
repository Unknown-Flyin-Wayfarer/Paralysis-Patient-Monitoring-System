

/***
 * 
 * 
  Basic demo for accelerometer readings from Adafruit MPU6050

  ESP32 Guide: https://RandomNerdTutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
  ESP8266 Guide: https://RandomNerdTutorials.com/esp8266-nodemcu-mpu-6050-accelerometer-gyroscope-arduino/
  Arduino Guide: https://RandomNerdTutorials.com/arduino-mpu-6050-accelerometer-gyroscope/
  
  For ubuntu
  Install mosquitto broker   http://www.steves-internet-guide.com/install-mosquitto-linux/
  Install MQTT explorer 
  Subscriber to topic gyro/value to receive gyro values with a delay of 5 seconds and similarly to accl/value

  For Windows
  Install emqtx broker
  Install mqtt explorer  
***/


#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Replace the next variables with your SSID/Password combination
const char *ssid = "*********";
const char *password = "**********";

Adafruit_MPU6050 mpu;

// Add your MQTT Broker IP address, example:
// const char* mqtt_server = "192.168.1.144";
const char *mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi();
void callback(char *topic, byte *message, unsigned int length);
void reconnect() ;

void setup(void)
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange())
  {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange())
  {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth())
  {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);
}

void loop()
{
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

    if (g.gyro.z > 0) { // Adjust this threshold as needed
    Serial.println("Turn left");
  } else  { // Adjust this threshold as needed
    Serial.println("Turn right");
  } 

  Serial.println("");
  delay(500);

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  if (client.connected()){
    char data[64];
    sprintf(data,"{\"x\": %f,\"y\":%f,\"z\":%f}",g.gyro.x,g.gyro.y,g.gyro.z);
    client.publish("gyro/value",data);
    sprintf(data,"{\"x\": %f,\"y\":%f,\"z\":%f}",a.acceleration.x,a.acceleration.y,a.acceleration.z);
    client.publish("accl/value",data);
   
     // Check if a turn is detected
    if (g.gyro.z > 0) { // Adjust this threshold as needed
    // Serial.println("Turn left");
    client.publish("Motion/Req","I need food");
    } else  { // Adjust this threshold as needed
    // Serial.println("Turn right");
    client.publish("Motion/Req","I need water");
    } 
    delay(1000);
  }

  long now = millis();
  if (now - lastMsg > 5000)
  {
    lastMsg = now;
  }
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "on")
    {
      Serial.println("on");
    }
    else if (messageTemp == "off")
    {
      Serial.println("off");
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      // client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}