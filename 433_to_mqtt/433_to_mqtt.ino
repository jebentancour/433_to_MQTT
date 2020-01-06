
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

// Update these with values suitable for your network.
byte mac[] = {  0xDE, 0xED, 0xBF, 0xFE, 0xFE, 0x7D };
IPAddress server(192, 168, 0, 3);

EthernetClient ethClient;
PubSubClient client(ethClient);

char payload_buff[64];
char topic_buff[32];

void output(unsigned long decimal, unsigned int length, unsigned int protocol) {
  Serial.print("Code: ");
  Serial.print(decimal);
  Serial.print(", Length: ");
  Serial.print(length);
  Serial.print(", Protocol: ");
  Serial.println(protocol);

  String payloadString = "{ \"code\":\"" + String(decimal) + "\", \"length\":\"" + String(length) + "\", \"protocol\":\"" + String(protocol) + "\" }";
  payloadString.toCharArray(payload_buff, payloadString.length()+1);

  client.publish("rcswitch/json",payload_buff);

  String topicString = "rcswitch/" + String(decimal);
  topicString.toCharArray(topic_buff, topicString.length()+1);

  client.publish(topic_buff,"ON");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClientRC", "rcswitch/status", 0, true, "offline")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("rcswitch/status","online", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      Ethernet.begin(mac);
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  client.setServer(server, 1883);

  Ethernet.begin(mac);
  // Allow the hardware to sort itself out
  delay(1500);
  
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (mySwitch.available()) {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedProtocol());
    mySwitch.resetAvailable();
  }
}
