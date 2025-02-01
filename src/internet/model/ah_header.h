#ifndef AH_HEADER_H
#define AH_HEADER_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

#include <stdint.h>
#include <string>

#include "ns3/header.h"
#include <vector>
#include <cstdint>

namespace ns3
{
/**
 * \ingroup ah
 * \brief Packet header for AH packets
 *
 * This class has fields corresponding to those in a network AH header
 * (next header, length, reserved) as well as methods for serialization
 * to and deserialization from a byte buffer.
 */
 
class AHHeader : public Header
{
  public:
  
    static uint32_t m_icvSize;
    /**
     * \param num the ah protocol field
     */
    void SetProtocol(uint8_t protocol);
    /**
     * \returns the protocol field of this packet
     */
    uint8_t GetProtocol() const;
    /**
     * \param length the length of the ah header
     */
    void SetLengthAH(uint8_t length);
    /**
     * \returns the length of the ah header in bytes
     */
    uint8_t GetLengthAH() const;
    /**
     * \param size the size of the payload in bytes
     */
    void SetReserved(uint16_t reserved);
    /**
     * \returns the size of the payload in bytes
     */
    uint16_t GetReserved() const;
    
    void SetSpi(uint32_t spi);
    
    uint32_t GetSpi() const;

    void SetSequenceNumber(uint32_t seqNum);
    
    uint32_t GetSequenceNumber() const;
    
    static void SetIcvSize(uint32_t icvSize);
    
    static uint32_t GetIcvSize();
    
  // Postavi ICV vrijednost
  void SetIcv(const std::vector<uint8_t>& icv);
  
  // Vrati ICV vrijednost
  std::vector<uint8_t> GetIcv() const;

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;


  private:
    // The magic values below are used only for debugging.
    // They can be used to easily detect memory corruption
    // problems so you can see the patterns in memory.
    uint8_t m_lengthAH;          //!< AH size
    uint8_t m_protocol;     //!< Protocol number
    uint16_t m_reserved;     //!< Protocol number
    uint32_t m_spi;             // Security Parameters Index
    uint32_t m_sequenceNumber;  // AH Sequence Number
    std::vector<uint8_t> m_icv;  // Polje za ICV
    Address m_source;           //!< Source IP address
    Address m_destination;      //!< Destination IP address
    
};

} // namespace ns3

#endif /* AH_HEADER */
