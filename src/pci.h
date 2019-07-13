#pragma once
#include <unordered_map>

class PCI {
 public:
  struct DeviceLocation {
    uint8_t bus;
    uint8_t device;
    uint8_t func;
  };

  void DetectDevices();
  void PrintDevices();
  const auto& GetDeviceList() const { return device_list_; }
  static const char* GetDeviceName(uint32_t key);

  static PCI* GetInstance() {
    if (!pci_)
      pci_ = new PCI();
    return pci_;
  }

 private:
  PCI(){};
  bool DetectDevice(int bus, int device, int func);

  static PCI* pci_;
  std::unordered_multimap<uint32_t, DeviceLocation> device_list_;
};
