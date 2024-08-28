#ifndef UTIL_HEADER_H
#define UTIL_HEADER_H

#include "ns3/header.h"

namespace ns3
{

class UtilHeader : public Header
{
public:

    UtilHeader();
    UtilHeader(const UtilHeader& UtilHeader);
    ~UtilHeader() override;

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    uint32_t GetSerializedSize() const override;

    bool Empty();

    void SetNodeId(uint32_t _nodeId);
    uint32_t GetNodeId();
    void SetPortId(uint32_t _portId);
    uint32_t GetPortId();
    void SetTime(uint32_t _time);
    uint32_t GetTime();
    void SetByte(uint32_t _byte);
    uint32_t GetByte();

    uint32_t Hash();

    bool operator == (const UtilHeader& o);

    uint32_t m_nodeId;
    uint32_t m_portId;
    uint32_t m_time;
    uint32_t m_byte;
};

bool operator < (const UtilHeader& a, const UtilHeader& b);

} // namespace ns3

#endif /* UTIL_HEADER_H */