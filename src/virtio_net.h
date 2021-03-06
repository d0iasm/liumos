#pragma once

#include <optional>

#include "generic.h"
#include "pci.h"

namespace Virtio {
class Net {
 public:
  struct PacketBufHeader {
    // virtio: 5.1.6 Device Operation
    uint8_t flags;
    uint8_t gso_type;
    uint16_t header_length;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
    //
    static constexpr uint8_t kFlagNeedsChecksum = 1;
    static constexpr uint8_t kGSOTypeNone = 0;
  };
  packed_struct IPv4Addr {
    uint8_t addr[4];
    bool IsEqualTo(IPv4Addr to) const {
      return *reinterpret_cast<const uint32_t*>(addr) ==
             *reinterpret_cast<const uint32_t*>(to.addr);
    }
  };
  static void PutIPv4Addr(Net::IPv4Addr addr);
  static constexpr IPv4Addr kBroadcastIPv4Addr = {0xFF, 0xFF, 0xFF, 0xFF};
  static constexpr IPv4Addr kWildcardIPv4Addr = {0x00, 0x00, 0x00, 0x00};
  packed_struct EtherAddr {
    uint8_t mac[6];
    bool IsEqualTo(EtherAddr to) const {
      return *reinterpret_cast<const uint32_t*>(mac) ==
                 *reinterpret_cast<const uint32_t*>(to.mac) &&
             *reinterpret_cast<const uint16_t*>(&mac[4]) ==
                 *reinterpret_cast<const uint16_t*>(&to.mac[4]);
    }
  };
  static constexpr EtherAddr kBroadcastEtherAddr = {0xFF, 0xFF, 0xFF,
                                                    0xFF, 0xFF, 0xFF};
  static void PutEtherAddr(Net::EtherAddr addr);
  packed_struct InternetChecksum {
    // https://tools.ietf.org/html/rfc1071
    uint8_t csum[2];

    void Clear() {
      csum[0] = 0;
      csum[1] = 0;
    }
    bool IsEqualTo(InternetChecksum to) const {
      return *reinterpret_cast<const uint16_t*>(csum) ==
             *reinterpret_cast<const uint16_t*>(to.csum);
    }
  };
  static InternetChecksum CalcChecksum(void* buf, size_t start, size_t end) {
    // https://tools.ietf.org/html/rfc1071
    uint8_t* p = reinterpret_cast<uint8_t*>(buf);
    uint32_t sum = 0;
    for (size_t i = start; i < end; i += 2) {
      sum += (static_cast<uint16_t>(p[i + 0])) << 8 | p[i + 1];
    }
    while (sum >> 16) {
      sum = (sum & 0xffff) + (sum >> 16);
    }
    sum = ~sum;
    return {static_cast<uint8_t>((sum >> 8) & 0xFF),
            static_cast<uint8_t>(sum & 0xFF)};
  }
  packed_struct EtherFrame {
    EtherAddr dst;
    EtherAddr src;
    uint8_t eth_type[2];
    static constexpr uint8_t kTypeARP[2] = {0x08, 0x06};
    static constexpr uint8_t kTypeIPv4[2] = {0x08, 0x00};
    void SetEthType(const uint8_t(&etype)[2]) {
      eth_type[0] = etype[0];
      eth_type[1] = etype[1];
    }
    bool HasEthType(const uint8_t(&etype)[2]) {
      return eth_type[0] == etype[0] && eth_type[1] == etype[1];
    }
  };
  packed_struct ARPPacket {
    EtherFrame eth;
    uint8_t hw_type[2];
    uint8_t proto_type[2];
    uint8_t hw_addr_len;
    uint8_t proto_addr_len;
    uint8_t op[2];
    EtherAddr sender_eth_addr;
    IPv4Addr sender_proto_addr;
    EtherAddr target_eth_addr;
    IPv4Addr target_proto_addr;
    /*
      ARP example : who has 10.10.10.135? Tell 10.10.10.90
      ff ff ff ff ff ff  // dst
      f8 ff c2 01 df 39  // src
      08 06              // ether type (ARP)
      00 01              // hardware type (ethernet)
      08 00              // protocol type (ipv4)
      06                 // hw addr len (6)
      04                 // proto addr len (4)
      00 01              // operation (arp request)
      f8 ff c2 01 df 39  // sender eth addr
      0a 0a 0a 5a        // sender protocol addr
      00 00 00 00 00 00  // target eth addr (0 since its unknown)
      0a 0a 0a 87        // target protocol addr
     */
    enum class Operation {
      kUnknown,
      kRequest,
      kReply,
    };
    Operation GetOperation() {
      if (op[0] != 0)
        return Operation::kUnknown;
      if (op[1] == 0x01)
        return Operation::kRequest;
      if (op[1] == 0x02)
        return Operation::kReply;
      return Operation::kUnknown;
    }
    void SetupRequest(const IPv4Addr target_ip, const IPv4Addr src_ip,
                      const EtherAddr src_mac) {
      for (int i = 0; i < 6; i++) {
        eth.dst.mac[i] = 0xff;
        target_eth_addr.mac[i] = 0x00;
      }
      eth.src = src_mac;
      sender_eth_addr = src_mac;
      sender_proto_addr = src_ip;
      target_proto_addr = target_ip;
      eth.SetEthType(EtherFrame::kTypeARP);
      hw_type[0] = 0x00;
      hw_type[1] = 0x01;
      proto_type[0] = 0x08;
      proto_type[1] = 0x00;
      hw_addr_len = 6;
      proto_addr_len = 4;
      op[0] = 0x00;
      op[1] = 0x01;  // Request
    }
    void SetupReply(const IPv4Addr target_ip, const IPv4Addr src_ip,
                    const EtherAddr target_mac, const EtherAddr src_mac) {
      eth.dst = target_mac;
      eth.src = src_mac;
      target_eth_addr = target_mac;
      sender_eth_addr = src_mac;
      sender_proto_addr = src_ip;
      target_proto_addr = target_ip;
      eth.SetEthType(EtherFrame::kTypeARP);
      hw_type[0] = 0x00;
      hw_type[1] = 0x01;
      proto_type[0] = 0x08;
      proto_type[1] = 0x00;
      hw_addr_len = 6;
      proto_addr_len = 4;
      op[0] = 0x00;
      op[1] = 0x02;  // Reply
    }
  };
  packed_struct IPv4Packet {
    enum class Protocol : uint8_t {
      kICMP = 1,
      kTCP = 6,
      kUDP = 17,
    };

    EtherFrame eth;
    uint8_t version_and_ihl;
    uint8_t dscp_and_ecn;
    uint8_t length[2];
    uint16_t ident;
    uint16_t flags;
    uint8_t ttl;
    Protocol protocol;
    InternetChecksum csum;  // for this header
    IPv4Addr src_ip;
    IPv4Addr dst_ip;

    void SetDataLength(uint16_t size) {
      size += sizeof(IPv4Packet) - sizeof(EtherFrame);  // IP header size
      size = (size + 1) & ~1;                           // make size odd
      length[0] = size >> 8;
      length[1] = size & 0xFF;
    }
    void CalcAndSetChecksum() {
      csum.Clear();
      csum = CalcChecksum(this, offsetof(IPv4Packet, version_and_ihl),
                          sizeof(IPv4Packet));
    }
  };
  packed_struct ICMPPacket {
    enum class Type : uint8_t {
      kEchoReply = 0,
      kEchoRequest = 8,
    };

    IPv4Packet ip;
    Type type;
    uint8_t code;
    InternetChecksum csum;
  };
  packed_struct IPv4UDPPacket {
    IPv4Packet ip;
    uint8_t src_port[2];  // optional
    uint8_t dst_port[2];
    uint8_t length[2];
    InternetChecksum csum;

    void SetSourcePort(uint16_t port) {
      src_port[0] = port >> 8;
      src_port[1] = port & 0xFF;
    }
    uint16_t GetSourcePort() {
      return static_cast<uint16_t>(src_port[0]) << 8 | src_port[1];
    }
    void SetDestinationPort(uint16_t port) {
      dst_port[0] = port >> 8;
      dst_port[1] = port & 0xFF;
    }
    uint16_t GetDestinationPort() {
      return static_cast<uint16_t>(dst_port[0]) << 8 | dst_port[1];
    }
    void SetDataSize(uint16_t size) {
      size += 8;               // UDP header size
      size = (size + 1) & ~1;  // make size odd
      length[0] = size >> 8;
      length[1] = size & 0xFF;
    }
  };
  static InternetChecksum CalcUDPChecksum(void* buf,
                                          size_t start,
                                          size_t end,
                                          Net::IPv4Addr src_addr,
                                          Net::IPv4Addr dst_addr,
                                          uint8_t (&udp_length)[2]) {
    // https://tools.ietf.org/html/rfc1071
    uint8_t* p = reinterpret_cast<uint8_t*>(buf);
    uint32_t sum = 0;
    // Pseudo-header
    sum += (static_cast<uint16_t>(src_addr.addr[0]) << 8) | src_addr.addr[1];
    sum += (static_cast<uint16_t>(src_addr.addr[2]) << 8) | src_addr.addr[3];
    sum += (static_cast<uint16_t>(dst_addr.addr[0]) << 8) | dst_addr.addr[1];
    sum += (static_cast<uint16_t>(dst_addr.addr[2]) << 8) | dst_addr.addr[3];
    sum += (static_cast<uint16_t>(udp_length[0]) << 8) | udp_length[1];
    sum += 17;  // Protocol: UDP
    for (size_t i = start; i < end; i += 2) {
      sum += (static_cast<uint16_t>(p[i + 0])) << 8 | p[i + 1];
    }
    while (sum >> 16) {
      sum = (sum & 0xffff) + (sum >> 16);
    }
    sum = ~sum;
    return {static_cast<uint8_t>((sum >> 8) & 0xFF),
            static_cast<uint8_t>(sum & 0xFF)};
  }
  packed_struct DHCPPacket {
    IPv4UDPPacket udp;
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    IPv4Addr ciaddr;
    IPv4Addr yiaddr;
    IPv4Addr siaddr;
    IPv4Addr giaddr;
    EtherAddr chaddr;
    uint8_t chaddr_padding[10];
    uint8_t sname[64];
    uint8_t file[128];
    void SetupRequest(const EtherAddr& src_eth_addr) {
      // ip.eth
      udp.ip.eth.dst = kBroadcastEtherAddr;
      udp.ip.eth.src = src_eth_addr;
      udp.ip.eth.SetEthType(EtherFrame::kTypeIPv4);
      // ip
      udp.ip.version_and_ihl =
          0x45;  // IPv4, header len = 5 * sizeof(uint32_t) = 20 bytes
      udp.ip.dscp_and_ecn = 0;
      udp.ip.SetDataLength(sizeof(DHCPPacket) - sizeof(IPv4Packet));
      udp.ip.ident = 0x426b;
      udp.ip.flags = 0;
      udp.ip.ttl = 0xFF;
      udp.ip.protocol = Net::IPv4Packet::Protocol::kUDP;
      udp.ip.src_ip = Net::kWildcardIPv4Addr;
      udp.ip.dst_ip = Net::kBroadcastIPv4Addr;
      udp.ip.CalcAndSetChecksum();
      // udp
      udp.SetSourcePort(68);
      udp.SetDestinationPort(67);
      udp.SetDataSize(sizeof(DHCPPacket) - sizeof(IPv4UDPPacket));
      udp.csum.Clear();
      // dhcp
      op = 1;
      htype = 1;
      hlen = 6;
      hops = 0;
      xid = 0x1234;
      secs = 0;
      flags = 0;
      ciaddr = Net::kWildcardIPv4Addr;
      yiaddr = Net::kWildcardIPv4Addr;
      siaddr = Net::kWildcardIPv4Addr;
      giaddr = Net::kWildcardIPv4Addr;
      chaddr = src_eth_addr;
      for (int i = 0; i < 10; i++) {
        chaddr_padding[i] = 0;
      }
      for (int i = 0; i < 64; i++) {
        sname[i] = 0;
      }
      for (int i = 0; i < 128; i++) {
        file[i] = 0;
      }
      //
      udp.csum = CalcUDPChecksum(this, offsetof(DHCPPacket, udp.src_port),
                                 sizeof(DHCPPacket), Net::kWildcardIPv4Addr,
                                 Net::kBroadcastIPv4Addr, udp.length);
    }
  };
  class Virtqueue {
   public:
    packed_struct Descriptor {
      volatile uint64_t addr;
      volatile uint32_t len;
      volatile uint16_t flags;
      volatile uint16_t next;
    };
    struct UsedRingEntry {
      volatile uint32_t id;   // index of start of used descriptor chain
      volatile uint32_t len;  // in byte
    };
    void Alloc(int queue_size);
    uint64_t GetPhysAddr();
    void SetDescriptor(int idx,
                       void* vaddr,
                       uint32_t len,
                       uint16_t flags,
                       uint16_t next);
    template <typename T = uint8_t*>
    T GetDescriptorBuf(int idx) {
      assert(0 <= idx && idx < queue_size_);
      return reinterpret_cast<T>(buf_[idx]);
    }
    uint32_t GetDescriptorSize(int idx) {
      assert(0 <= idx && idx < queue_size_);
      Descriptor& desc =
          *reinterpret_cast<Descriptor*>(base_ + sizeof(Descriptor) * idx);
      return desc.len;
    }
    void SetAvailableRingEntry(int idx, uint16_t desc_idx) {
      assert(0 <= idx && idx < queue_size_);
      volatile uint16_t* ring = reinterpret_cast<volatile uint16_t*>(
          base_ + sizeof(Descriptor) * queue_size_ + sizeof(uint16_t) * 2);
      ring[idx] = desc_idx;
    }
    void SetAvailableRingIndex(int idx) {
      volatile uint16_t& pidx = *reinterpret_cast<volatile uint16_t*>(
          base_ + sizeof(Descriptor) * queue_size_ + sizeof(uint16_t));
      pidx = idx;
    }
    uint16_t GetUsedRingIndex();
    UsedRingEntry& GetUsedRingEntry(int idx);

   private:
    static constexpr int kMaxQueueSize = 0x100;
    int queue_size_;
    uint8_t* base_;
    void* buf_[kMaxQueueSize];
  };

  void PollRXQueue();
  void Init();

  template <typename T = uint8_t*>
  T GetNextTXPacketBuf(size_t size) {
    uint32_t buf_size = static_cast<uint32_t>(sizeof(PacketBufHeader) + size);
    assert(buf_size < kPageSize);
    auto& txq = vq_[kIndexOfTXVirtqueue];
    const int idx =
        vq_cursor_[kIndexOfTXVirtqueue] % vq_size_[kIndexOfTXVirtqueue];
    txq.SetDescriptor(idx, txq.GetDescriptorBuf(idx), buf_size, 0, 0);
    return reinterpret_cast<T>(txq.GetDescriptorBuf(idx) +
                               sizeof(PacketBufHeader));
  }
  const IPv4Addr GetSelfIPv4Addr() { return self_ip_; }
  void SetSelfIPv4Addr(IPv4Addr addr) { self_ip_ = addr; }
  const EtherAddr GetSelfEtherAddr() { return {mac_addr_}; }
  void SendPacket() {
    const int idx =
        vq_cursor_[kIndexOfTXVirtqueue] % vq_size_[kIndexOfTXVirtqueue];
    auto& txq = vq_[kIndexOfTXVirtqueue];
    PacketBufHeader& hdr = *txq.GetDescriptorBuf<PacketBufHeader*>(idx);
    hdr.flags = 0;
    hdr.gso_type = PacketBufHeader::kGSOTypeNone;
    hdr.header_length = 0x00;
    hdr.gso_size = 0;
    hdr.csum_start = 0;
    hdr.csum_offset = 0;
    txq.SetAvailableRingEntry(idx, idx);
    vq_cursor_[kIndexOfTXVirtqueue]++;
    txq.SetAvailableRingIndex(idx + 1);
    WriteConfigReg16(16 /* Queue Notify */, kIndexOfTXVirtqueue);
  }

  static Net& GetInstance();

 private:
  static constexpr int kNumOfVirtqueues = 3;

  static constexpr int kIndexOfRXVirtqueue = 0;
  static constexpr int kIndexOfTXVirtqueue = 1;

  static Net* net_;
  PCI::DeviceLocation dev_;
  EtherAddr mac_addr_;
  uint16_t config_io_addr_base_;
  Virtqueue vq_[kNumOfVirtqueues];
  uint16_t vq_size_[kNumOfVirtqueues];
  uint16_t vq_cursor_[kNumOfVirtqueues];
  IPv4Addr self_ip_;

  void ProcessPacket(uint8_t* buf, size_t buf_size);

  uint8_t ReadConfigReg8(int ofs);
  uint16_t ReadConfigReg16(int ofs);
  uint32_t ReadConfigReg32(int ofs);

  void WriteConfigReg8(int ofs, uint8_t data);
  void WriteConfigReg16(int ofs, uint16_t data);
  void WriteConfigReg32(int ofs, uint32_t data);

  uint8_t ReadDeviceStatus();
  void WriteDeviceStatus(uint8_t);
  void SetFeatures(uint32_t);
};
};  // namespace Virtio
