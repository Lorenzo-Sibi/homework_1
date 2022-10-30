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

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"

// Network topology (default)
//
//        n2 n3 n4              .
//         \ | /                .
//          \|/                 .
//     n1--- n0---n5            .
//          /|\                 .
//         / | \                .
//        n8 n7 n6              .
//

// Network topology (default)
//
//           n1                 .
//           |                  .
//           |      lan      lan.
//     n2--- n0---n4-n-n --- n-n-n           .
//           |                  .
//           |                  .
//           n3                 .
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Star");

int
main(int argc, char* argv[])
{
    //
    // Set up some default values for the simulation.
    //
    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(137));

    // ??? try and stick 15kb/s into the data rate
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("14kb/s"));

    //
    // Default number of nodes in the star.  Overridable by command line argument.
    //
    uint32_t nSpokes = 4;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nSpokes", "Number of nodes to place in the star", nSpokes);
    cmd.Parse(argc, argv);

    NS_LOG_INFO("Build star topology.");
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    PointToPointStarHelper star(nSpokes, pointToPoint);

    //Creo la prima lan
    NodeContainer csmaNodes1;
    csmaNodes1.Add(star.GetSpokeNode(3));
    csmaNodes1.Create(2);

    CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices1;
    csmaDevices1 = csma1.Install(csmaNodes1);


    //Creo la seconda lan
    NodeContainer csmaNodes2;
    csmaNodes2.Create(3);

    CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices2;
    csmaDevices2 = csma2.Install(csmaNodes2);

    //Creo conteiner con nodi delle 2 lan da connettere
    NodeContainer cn1_cn2;
    cn1_cn2.Add(csmaNodes1.Get(2));
    cn1_cn2.Add(csmaNodes2.Get(0));

    //Creiamo la point to point fra le due lan
    PointToPointHelper pointToPoint2;
    pointToPoint2.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint2.SetChannelAttribute("Delay", StringValue("2ms"));
    
    //Installiamo la point to point fra le 2 lan
    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(cn1_cn2);



    NS_LOG_INFO("Install internet stack on all nodes.");
    InternetStackHelper internet;
    star.InstallStack(internet);
    internet.Install(csmaNodes1.Get(1));//
    internet.Install(csmaNodes1.Get(2));//
    internet.Install(csmaNodes2);

    NS_LOG_INFO("Assign IP Addresses.");
    star.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"));
    //ip lan1
    Ipv4AddressHelper address1;
    address1.SetBase("192.118.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csma1Interfaces;
    csma1Interfaces = address1.Assign(csmaDevices1);
    
    //ip lan2
    Ipv4AddressHelper address2;
    address2.SetBase("192.118.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csma2Interfaces;
    csma2Interfaces = address2.Assign(csmaDevices2);

    //ip p2p
    Ipv4AddressHelper address3;
    address3.SetBase("192.118.3.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address3.Assign(p2pDevices);

    /*

    NS_LOG_INFO("Create applications.");
    //
    // Create a packet sink on the star "hub" to receive packets.
    //
    uint16_t port = 50000;
    Address hubLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", hubLocalAddress);
    ApplicationContainer hubApp = packetSinkHelper.Install(star.GetHub());
    hubApp.Start(Seconds(1.0));
    hubApp.Stop(Seconds(10.0));

    //
    // Create OnOff applications to send TCP to the hub, one on each spoke node.
    //
    OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
    onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer spokeApps;

    for (uint32_t i = 0; i < star.SpokeCount(); ++i)
    {
        AddressValue remoteAddress(InetSocketAddress(star.GetHubIpv4Address(i), port));
        onOffHelper.SetAttribute("Remote", remoteAddress);
        spokeApps.Add(onOffHelper.Install(star.GetSpokeNode(i)));
    }
    spokeApps.Start(Seconds(1.0));
    spokeApps.Stop(Seconds(10.0));

    */

   //Prova con udp
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(star.GetSpokeNode(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(star.GetSpokeIpv4Address(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(csmaNodes2.Get(2));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    NS_LOG_INFO("Enable static global routing.");
    //
    // Turn on global static routing so we can actually be routed across the star.
    //
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_INFO("Enable pcap tracing.");
    //
    // Do pcap tracing on all point-to-point devices on all nodes.
    //
    
    //pointToPoint.EnablePcapAll("Moddedstar");
    //csma1.EnablePcapAll("Ms lan1");
    csma2.EnablePcapAll("Ms lan2");

    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    return 0;
}
