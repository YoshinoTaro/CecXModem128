/*
 *  Spresense Simple XModem library 
 *  Copyright 2020 Yoshino Taro
 * 
 *  This xmodem library only supports 128 packet and CRC16
 *
 *  This library is made for Sony Spresense. 
 *  Copatibility to other platforms is not confirmed. 
 *  You need Spresense boards below when you will try this sketch
 *    Spresense Main Board
 *  See the detail: https://developer.sony.com/develop/spresense/
 *
 *  The license of this source code is LGPL v2.1 
 *
 */

#ifndef _CRCXMODEM128_H
#define _CRCXMODEM128_H

#include <Arduino.h>
#include <File.h>

#define	SOH  0x01
#define	EOT  0x04
#define	ENQ  0x05
#define	ACK  0x06
#define	LF   0x0a
#define	CR   0x0d
#define	NAK  0x15
#define	CAN  0x18

#define PACKET_LEN 128

class CrcXModem128
{
	public:
		CrcXModem128();
    ~CrcXModem128() { }
    bool begin(Stream *port);
    bool setDebug(Stream *port);
		int8_t sendFile(File &sndFile);
    int8_t recvFile(File &rcvFile);
	private:
    bool m_debug;
		uint16_t m_crc16;
		Stream *m_x_port;
		Stream *m_d_port;

		void write_payload(uint8_t val);
    char read_payload();
    void debug_print(String str, bool fatal=false);
};

#endif
