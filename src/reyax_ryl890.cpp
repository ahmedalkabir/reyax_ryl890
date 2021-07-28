#include "reyax_ryl890.h"

// internal methods
bool rylr890::parse(const char *candicate)
{
  if (_transport->available())
  {
    auto last_time = millis();
    // TODO: make a timeout as configurable variable
    // state machine style
    int state = 0;
    const char *_cr = candicate;
    while (millis() - last_time < 3000)
    {
      char ch = _transport->read();
      if (ch == *_cr && state == 0)
      {
        _cr += 1; // increament cursor to the start of the word
        state = 1;
        ch = _transport->read(); // read next character
      }

      if (state = 1)
      {
        if (ch == *_cr++)
        {
          if (ch == '\r')
          {
            _transport->read(); // read last character '\n'
            return true;
          }
        }
        else
          return false;
      }
    }
  }
  return false;
}

void rylr890::begin(Stream *_t)
{
  _transport = _t;
}

void rylr890::setup_callback(callback_receiver func)
{
  _func = func;
}

const char *rylr890::get_error()
{
  return msg;
}

void rylr890::loop()
{
  if (_transport->available())
  {
    delay(2); // wait for a little bit to fill the buffer
    char ch = _transport->read();
    if (ch == '+')
    {
      ch = _transport->read();
      if (ch == 'R')
      {
        _transport->readBytesUntil('\r', _received_data, 300);
        char *buffer = reinterpret_cast<char *>(_received_data + 3);
        char *word;
        int counter = 0;
        while ((word = strsep(&buffer, ",")) != NULL)
        {
          switch (counter)
          {
          case 0:
            _msg.address = atoi(word);
            break;
          case 1:
            _msg.length = atoi(word);
            break;
          case 2:
            _msg.data = reinterpret_cast<uint8_t *>(word);
            break;
          case 3:
            _msg.RSSI = atoi(word);
            break;
          case 4:
            _msg.SNR = atoi(word);
            break;
          default:
            break;
          }
          counter += 1;
        }

        _func(&_msg);
      }
    }
  }
}

rylr890::STATUS rylr890::set_address(uint16_t address)
{
  if (address > 0 && address < 65536)
  {
    // AT+ADDRESS=<ADDR>\r\n
    auto s = sprintf(_internal_buffer, "%s+%s=%d%s", _at, _address, address, _end);
    _transport->write(_internal_buffer, s);
    delay(200);
    if (parse(_ok))
      return rylr890::STATUS::OK;
  }
  msg = error_1;
  return rylr890::STATUS::ERROR;
}

rylr890::STATUS rylr890::set_networkid(uint8_t id)
{
  if (id >= 0 && id <= 16)
  {
    // AT+NETWORKID=<ID>\r\n
    auto s = sprintf(_internal_buffer, "%s+%s=%d%s", _at, _networkid, id, _end);
    _transport->write(_internal_buffer, s);
    delay(200);
    if (parse(_ok))
      return rylr890::STATUS::OK;
  }
  msg = error_2;
  return rylr890::STATUS::ERROR;
}

rylr890::STATUS rylr890::test_at()
{
  auto s = sprintf(_internal_buffer, "%s%s", _at, _end);
  _transport->write(_internal_buffer, s);

  // read buffer
  delay(200); // some delay to fill the buffer
  if (parse(_ok))
    return rylr890::STATUS::OK;

  msg = common_error;
  return rylr890::STATUS::ERROR;
}

rylr890::STATUS rylr890::send(uint16_t address, const uint8_t *payload, uint8_t length)
{
  if ((length <= 240) && (address <= 65535))
  {
    auto s = sprintf(_internal_buffer, "%s+%s=%d,%d,%s%s",
                     _at, _send, address, length, payload, _end);
    _transport->write(_internal_buffer, s);

    delay(1000); // too much I guess
    if (parse(_ok))
    {
      return rylr890::STATUS::OK;
    }
  }
  return rylr890::STATUS::ERROR;
}