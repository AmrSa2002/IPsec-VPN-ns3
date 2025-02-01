#include "ah-tag.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("AhTagApp");

namespace ns3 {

AhTag::AhTag() : m_flag(false) {}

TypeId AhTag::GetTypeId() {
    static TypeId tid = TypeId ("ns3::AhTag")
        .SetParent<Tag>()
        .AddConstructor<AhTag>();
    return tid;
}

TypeId AhTag::GetInstanceTypeId() const {
    return GetTypeId();
}

void AhTag::Serialize(TagBuffer i) const {
    i.WriteU8(m_flag ? 1 : 0);
}

void AhTag::Deserialize(TagBuffer i) {
    m_flag = (i.ReadU8() == 1);
}

uint32_t AhTag::GetSerializedSize() const {
    return 1;
}

void AhTag::Print(std::ostream &os) const {
    os << "AH Flag=" << (m_flag ? "True" : "False");
}

void AhTag::SetFlag(bool flag) {
    m_flag = flag;
}

bool AhTag::GetFlag() const {
    return m_flag;
}

} // namespace ns3

