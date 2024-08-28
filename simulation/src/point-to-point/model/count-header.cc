#include "count-header.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/header.h"
#include "ns3/log.h"

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CountHeader");

NS_OBJECT_ENSURE_REGISTERED(CountHeader);


CMSketch::CMSketch()
{
    memset(values, 0, sizeof(int32_t) * OURS_SKETCH_HASH * OURS_SKETCH_LENGTH);
}

MyFlowId::MyFlowId()
{
    m_srcIP = 0;
    m_dstIP = 0;
    m_srcPort = 0;
    m_dstPort = 0;
}

MyFlowId::MyFlowId(const MyFlowId& o)
{
    m_srcIP = o.m_srcIP;
    m_dstIP = o.m_dstIP;
    m_srcPort = o.m_srcPort;
    m_dstPort = o.m_dstPort;
}

bool operator == (const MyFlowId& a, const MyFlowId& b){
    return (a.m_srcIP == b.m_srcIP) && (a.m_dstIP == b.m_dstIP) &&
        (a.m_srcPort == b.m_srcPort) && (a.m_dstPort == b.m_dstPort);
}

bool operator < (const MyFlowId& a, const MyFlowId& b){
    return std::tie(a.m_srcIP, a.m_dstIP, a.m_srcPort, a.m_dstPort) <
          std::tie(b.m_srcIP, b.m_dstIP, b.m_srcPort, b.m_dstPort);
}

CountHeader::CountHeader()
{
    m_nodeId = 0;
    m_position = 0;
    m_count = 0;
}

CountHeader::CountHeader(const CountHeader& o)
{
    m_nodeId = o.m_nodeId;
    m_position = o.m_position;
    m_count = o.m_count;
}

CountHeader::~CountHeader()
{
}

TypeId
CountHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CountHeader")
                            .SetParent<Header>()
                            .SetGroupName("PointToPoint")
                            .AddConstructor<CountHeader>();
    return tid;
}

TypeId
CountHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
CountHeader::Print(std::ostream& os) const
{
}

uint32_t
CountHeader::GetSerializedSize() const
{
    return 12;
}

void
CountHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU32(m_nodeId);
    start.WriteHtonU32(m_position);
    start.WriteHtonU32(m_count);
}

uint32_t
CountHeader::Deserialize(Buffer::Iterator start)
{
    m_nodeId = start.ReadNtohU32();
    m_position = start.ReadNtohU32();
    m_count = start.ReadNtohU32();
    return GetSerializedSize();
}

bool 
CountHeader::operator == (const CountHeader& o)
{
    return (m_nodeId == o.m_nodeId) && (m_position == o.m_position) &&
        (m_count == o.m_count);
}

bool 
CountHeader::Empty()
{
    return (m_nodeId == 0) && (m_position == 0) &&
        (m_count == 0);
}

uint32_t
CountHeader::Hash()
{
    Hasher hasher;
    hasher.GetHash32((char*)(&m_nodeId), 4);
    hasher.GetHash32((char*)(&m_position), 4);
    return hasher.GetHash32((char*)(&m_count), 4);
}

void 
CountHeader::SetNodeId(uint32_t _nodeId)
{
    m_nodeId = _nodeId;
}

uint32_t 
CountHeader::GetNodeId()
{
    return m_nodeId;
}

void 
CountHeader::SetPosition(uint32_t _position)
{
    m_position = _position;
}

uint32_t 
CountHeader::GetPosition()
{
    return m_position;
}


void 
CountHeader::SetCount(int32_t _count)
{
    m_count = _count;
}

int32_t 
CountHeader::GetCount()
{
    return m_count;
}

bool operator < (const CountHeader& a, const CountHeader& b)
{
    return std::tie(a.m_nodeId, a.m_position, a.m_count) <
          std::tie(b.m_nodeId, b.m_position, b.m_count);
}

} // namespace ns3