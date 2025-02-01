/*
 * Copyright (c) 2005 INRIA
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
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "udp-l4-protocol.h"
#include "ipsec_ah.h"
#include "ah_header.h"
#include "ipv4-end-point-demux.h"
#include "ipv4-end-point.h"
#include "ipv4-route.h"
#include "ipv4.h"
#include "ipv6-end-point-demux.h"
#include "ipv6-end-point.h"
#include "ipv6-route.h"
#include "ipv6.h"
#include "udp-header.h"
#include "udp-socket-factory-impl.h"
#include "udp-socket-impl.h"
#include <cryptopp/sha.h>       // Za SHA-1, SHA-256, itd.
#include <cryptopp/md5.h>       // Za MD5
#include <cryptopp/hmac.h>      // Za HMAC
#include <cryptopp/hex.h>       // Za Hex kodiranje/decodiranje (opcionalno)
#include <cryptopp/filters.h>   // Za Filter klase
#include <cryptopp/secblock.h>  // Za sigurne blokove podataka


#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/object-map.h"
#include "ns3/packet.h"
#include "ns3/ah-tag.h"


#include <unordered_map>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("AHProtocol");

NS_OBJECT_ENSURE_REGISTERED(AHProtocol);

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t AHProtocol::PROT_NUMBER = 51;

TypeId
AHProtocol::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::AHProtocol")
            .SetParent<IpL4Protocol>()
            .SetGroupName("Internet")
            .AddConstructor<AHProtocol>();
    return tid;
}

uint32_t AHProtocol::m_sequenceNumber = 2;  

AHProtocol::AHProtocol()
    : m_endPoints(new Ipv4EndPointDemux()), m_spi(255)
{
    m_spiToHashAlgorithm[1] = "SHA-256";
    m_spiToHashAlgorithm[2] = "SHA-512";
    m_spiToHashAlgorithm[3] = "MD5";

    NS_LOG_FUNCTION(this);
}

AHProtocol::~AHProtocol()
{
    NS_LOG_FUNCTION(this);
}


void
AHProtocol::SetNode(Ptr<Node> node)
{
    m_node = node;
}

/*
 * This method is called by AggregateObject and completes the aggregation
 * by setting the node in the udp stack and link it to the ipv4 object
 * present in the node along with the socket factory
 */
void
AHProtocol::NotifyNewAggregate()
{
    NS_LOG_FUNCTION(this);
    Ptr<Node> node = this->GetObject<Node>();
    Ptr<Ipv4> ipv4 = this->GetObject<Ipv4>();

    if (ipv4 && m_downTarget.IsNull())
    {
        ipv4->Insert(this);
        this->SetDownTarget(MakeCallback(&Ipv4::Send, ipv4));
    }

    IpL4Protocol::NotifyNewAggregate();
}


int
AHProtocol::GetProtocolNumber() const
{
    return PROT_NUMBER;
}

void
AHProtocol::DoDispose()
{
    NS_LOG_FUNCTION(this);

    IpL4Protocol::DoDispose();
}


Ptr<IpL4Protocol>
AHProtocol::GetProtocol(uint8_t nextHeaderProtocol, Ptr<Ipv4Interface> interface) const
{
    if (nextHeaderProtocol == 17) // UDP
    {
        // Provjeravamo postoji li već UdpL4Protocol u čvoru
        Ptr<UdpL4Protocol> udpProtocol = this->GetObject<UdpL4Protocol>();
        if (udpProtocol)
        {
            return udpProtocol; // Vraćamo postojeću instancu
        }
        else
        {
            NS_LOG_ERROR("UdpL4Protocol nije pronađen na čvoru.");
            return nullptr;
        }
    }

    NS_LOG_ERROR("Nepoznat protokol: " << nextHeaderProtocol);
    return nullptr;
}


IpL4Protocol::RxStatus AHProtocol::Receive(Ptr<Packet> packet, const Ipv4Header& header, Ptr<Ipv4Interface> interface)
{
    NS_LOG_FUNCTION(this << packet << header);
    AHHeader ahHeader;

    packet->RemoveHeader(ahHeader);

    // Dohvati SPI vrednost iz AH zaglavlja
    uint32_t spi = ahHeader.GetSpi();
    std::vector<uint8_t> originalIcv = ahHeader.GetIcv();

    NS_LOG_INFO("SPI vrednost: " << spi);

    // Dohvati algoritam hashiranja na osnovu SPI vrednosti
    std::string hashAlgorithm = "SHA-256";  // Podrazumjevano
    auto it = m_spiToHashAlgorithm.find(spi);
    if (it != m_spiToHashAlgorithm.end()) {
        hashAlgorithm = it->second;
    }
    NS_LOG_INFO("Koristi se hash algoritam: " << hashAlgorithm);

    // Izračunaj očekivani ICV
    uint8_t* buffer = new uint8_t[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    NS_LOG_INFO("Velicina paketa na prijemnoj strani:" << packet->GetSize());

    std::vector<uint8_t> calculatedIcv;
    if (hashAlgorithm == "SHA-256") {
        calculatedIcv.resize(32);
        CryptoPP::SHA256 hash;
        hash.CalculateDigest(calculatedIcv.data(), buffer, packet->GetSize());
    } else if (hashAlgorithm == "SHA-512") {
        calculatedIcv.resize(64);
        CryptoPP::SHA512 hash;
        hash.CalculateDigest(calculatedIcv.data(), buffer, packet->GetSize());
    } else if (hashAlgorithm == "MD5") {
       calculatedIcv.resize(16); // MD5 izlaz (ne preporučuje se)
       CryptoPP::MD5 hash;
       hash.CalculateDigest(calculatedIcv.data(), buffer, packet->GetSize()); 
    } else {
        NS_LOG_ERROR("Nepoznati hash algoritam! Validacija ne može biti izvršena.");
        delete[] buffer;
        return IpL4Protocol::RX_ENDPOINT_UNREACH;
    }

    delete[] buffer;

    // Provjera da li se ICV podudara
    if (originalIcv == calculatedIcv) {
        NS_LOG_INFO("ICV je validan. Paket je autentičan.");
    } else {
        NS_LOG_ERROR("ICV nije validan! Paket je kompromitovan.");
        return IpL4Protocol::RX_ENDPOINT_UNREACH;
    }

    uint8_t nextHeaderProtocol = ahHeader.GetProtocol();

    Ptr<IpL4Protocol> protocol = GetProtocol(nextHeaderProtocol, interface);
    if (!protocol) {
        NS_LOG_ERROR("Protokol nije pronađen za NextHeader: " << static_cast<int>(nextHeaderProtocol));
        return IpL4Protocol::RX_ENDPOINT_UNREACH;
    }

    NS_LOG_FUNCTION("Imamo protokol: " << protocol);
    Ptr<Packet> copy = packet->Copy();
    protocol->Receive(packet, header, interface);

    return IpL4Protocol::RX_OK;
}


IpL4Protocol::RxStatus
AHProtocol::Receive(Ptr<Packet> packet, const Ipv6Header& header, Ptr<Ipv6Interface> interface)
{
    AHHeader ahHeader;

    packet->RemoveHeader(ahHeader);

    return IpL4Protocol::RX_OK;
}

/*void
AHProtocol::Send(Ptr<Packet> packet,
                 Ipv4Address saddr,
                 Ipv4Address daddr,
                 uint8_t protocol)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr << protocol);

    // Dodavanje AH zaglavlja
    AHHeader ahHeader;
    ahHeader.SetProtocol(protocol); // Postavljanje protokola za identifikaciju
    packet->AddHeader(ahHeader);

    // Prosljeđivanje paketa IPv4 sloju putem m_downTarget
    m_downTarget(packet, saddr, daddr, PROT_NUMBER, nullptr);
}*/



void AHProtocol::Send(Ptr<Packet> packet,
                      Ipv4Address saddr,
                      Ipv4Address daddr,
                      uint8_t protocol,
                      Ptr<Ipv4Route> route)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr << protocol << route);

    // Dodavanje AH zaglavlja
    AhTag tag;
    if (packet->PeekPacketTag(tag)) { // Provera da li paket ima AH tag
    if (tag.GetFlag()) {
        NS_LOG_INFO("Imamo tag, započinjemo agregaciju AH protokola.");
    }
    }
    AHHeader ahHeader;
    ahHeader.SetProtocol(protocol); // Postavljanje protokola za identifikaciju
    ahHeader.SetReserved(0); // Primer za postavljanje polja na nulu
    ahHeader.SetSpi(3);  // Koristi SPI iz inicijalizacije ili SA mape
    ahHeader.SetSequenceNumber(m_sequenceNumber++); // Povećaj Sequence Number

    // Dohvati algoritam na osnovu SPI vrednosti
    std::string hashAlgorithm = "SHA-256";  // Podrazumjevano
    auto it = m_spiToHashAlgorithm.find(ahHeader.GetSpi());
    if (it != m_spiToHashAlgorithm.end()) {
       hashAlgorithm = it->second;
    }
    NS_LOG_INFO("Koristi se hash algoritam: " << hashAlgorithm);

    uint8_t* buffer = new uint8_t[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    NS_LOG_INFO("Velicina paketa na predajnoj strani:"<< packet->GetSize());
    // Generiši hash vrijednost koristeći SHA-256
    std::vector<uint8_t> icv;

    if (hashAlgorithm == "SHA-256") {
       icv.resize(32); // SHA-256 izlaz
       CryptoPP::SHA256 hash;
       hash.CalculateDigest(icv.data(), buffer, packet->GetSize());
    } else if (hashAlgorithm == "SHA-512") {
       icv.resize(64); // SHA-512 izlaz
       CryptoPP::SHA512 hash;
       hash.CalculateDigest(icv.data(), buffer, packet->GetSize());
    } else if (hashAlgorithm == "MD5") {
       icv.resize(16); // MD5 izlaz (ne preporučuje se)
       CryptoPP::MD5 hash;
       hash.CalculateDigest(icv.data(), buffer, packet->GetSize()); 
    } else {
       NS_LOG_ERROR("Nepoznati hash algoritam!"); 
}
   NS_LOG_ERROR("Velicina icv"<<icv.size()); 
    ahHeader.SetIcvSize(icv.size());
     ahHeader.SetLengthAH(((12+icv.size())/4)-2);
  ahHeader.SetIcv(icv);

  delete[] buffer;


    // Dodaj AH zaglavlje u paket
    packet->AddHeader(ahHeader);
    NS_LOG_DEBUG("Packet size after adding AH header: " << packet->GetSize());
    
    // Prosljeđivanje paketa IPv4 sloju putem m_downTarget
    m_downTarget(packet, saddr, daddr, PROT_NUMBER, route);
}



void
AHProtocol::SetDownTarget(IpL4Protocol::DownTargetCallback callback)
{
    NS_LOG_FUNCTION(this);
    m_downTarget = callback;
}

IpL4Protocol::DownTargetCallback
AHProtocol::GetDownTarget() const
{
    return m_downTarget;
}

void
AHProtocol::SetDownTarget6(IpL4Protocol::DownTargetCallback6 callback)
{
    NS_LOG_FUNCTION(this);
    m_downTarget6 = callback;
}

IpL4Protocol::DownTargetCallback6
AHProtocol::GetDownTarget6() const
{
    return m_downTarget6;
}


} // namespace ns3
