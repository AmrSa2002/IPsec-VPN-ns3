/*
 * Copyright (c) 2005,2006,2007 INRIA
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

#ifndef IPSEC_AH_PROTOCOL_H
#define IPSEC_AH_PROTOCOL_H

#include "ip-l4-protocol.h"

#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <stdint.h>
#include <unordered_map>

namespace ns3
{

class Node;
class Socket;
class Ipv4EndPointDemux;
class Ipv4EndPoint;
class Ipv6EndPointDemux;
class Ipv6EndPoint;
class UdpSocketImpl;
class NetDevice;

// Ovdje definiraj algoritme za hashiranje (SHA-256, SHA-1, MD5)
class AHProtocol : public IpL4Protocol
{
  public:
    static TypeId GetTypeId();
    static const uint8_t PROT_NUMBER; //!< protocol number (0x33)

    AHProtocol();
    ~AHProtocol() override;

    // Delete copy constructor and assignment operator to avoid misuse
    AHProtocol(const AHProtocol&) = delete;
    AHProtocol& operator=(const AHProtocol&) = delete;

    /**
     * Set node associated with this stack
     * \param node the node
     */
    void SetNode(Ptr<Node> node);

    int GetProtocolNumber() const override;

    /*void Send(Ptr<Packet> packet,
              Ipv4Address saddr,
              Ipv4Address daddr,
              uint8_t protocol);*/
    /**
     * \brief Send a packet via UDP (IPv4)
     * \param packet The packet to send
     * \param saddr The source Ipv4Address
     * \param daddr The destination Ipv4Address
     * \param sport The source port number
     * \param dport The destination port number
     * \param route The route
     */
              
    void Send(Ptr<Packet> packet,
                 Ipv4Address saddr,
                 Ipv4Address daddr,
                 uint8_t protocol,
                 Ptr<Ipv4Route> route);

    // inherited from Ipv4L4Protocol
    IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
                                   const Ipv4Header& header,
                                   Ptr<Ipv4Interface> interface) override;
    IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
                                   const Ipv6Header& header,
                                   Ptr<Ipv6Interface> interface) override;


    // From IpL4Protocol
    void SetDownTarget(IpL4Protocol::DownTargetCallback cb) override;
    void SetDownTarget6(IpL4Protocol::DownTargetCallback6 cb) override;
    // From IpL4Protocol
    IpL4Protocol::DownTargetCallback GetDownTarget() const override;
    IpL4Protocol::DownTargetCallback6 GetDownTarget6() const override;

  protected:
    void DoDispose() override;
    /*
     * This function will notify other components connected to the node that a new stack member is
     * now connected This will be used to notify Layer 3 protocol of layer 4 protocol stack to
     * connect them together.
     */
    void NotifyNewAggregate() override;

  private:
  Ipv4EndPointDemux* m_endPoints;  //!< A list of IPv4 end points.
  uint32_t m_spi;             // Security Parameters Index
  static uint32_t m_sequenceNumber;  // Sequence Number za AH protokol
    Ptr<Node> m_node;                //!< The node this stack is associated with
    std::unordered_map<uint32_t, std::string> m_spiToHashAlgorithm;
    Ipv6EndPointDemux* m_endPoints6; //!< A list of IPv6 end points.
    Ptr<IpL4Protocol> GetProtocol(uint8_t nextHeaderProtocol, Ptr<Ipv4Interface> interface) const;
    std::unordered_map<uint64_t, Ptr<UdpSocketImpl>>
        m_sockets;             //!< Unordered map of socket IDs and corresponding sockets
    uint64_t m_socketIndex{0}; //!< Index of the next socket to be created
    IpL4Protocol::DownTargetCallback m_downTarget;   //!< Callback to send packets over IPv4
    IpL4Protocol::DownTargetCallback6 m_downTarget6; //!< Callback to send packets over IPv6
};

} // namespace ns3

#endif /* UDP_L4_PROTOCOL_H */
