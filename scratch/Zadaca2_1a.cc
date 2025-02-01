/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-helper.h" 
#include "ns3/applications-module.h" 
#include "ns3/nix-vector-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/aodv-module.h" 
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"  
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-helper.h"
//#include "ns3/ah-header.h"

// Dodavanje zaglavlja za vaš AH sloj


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SPTMExample");

uint32_t m_bytes_sent = 0; 
uint32_t m_bytes_received = 0; 
uint32_t m_packets_sent = 0; 
uint32_t m_packets_received = 0; 
double temp_time1 = 0; 
double temp_time2 = 0; 
double m_time = 0;

std::map<uint32_t, double> m_delayTable;

void
SentPacket(std::string context, Ptr<const Packet> p){
    m_bytes_sent  += p->GetSize(); 
    m_packets_sent++;
    m_delayTable.insert (std::make_pair (p->GetUid(), (double)Simulator::Now().GetSeconds()));
}

void
ReceivedPacket(std::string context, Ptr<const Packet> p, const Address& addr){
    std::map<uint32_t, double >::iterator i = m_delayTable.find (p->GetUid());
    double temp = (double)Simulator::Now().GetSeconds();  
    
    if (context.find("NodeList/1/") != std::string::npos) {
        temp_time1 = temp_time1 + (temp - i->second);
        std::cout << "Paket " << p->GetUid()/2 
                  << " Delay (N2->N1): " << temp_time1 << "  ";
    } else if (context.find("NodeList/2/") != std::string::npos) {
        temp_time2 = temp_time2 + (temp - i->second);
        std::cout << " Delay (N3->N2): " << temp_time2 << " \n";
    }

    if (i != m_delayTable.end()){
        m_delayTable.erase(i);
    }

    m_bytes_received += p->GetSize(); 
    m_packets_received++;
}

void SetStaticRoute
(Ptr<Node> n, const char* destination, const char* nextHop, uint32_t interface)
{
Ipv4StaticRoutingHelper staticRouting;
Ptr<Ipv4> ipv4 = n->GetObject<Ipv4> ();
Ptr<Ipv4StaticRouting> a = staticRouting.GetStaticRouting (ipv4);
a->AddHostRouteTo (Ipv4Address (destination), Ipv4Address (nextHop), interface);
}

void
Ratio(){
    std::cout << "Sent (bytes):\t" <<  m_bytes_sent
    << "\tReceivede (bytes):\t" << m_bytes_received 
    << "\nSent (Packets):\t" <<  m_packets_sent
    << "\tReceived (Packets):\t" << m_packets_received 
    << "\nRatio (bytes):\t" << (float)m_bytes_received/(float)m_bytes_sent
    << "\tRatio (packets):\t" << (float)m_packets_received/(float)m_packets_sent << "\n";
}

static void
SetPosition (Ptr<Node> node, double a, double b)
{
Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
Vector pos = mobility->GetPosition();
pos.x = a;
pos.y = b;
mobility->SetPosition(pos);
}

int
main (int argc, char *argv[])
{
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL); 
  LogComponentEnable ("SPTMExample", LOG_LEVEL_ALL);

  Packet::EnablePrinting(); 
  PacketMetadata::Enable ();
  
  bool enablePcap = true;
  double simulationTime = 300;  
  double numberOfNodes = 4;  
  bool enableApplication = true; 
  int packetSize = 150; 
  std::string lat1 = "2ms";
  std::string lat2 = "5ms";
  std::string rate = "500kb/s"; // P2P link
  
  CommandLine cmd; 
  cmd.AddValue ("simulationTime", "simulationTime", simulationTime); 
  cmd.AddValue ("packetSize", "packetSize", packetSize);
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (numberOfNodes); 
 
 MobilityHelper mobility;
    // setup the grid itself: objects are laid out
    // started from (-100,-100) with 20 objects per row,
    // the x interval between each object is 5 meters
    // and the y interval between each object is 20 meters
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                              "MinX", DoubleValue(-20.0),
                              "MinY", DoubleValue(-20.0),
                              "DeltaX", DoubleValue(10.0),
                              "DeltaY", DoubleValue(10.0),
                              "GridWidth", UintegerValue(2),
                              "LayoutType", StringValue("RowFirst"));

    // each object will be attached a static position.
    // i.e., once set by the "position allocator", the
    // position will never change.
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // finalize the setup by attaching to each object
    // in the input array a position and initializing
    // this position with the calculated coordinates.
    mobility.Install(nodes);

  NodeContainer n2n0 = NodeContainer (nodes.Get (2), nodes.Get (0));
  NodeContainer n1n0 = NodeContainer (nodes.Get (1), nodes.Get (0));
  NodeContainer n0n3 = NodeContainer (nodes.Get (0), nodes.Get (3));

  InternetStackHelper internet;
  internet.Install (nodes);

  PointToPointHelper p2p1;
  p2p1.SetDeviceAttribute ("DataRate", StringValue (rate));
  p2p1.SetChannelAttribute ("Delay", StringValue (lat1));
  NetDeviceContainer d2d0 = p2p1.Install (n2n0);
  NetDeviceContainer d1d0 = p2p1.Install (n1n0);
  
  PointToPointHelper p2p2;
  p2p2.SetDeviceAttribute ("DataRate", StringValue (rate));
  p2p2.SetChannelAttribute ("Delay", StringValue (lat2));
  NetDeviceContainer d0d3 = p2p2.Install (n0n3);
  
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i0 = ipv4.Assign (d2d0);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i0 = ipv4.Assign (d1d0);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i3 = ipv4.Assign (d0d3);
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Ipv4GlobalRoutingHelper::RecomputeRoutingTables();


  if(enableApplication){

      uint16_t port1 = 9;   // UDP port
      uint16_t port2 = 10;   // UDP port

      SptmClientHelper sptmClient1 ("ns3::UdpSocketFactory", InetSocketAddress (i1i0.GetAddress (0), port1));
      sptmClient1.SetAttribute("MaxPackets", UintegerValue (5));
      sptmClient1.SetAttribute("PacketSize", UintegerValue (packetSize));
      
     SptmClientHelper sptmClient2 ("ns3::UdpSocketFactory", InetSocketAddress (i2i0.GetAddress (0), port2));
      sptmClient2.SetAttribute("PacketSize", UintegerValue (packetSize));

      ApplicationContainer apps1 = sptmClient1.Install (nodes.Get (2));
      apps1.Start (Seconds (15.0));
      apps1.Stop (Seconds (simulationTime));
      
      ApplicationContainer apps2 = sptmClient2.Install (nodes.Get (3));
      apps2.Start (Seconds (15.0));
      apps2.Stop (Seconds (simulationTime));

      SptmSinkHelper sptmSink1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port1));
      SptmSinkHelper sptmSink2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port2));
      apps1 = sptmSink1.Install (nodes.Get (1));

      apps1.Start (Seconds (15.0));
      Simulator::Schedule (Seconds (16.0), &SetPosition, nodes.Get (2), -10.00, -15.00);
	Simulator::Schedule (Seconds (16.0), &SetPosition, nodes.Get (1), -25.00, -10.00);
      apps1.Stop (Seconds (simulationTime)); 
      
      apps2 = sptmSink2.Install (nodes.Get (2));

      apps2.Start (Seconds (15.0));
      apps2.Stop (Seconds (simulationTime)); 
  }

  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::SptmClient/Tx", MakeCallback(&SentPacket)); 
  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::SptmSink/Rx", MakeCallback(&ReceivedPacket));
  AnimationInterface anim("anim_Zadaca2a.xml");
 
  if(enablePcap){
    p2p1.EnablePcapAll ("sptm_Zadaca2a_log1"); 
    p2p2.EnablePcapAll ("sptm_Zadaca2a_log2");
  }

  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();

  if(enableApplication) {
      Ratio();
  }

  Simulator::Destroy();
  Names::Clear(); 
}

