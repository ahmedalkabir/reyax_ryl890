#ifndef REYAX_RYL890_H_
#define REYAX_RYL890_H_

#include <Arduino.h>
#include <stdint.h>

struct MSG
{
  int address;
  int RSSI;
  int SNR;
  int length;
  uint8_t *data;
};
using callback_receiver = void (*)(MSG *message);

class rylr890
{
public:
  enum class STATUS
  {
    OK,
    ERROR,
  };

  void begin(Stream *_t);
  // void begin(Stream *_t, /*func*/);
  void setup_callback(callback_receiver func);

  void loop();
  STATUS send(uint16_t address, const uint8_t *payload, uint8_t length);
  // The Address will be memorized in EEPROM.
  // range between 0 ~ 65535
  STATUS set_address(uint16_t address);
  STATUS set_networkid(uint8_t id);
  STATUS set_band();
  STATUS set_parameter();
  STATUS set_mode();
  STATUS set_ipr();
  STATUS set_cpin();
  STATUS test_at();
  const char *get_error();

private:
  Stream *_transport;
  callback_receiver _func;
  MSG _msg;
  uint8_t _received_data[300];
  char _internal_buffer[50];
  const char *msg;
  const char *error_1 = "out of range"; // might delete in case of memory saving
  const char *error_2 = "out of range 2";
  const char *common_error = "Not Connected Well.";

  // commands
  const char *_at = "AT";
  const char *_end = "\r\n";
  const char *_ok = "+OK\r\n";
  const char *_address = "ADDRESS";
  const char *_networkid = "NETWORKID";
  const char *_send = "SEND";

  // internal methods
  bool parse(const char *candicate);
};

#endif