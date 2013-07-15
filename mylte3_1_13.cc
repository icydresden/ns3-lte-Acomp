/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/radio-bearer-stats-calculator.h"
#include "ns3/mac-stats-calculator.h"
#include <iomanip>
#include "ns3/flow-monitor-module.h"
#include <string>
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/flow-monitor-helper.h"
#include <ctime>

using namespace ns3;

void PrintNodePlace(NodeContainer node, int numUes)
{
	  for(int i=0; i<numUes; i++)
		  {
			  Ptr<MobilityModel> mob = node.Get(i)->GetObject<MobilityModel>();
			  Vector pos = mob->GetPosition ();
			  std::cout << pos.x << "\t" << pos.y << std::endl;
		  }
}

void PrintNodeApplication(NodeContainer node, int numUes)
{
  for(int v = 0 ; v < numUes ; v++)
  {
    std::cout<<v<<"    "<<(node.Get (v)->GetApplication(0)->GetTypeId()).GetName()<<"\n";
  }
}

void PrintNodeIP(NodeContainer node, int numUes)
{
  for(int v = 0 ; v < numUes ; v++)
  {
        Ptr<Ipv4> ipv4 = node.Get (v)->GetObject<Ipv4>();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
        Ipv4Address addri = iaddr.GetLocal (); 
        std::cout<<"   "<<addri<<"\n";
  }
}

/**
 * This simulation script creates two eNodeBs and drops randomly several UEs in
 * a disc around them (same number on both). The number of UEs , the radius of
 * that disc and the distance between the eNodeBs can be configured.
 */
int main (int argc, char *argv[])
{
  double enbDist = 100.0;
  double radius = 50.0;
  uint32_t numUes = 10;

  int isComp = 0;
  int sec = 1;

  CommandLine cmd;
  cmd.AddValue ("enbDist", "distance between the two eNBs", enbDist);
  cmd.AddValue ("radius", "the radius of the disc where UEs are placed around an eNB", radius);
  cmd.AddValue ("numUes", "how many UEs are attached to each eNB", numUes);
  cmd.AddValue ("isComp", "is it a comp scenario ", isComp );
  cmd.AddValue ("sec", "seconds of simualtion ", sec );

  cmd.Parse (argc, argv);
  IntegerValue runValue;
  GlobalValue::GetValueByName ("RngRun", runValue);

  // this tag is then appended to all filenames
  std::ostringstream tag;
  tag  << "_enbDist" << std::setw (3) << std::setfill ('0') << std::fixed << std::setprecision (0) << enbDist
       << "_radius"  << std::setw (3) << std::setfill ('0') << std::fixed << std::setprecision (0) << radius
       << "_numUes"  << std::setw (3) << std::setfill ('0')  << numUes
       << "_isComp"  << std::setw (3) << std::setfill ('0')  << isComp
       << "_rngRun"  << std::setw (3) << std::setfill ('0')  << runValue.Get () 
       << "_sec"  << std::setw (3) << std::setfill ('0')  << sec;
       
  // Insert RLC Performance Calculator
  std::ostringstream prefixString;
  prefixString<<enbDist<<"_"<<radius<<"_"<<numUes<<"_";
  std::string dlOutFname = "DlRlcStats";
  dlOutFname.append (tag.str ());

  std::string ulOutFname = "UlRlcStats";
  ulOutFname.append (tag.str ());

  std::string xmlOut = "Lte";
  xmlOut.append(tag.str ());
  xmlOut.append(".xml");

  std::string outputpath = "Results" ;
  Config::SetDefault ("ns3::RadioBearerStatsCalculator::DlRlcOutputFilename", StringValue (outputpath+"/DlRlcStats" + tag.str() + ".txt"));
  Config::SetDefault ("ns3::RadioBearerStatsCalculator::UlRlcOutputFilename", StringValue (outputpath+"/UlRlcStats"+ tag.str() +".txt"));
  Config::SetDefault ("ns3::RadioBearerStatsCalculator::DlPdcpOutputFilename", StringValue (outputpath+"/DlPdcpStats"+ tag.str() +".txt"));
  Config::SetDefault ("ns3::RadioBearerStatsCalculator::UlPdcpOutputFilename", StringValue (outputpath+"/UlPdcpStats"+ tag.str() +".txt"));
  Config::SetDefault ("ns3::MacStatsCalculator::DlOutputFilename", StringValue (outputpath+"/DlMacStats"+ tag.str() +".txt"));
  Config::SetDefault ("ns3::MacStatsCalculator::UlOutputFilename", StringValue (outputpath+"/UlMacStats"+ tag.str() +".txt"));

  //ConfigStore inputConfig;
  //inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  //enable logs
  LogComponentEnable("LteHelper", LogLevel(LOG_LEVEL_ALL | LOG_DEBUG | LOG_LOGIC| LOG_PREFIX_FUNC| LOG_PREFIX_TIME));

  // determine the string tag that identifies this simulation run
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<EpcHelper>  epcHelper = CreateObject<EpcHelper> ();
  //lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));

  // Create Nodes: eNodeB and UE
  int UesNumber = 4; 

  NodeContainer enbNodes;
  NodeContainer ueNodes[4];
  enbNodes.Create (4);

  for( int v = 0; v< UesNumber ; v++)
  {
    ueNodes[v].Create (numUes); 
  }
  
  // Position of eNBs
  double max;
  double delta;
  max = enbDist * 2;
  delta = enbDist / 2;

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (delta, delta, 0.0));
  positionAlloc->Add (Vector (max - delta, delta, 0.0));
  positionAlloc->Add (Vector (delta, max - delta, 0.0));
  positionAlloc->Add (Vector (max - delta, max - delta, 0.0));


  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator (positionAlloc);
  enbMobility.Install (enbNodes);

  // Position of UEs attached to eNB 1
  for(int z = 0; z< UesNumber; z++)
  {
    MobilityHelper uemobility;
    uemobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                      "X", DoubleValue (enbDist),
                                      "Y", DoubleValue (enbDist),
                                      "rho", DoubleValue (enbDist));
    uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    uemobility.Install (ueNodes[z]);
  }
  
  Ptr<Node> pgw = epcHelper->GetPgwNode ();


// Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);


  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs[UesNumber];
  
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);

  for( int z = 0 ; z< UesNumber; z++)
  {
    ueDevs[z] = lteHelper->InstallUeDevice (ueNodes[z]);
  }
  
  // Attach UEs to a eNB
  if (isComp==1)
  {
    for( int z = 0 ; z< UesNumber; z++)
    {
	   lteHelper->Attach (ueDevs[z], enbDevs.Get (z));
    }
	 
  }
  else
  {
	  for (uint32_t i=0; i<numUes; i++)
	  {
      for( int z = 0 ; z< UesNumber; z++)
      {
		  lteHelper->AttachToClosestEnb(ueDevs[z].Get(i), enbDevs);
		  }
	  }
  }

 // Install the IP stack on the UEs
  for( int z = 0 ; z< UesNumber; z++)
  {
    internet.Install (ueNodes[z]);
  }


  Ipv4InterfaceContainer ueIpIface[UesNumber];

  for( int z = 0 ; z< UesNumber; z++)
  {
    ueIpIface[z] = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs[z]));
  }

  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < numUes; ++u)
  {
    for( int z = 0 ; z< UesNumber; z++)
    {
      Ptr<Node> ueNode = ueNodes[z].Get (u);
      
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
  }
 

   // SETTING DEL TFT
  Ptr<EpcTft> tftVIDEODOWN = Create<EpcTft> ();
  Ptr<EpcTft> tftVIDEOUP = Create<EpcTft> ();
  Ptr<EpcTft> tftHTTP = Create<EpcTft> ();
  
  EpcTft::PacketFilter filter[UesNumber];
  
  
  filter[0].localPortStart = 3478;
  filter[0].localPortEnd = 3478;
  tftVIDEODOWN->Add(filter[0]);
  
  filter[1].remotePortStart = 3478;
  filter[1].remotePortEnd = 3478;
  tftVIDEOUP->Add(filter[1]);
  
  filter[2].remotePortStart = 80;
  filter[2].remotePortEnd = 80;
  tftHTTP->Add(filter[2]);
  
  filter[3].localPortStart = 80;
  filter[3].localPortEnd = 80;
  tftHTTP->Add(filter[3]);

  // Activate an EPS bearer on all UEs
  enum EpsBearer::Qci q[4] ;

  q[0] = EpsBearer::GBR_CONV_VOICE;
  q[1] = EpsBearer::GBR_CONV_VIDEO;
  q[2] = EpsBearer::GBR_NON_CONV_VIDEO;
  q[3] = EpsBearer::GBR_CONV_VIDEO;

  EpsBearer bearer0(q[0]);
  EpsBearer bearer1(q[1]);
  EpsBearer bearer2(q[2]);
  EpsBearer bearer3(q[3]);
  
  
  lteHelper->ActivateEpsBearer (ueDevs[0], bearer0, tftHTTP);
	lteHelper->ActivateEpsBearer (ueDevs[1], bearer1,tftVIDEOUP);
  lteHelper->ActivateEpsBearer (ueDevs[2], bearer2, tftVIDEODOWN);
	lteHelper->ActivateEpsBearer (ueDevs[3], bearer3, EpcTft::Default ());

  //Servers and clients connections
/*  UdpEchoServerHelper ServerHTTP (80);      
  ApplicationContainer serverAppsHTTP1 = ServerHTTP.Install (enbNodes.Get(0));

  UdpEchoServerHelper ueServerVIDEO (3478);      
  ApplicationContainer serverAppsVIDEO1 = ueServerVIDEO.Install (enbNodes.Get(1));
  
  UdpEchoServerHelper ServerHTTP2 (80);      
  ApplicationContainer serverAppsHTTP2 = ServerHTTP2.Install (enbNodes.Get(2));

  UdpEchoServerHelper ueServerVIDEO2 (3478);      
  ApplicationContainer serverAppsVIDEO2 = ueServerVIDEO2.Install (enbNodes.Get(3));

  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);  
  for(uint v = 0 ; v < numUes ; v++)
  {
    for( int z = 0 ; z< UesNumber; z++)
    {
      // UE(0) client - RemoteHost Server
      UdpEchoClientHelper ClientHTTP1 (remoteHostAddr, 80);
      ClientHTTP1.SetAttribute ("MaxPackets", UintegerValue (2));
      ClientHTTP1.SetAttribute ("Interval", TimeValue (Seconds (0.01)));
      ClientHTTP1.SetAttribute ("PacketSize", UintegerValue (1024));
      ApplicationContainer clientAppsHTTP1 = ClientHTTP1.Install (ueNodes[z].Get (v));
      clientAppsHTTP1.Start (Seconds (0.01));
    }
  }*/
   

   /*   UdpEchoClientHelper ClientHTTP2 (enbNodes.Get(1), 80);
      ClientHTTP2.SetAttribute ("MaxPackets", UintegerValue (2));
      ClientHTTP2.SetAttribute ("Interval", TimeValue (Seconds (0.01)));
      ClientHTTP2.SetAttribute ("PacketSize", UintegerValue (1024));
      ApplicationContainer clientAppsHTTP2 = ClientHTTP2.Install (ueNodes2.Get (v));
      
      // UE(1) client - UE(2) Server
      UdpEchoClientHelper ueClientVIDEO1 (enbNodes.Get(3), 3478);
      ueClientVIDEO1.SetAttribute ("MaxPackets", UintegerValue (2));
      ueClientVIDEO1.SetAttribute ("Interval", TimeValue (Seconds (0.01)));
      ueClientVIDEO1.SetAttribute ("PacketSize", UintegerValue (1024));
      ApplicationContainer clientAppsVIDEO1 = ueClientVIDEO1.Install (ueNodes3.Get (v));

      UdpEchoClientHelper ueClientVIDEO2 (enbNodes.Get(4), 3478);
      ueClientVIDEO2.SetAttribute ("MaxPackets", UintegerValue (2));
      ueClientVIDEO2.SetAttribute ("Interval", TimeValue (Seconds (0.01)));
      ueClientVIDEO2.SetAttribute ("PacketSize", UintegerValue (1024));      
      ApplicationContainer clientAppsVIDEO2 = ueClientVIDEO2.Install (ueNodes4.Get (v));
      
      clientAppsHTTP1.Start (Seconds (0.01));
      clientAppsVIDEO1.Start (Seconds (0.01));
      clientAppsHTTP2.Start (Seconds (0.01));
      clientAppsVIDEO2.Start (Seconds (0.01));
      
  }
  
  serverAppsHTTP1.Start (Seconds (0.01));
  serverAppsVIDEO1.Start (Seconds (0.01));
  */
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  
  for( int z = 0 ; z< UesNumber; z++)
  {
    PrintNodePlace(ueNodes[z],numUes);
  }
  
  for( int z = 0 ; z< UesNumber; z++)
  {
    PrintNodeIP(ueNodes[z],numUes);
  }
  
  std::cout<<"Simulation\n";
  std::cout<<"Number of Base Stations: 4'\n";
  std::cout<<"Distance Between every Base Station: "<<enbDist<<"\n";
  std::cout<<"Number of Ues by Base Station: "<<numUes<<"\n";
  std::cout<<"Comp: "<<isComp<<"\n";

  // Tracing
  /*Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowMonHelper;
  flowMonHelper.SetMonitorAttribute("StartTime", TimeValue(Time(1.0)));
  flowMonHelper.SetMonitorAttribute("JitterBinWidth", ns3::DoubleValue(0.001));
  flowMonHelper.SetMonitorAttribute("DelayBinWidth", ns3::DoubleValue(0.001));
  flowMonHelper.SetMonitorAttribute("PacketSizeBinWidth", DoubleValue(20));
  flowmon = flowMonHelper.InstallAll(); */
  /*flowmon = flowMonHelper.Install (ueNodes1);
  flowmon = flowMonHelper.Install (ueNodes2);
  flowmon = flowMonHelper.Install (ueNodes3);
  flowmon = flowMonHelper.Install (ueNodes4);*/
  
  Simulator::Run ();
  Simulator::Stop(Seconds(sec));
  
  //flowmon->SerializeToXmlFile (xmlOut, false, false);
  Simulator::Destroy ();
  std::cout<<"Simulation finish "<<numUes<<"\n";

  return 0;
}
