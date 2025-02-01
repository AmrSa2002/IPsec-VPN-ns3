#ifndef AH_TAG_H
#define AH_TAG_H

#include "ns3/tag.h"
#include "ns3/nstime.h"

namespace ns3 {

class AhTag : public Tag
{
public:
    AhTag();
    
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    
    void Serialize (TagBuffer i) const override;
    void Deserialize (TagBuffer i) override;
    uint32_t GetSerializedSize () const override;
    void Print (std::ostream &os) const override;
    
    void SetFlag (bool flag);
    bool GetFlag () const;

private:
    bool m_flag;  // Ako je true, paket ide na AH
};

} // namespace ns3

#endif /* AH_TAG_H */

