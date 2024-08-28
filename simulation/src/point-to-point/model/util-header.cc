#include "util-header.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/header.h"
#include "ns3/log.h"

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UtilHeader");

NS_OBJECT_ENSURE_REGISTERED(UtilHeader);

UtilHeader::UtilHeader()
{
    m_nodeId = 0;
    m_portId = 0;
    m_time = 0;
    m_byte = 0;
}

UtilHeader::UtilHeader(const UtilHeader& o)
{
    m_nodeId = o.m_nodeId;
    m_portId = o.m_portId;
    m_time = o.m_time;
    m_byte = o.m_byte;
}

UtilHeader::~UtilHeader()
{
}

TypeId
UtilHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::UtilHeader")
                            .SetParent<Header>()
                            .SetGroupName("PointToPoint")
                            .AddConstructor<UtilHeader>();
    return tid;
}

TypeId
UtilHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
UtilHeader::Print(std::ostream& os) const
{
}

uint32_t
UtilHeader::GetSerializedSize() const
{
    return 16;
}

void
UtilHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU32(m_nodeId);
    start.WriteHtonU32(m_portId);
    start.WriteHtonU32(m_time);
    start.WriteHtonU32(m_byte);
}

uint32_t
UtilHeader::Deserialize(Buffer::Iterator start)
{
    m_nodeId = start.ReadNtohU32();
    m_portId = start.ReadNtohU32();
    m_time = start.ReadNtohU32();
    m_byte = start.ReadNtohU32();
    return GetSerializedSize();
}

bool 
UtilHeader::operator == (const UtilHeader& o)
{
    return (m_nodeId == o.m_nodeId) && (m_portId == o.m_portId) && 
            (m_time == o.m_time) && (m_byte == o.m_byte);
}

bool 
UtilHeader::Empty()
{
    return (m_nodeId == 0) && (m_portId == 0) &&
            (m_time == 0) && (m_byte == 0);
}

uint32_t
UtilHeader::Hash()
{
    Hasher hasher;
    hasher.GetHash32((char*)(&m_nodeId), 4);
    hasher.GetHash32((char*)(&m_portId), 4);
    hasher.GetHash32((char*)(&m_time), 4);
    return hasher.GetHash32((char*)(&m_byte), 4);
}

void 
UtilHeader::SetNodeId(uint32_t _nodeId)
{
    m_nodeId = _nodeId;
}

uint32_t 
UtilHeader::GetNodeId()
{
    return m_nodeId;
}

void 
UtilHeader::SetPortId(uint32_t _portId)
{
    m_portId = _portId;
}

uint32_t 
UtilHeader::GetPortId()
{
    return m_portId;
}

void 
UtilHeader::SetTime(uint32_t _time)
{
    m_time = _time;
}

uint32_t 
UtilHeader::GetTime()
{
    return m_time;
}


void 
UtilHeader::SetByte(uint32_t _byte)
{
    m_byte = _byte;
}

uint32_t 
UtilHeader::GetByte()
{
    return m_byte;
}


bool operator < (const UtilHeader& a, const UtilHeader& b)
{
    return std::tie(a.m_nodeId, a.m_portId, a.m_time, a.m_byte) <
          std::tie(b.m_nodeId, b.m_portId, b.m_time, b.m_byte);
}

} // namespace ns3