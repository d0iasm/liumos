#pragma once
#include "generic.h"

class Sheet;
class SerialPort;

class Console {
 public:
  Console()
      : cursor_x_(0), cursor_y_(0), sheet_(nullptr), serial_port_(nullptr) {}
  void ResetCursorPosition() {
    cursor_x_ = 0;
    cursor_y_ = 0;
  }
  void SetSheet(Sheet* sheet) { sheet_ = sheet; }
  void SetSerial(SerialPort* serial_port) { serial_port_ = serial_port; }
  void PutChar(char c);
  void PutString(const char* s);
  void PutChars(const char* s, int n);
  void PutHex64(uint64_t value);
  void PutHex64ZeroFilled(uint64_t value);
  void PutHex8ZeroFilled(uint8_t value);
  void PutStringAndHex(const char* s, uint64_t value);
  void PutStringAndHex(const char* s, void* value);
  void PutStringAndBool(const char* s, bool cond);
  void PutAddressRange(uint64_t addr, uint64_t size);
  void PutAddressRange(void* addr, uint64_t size);

 private:
  int cursor_x_, cursor_y_;
  Sheet* sheet_;
  SerialPort* serial_port_;
};

void PutChar(char c);
void PutString(const char* s);
void PutStringAndHex(const char* s, uint64_t value);
void PutStringAndHex(const char* s, void* value);
void PutStringAndBool(const char* s, bool cond);
void PutHex64(uint64_t value);
void PutAddressRange(uint64_t addr, uint64_t size);
void PutAddressRange(void* addr, uint64_t size);
void PutHex64ZeroFilled(uint64_t value);
void PutHex64ZeroFilled(void* value);
void PutHex8ZeroFilled(uint8_t value);
