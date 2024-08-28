#include "path-header.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/header.h"
#include "ns3/log.h"

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("PathHeader");

NS_OBJECT_ENSURE_REGISTERED(PathHeader);

PathHeader::PathHeader()
{
    m_srcIP = 0;
    m_dstIP = 0;
    m_srcPort = 0;
    m_dstPort = 0;
    m_nodeId = 0;
    m_protocol = 0;
    m_ttl = 0;
}

PathHeader::PathHeader(const PathHeader& o)
{
    m_srcIP = o.m_srcIP;
    m_dstIP = o.m_dstIP;
    m_srcPort = o.m_srcPort;
    m_dstPort = o.m_dstPort;
    m_nodeId = o.m_nodeId;
    m_protocol = o.m_protocol;
    m_ttl = o.m_ttl;
}

PathHeader::~PathHeader()
{
}

TypeId
PathHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PathHeader")
                            .SetParent<Header>()
                            .SetGroupName("PointToPoint")
                            .AddConstructor<PathHeader>();
    return tid;
}

TypeId
PathHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
PathHeader::Print(std::ostream& os) const
{
}

uint32_t
PathHeader::GetSerializedSize() const
{
    return 18;
}

void
PathHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU32(m_srcIP);
    start.WriteHtonU32(m_dstIP);
    start.WriteHtonU16(m_srcPort);
    start.WriteHtonU16(m_dstPort);
    start.WriteHtonU32(m_nodeId);
    start.WriteU8(m_protocol);
    start.WriteU8(m_ttl);
}

uint32_t
PathHeader::Deserialize(Buffer::Iterator start)
{
    m_srcIP = start.ReadNtohU32();
    m_dstIP = start.ReadNtohU32();
    m_srcPort = start.ReadNtohU16();
    m_dstPort = start.ReadNtohU16();
    m_nodeId = start.ReadNtohU32();
    m_protocol = start.ReadU8();
    m_ttl = start.ReadU8();
    return GetSerializedSize();
}

bool 
PathHeader::operator == (const PathHeader& o)
{
    return (m_srcIP == o.m_srcIP) && (m_dstIP == o.m_dstIP) &&
        (m_srcPort == o.m_srcPort) && (m_dstPort == o.m_dstPort) &&
        (m_nodeId == o.m_nodeId) && (m_protocol == o.m_protocol) && (m_ttl == o.m_ttl);
}

bool 
PathHeader::Empty()
{
    return (m_srcIP == 0) && (m_dstIP == 0) &&
        (m_srcPort == 0) && (m_dstPort == 0) &&
        (m_nodeId == 0) && (m_protocol == 0) && (m_ttl == 0);
}

uint32_t
PathHeader::Hash()
{
    Hasher hasher;
    hasher.GetHash32((char*)(&m_srcIP), 4);
    hasher.GetHash32((char*)(&m_dstIP), 4);
    hasher.GetHash32((char*)(&m_srcPort), 2);
    hasher.GetHash32((char*)(&m_dstPort), 2);
    hasher.GetHash32((char*)(&m_nodeId), 4);
    hasher.GetHash32((char*)(&m_protocol), 1);
    return hasher.GetHash32((char*)(&m_ttl), 1);
}

void 
PathHeader::SetSrcIP(uint32_t _srcIP)
{
    m_srcIP = _srcIP;
}

uint32_t 
PathHeader::GetSrcIP()
{
    return m_srcIP;
}

void 
PathHeader::SetDstIP(uint32_t _dstIP)
{
    m_dstIP = _dstIP;
}

uint32_t 
PathHeader::GetDstIP()
{
    return m_dstIP;
}

void 
PathHeader::SetSrcPort(uint16_t _srcPort)
{
    m_srcPort = _srcPort;
}

uint16_t 
PathHeader::GetSrcPort()
{
    return m_srcPort;
}

void 
PathHeader::SetDstPort(uint16_t _dstPort)
{
    m_dstPort = _dstPort;
}

uint16_t 
PathHeader::GetDstPort()
{
    return m_dstPort;
}

void 
PathHeader::SetNodeId(uint32_t _nodeId)
{
    m_nodeId = _nodeId;
}

uint32_t 
PathHeader::GetNodeId()
{
    return m_nodeId;
}

void 
PathHeader::SetProtocol(uint8_t _protocol)
{
    m_protocol = _protocol;
}

uint8_t 
PathHeader::GetProtocol()
{
    return m_protocol;
}

void 
PathHeader::SetTTL(uint8_t _ttl)
{
    m_ttl = _ttl;
}

uint8_t 
PathHeader::GetTTL()
{
    return m_ttl;
}

bool operator < (const PathHeader& a, const PathHeader& b)
{
    return std::tie(a.m_srcIP, a.m_dstIP, a.m_srcPort, a.m_dstPort, a.m_nodeId, a.m_protocol, a.m_ttl) <
          std::tie(b.m_srcIP, b.m_dstIP, b.m_srcPort, b.m_dstPort, b.m_nodeId, b.m_protocol, b.m_ttl);
}

} // namespace ns3