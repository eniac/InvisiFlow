#include "collector-node.h"

#include "ns3/application.h"
#include "ns3/net-device.h"
#include "ns3/node-list.h"
#include "ns3/ipv4-header.h"

#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/global-value.h"
#include "ns3/log.h"
#include "ns3/object-vector.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/socket.h"

#include "tele-header.h"
#include "timestamp-tag.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CollectorNode");

NS_OBJECT_ENSURE_REGISTERED(CollectorNode);


TypeId
CollectorNode::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CollectorNode")
            .SetParent<Node>()
            .SetGroupName("PointToPoint")
            .AddConstructor<CollectorNode>();
    return tid;
}

CollectorNode::CollectorNode() : Node() {
    m_orbweaver = false;
    m_delay.resize(m_delaySize);
    Simulator::Schedule(Seconds(2), &CollectorNode::GeneratePacket, this);
}

CollectorNode::~CollectorNode(){
    if(m_types.find(1) != m_types.end()){
        if(m_record){
            std::cout << "Receive path entries: " << m_paths.size() << std::endl;
            std::cout << "Number of duplicates: " << m_duplicates << std::endl;

            FILE* fout = fopen((output_file + ".collector.path").c_str(), "a");
            for(auto path : m_paths){
                fprintf(fout, "%d %d ", path.GetSrcIP(), path.GetDstIP());
                fprintf(fout, "%d %d ", path.GetSrcPort(), path.GetDstPort());
                fprintf(fout, "%d %d ", path.GetNodeId(), (int)path.GetProtocol());
                fprintf(fout, "%d\n", (int)path.GetTTL());
            }
            fflush(fout);
            fclose(fout);
        }
    }

    if(m_types.find(2) != m_types.end()){
        std::cout << "Receive util entries: " << m_receive[2] << std::endl;

        if(m_record){
            FILE* fout = fopen((output_file + ".collector.util").c_str(), "a");
            for(auto util : m_utils){
                fprintf(fout, "%d %d ", util.GetNodeId(), util.GetPortId());
                fprintf(fout, "%d %d\n", util.GetTime(), util.GetByte());
                fflush(fout);
            }
            fclose(fout);
        }
    }
    
    if(m_types.find(4) != m_types.end()){
        std::cout << "Receive drop entries: " << m_receive[4] << std::endl;

        if(m_record){
            FILE* fout = fopen((output_file + ".collector.drop").c_str(), "a");
            for(auto drop : m_drops){
                fprintf(fout, "%d %d ", drop.GetSrcIP(), drop.GetDstIP());
                fprintf(fout, "%d %d ", drop.GetSrcPort(), drop.GetDstPort());
                fprintf(fout, "%d %d ", drop.GetNodeId(), (int)drop.GetProtocol());
                fprintf(fout, "%d\n", drop.GetTime());
                fflush(fout);
            }
            fflush(fout);
            fclose(fout);
        }
    }

    if(m_types.find(8) != m_types.end()){
        std::cout << "Receive count entries: " << m_receive[8] << std::endl;

        if(m_record){
            FILE* fout = fopen((output_file + ".collector.count").c_str(), "a");
            for(auto it = m_counts.begin();it != m_counts.end();++it){
                for(int i = 0;i < OURS_SKETCH_HASH;++i){
                    fprintf(fout, "%d", it->first);
                    for(int j = 0;j < OURS_SKETCH_LENGTH;++j){
                        fprintf(fout, " %d", it->second.values[i][j]);
                    }
                    fprintf(fout, "\n");
                    fflush(fout);
                }
            }
            fflush(fout);
            fclose(fout);
        }
    }

    FILE* fout = fopen((output_file + ".collector.delay").c_str(), "a");
    for(int delay : m_delay){
        fprintf(fout, "%d,", delay);
    }
    fprintf(fout, "\n");
    fflush(fout);
    fclose(fout);

    for(int dest = 0;dest < 3;++dest){
        std::cout << "Max size for destination " << dest << " is " << 
            m_teleQueue.maxSize[dest] << std::endl;
    }
}

void 
CollectorNode::SetRecord(uint32_t record){
    m_record = record;
}

void 
CollectorNode::SetOutput(std::string output){
    output_file = output;
    FILE* fout = fopen((output_file + ".collector.path").c_str(), "w");
    fclose(fout);
    fout = fopen((output_file + ".collector.drop").c_str(), "w");
    fclose(fout);
    fout = fopen((output_file + ".collector.util").c_str(), "w");
    fclose(fout);
    fout = fopen((output_file + ".collector.count").c_str(), "w");
    fclose(fout);
    fout = fopen((output_file + ".collector.delay").c_str(), "w");
    fclose(fout);
}

uint32_t
CollectorNode::AddDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    uint32_t index = m_devices.size();
    m_devices.push_back(device);
    device->SetNode(this);
    device->SetIfIndex(index);
    device->SetReceiveCallback(MakeCallback(&CollectorNode::ReceiveFromDevice, this));
    Simulator::ScheduleWithContext(GetId(), Seconds(0.0), &NetDevice::Initialize, device);
    NotifyDeviceAdded(device);
    return index;
}

void
CollectorNode::SetOrbWeaver(uint32_t OrbWeaver){
    m_orbweaver = ((OrbWeaver & 0x1) == 0x1);
    m_postcard = (OrbWeaver == 0x2);
    m_basic = ((OrbWeaver & 0x3) == 0x3);
    m_pull = ((OrbWeaver & 0x9) == 0x9);
    m_final = ((OrbWeaver & 0x11) == 0x11);
    m_push = ((OrbWeaver & 0x21) == 0x21);
}

void 
CollectorNode::SetPriority(uint8_t dest, uint32_t priority){
    m_priority[dest] = priority;
}

void 
CollectorNode::SetDeviceGenerateGap(uint32_t devId, uint32_t generateGap){
    DeviceProperty tmp;
    tmp.devId = devId;
    tmp.generateGap = generateGap;
    m_deviceMap[m_devices[devId]] = tmp;
}

Ptr<Packet> 
CollectorNode::CreatePacket(uint8_t priority)
{
    Ptr<Packet> packet = Create<Packet>();
    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(priority);
    packet->ReplacePacketTag(priorityTag);
    return packet;
}

void
CollectorNode::GeneratePacket(){
    if(m_pull || m_final || m_push){
        int64_t nsNow = Simulator::Now().GetNanoSeconds();
        int64_t nextTime = 0xffffffffffffL;

        for(auto it = m_deviceMap.begin();it != m_deviceMap.end();++it){
            if(nsNow >= it->second.m_lastTime + it->second.generateGap){
                it->second.m_lastTime = nsNow;

                for(auto pit = m_priority.begin();pit != m_priority.end();++pit){
                    if(m_pull && pit->second > 0)
                        continue;

                    Ptr<Packet> packet = CreatePacket(2);
                
                    TeleHeader teleHeader;
                    teleHeader.SetDest(pit->first);
                    teleHeader.SetSize(pit->second);
                    packet->AddHeader(teleHeader);

                    (it->first)->Send(packet, (it->first)->GetBroadcast(), 0x0172);
                }
            }
            nextTime = std::min(nextTime, it->second.m_lastTime + it->second.generateGap);
        }
        Simulator::Schedule(NanoSeconds(nextTime - nsNow), &CollectorNode::GeneratePacket, this);
    }
}

void 
CollectorNode::SendPacket(Ptr<NetDevice> dev, Ptr<Packet> packet, uint16_t protocol){
    dev->Send(packet, dev->GetBroadcast(), protocol);
}

bool
CollectorNode::MainCollect(Ptr<Packet> packet, TeleHeader teleHeader){
    m_types.insert(teleHeader.GetType());

    TimestampTag timestamp;
    if (packet->PeekPacketTag(timestamp))
    {
        int64_t sendNs = timestamp.GetTimestamp().GetNanoSeconds();
        int64_t delay = Simulator::Now().GetNanoSeconds() - sendNs;
        delay = delay / 1000;

        if(delay >= m_delaySize)
            delay = m_delaySize - 1;

        m_delay[delay] += 1;
        // std::cout << "delay is " << delay << std::endl;
    }

    switch(teleHeader.GetType()){
        case 1 :
            for(uint32_t i = 0;i < teleHeader.GetSize();++i){
                PathHeader pathHeader;
                packet->RemoveHeader(pathHeader);

                if(m_record && m_paths.find(pathHeader) == m_paths.end()){
                    m_paths.insert(pathHeader);
                    if(m_paths.size() % 10000 == 9999){
                        std::cout << "Receive path entries: " << m_paths.size() << std::endl;
                        std::cout << "Number of duplicates: " << m_duplicates << std::endl;
                    }

                    /*  
                    std::cout << "PathHeader: " << (int)teleHeader.GetDest() << " " 
                        << pathHeader.GetSrcIP() << " "
                        << pathHeader.GetDstIP() << " "
                        << pathHeader.GetSrcPort() << " "
                        << pathHeader.GetDstPort() << " "
                        << pathHeader.GetNodeId() << " "
                        << int(pathHeader.GetTTL()) << " "
                        << std::endl;
                    */
                }
                else
                    m_duplicates += 1;
            }
            break;
        case 2 :
            for(uint32_t i = 0;i < teleHeader.GetSize();++i){
                UtilHeader utilHeader;
                packet->RemoveHeader(utilHeader);

                if(m_utils.find(utilHeader) == m_utils.end())
                    m_utils.insert(utilHeader);

                m_receive[2] += 1;
                        
                if(m_receive[2] % 10000 == 9999)
                    std::cout << "Receive util entries: " << m_receive[2] << std::endl;

                /*
                std::cout << "UtilHeader: " << (int)teleHeader.GetDest() << " " 
                        << utilHeader.GetNodeId() << " "
                        << utilHeader.GetPortId() << " "
                        << utilHeader.GetTime() << " "
                        << utilHeader.GetByte() << " "
                        << std::endl;
                */
            }
            break;
        case 4 :
            for(uint32_t i = 0;i < teleHeader.GetSize();++i){
                DropHeader dropHeader;
                packet->RemoveHeader(dropHeader);

                m_receive[4] += 1;
                        
                if(m_receive[4] % 10000 == 9999)
                    std::cout << "Receive drop entries: " << m_receive[4] << std::endl;

                if(m_record && m_drops.find(dropHeader) == m_drops.end()){
                    m_drops.insert(dropHeader);

                    /*
                    std::cout << "DropHeader: " << (int)teleHeader.GetDest() << " " 
                            << dropHeader.GetSrcIP() << " "
                            << dropHeader.GetDstIP() << " "
                            << dropHeader.GetSrcPort() << " "
                            << dropHeader.GetDstPort() << " "
                            << dropHeader.GetNodeId() << " "
                            << std::endl;
                    */
                }
                
            }
            break;
        case 8:
            for(uint32_t i = 0;i < teleHeader.GetSize();++i){
                CountHeader countHeader;
                packet->RemoveHeader(countHeader);

                m_receive[8] += 1;

                if(m_receive[8] % 10000 == 9999)
                    std::cout << "Receive count entries: " << m_receive[8] << std::endl;

                if(m_record){
                    uint32_t id = countHeader.GetNodeId();
                    uint32_t position = countHeader.GetPosition();

                    uint32_t row = position / OURS_SKETCH_LENGTH;
                    uint32_t column = position % OURS_SKETCH_LENGTH;

                    m_counts[id].values[row][column] = std::max(countHeader.GetCount(),
                        m_counts[id].values[row][column]); 

                    /*
                    std::cout << "DropHeader: " << (int)teleHeader.GetDest() << " " 
                            << dropHeader.GetSrcIP() << " "
                            << dropHeader.GetDstIP() << " "
                            << dropHeader.GetSrcPort() << " "
                            << dropHeader.GetDstPort() << " "
                            << dropHeader.GetNodeId() << " "
                            << std::endl;
                    */
                }
            }
            break;
        default : std::cout << "Unknown task " << teleHeader.GetType() << std::endl; break;
    }
    return true;
}

void 
CollectorNode::BufferData(Ptr<Packet> packet, TeleHeader teleHeader){
    packet->AddHeader(teleHeader);
    uint32_t dest = teleHeader.GetDest();
    m_teleQueue.packets[dest].push(packet);
    m_teleQueue.size[dest] += packet->GetSize();
    m_teleQueue.maxSize[dest] = std::max(m_teleQueue.maxSize[dest], m_teleQueue.size[dest]);
}

Ptr<Packet> 
CollectorNode::GetTelePacket(uint32_t priority, uint8_t dest)
{
    Ptr<Packet> packet = nullptr;

    if(!m_teleQueue.packets[dest].empty()){
        packet = m_teleQueue.packets[dest].front();
        m_teleQueue.packets[dest].pop();
        m_teleQueue.size[dest] -= packet->GetSize();

        SocketPriorityTag priorityTag;
        priorityTag.SetPriority(priority);
        packet->ReplacePacketTag(priorityTag);
    }

    return packet;
}

bool 
CollectorNode::TempStore(Ptr<Packet> packet, TeleHeader teleHeader, uint16_t protocol, Ptr<NetDevice> dev){
    uint8_t dest;
    uint16_t size;
    uint32_t bufferSize;

    switch(protocol){
        case 0x0171 :
            BufferData(packet, teleHeader);
            return true;
        case 0x0172 :
            dest = teleHeader.GetDest();
            size = teleHeader.GetSize();

            bufferSize = m_teleQueue.size[dest];
            if(bufferSize > 0 && size < m_priority[dest]){
                packet = GetTelePacket(1, dest);
                if(packet != nullptr){
                    Simulator::Schedule(NanoSeconds(1), &CollectorNode::SendPacket, this, dev, packet, 0x0171);
                }
                return true;
            }

            if(m_priority[dest] < size){
                teleHeader.SetSize(m_priority[dest]);
                packet->AddHeader(teleHeader);
                Simulator::Schedule(NanoSeconds(1), &CollectorNode::SendPacket, this, dev, packet, 0x0172);
            }
            return true;
        default : 
            std::cout << "Unknown protocol for colllector" << std::endl;
            return false;
    }
}

bool
CollectorNode::ReceiveFromDevice(Ptr<NetDevice> device,
                                  Ptr<const Packet> p,
                                  uint16_t protocol,
                                  const Address& from)
{
    Ptr<Packet> packet = p->Copy();
    TeleHeader teleHeader;
    packet->RemoveHeader(teleHeader);

    if(m_priority.find(teleHeader.GetDest()) == m_priority.end())
        std::cout << "Collector Dest Error" << std::endl;

    if(m_priority[teleHeader.GetDest()] == 0){
        if(protocol == 0x0171)
            return MainCollect(packet, teleHeader);
        std::cout << "Unknown propotol for main collector" << std::endl;
        return false;
    }
    else{
        if(protocol == 0x0171 || protocol == 0x0172){
            if(m_final || m_push)
                return TempStore(packet, teleHeader, protocol, device);
            return true;
        }
        std::cout << "Unknown propotol for temp storage" << std::endl;
        return false;
    }
}

} // namespace ns3
