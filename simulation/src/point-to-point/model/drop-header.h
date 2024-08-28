#ifndef DROP_HEADER_H
#define DROP_HEADER_H

#include "ns3/header.h"

namespace ns3
{

class DropHeader : public Header
{
public:

    DropHeader();
    DropHeader(const DropHeader& pathHeader);
    ~DropHeader() override;

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    uint32_t GetSerializedSize() const override;

    bool Empty();

    void SetSrcIP(uint32_t _srcIP);
    uint32_t GetSrcIP();
    void SetDstIP(uint32_t _dstIP);
    uint32_t GetDstIP();
    void SetSrcPort(uint16_t _srcPort);
    uint16_t GetSrcPort();
    void SetDstPort(uint16_t _dstPort);
    uint16_t GetDstPort();
    void SetNodeId(uint32_t _nodeId);
    uint32_t GetNodeId();
    void SetTime(uint32_t _time);
    uint32_t GetTime();
    void SetProtocol(uint8_t _protocol);
    uint8_t GetProtocol();

    uint32_t Hash();

    bool operator == (const DropHeader& o);

    uint32_t m_srcIP;
    uint32_t m_dstIP;
    uint16_t m_srcPort;
    uint16_t m_dstPort;
    uint32_t m_nodeId;
    uint32_t m_time;
    uint8_t m_protocol;
};

bool operator < (const DropHeader& a, const DropHeader& b);

} // namespace ns3

#endif /* DROP_HEADER_H */