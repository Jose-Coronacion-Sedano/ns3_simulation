#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimulacionBasica");

int
main(int argc, char* argv[])
{
    Time::SetResolution(Time::NS);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Se crean dos nodos: uno sera cliente y el otro servidor.
    NodeContainer nodos;
    nodos.Create(2);

    // Se configura un enlace punto a punto entre ambos nodos.
    PointToPointHelper enlace;
    enlace.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    enlace.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer dispositivos = enlace.Install(nodos);

    // Se instala la pila TCP/IP para poder usar direcciones IP.
    InternetStackHelper pilaInternet;
    pilaInternet.Install(nodos);

    Ipv4AddressHelper direcciones;
    direcciones.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = direcciones.Assign(dispositivos);

    uint16_t puertoServidor = 9;

    // Nodo 1: servidor UDP Echo.
    UdpEchoServerHelper servidor(puertoServidor);
    ApplicationContainer appsServidor = servidor.Install(nodos.Get(1));
    appsServidor.Start(Seconds(1.0));
    appsServidor.Stop(Seconds(10.0));

    // Nodo 0: cliente que envia 5 paquetes al servidor.
    UdpEchoClientHelper cliente(interfaces.GetAddress(1), puertoServidor);
    cliente.SetAttribute("MaxPackets", UintegerValue(5));
    cliente.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    cliente.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer appsCliente = cliente.Install(nodos.Get(0));
    appsCliente.Start(Seconds(2.0));
    appsCliente.Stop(Seconds(10.0));

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
