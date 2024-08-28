#ifndef TIMESTAMP_TAG_H
#define TIMESTAMP_TAG_H

#include "ns3/tag.h"
#include "ns3/nstime.h"

namespace ns3
{

/**
 * Timestamp tag - it carries when the packet has been sent.
 *
 * It would have been more realistic to include this info in
 * a header. Here we show how to avoid the extra overhead in
 * a simulation.
 */
class TimestampTag : public Tag
{
  public:
    /**
     * \brief Get the type ID.
     * \return The object TypeId.
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;

    /**
     * Set the timestamp.
     * \param time The timestamp.
     */
    void SetTimestamp(Time time);
    /**
     * Get the timestamp.
     * \return the timestamp.
     */
    Time GetTimestamp() const;

    void Print(std::ostream& os) const override;

  private:
    Time m_timestamp; //!< Timestamp.

    // end class TimestampTag
};

} // namespace ns3

#endif /* TIMESTAMP_TAG_H */