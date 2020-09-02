/*
 *  Sample sketch for Spresense Simple XModem library 
 *  Copyright 2020 Yoshino Taro
 * 
 *  This xmodem library only supports 128 packet and CRC16
 *
 *  This library is made for Sony Spresense. 
 *  Other platforms are not supported. 
 *  You need Spresense boards below when you will try this sketch
 *    Spresense Main Board
 *  See the detail: https://developer.sony.com/develop/spresense/
 *
 *  The license of this source code is LGPL v2.1 
 *
 */

#include <CrcXModem128.h>
#include <SDHCI.h>

CrcXModem128 xmodem;
SDClass SD;

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200);
  SD.begin();
  xmodem.begin(&Serial);
  xmodem.setDebug(&Serial2);

  Serial.println("ready to receive");

  /* Spresense -> PC */
  File sndFile = SD.open("sndtxt.txt");
  Serial.println("ready to send");

  if (sndFile) {
    Serial.println("XModem ready!");
    xmodem.sendFile(sndFile);
  } else {
    Serial.println("sndtxt.txt doesnot exists");
  }

  sndFile.close();

}

void loop()
{

}
