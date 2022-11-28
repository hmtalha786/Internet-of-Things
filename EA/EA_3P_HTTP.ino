#include <WiFi.h>
#include <ESP32Ping.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ModbusMaster.h>

#define MAX485_DE 14    // Driver Enable
#define MAX485_RE 32    // Receiver Enable

ModbusMaster node;

// JSON Packet Sending time Counter ================================================

unsigned long Time_Checker = 0;
unsigned long timer = 30000;

const char* ssid = "Procheck";
const char* pass = "Procheck@123";

// Duration Handling ================================================================================================================================================================

unsigned long Duration = 1000;

// EA Data Registers  ===============================================================================================================================================================

#define VDR_L1 0x0280           // L1 Voltage Data Register
#define ADR_L1 0x0282           // L1 Current Data Register
#define TPDR_L1 0x0284          // L1 True Power Data Register
#define APDR_L1 0x02A0          // L1 Apparent Power Data Register

#define VDR_L2 0x0286           // L2 Voltage Data Register
#define ADR_L2 0x0288           // L2 Current Data Register
#define TPDR_L2 0x028A          // L2 True Power Data Register
#define APDR_L2 0x02A2          // L2 Apparent Power Data Register

#define VDR_L3 0x028C           // L3 Voltage Data Register
#define ADR_L3 0x028E           // L3 Current Data Register
#define TPDR_L3 0x0290          // L3 True Power Data Register
#define APDR_L3 0x02A4          // L3 Apparent Power Data Register

#define TTPDR 0x029E            // Total True Power Data Register
#define TAPDR 0x02A6            // Total Apparent Power Data Register

#define PVDR_L12 0x0292         // L1 - L2 Phase Voltage Data Register
#define PVDR_L23 0x0294         // L2 - L3 Phase Voltage Data Register
#define PVDR_L31 0x0296         // L3 - L1 Phase Voltage Data Register

#define TFDR 0x02B8             // Total Frequency Data Register

// L1 Phase Parameters ==============================================================================================================================================================

float V_L1;     // L1 Voltage
float A_L1;     // L1 Current
float TP_L1;    // L1 True Power
float AP_L1;    // L1 Apparent Power
float RP_L1;    // L1 Reactive Power
float PF_L1;    // L1 Power Factor

// L2 Phase Parameters ==============================================================================================================================================================

float V_L2;     // L2 Voltage
float A_L2;     // L2 Current
float TP_L2;    // L2 True Power
float AP_L2;    // L2 Apparent Power
float RP_L2;    // L2 Reactive Power
float PF_L2;    // L2 Power Factor

// L3 Phase Parameters ===============================================================================================================================================================

float V_L3;     // L3 Voltage
float A_L3;     // L3 Current
float TP_L3;    // L3 True Power
float AP_L3;    // L3 Apparent Power
float RP_L3;    // L3 Reactive Power
float PF_L3;    // L3 Power Factor

// 3 Phase Aggregated Parameters ===================================================================================================================================================================

float TTP;      // Total True Power
float TAP;      // Total Apparent Power
float TRP;      // Total Reactive Power
float TPF;      // Total Power Factor
float TF;       // Total Frequency

// Line to Line Voltages ===================================================================================================================================================================

float LV_12;    // L1-L2 Voltages
float LV_23;    // L2-L3 Voltages
float LV_31;    // L3-L1 Voltages

// =======================================================================================================================================================================================

void preTransmission(){  digitalWrite(MAX485_RE, 1);  digitalWrite(MAX485_DE, 1);  }

void postTransmission(){  digitalWrite(MAX485_RE, 0);  digitalWrite(MAX485_DE, 0); }

// =======================================================================================================================================================================================

float getData( uint16_t dataRegister, float correction_factor ) {
  float result;
  uint8_t dataRegisterNode = node.readHoldingRegisters(dataRegister, 2);
  if ( dataRegisterNode == node.ku8MBSuccess )                       // [readHoldingRegisters(Address of the register, Number of registers want to be read from this address)]
  {
    uint16_t data_LSB = node.getResponseBuffer(0x00);
    uint16_t data_MSB = node.getResponseBuffer(0x01);

    uint16_t Data[2] = {data_LSB, data_MSB};
    uint16_t joinedData;
    memcpy(&joinedData, Data, 4);
    result = joinedData * correction_factor;
  }
  return result;
}

// =======================================================================================================================================================================================

void setup() {

  pinMode(MAX485_RE, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);

  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);

  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 16, 17);  // ( Baud rate = 9600, Configuration = [8 Data bits,No parity,1 Stop bit], Rx pin, Tx pin)

  node.begin(1, Serial1);                   // Modbus slave ID 1

  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  /* ----- Connect to WiFi through saved credentials ----- */
  Connect_To_WiFi(ssid, pass);

}

// =======================================================================================================================================================================================

void loop() {

  // JSON Packet Sent after every 30 sec ...................................................
  if ( millis() >= timer )
  {
    timer = millis() + 30000UL;
    json_packet_sender();
  }

  if ( millis() >= Duration ) {

    Duration = millis() + 1000UL;

    Serial.println(" ________________________________________________________ Voltage ________________________________________________________ ");
    
    V_L1 = getData(VDR_L1, 0.1);   Serial.print("L1 Voltage : ");    Serial.println(V_L1);    delay(10);
    V_L2 = getData(VDR_L2, 0.1);   Serial.print("L2 Voltage : ");    Serial.println(V_L2);    delay(10);
    V_L3 = getData(VDR_L3, 0.1);   Serial.print("L3 Voltage : ");    Serial.println(V_L3);    delay(10);
  
    Serial.println(" ________________________________________________________ Current ________________________________________________________ ");
    
    A_L1 = getData(ADR_L1, 0.007);   Serial.print("L1 Current : ");    Serial.println(A_L1);    delay(10);
    A_L2 = getData(ADR_L2, 0.007);   Serial.print("L2 Current : ");    Serial.println(A_L2);    delay(10);
    A_L3 = getData(ADR_L3, 0.007);   Serial.print("L3 Current : ");    Serial.println(A_L3);    delay(10);

    Serial.println(" ________________________________________________________ True Power ________________________________________________________ ");
    
    TP_L1 = getData(TPDR_L1, 0.7);   Serial.print("L1 True Power : ");    Serial.println(TP_L1);    delay(10);
    TP_L2 = getData(TPDR_L2, 0.7);   Serial.print("L2 True Power : ");    Serial.println(TP_L2);    delay(10);
    TP_L3 = getData(TPDR_L3, 0.7);   Serial.print("L3 True Power : ");    Serial.println(TP_L3);    delay(10);

    Serial.println(" ________________________________________________________ Apparent Power ________________________________________________________ ");
    
    AP_L1 = getData(APDR_L1, 0.7);   Serial.print("L1 Apparent Power : ");    Serial.println(AP_L1);    delay(10);
    AP_L2 = getData(APDR_L2, 0.7);   Serial.print("L2 Apparent Power : ");    Serial.println(AP_L2);    delay(10);
    AP_L3 = getData(APDR_L3, 0.7);   Serial.print("L3 Apparent Power : ");    Serial.println(AP_L3);    delay(10);

    Serial.println(" ________________________________________________________ Reactive Power ________________________________________________________ ");

    RP_L1 = sqrt( sq(AP_L1) - sq(TP_L1) );   Serial.print("L1 Reactive Power : ");    Serial.println(RP_L1);    delay(10);
    RP_L2 = sqrt( sq(AP_L2) - sq(TP_L2) );   Serial.print("L2 Reactive Power : ");    Serial.println(RP_L2);    delay(10);
    RP_L3 = sqrt( sq(AP_L3) - sq(TP_L3) );   Serial.print("L3 Reactive Power : ");    Serial.println(RP_L3);    delay(10);

    Serial.println(" ________________________________________________________ Power Factor ________________________________________________________ ");
    
    if (AP_L1 > 0) { PF_L1 = TP_L1 / AP_L1; } else { PF_L1 = 0.0; }
    if (AP_L2 > 0) { PF_L2 = TP_L2 / AP_L2; } else { PF_L2 = 0.0; }
    if (AP_L3 > 0) { PF_L3 = TP_L3 / AP_L3; } else { PF_L1 = 0.0; }
    
    Serial.print("L1 Power Factor : ");   Serial.println(PF_L1);    delay(10);
    Serial.print("L2 Power Factor : ");   Serial.println(PF_L2);    delay(10);
    Serial.print("L3 Power Factor : ");   Serial.println(PF_L3);    delay(10);
   
    Serial.println(" ________________________________________________________ Total 3 Phase Aggregated Values ________________________________________________________ ");
  
    TF = getData(TFDR, 0.1);           Serial.print("Total Frequency ");                 Serial.println(TF);    delay(10);
    
    TTP = getData(TTPDR, 0.7);         Serial.print("Total 3 Phase True Power ");        Serial.println(TTP);   delay(10);
    
    TAP = getData(TAPDR, 0.7);         Serial.print("Total 3 Phase Apparent Power ");    Serial.println(TAP);   delay(100);
    
    TRP = sqrt( sq(TAP) - sq(TTP) );   Serial.print("Total 3 Phase Reactive Power ");    Serial.println(TRP);   delay(100);
  
    if (TAP > 0) { TPF = TTP / TAP; } else { TPF = 0.0; }

    Serial.print("Total 3 Phase Power Factor ");    Serial.println(TPF);    delay(10);
  
    Serial.println(" ________________________________________________________ Line to Line / Phase Voltages  ________________________________________________________ ");
    
    LV_12 = getData(PVDR_L12, 0.1);   Serial.print("L1-L2 Phase Voltage ");   Serial.println(LV_12);   delay(10);
  
    LV_23 = getData(PVDR_L23, 0.1);   Serial.print("L2-L3 Phase Voltage ");   Serial.println(LV_23);   delay(10);
  
    LV_31 = getData(PVDR_L31, 0.1);   Serial.print("L3-L1 Phase Voltage ");   Serial.println(LV_31);   delay(10);

  }
}

// ==================================================================================================================================================================================

void json_packet_sender() {

  StaticJsonBuffer<500> JSON_Packet;
  JsonObject& JSON_Entry = JSON_Packet.createObject();

  JSON_Entry["device"] = "PROCHECK-EA-1";
  
  JSON_Entry["OSF"] = TF;
  
  JSON_Entry["V1"] = V_L1;
  JSON_Entry["V2"] = V_L2;
  JSON_Entry["V3"] = V_L3;
  
  JSON_Entry["C1"] = A_L1;
  JSON_Entry["C2"] = A_L2;
  JSON_Entry["C3"] = A_L3;
  
  JSON_Entry["TP1"] = TP_L1;
  JSON_Entry["TP2"] = TP_L2;
  JSON_Entry["TP3"] = TP_L3;
  
  JSON_Entry["AP1"] = AP_L1;
  JSON_Entry["AP2"] = AP_L2;
  JSON_Entry["AP3"] = AP_L3;
  
  JSON_Entry["RP1"] = RP_L1;
  JSON_Entry["RP2"] = RP_L2;
  JSON_Entry["RP3"] = RP_L3;
  
  JSON_Entry["PF1"] = PF_L1;
  JSON_Entry["PF2"] = PF_L2;
  JSON_Entry["PF3"] = PF_L3;

  JSON_Entry["TTP"] = TTP;
  JSON_Entry["TAP"] = TAP;
  JSON_Entry["TRP"] = TRP;
  JSON_Entry["TPF"] = TPF;

  JSON_Entry["L12"] = LV_12;
  JSON_Entry["L23"] = LV_23;
  JSON_Entry["L31"] = LV_31;

  char JSONmessageBuffer[500];
  JSON_Entry.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.print(JSONmessageBuffer);

  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;
    http.begin("https://api.datacake.co/integrations/api/cfd71cb9-5d27-49e3-af94-75aad1b72742/");  //Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json");             // Specify content-type header
    int httpResponseCode = http.POST(JSONmessageBuffer);            // Send the actual POST request

    if (httpResponseCode > 0) {
      String response = http.getString();   // Get the response to the request
      Serial.println(httpResponseCode);     // Print return code
      Serial.println(response);             // Print request answer
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();  //Free resources
  } else {
    Serial.println("Error in WiFi connection");
  }
}

// ====================================================================================================================================================================================

void Connect_To_WiFi(const char * ssid, const char * pwd) {

  int ledState = 0;
  Time_Checker = millis();

  Serial.println();
  Serial.println("Connecting to WiFi network: " + String(ssid));
  Serial.println();

  WiFi.begin(ssid, pwd);

  while (WiFi.status() != WL_CONNECTED) {

    /* Blink LED while connecting to WiFi */
    digitalWrite(14, ledState);

    /* Flip LED State */
    ledState = (ledState + 1) % 2;
    delay(500);
    Serial.println("Connecting to " + String(ssid));

    /* Restart ESP32 if not connected for 5 minutes */
    if ( millis() - Time_Checker > 600000 ) {
      ESP.restart();
    }
  }

  digitalWrite(14, 0);
  Serial.println();

  /* Printing Network Credentials */
  Serial.println("WiFi connected!");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Primary DNS: ");
  Serial.println(WiFi.dnsIP(0));
  Serial.print("Secondary DNS: ");
  Serial.println(WiFi.dnsIP(1));

  /* Making Ping to Google to Check Internet Connection */
  bool Ping_Success = Ping.ping("www.google.com", 3);

  if (!Ping_Success) {
    Serial.println("Failed to Ping www.google.com");
    ESP.restart();
  } else {
    Serial.println();
    Serial.println("Ping successful to www.google.com");
    Serial.println();
  }
}
