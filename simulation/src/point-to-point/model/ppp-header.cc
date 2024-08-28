/*
 * Copyright (c) 2008 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ppp-header.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/header.h"
#include "ns3/log.h"

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("PppHeader");

NS_OBJECT_ENSURE_REGISTERED(PppHeader);

PppHeader::PppHeader()
{
}

PppHeader::~PppHeader()
{
}

TypeId
PppHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PppHeader")
                            .SetParent<Header>()
                            .SetGroupName("PointToPoint")
                            .AddConstructor<PppHeader>();
    return tid;
}

TypeId
PppHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
PppHeader::Print(std::ostream& os) const
{
    std::string proto;

    switch (m_protocol)
    {
    case 0x0021: /* IPv4 */
        proto = "IP (0x0021)";
        break;
    case 0x0057: /* IPv6 */
        proto = "IPv6 (0x0057)";
        break;
    case 0x0170:
        proto = "Telemetry SEED (0x0170)";
        break;
    case 0x0171:
        proto = "Telemetry PUSH (0x0171)";
        break;
    case 0x0172:
        proto = "Telemetry PULL (0x0172)";
        break;
    default:
        NS_ASSERT_MSG(false, "PPP Protocol number not defined!");
    }
    os << "Point-to-Point Protocol: " << proto;
}

uint32_t
PppHeader::GetSerializedSize() const
{
    return 14 + m_padding;
}

void
PppHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU32(m_source);
    start.WriteHtonU32(m_destination);
    start.WriteHtonU16(m_empty);
    start.WriteHtonU16(m_protocol);
    start.WriteHtonU16(m_padding);
    for(uint32_t i = 0;i < m_padding;++i)
        start.WriteU8(0);
}

uint32_t
PppHeader::Deserialize(Buffer::Iterator start)
{
    m_source = start.ReadNtohU32();
    m_destination = start.ReadNtohU32();
    m_empty = start.ReadNtohU16();
    m_protocol = start.ReadNtohU16();
    m_padding = start.ReadNtohU16();
    for(uint32_t i = 0;i < m_padding;++i)
        start.ReadU8();
    return GetSerializedSize();
}

void
PppHeader::SetProtocol(uint16_t protocol)
{
    m_protocol = protocol;
}

uint16_t
PppHeader::GetProtocol()
{
    return m_protocol;
}

void
PppHeader::SetPadding(uint8_t padding)
{
    m_padding = padding;
}

uint8_t
PppHeader::GetPadding()
{
    return m_padding;
}

} // namespace ns3
