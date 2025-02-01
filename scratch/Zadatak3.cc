#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/onoff-application.h"
#include "ns3/netanim-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Static Routing");

//Create variables to test the results 
uint32_t m_bytes_sent = 0; 
uint32_t m_bytes_received = 0; 

uint32_t m_packets_sent = 0; 
uint32_t m_packets_received = 0; 

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

void SetStaticRoute
(Ptr<Node> n, const char* destination, const char* nextHop, uint32_t interface)
{
Ipv4StaticRoutingHelper staticRouting;
Ptr<Ipv4> ipv4 = n->GetObject<Ipv4> ();
Ptr<Ipv4StaticRouting> a = staticRouting.GetStaticRouting (ipv4);
a->AddHostRouteTo (Ipv4Address (destination), Ipv4Address (nextHop), interface);
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
    std::cout << p->GetUid() << "\t" << temp - i->second << "\n";

    //Remove the entry from the delayTable to clear the RAM memroy and obey memory leakage
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


int 
main (int argc, char *argv[])
{
  // The below value configures the default behavior of global routing.
  // By default, it is disabled.  To respond to interface events, set to true
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));

  // Allow the user to override any of the defaults and the above
  // Bind ()s at run-time, via command-line arguments
  CommandLine cmd;

  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (3);
  NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer n0n2 = NodeContainer (c.Get (0), c.Get (2));
  NodeContainer n1n2 = NodeContainer (c.Get (1), c.Get (2));


  InternetStackHelper internet;
  internet.Install (c);

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer d0d1 = p2p.Install (n0n1);
  NetDeviceContainer d0d2 = p2p.Install (n0n2);
  NetDeviceContainer d1d2 = p2p.Install (n1n2);


  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i1 = ipv4.Assign (d0d1);


  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i2 = ipv4.Assign (d0d2);


  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);


  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  SetStaticRoute(c.Get(0), "10.1.3.2", "10.1.1.2", 1);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Ipv4GlobalRoutingHelper::RecomputeRoutingTables();

  // Create the OnOff application to send UDP datagrams 
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)
  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     InetSocketAddress (i1i2.GetAddress (1), port));
  
  onoff.SetAttribute("DataRate",StringValue("5kbps")); 
  onoff.SetAttribute ("PacketSize", UintegerValue (50));

  ApplicationContainer apps = onoff.Install (c.Get (0));
  apps.Start (Seconds (10.0));
  apps.Stop (Seconds (40.0));

  OnOffHelper onoff2 ("ns3::UdpSocketFactory",
                   InetSocketAddress (i0i2.GetAddress (1), port));

  onoff2.SetAttribute ("DataRate", StringValue ("5kbps"));
  onoff2.SetAttribute ("PacketSize", UintegerValue (50));


  // Create an optional packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  apps = sink.Install (c.Get (2));
  apps.Start (Seconds (10.0));
  apps.Stop (Seconds (40.0));
  NS_LOG_INFO ("Configure Tracing."); 


  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx", MakeCallback(&SentPacket)); 
  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback(&ReceivedPacket));


  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("static_r.tr");
  p2p.EnableAsciiAll (stream);
  internet.EnableAsciiIpv4All (stream);
  
  AnimationInterface anim("anim.xml");
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();

  Ratio();   

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
