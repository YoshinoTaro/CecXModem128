# CecXModem128

## Overall
Simple XModem Library for Arduino. This library is made for Sony Spresense. Compatiblity to other platforms have not been confirmed.
This library supports only CRC16 and 128 packet size. Legacy checksum and XModem-1K are not supported.

## How to use it
You can use this library to both SDCard or Embedded Flash. If you want to use SDCard as the storage, 
please use SDHCI library instead of Flash library. This example uses Flash library.


### Send example (Spresense SD card -> PC)

    #include <CrcXModem128.h>
    #include <Flash.h>
    
    CrcXModem128 xmodem;
    
    void setup() {
      Serial.begin(115200);
      Flash.begin();
      xmodem.begin(&Serial);

      Serial.println("ready to receive");

      /* Spresense -> PC */
      File sndFile = Flash.open("sndtxt.txt");
      Serial.println("ready to send");
    
      if (sndFile) {
        Serial.println("XModem ready!");
        xmodem.sendFile(sndFile);
      } else {
        Serial.println("sndtxt.txt doesnot exists");
      }
      sndFile.close();
    }
    
    void loop() { }    

### Receive example (Spresense SD card -> PC)
    #include <CrcXModem128.h>
    #include <Flash.h>
    
    CrcXModem128 xmodem;
    
    void setup() {
      Serial.begin(115200);
      Flash.begin();
      xmodem.begin(&Serial);
      
      Serial.println("ready to receive");
    
      /* PC -> Spresense */
      File rcvFile = Flash.open("rcvtxt.txt", FILE_WRITE);
    
      if (rcvFile) {  
        Serial.println("XModem ready!");
        xmodem.recvFile(rcvFile);
      } else {
        Serial.println("rcvtxt.txt cannot create");
      }
      rcvFile.close();
    }
    
    void loop() { }
    
## Tips
 If you use TeraTerm as a host application on your PC. Please check "CRC" on the dialog to specify the local file when you receive data from Spresense, .
 
![dialog](https://user-images.githubusercontent.com/18510684/91984640-36801700-ed67-11ea-908b-521bd6db5cc3.png)
