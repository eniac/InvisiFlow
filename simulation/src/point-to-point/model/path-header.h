#ifndef PATH_HEADER_H
#define PATH_HEADER_H

#include "ns3/header.h"

namespace ns3
{

class PathHeader : public Header
{
public:

    PathHeader();
    PathHeader(const PathHeader& pathHeader);
    ~PathHeader() override;

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
    void SetProtocol(uint8_t _protocol);
    uint8_t GetProtocol();
    void SetTTL(uint8_t _ttl);
    uint8_t GetTTL();

    uint32_t Hash();

    bool operator == (const PathHeader& o);

    uint32_t m_srcIP;
    uint32_t m_dstIP;
    uint16_t m_srcPort;
    uint16_t m_dstPort;
    uint32_t m_nodeId;
    uint8_t m_protocol;
    uint8_t m_ttl;
};

bool operator < (const PathHeader& a, const PathHeader& b);

} // namespace ns3

#endif /* PATH_HEADER_H */