#include "tele-header.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/header.h"
#include "ns3/log.h"

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TeleHeader");

NS_OBJECT_ENSURE_REGISTERED(TeleHeader);

TeleHeader::TeleHeader()
{
    m_type = 0;
    m_dest = 0;
    m_size = 0;
}

TeleHeader::TeleHeader(const TeleHeader& o)
{
    m_type = o.m_type;
    m_dest = o.m_dest;
    m_size = o.m_size;
}

TeleHeader::~TeleHeader()
{
}

TypeId
TeleHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TeleHeader")
                            .SetParent<Header>()
                            .SetGroupName("PointToPoint")
                            .AddConstructor<TeleHeader>();
    return tid;
}

TypeId
TeleHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
TeleHeader::Print(std::ostream& os) const
{
}

uint32_t
TeleHeader::GetSerializedSize() const
{
    return 6;
}

void
TeleHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteU8(m_type);
    start.WriteU8(m_dest);
    start.WriteHtonU32(m_size);
}

uint32_t
TeleHeader::Deserialize(Buffer::Iterator start)
{
    m_type = start.ReadU8();
    m_dest = start.ReadU8();
    m_size = start.ReadNtohU32();
    return GetSerializedSize();
}

void 
TeleHeader::SetType(int8_t _type)
{
    m_type = _type;
}

int8_t
TeleHeader::GetType()
{
    return m_type;
}

void 
TeleHeader::SetDest(uint8_t _dest)
{
    m_dest = _dest;
}

uint8_t 
TeleHeader::GetDest()
{
    return m_dest;
}

void 
TeleHeader::SetSize(uint32_t _size)
{
    m_size = _size;
}

uint32_t
TeleHeader::GetSize()
{
    return m_size;
}

} // namespace ns3