#ifndef COUNT_HEADER_H
#define COUNT_HEADER_H

#include "ns3/header.h"

namespace ns3
{

#define OURS_SKETCH_HASH 3
#define OURS_SKETCH_LENGTH 131072

struct CMSketch{
    int32_t values[OURS_SKETCH_HASH][OURS_SKETCH_LENGTH];

    CMSketch();
};

struct MyFlowId
{
    uint32_t m_srcIP;
    uint32_t m_dstIP;
    uint16_t m_srcPort;
    uint16_t m_dstPort;

    MyFlowId();
    MyFlowId(const MyFlowId& flow);
};

bool operator == (const MyFlowId& a, const MyFlowId& b);
bool operator < (const MyFlowId& a, const MyFlowId& b);

class CountHeader : public Header
{
public:

    CountHeader();
    CountHeader(const CountHeader& countHeader);
    ~CountHeader() override;

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    uint32_t GetSerializedSize() const override;

    bool Empty();

    void SetNodeId(uint32_t _nodeId);
    uint32_t GetNodeId();
    void SetPosition(uint32_t _position);
    uint32_t GetPosition();
    void SetCount(int32_t _count);
    int32_t GetCount();

    uint32_t Hash();

    bool operator == (const CountHeader& o);

    uint32_t m_nodeId;
    uint32_t m_position;
    int32_t m_count;
};

bool operator < (const CountHeader& a, const CountHeader& b);

} // namespace ns3

#endif /* COUNT_HEADER_H */