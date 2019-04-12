bool datasend(String TESTFILEX) {
const char* host = "customscrypts.com";
const char* fingerprint = "A0 91 51 07 7D D9 6C 4D 26 A0 45 D0 BA 10 3C 92 85 8A 5F E4";
const int httpsPort = 443;
unsigned long before_send=0;
unsigned long after_send=0;
  Serial.print("At Data send");
 if ((WiFi.status() == WL_CONNECTED)) {
  
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect("customscrypts.com", httpsPort)) {
    Serial.println("connection failed");
    return 0;
  }

  if (client.verify(fingerprint, "customscrypts.com")) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  String url2 = "/instrux/crud/bulk.php";
  Serial.print("requesting URL: ");
  Serial.println(url2);
  if (SPIFFS.exists(TESTFILEX)) {
    f = SPIFFS.open(TESTFILEX, "r"); if (!f){
    Serial.print("No file Exist with name:"); Serial.println(TESTFILEX); return 0;
  } else {
        Serial.print("file loaded:");
        Serial.println(TESTFILEX);
      }
      }
      else{ Serial.print("File Does Not Exist: "); Serial.println(TESTFILEX);  return 0;}
     int sendfilesize=f.size()+134;
        before_send=millis();
  client.print(String("POST ") + url2 + " HTTP/1.1\r\n" +
          "Host: " + host + "\r\n" +
          "Connection: close\r\n" +
          "Accept: */*\r\n" +
          "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n" +
          "Content-Type: application/json;charset=utf-8\r\n" +
          "Content-Length: "+sendfilesize+ "\r\n");
        //  +         "\r\n" + JSONstringBuffer + "\r\n");
        client.println();
client.print("{\"return\":0,\"mac\":\"CC:50:E3:0A:0D:73\",\"password\":123456,\"localId\":1,\"params\":[\"timestamp\",\"temperature\",\"humidity\",\"ms\"],\"values\":[");

 client.write(f);
 client.print("]}");
 client.println();
 f.close();        
  Serial.println("request sent");
 Serial.print("JSONstringBuffer length= ");
 Serial.println(JSONstringBuffer.length());
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
    String myline=client.readStringUntil('\r');
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println(myline);
  Serial.println("==========");
  Serial.println("closing connection");
  if(myline.startsWith("{\"status\":\"Successfully Added Data\""))
  {Serial.println("Status: success"); 
  after_send=millis(); 
  Serial.println(before_send);
  Serial.println(after_send); 
    Serial.print("Time spent in sending= ");
  Serial.println(after_send-before_send); 
  Serial.println(" ms");
  return 1;}
  else return 0;
  }
  else{ Serial.println("WiFi or Internet issue.. Turn on Debugging"); return 0;}
}
