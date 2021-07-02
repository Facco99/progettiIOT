//
//  Created by Flavio Ferrandi on 14/06/21.
//  Copyright © 2021 Flavio Ferrandi. All rights reserved.
//

#ifndef TrackleEsp32Ethernet_h
#define TrackleEsp32Ethernet_h

#include "trackle_base.h"

#ifndef RESET_PIN
#define RESET_PIN 0
#endif

#ifndef CS_PIN
#define CS_PIN 0
#endif

#include <SPI.h>
#include <Ethernet.h>
EthernetUDP udp;

#include <Dns.h>
DNSClient dns;
IPAddress googleDNS(8, 8, 8, 8);

IPAddress server_ip;
int server_port;

static uint32_t getMillis(void)
{
  return millis();
}

void log_callback(const char *msg, int level, const char *category, void *attribute, void *reserved)
{
  printf("%s: %s\n", category, msg);
}

int connect_cb_udp(const char *address, int port)
{
  int ret = -1;
  printf("connect_cb_udp\n");
  server_port = port;
  udp.begin(server_port);
  udp.setTimeout(100);

  dns.begin(googleDNS);
  ret = dns.getHostByName(address, server_ip);
  printf("%s %d.%d.%d.%d\n", address, server_ip[0], server_ip[1], server_ip[2], server_ip[3]);
  if (ret == 1)
    trackle.connectionCompleted();

  return ret;
}

int disconnect_cb_udp()
{
  udp.stop();
  return 1;
}

void time_cb(time_t time, unsigned int param, void *reserved)
{
  printf("time_cb");
}

int send_cb_udp(const unsigned char *buf, uint32_t buflen, void *tmp)
{
  int sent = 0;
  udp.beginPacket(server_ip, server_port);
  sent = udp.write(buf, buflen);
  udp.endPacket();

  if (sent > 0)
  {
    //printf("sent ");
    //printfln(sent);
  }
  return sent;
}

int receive_cb_udp(unsigned char *buf, uint32_t buflen, void *tmp)
{
  int rcv = 0;
  if (udp.parsePacket())
  {
    rcv = udp.read(buf, buflen);
  }
  if (rcv > 0)
  {
    //printf("rcv ");
    //printfln(rcv);
  }
  return rcv;
}

int my_connect(uint8_t *mac)
{
  printf("-------- %d %d -------", RESET_PIN, CS_PIN);

  // start the Ethernet connection
  if (RESET_PIN > 0)
  {
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW);
    delay(600);
    digitalWrite(RESET_PIN, HIGH);
  }

  if (CS_PIN > 0)
  {
    Ethernet.init(CS_PIN);
  }
  else
  {
    Ethernet.init();
  }

  // start the Ethernet connection:
  printf("Initialize Ethernet with DHCP:");
  if (mac == NULL) //if no mac address, read from hw
    Ethernet.MACAddress(mac);
  if (Ethernet.begin(mac) == 0)
  {
    printf("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      printf("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    else if (Ethernet.linkStatus() == LinkOFF)
    {
      printf("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    return -1;
  }

  return 1;
}

void my_loop()
{
  /*if (millis() - last_check_wifi > CHECK_WIFI_INTERVAL)
  {
    last_check_wifi = millis();
    printf("status: %d\n", WiFi.status());
    if (!WiFi.isConnected())
    {
      printf("Connecting to %s\n", ssid);
      WiFi.begin(ssid, pass);
    }
  }*/
}

#endif /* TrackleEsp32Ethernet_h */
