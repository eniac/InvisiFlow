#ifndef TELE_HEADER_H
#define TELE_HEADER_H

#include "ns3/header.h"

namespace ns3
{

class TeleHeader : public Header
{
public:

    TeleHeader();
    TeleHeader(const TeleHeader& TeleHeader);
    ~TeleHeader() override;

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    uint32_t GetSerializedSize() const override;

    void SetType(int8_t _type);
    int8_t GetType();
    void SetDest(uint8_t _dest);
    uint8_t GetDest();
    void SetSize(uint32_t _size);
    uint32_t GetSize();

    int8_t m_type;
    uint8_t m_dest;
    uint32_t m_size;
};

} // namespace ns3

#endif /* TELE_HEADER_H */