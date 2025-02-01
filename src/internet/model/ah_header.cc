#include "ah_header.h"

#include "ns3/address-utils.h"
#include "ah_header.h"
#include "ns3/buffer.h"


namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(AHHeader);
NS_LOG_COMPONENT_DEFINE ("AHHeader");

uint32_t AHHeader::m_icvSize = 32;

void
AHHeader::SetLengthAH(uint8_t length)
{
    NS_LOG_FUNCTION (this << length);
    m_lengthAH = length;
}


uint8_t
AHHeader::GetLengthAH() const
{
    return m_lengthAH;
}

uint8_t
AHHeader::GetProtocol() const
{
    return m_protocol;
}

void
AHHeader::SetProtocol(uint8_t protocol)
{
    NS_LOG_FUNCTION (this << protocol);
    m_protocol = protocol;
}

uint16_t
AHHeader::GetReserved() const
{
    return m_reserved;
}

void
AHHeader::SetReserved(uint16_t reserved)
{
    NS_LOG_FUNCTION (this << reserved);
    m_reserved = reserved;
}

void 
AHHeader::SetSpi(uint32_t spi) {
  m_spi = spi;
}

uint32_t AHHeader::GetSpi() const {
  return m_spi;
}

void AHHeader::SetSequenceNumber(uint32_t seqNum) {
  m_sequenceNumber = seqNum;
}

uint32_t AHHeader::GetSequenceNumber() const {
  return m_sequenceNumber;
}

void AHHeader::SetIcvSize(uint32_t icvSize) {
  m_icvSize = icvSize;
}

uint32_t AHHeader::GetIcvSize() {
  return m_icvSize;
}


void AHHeader::SetIcv(const std::vector<uint8_t>& icv) {
  m_icv = icv;
}

std::vector<uint8_t> AHHeader::GetIcv() const {
  return m_icv;
}



TypeId
AHHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::AHHeader")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<AHHeader>();
    return tid;
}

TypeId
AHHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
AHHeader::Print(std::ostream& os) const
{
    os << "length: " << m_lengthAH + GetSerializedSize() << " " << m_protocol << " > "
       << m_reserved;
}

uint32_t
AHHeader::GetSerializedSize() const
{
uint32_t m_temp=12+GetIcvSize();
NS_LOG_FUNCTION (this<<m_temp);
    return m_temp;
}

void
AHHeader::Serialize(Buffer::Iterator start) const
{
NS_LOG_FUNCTION (this);
    Buffer::Iterator i = start;
    i.WriteU8(m_protocol);
    i.WriteU8(m_lengthAH);
    i.WriteHtonU16(m_reserved);
    i.WriteHtonU32(m_spi);             
    i.WriteHtonU32(m_sequenceNumber); 
    for (uint8_t byte : m_icv) {
    i.WriteU8(byte);
  }

}

uint32_t
AHHeader::Deserialize(Buffer::Iterator start)
{
NS_LOG_FUNCTION (this);
uint32_t m_temp=GetIcvSize();
NS_LOG_FUNCTION ("m_icvSize"<<m_temp);
    Buffer::Iterator i = start;
    m_protocol = i.ReadU8();
    m_lengthAH = i.ReadU8();
    m_reserved = i.ReadNtohU16();
    m_spi = i.ReadNtohU32();          
    m_sequenceNumber = i.ReadNtohU32(); 
    m_icv.clear();
  for (uint32_t k = 0; k < m_temp; ++k) {
    m_icv.push_back(i.ReadU8());
  }
    return GetSerializedSize();
}

} // namespace ns3
