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
//#include "ns3/wifi-module.h"
 
#include <vector>
#include <sstream>
#include <iostream>
#include <iterator>
#include <fstream> 
 
#include "ns3/aodv-module.h" 
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"  
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-helper.h"
#include <chrono>
#include "ns3/ipv4-flow-classifier.h"


#include <sys/resource.h>
#include <sys/time.h>



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SPTMExample");

//Create variables to test the results 
uint32_t m_bytes_sent = 0; 
uint32_t m_bytes_received = 0; 

uint32_t m_packets_sent = 0; 
uint32_t m_packets_received = 0; 
double temp_time1 = 0; 
double temp_time2 = 0; 

//Create help variable m_time
double m_time = 0;

//Create c++ map for measuring delay time
std::map<uint32_t, double> m_delayTable;
 
/**
   * \brief Each time the packet is sent, this function is called using TracedCallback and information about packet is stored in m_delayTable
   * \param context - used to display full path of the information provided (useful for taking information about the nodes or applications)
   * \param packet - Ptr reference to the packet itself
   */
void
SentPacket(std::string context, Ptr<const Packet> p){
    
    /*
    //HELP LINES USED FOR TESTING
    std::cout << "\n ..................SentPacket....." << p->GetUid() << "..." <<  p->GetSize() << ".......  \n";
    p->Print(std::cout);                
    std::cout << "\n ............................................  \n";  
    */

    //Sum bytes of the packet that was sent
    m_bytes_sent  += p->GetSize(); 
    m_packets_sent++;

    //Insert in the delay table details about the packet that was sent
    m_delayTable.insert (std::make_pair (p->GetUid(), (double)Simulator::Now().GetSeconds()));
}

void
ReceivedPacket(std::string context, Ptr<const Packet> p, const Address& addr){
    
    /*
    //HELP LINES USED FOR TESTING
    std::cout << "\n ..................ReceivedPacket....." << p->GetUid() << "..." <<  p->GetSize() << ".......  \n";
    p->Print(std::cout);                
    std::cout << "\n ............................................  \n";  
    */

    //Find the record in m_delayTable based on packetID
    std::map<uint32_t, double >::iterator i = m_delayTable.find ( p->GetUid() );
    //Get the current time in the temp variable
    double temp = (double)Simulator::Now().GetSeconds();  
    
    //Display the delay for the packet in the form of "packetID delay" where delay is calculated as the current time - time when the packet was sent
    
    if (context.find("NodeList/1/") != std::string::npos) {
        temp_time1=temp_time1+(temp-i->second);
        // Ovo je tok između čvorova N2 i N1
        std::cout << "Paket " << p->GetUid()/2 
                  << " Delay (N2->N1): " << temp_time1 << "  ";
                        std::cout << "\n";
    } else if (context.find("NodeList/2/") != std::string::npos) {
        // Ovo je tok između čvorova N3 i N2
        temp_time2=temp_time2+(temp-i->second);
        std::cout << "        Delay (N3->N2): " << temp_time2 << " \n";
              std::cout << "\n";
    }
    //std::cout << p->GetUid() << "\t" << temp_time << "\n";
    //Remove the entry from the delayTable to clear the RAM memory and obey memory leakage
    if(i != m_delayTable.end()){
        m_delayTable.erase(i);
    }

    //Sum bytes and number of packets that were sent
    m_bytes_received += p->GetSize(); 
    m_packets_received++;
}

void
Ratio(){

    std::cout << "Sent (bytes):\t" <<  m_bytes_sent
    << "\tReceived (bytes):\t" << m_bytes_received 
    << "\nSent (Packets):\t" <<  m_packets_sent
    << "\tReceived (Packets):\t" << m_packets_received 
    << "\nRatio (bytes):\t" << (float)m_bytes_received/(float)m_bytes_sent
    << "\tRatio (packets):\t" << (float)m_packets_received/(float)m_packets_sent << "\n";
}

void PrintCPUUsage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    std::cout << "CPU Time (User): " << usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6 << " s\n";
    std::cout << "CPU Time (System): " << usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6 << " s\n";
}

int
main (int argc, char *argv[])
{
auto start = std::chrono::high_resolution_clock::now();
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL); 
  //LogComponentEnable ("UdpL4Protocol", LOG_LEVEL_ALL); 
  //LogComponentEnable ("SptmClient", LOG_LEVEL_ALL);
  LogComponentEnable ("SPTMExample", LOG_LEVEL_ALL);
  //LogComponentEnable ("AHProtocol", LOG_LEVEL_ALL);
//LogComponentEnable ("UDPHeader", LOG_LEVEL_ALL);
  Packet::EnablePrinting(); 
  PacketMetadata::Enable ();
  bool enableFlowMonitor = false;
  bool      enablePcap = true;
  double    simulationTime = 300;  
  double    numberOfNodes = 4;  
  bool      enableApplication = true;  
  
  std::string lat1 = "2ms";
  std::string lat2 = "5ms";
  std::string rate = "500kb/s"; // P2P link
  
  CommandLine cmd; 
  cmd.AddValue ("simulationTime", "simulationTime", simulationTime); 
  cmd.AddValue("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  cmd.Parse (argc, argv);
 
  //
  // Explicitly create the nodes required by the topology (shown above).
  //
 // NS_LOG_INFO ("Prije kreiranja nodova");
  NodeContainer nodes;
  nodes.Create (numberOfNodes); 
  // NS_LOG_INFO ("Poslije kreiranja nodova");
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
  
  //MobilityHelper mobility;
  //mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  //mobility.Install(nodes);

 // NS_LOG_INFO ("Prije kreiranja kontejnera");
  NodeContainer n2n0 = NodeContainer (nodes.Get (2), nodes.Get (0));
  NodeContainer n1n0 = NodeContainer (nodes.Get (1), nodes.Get (0));
  NodeContainer n0n3 = NodeContainer (nodes.Get (0), nodes.Get (3));

  //
  // Install Internet Stack
  //
  InternetStackHelper internet;
  internet.Install (nodes);
  

// NS_LOG_INFO ("Nakon kreiranja interneta");
  // We create the channels first without any IP addressing information
  // NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p1;
  p2p1.SetDeviceAttribute ("DataRate", StringValue (rate));
  p2p1.SetChannelAttribute ("Delay", StringValue (lat1));
  NetDeviceContainer d2d0 = p2p1.Install (n2n0);
  NetDeviceContainer d1d0 = p2p1.Install (n1n0);
  
  PointToPointHelper p2p2;
  p2p2.SetDeviceAttribute ("DataRate", StringValue (rate));
  p2p2.SetChannelAttribute ("Delay", StringValue (lat2));
  NetDeviceContainer d0d3 = p2p2.Install (n0n3);
  
  // Add IP addresses.
  // NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i0 = ipv4.Assign (d2d0);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i0 = ipv4.Assign (d1d0);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i3 = ipv4.Assign (d0d3);

  // NS_LOG_INFO ("Enable static global routing.");
  //
  // Turn on global static routing so we can actually be routed across the network.
  //
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
 
  if(enableApplication){

      uint16_t port1 = 9;   // UDP port
      uint16_t port2 = 10;   // UDP port
      //uint16_t serverPosition1 = 0;
      //uint16_t serverPosition2 = 2; 
     // uint16_t clientPosition1 = 0;
      //uint16_t clientPosition2 = 3;

      // UDP connection from N2 to N0
      std::cout << "Sender and reaciver address on N2->N1 link \n";
      std::cout << "  sender IP address:   " << i2i0.GetAddress (0) << "\n";
      std::cout << "  receiver IP address:   " << i1i0.GetAddress (0) << "\n";
      
      std::cout << "Sender and reaciver address on N3->N2 link \n";
      std::cout << "  sender IP address:   " << i0i3.GetAddress (0) << "\n";
      std::cout << "  receiver IP address:   " << i2i0.GetAddress (0) << "\n";
      std::cout << "\n";
      
      SptmClientHelper sptmClient1 ("ns3::UdpSocketFactory", InetSocketAddress (i1i0.GetAddress (0), port1));
      sptmClient1.SetAttribute("MaxPackets", UintegerValue (5));
      sptmClient1.SetAttribute("PacketSize", UintegerValue (100));
      sptmClient1.SetAttribute("AHEnable", UintegerValue (1));
      
      SptmClientHelper sptmClient2 ("ns3::UdpSocketFactory", InetSocketAddress (i2i0.GetAddress (0), port2));
      sptmClient2.SetAttribute("PacketSize", UintegerValue (100));

      ApplicationContainer apps1 = sptmClient1.Install (nodes.Get (2));
      apps1.Start (Seconds (15.0));
      apps1.Stop (Seconds (simulationTime));
      
      ApplicationContainer apps2 = sptmClient2.Install (nodes.Get (3));
      apps2.Start (Seconds (15.0));
      apps2.Stop (Seconds (simulationTime));

      // Create a packet sptmSink to receive these packets
      SptmSinkHelper sptmSink1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port1));
      SptmSinkHelper sptmSink2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port2));
      apps1 = sptmSink1.Install (nodes.Get (1));

      apps1.Start (Seconds (15.0));
      apps1.Stop (Seconds (simulationTime)); 
      
      apps2 = sptmSink2.Install (nodes.Get (2));

      apps2.Start (Seconds (15.0));
      apps2.Stop (Seconds (simulationTime)); 
 
  }
  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::SptmClient/Tx", MakeCallback(&SentPacket)); 
  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::SptmSink/Rx", MakeCallback(&ReceivedPacket));
  AnimationInterface anim("anim.xml");
 
  if(enablePcap){
    p2p1.EnablePcapAll ("sptm_logAmr1"); 
    p2p2.EnablePcapAll ("sptm_logAmr2");
  }

  //
  // Now, do the actual simulation.
  // 
        
  // Run the simulator
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();

  if(enableApplication) {
      Ratio();
  }
  std::cout << "\n";
  PrintCPUUsage();

  //Finally print the graphs 
  Simulator::Destroy();
  Names::Clear(); 
  
}
