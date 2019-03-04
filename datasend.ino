bool datasend() {
const char* host = "customscrypts.com";
const char* fingerprint = "A0 91 51 07 7D D9 6C 4D 26 A0 45 D0 BA 10 3C 92 85 8A 5F E4";
const int httpsPort = 443;

  Serial.print("At Data send");
  if ((WiFiMulti.run() == WL_CONNECTED)) {
  
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

  client.print(String("POST ") + url2 + " HTTP/1.1\r\n" +
          "Host: " + host + "\r\n" +
          "Connection: close\r\n" +
          "Accept: */*\r\n" +
          "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n" +
          "Content-Type: application/json;charset=utf-8\r\n" +
          "Content-Length: "+JSONstringBuffer.length()+"\r\n" +
          "\r\n" + JSONstringBuffer + "\r\n");
  Serial.println("request sent");
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
  {Serial.println("Status: success"); return 1;}
  else return 0;
  }
  else{ Serial.println("WiFi or Internet issue.. Turn on Debugging"); return 0;}
}
