// Note: Some mail servers don't require you to login
#define SMTP_SERVER ""
#define SMTP_USER_BASE64 ""
#define SMTP_PASS_BASE64 ""

// Add "To" and "From" email addresses.
#define SMTP_FROM_EMAIL ""
#define SMTP_TO_EMAIL ""
#define SMTP_SUBJECT "Front Door Bell [eom]"
#define SMTP_BODY " " // No email body required as indicated in subject

#define DOORBELL D0
 
#include "application.h"
#include <string.h>
 
TCPClient client;
int sentOnce = 0;
 
void setup() {
  pinMode(DOORBELL, INPUT);
  //Spark.variable("emailSent", &sentOnce, INT);
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(DOORBELL) == HIGH) {
    if (sentOnce == 0) {
      RGB.control(true);
      RGB.color(250, 0,0);
 
      sentOnce = SendEmail(SMTP_SERVER, SMTP_USER_BASE64, SMTP_PASS_BASE64, SMTP_FROM_EMAIL, SMTP_TO_EMAIL, SMTP_SUBJECT, SMTP_BODY);
      Serial.write("Email Sent!!!!!\n");
      if (sentOnce) {
        RGB.color(0, 250, 0);
        delay(5000);
        sentOnce = 0;
      }
      RGB.control(false);
    }
  }
}
 
void idle() {
  SPARK_WLAN_Loop();
}

/**
* Thank you @jboswell !
*/
int SendEmail(char *smtpServer, char *username, char *password, char *fromEmail, char *toEmail, char *subject, char *body) {
 
  if (!client.connect(smtpServer, 25)) {
    return 0;
  }
 
 
  Serial.write("Connected!\n");
  if (!handshake(smtpServer)) {
    Serial.write("handshake failed...");
    client.stop();
    return 0;
  }

  if (!authenticate(username, password)) {
    Serial.write("auth failed...");
    client.stop();
    return 0;
  }

  Serial.write("sending mail...\n");
  echoSocketWrite(client, String("MAIL FROM:<") + String(fromEmail) + String(">") + String("\r\n"));
  echoSocketWrite(client, String("RCPT TO:<") + String(toEmail) + String(">") + String("\r\n"));
  flushToSerial(client); //client.flush();
 
  echoSocketWrite(client, "DATA\r\n");
  echoSocketWrite(client, String("from: ") + String(fromEmail) + String("\r\n"));
  echoSocketWrite(client, String("to: ") + String(toEmail) + String("\r\n"));
  echoSocketWrite(client, String("subject: ") + String(subject) + String("\r\n"));
  flushToSerial(client); //client.flush();
 
  echoSocketWrite(client, "\r\n");
  echoSocketWrite(client, body);
  echoSocketWrite(client, "\r\n");
  echoSocketWrite(client, ".");
  echoSocketWrite(client, "\r\n");
  echoSocketWrite(client, "\r\n");
  flushToSerial(client); //client.flush();
 
  if (blockForResponse("250", 2500) == 1) {
    Serial.write("Email sent.\n");
  }
 
  echoSocketWrite(client, "quit\r\n");
  echoSocketWrite(client, "\r\n");

  if (blockForResponse("220", 2500) == 1) {
    Serial.write("Said goodbye.\n");
  }
  
  flushToSerial(client); //client.flush();
  client.stop();
  delay(100);
}
 
 
int handshake(char *smtpServer) {

  //wait for the server to say something.
  flushToSerial(client); //client.flush();
  Serial.write("waiting for the server to say hello...\n");
  if (blockForResponse("220", 5000) == 0) {
    //handshake failed.
    return 0;
  }
  idle();

  Serial.write("Saying hello back.\n");
  echoSocketWrite(client, String("EHLO ") + String(smtpServer) + String("\r\n"));
  if (blockForResponse("250", 1250) == 0) {
    //handshake failed.
    return 0;
  }
  
  //idle();
  return 1;
}
 
int authenticate(char *username, char *password) {
  if (strlen(password)) {
    Serial.write("logging in...\n");

    echoSocketWrite(client, "AUTH LOGIN\r\n");
    blockForResponse("334", 5000);
 
    echoSocketWrite(client, String(username) + String("\r\n"));
    blockForResponse("334", 5000);
 
    echoSocketWrite(client, String(password) + String("\r\n"));
    blockForResponse("235", 5000);
    idle();
  }
  return 1;
}
 
void echoSocketWrite(TCPClient src, const char *line) {
  Serial.write(line);
  src.print(line);
}

void echoSocketWrite(TCPClient src, String line) {
  flushToSerial(src);
  delay(10);
 
  Serial.write(line.c_str());
  src.print(line.c_str());
}
 
void flushToSerial(TCPClient src) {
  while (src.available()) {
    Serial.write(src.read());
  }
}
 
/* really basic / error prone way of listening for and parsing results */
int blockForResponse(char *response, unsigned int maxWait) {
  unsigned int startTime = millis();
  int idx = 0;
  char buffer[16];

  while ((millis() - startTime) < maxWait) {
    while (client.available()) {
      buffer[idx] = client.read();
      Serial.write(buffer[idx]);
      idx++;
      if (idx > 16) {
        idx = 0;
      }
      if (strstr(buffer, response) != NULL) {
        Serial.write("heard response.\n");
        return 1;
      }
    }
    delay(100);
  }
  Serial.write("timed out.\n");
  return 0;
}
