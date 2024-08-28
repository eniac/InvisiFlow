#include "drop-header.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/header.h"
#include "ns3/log.h"

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DropHeader");

NS_OBJECT_ENSURE_REGISTERED(DropHeader);

DropHeader::DropHeader()
{
    m_srcIP = 0;
    m_dstIP = 0;
    m_srcPort = 0;
    m_dstPort = 0;
    m_nodeId = 0;
    m_time = 0;
    m_protocol = 0;
}

DropHeader::DropHeader(const DropHeader& o)
{
    m_srcIP = o.m_srcIP;
    m_dstIP = o.m_dstIP;
    m_srcPort = o.m_srcPort;
    m_dstPort = o.m_dstPort;
    m_nodeId = o.m_nodeId;
    m_time = o.m_time;
    m_protocol = o.m_protocol;
}

DropHeader::~DropHeader()
{
}

TypeId
DropHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::DropHeader")
                            .SetParent<Header>()
                            .SetGroupName("PointToPoint")
                            .AddConstructor<DropHeader>();
    return tid;
}

TypeId
DropHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
DropHeader::Print(std::ostream& os) const
{
}

uint32_t
DropHeader::GetSerializedSize() const
{
    return 21;
}

void
DropHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU32(m_srcIP);
    start.WriteHtonU32(m_dstIP);
    start.WriteHtonU16(m_srcPort);
    start.WriteHtonU16(m_dstPort);
    start.WriteHtonU32(m_nodeId);
    start.WriteHtonU32(m_time);
    start.WriteU8(m_protocol);
}

uint32_t
DropHeader::Deserialize(Buffer::Iterator start)
{
    m_srcIP = start.ReadNtohU32();
    m_dstIP = start.ReadNtohU32();
    m_srcPort = start.ReadNtohU16();
    m_dstPort = start.ReadNtohU16();
    m_nodeId = start.ReadNtohU32();
    m_time = start.ReadNtohU32();
    m_protocol = start.ReadU8();
    return GetSerializedSize();
}

bool 
DropHeader::operator == (const DropHeader& o)
{
    return (m_srcIP == o.m_srcIP) && (m_dstIP == o.m_dstIP) &&
        (m_srcPort == o.m_srcPort) && (m_dstPort == o.m_dstPort) &&
        (m_nodeId == o.m_nodeId) && (m_time == o.m_time) && 
        (m_protocol == o.m_protocol);
}

bool 
DropHeader::Empty()
{
    return (m_srcIP == 0) && (m_dstIP == 0) &&
        (m_srcPort == 0) && (m_dstPort == 0) &&
        (m_nodeId == 0) && (m_time == 0) && 
        (m_protocol == 0);
}

uint32_t
DropHeader::Hash()
{
    Hasher hasher;
    hasher.GetHash32((char*)(&m_srcIP), 4);
    hasher.GetHash32((char*)(&m_dstIP), 4);
    hasher.GetHash32((char*)(&m_srcPort), 2);
    hasher.GetHash32((char*)(&m_dstPort), 2);
    hasher.GetHash32((char*)(&m_nodeId), 4);
    hasher.GetHash32((char*)(&m_time), 4);
    return hasher.GetHash32((char*)(&m_protocol), 1);
}

void 
DropHeader::SetSrcIP(uint32_t _srcIP)
{
    m_srcIP = _srcIP;
}

uint32_t 
DropHeader::GetSrcIP()
{
    return m_srcIP;
}

void 
DropHeader::SetDstIP(uint32_t _dstIP)
{
    m_dstIP = _dstIP;
}

uint32_t 
DropHeader::GetDstIP()
{
    return m_dstIP;
}

void 
DropHeader::SetSrcPort(uint16_t _srcPort)
{
    m_srcPort = _srcPort;
}

uint16_t 
DropHeader::GetSrcPort()
{
    return m_srcPort;
}

void 
DropHeader::SetDstPort(uint16_t _dstPort)
{
    m_dstPort = _dstPort;
}

uint16_t 
DropHeader::GetDstPort()
{
    return m_dstPort;
}

void 
DropHeader::SetNodeId(uint32_t _nodeId)
{
    m_nodeId = _nodeId;
}

uint32_t 
DropHeader::GetNodeId()
{
    return m_nodeId;
}

void 
DropHeader::SetTime(uint32_t _time)
{
    m_time = _time;
}

uint32_t 
DropHeader::GetTime()
{
    return m_time;
}

void 
DropHeader::SetProtocol(uint8_t _protocol)
{
    m_protocol = _protocol;
}

uint8_t 
DropHeader::GetProtocol()
{
    return m_protocol;
}

bool operator < (const DropHeader& a, const DropHeader& b)
{
    return std::tie(a.m_srcIP, a.m_dstIP, a.m_srcPort, a.m_dstPort, a.m_nodeId, a.m_time, a.m_protocol) <
          std::tie(b.m_srcIP, b.m_dstIP, b.m_srcPort, b.m_dstPort, b.m_nodeId, b.m_time, b.m_protocol);
}

} // namespace ns3