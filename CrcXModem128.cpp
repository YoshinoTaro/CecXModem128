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

#include <CrcXModem128.h>

#define SYNC_TIMEOUT 30
#define MAX_RETRY    30


/*
 * constructor
 */
CrcXModem128::CrcXModem128():
   m_debug(false)
  ,m_crc16(0)
  ,m_x_port(NULL)
  ,m_d_port(NULL)
{ }

/*
 *  begin()
 *   Set UART for XModem
 */
bool CrcXModem128::begin(Stream *port) {
  if (port == NULL) return false;
  m_x_port = port;
  return true;
}

/*
 * setDebug()
 *  Set UART for DebugMessage
 *  The port must be other than the port assigned to XModem
 */
bool CrcXModem128::setDebug(Stream *port) {
  if (port == NULL) return false;
  m_d_port = port;
  m_d_port->println("Ready to debug");
  m_debug = true;
  return true;
}

/*  sendFile(File sndFile)
 *  return: 
 *      0 success
 *     -1 sync_error
 *     -2 retry_error
 *     -3 ack_error
 *     -4 nack_error
 *   -100 canceled
 */
int8_t CrcXModem128::sendFile(File &sndFile)
{
  char val;
  uint16_t try_num = 0;

  // initialize
  m_x_port->println("Ready to send");
  debug_print("Ready to send");

  sndFile.seek(0);
  m_x_port->setTimeout(3000);

  // wait for 'C' from your host 
  try_num = 0;
  do {
    m_x_port->readBytes(&val, 1);
    if (try_num++ == SYNC_TIMEOUT) {
      debug_print("SYNC ERROR");
      return -1;
    }
  } while (val != 'C');


  // send a packet including SOH/payload(128 bytes)/ACK
  bool fin = false;
  uint16_t packet_num = 1;
  while(!fin) {
   
    m_crc16 = 0x00; 

    // write SOH
    m_x_port->write(SOH);
    debug_print("write SOH");

    m_x_port->setTimeout(1000);
    // write packet_num and ~packet_num
    m_x_port->write(packet_num);
    debug_print("packet_num: " + String(packet_num, HEX));
    m_x_port->write(~packet_num);
    debug_print("inverted packet_num: " + String(~packet_num, HEX));

    // send payload
    for (int i = 0; i < PACKET_LEN; ++i) {
      if (sndFile.available()) {
        val = sndFile.read();
      } else {
        val = 0x1A; /* EOF */
        fin = true;
      }
      this->write_payload(val);
    }

    // write CRC16
    debug_print("write CRC16: " + String(m_crc16, HEX));
    m_x_port->write((char) (m_crc16 >>8));
    m_x_port->write((char) (m_crc16 & 0xFF));
      
    // wait for ACK
    uint8_t count_retry = 0;
    try_num = 0;
    do {
      m_x_port->readBytes(&val, 1);
      if (count_retry++ > 200) {
        debug_print("NO ACK", true);
        return -3;
      }
      if (val == NAK) {
        debug_print("NACK RECEIVED", true);
        return -4;
      } else if (val == CAN) {
        debug_print("CANCELED BY HOST", true);
        return -100;
      }
      if (try_num++ > MAX_RETRY) {
        debug_print("MAX_RETRY ERROR", true);
        return -2;
      }
    } while (val != ACK);

    ++packet_num;
  } 

  m_x_port->setTimeout(3000);

  // finalize the transaction
  try_num = 0;
  do {
    // send EOT
    m_x_port->write(EOT);
    debug_print("write EOT");

    // wait for ACK
    uint8_t count_retry = 0;
    do {
      m_x_port->readBytes(&val, 1);
      if (count_retry++ > 200) {
        debug_print("NO ACK", true);
        return -3;
      }
      if (val == NAK) {
        debug_print("NACK RECEIVED", true);
        return -4;
      } else if (val == CAN) {
        debug_print("CANCELED BY HOST", true);
        return -100;
      }
    } while (val != ACK);
    if (try_num++ == SYNC_TIMEOUT) {
      debug_print("SYNC_TIMEOUT ERROR", true);
      return -3;
    }
  } while (val != ACK);

  m_x_port->println("Finished");
  debug_print("Finished");

  return 0;
}

/*  recvFile(File rcvFile)
 *  return: 
 *      0 success
 *     -1 packet_num error
 *     -2 CRC error
 */
int8_t CrcXModem128::recvFile(File &rcvFile)
{
  char val;
  uint16_t try_num = 0;

  // initialize
  m_x_port->println("Ready to receive");
  debug_print("Ready to receive");

  rcvFile.seek(0);
  m_x_port->setTimeout(3000);

  uint16_t packet_num = 0;
  bool connected = false;
  while (true) {
    do {
      // wait for the response
      if (connected == false) {
        m_x_port->write('C');  // set 'C' : CRC
        debug_print("write C");
      }

      // waint for SOH
      m_x_port->readBytes(&val, 1);  // read a block number
    } while (val != SOH);
    connected = true;

    debug_print("SOH received");
    m_x_port->setTimeout(1000);

    // receive the packet number
    m_x_port->readBytes(&val, 1);  // read a packet number
    debug_print("packet num: " + String(val, HEX));
    packet_num = val;
    m_x_port->readBytes(&val, 1);  // read a inverted packet number
    debug_print("inverted packet num: " + String(val, HEX));
    val = ~val;
    if (packet_num != val) {
      debug_print("invalid packet number: " + String(packet_num) + ":" + String(val), true);
      return -1;
    }

    // receive the payload
    char in_buff[PACKET_LEN];
    memset(in_buff, 0, PACKET_LEN);
    for (int i = 0; i < PACKET_LEN; ++i) {  // receiving data
      in_buff[i] = read_payload();
    }

    // read CRC15 and check it
    char upper, lower;
    m_x_port->readBytes(&upper, 1); // read CRC16
    m_x_port->readBytes(&lower, 1);
    unsigned short rcv_crc16 = (unsigned short)(upper << 8 | lower);
    debug_print("received CRC16: " + String(rcv_crc16, HEX));
    debug_print("calculated CRC16: " + String(m_crc16, HEX));
    if (rcv_crc16 != m_crc16) {
      debug_print("CRC16 error", true);
      return -2;
    }

    // Send ACK 
    debug_print("Send ACK");
    m_x_port->write(ACK);

    // write the payload to the file
    debug_print("Write the file");
    uint8_t check_eof = 0;
    for (int i = 0; i < PACKET_LEN; ++i) {
      rcvFile.write(in_buff[i]);
      if (in_buff[i] == 0x1A /* EOF */) {
        ++check_eof;
      }
    }

    // check EOF
    m_crc16 = 0;
    if (check_eof > 2 && in_buff[PACKET_LEN-1] == 0x1A /* EOF */) {
      debug_print("EOF(0x1A) detected :" + String(in_buff[PACKET_LEN-1], HEX));
      break;
    }
  }

  // finalize the transaction
  m_x_port->setTimeout(3000);
  do {
    m_x_port->readBytes(&val, 1);
  } while (val != EOT); // wait for EOT

  m_x_port->write(ACK); // send ACK

  m_x_port->println("Finished");
  debug_print("Finished");

  rcvFile.flush();

  return 0;
}


/*
 * write_payload()
 * calculate CRC16 and send payload data
 */
void CrcXModem128::write_payload(uint8_t val)
{
  m_crc16 = m_crc16 ^ (int) val << 8;
  uint8_t j = 8;
  do {
    if (m_crc16 & 0x8000)
      m_crc16 = m_crc16 << 1 ^ 0x1021;
    else
      m_crc16 = m_crc16 << 1;
  } while(--j);
  m_x_port->write(val);
}

/*
 * read_payload()
 * calculate CRC16 and read payload data
 */
char CrcXModem128::read_payload()
{
  char val;
  m_x_port->readBytes(&val, 1);

  m_crc16 = m_crc16 ^ (uint16_t)(val << 8);
  uint8_t j = 8;
  do {
    if (m_crc16 & 0x8000)
      m_crc16 = m_crc16 << 1 ^ 0x1021;
    else
      m_crc16 = m_crc16 << 1;
  } while(--j);
  return val;
}


/*
 * debug_print()
 * print debug message if setDebug was set
 */
void  CrcXModem128::debug_print(String str, bool fatal) {
  if (fatal) {
    m_x_port->write(CAN);
    usleep(100);
    m_x_port->println(str);
  }

  if (m_debug) {
    m_d_port->println(str);
  }
  return;
}


