#include "timestamp-tag.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/tag.h"
#include "ns3/nstime.h"
#include "ns3/log.h"

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TimestampTag");

NS_OBJECT_ENSURE_REGISTERED(TimestampTag);


TypeId
TimestampTag::GetTypeId()
{
    static TypeId tid = TypeId("TimestampTag")
                            .SetParent<Tag>()
                            .AddConstructor<TimestampTag>()
                            .AddAttribute("Timestamp",
                                          "Some momentous point in time!",
                                          EmptyAttributeValue(),
                                          MakeTimeAccessor(&TimestampTag::GetTimestamp),
                                          MakeTimeChecker());
    return tid;
}

TypeId
TimestampTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
TimestampTag::GetSerializedSize() const
{
    return 8;
}

void
TimestampTag::Serialize(TagBuffer i) const
{
    int64_t t = m_timestamp.GetNanoSeconds();
    i.Write((const uint8_t*)&t, 8);
}

void
TimestampTag::Deserialize(TagBuffer i)
{
    int64_t t;
    i.Read((uint8_t*)&t, 8);
    m_timestamp = NanoSeconds(t);
}

void
TimestampTag::SetTimestamp(Time time)
{
    m_timestamp = time;
}

Time
TimestampTag::GetTimestamp() const
{
    return m_timestamp;
}

void
TimestampTag::Print(std::ostream& os) const
{
    os << "t=" << m_timestamp;
}

} // namespace ns3