#include "switch-node.h"

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
#include "point-to-point-net-device.h"
#include "my-queue.h"
#include "ppp-header.h"
#include "timestamp-tag.h"

#include <unordered_set>
#include <unordered_map>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SwitchNode");

NS_OBJECT_ENSURE_REGISTERED(SwitchNode);

typedef std::map<MyFlowId, int32_t> HashMap;

TypeId
SwitchNode::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SwitchNode")
            .SetParent<Node>()
            .SetGroupName("PointToPoint")
            .AddConstructor<SwitchNode>();
    return tid;
}

SwitchNode::SwitchNode() : Node() {
    std::random_device rd;
    m_rng.seed(rd());

    m_orbweaver = false;
    m_table.resize(arrSize);
    Simulator::Schedule(Seconds(1), &SwitchNode::Init, this);
}

SwitchNode::~SwitchNode(){
    //delete m_heap;

    if(m_record){
        std::string out_file;
        FILE* fout;

        out_file = output_file + ".switch.path";
        fout = fopen(out_file.c_str(), "a");
        for(uint32_t dest = 0;dest < m_collector;++dest){
            fprintf(fout, "%d,%d,%d,%d,%d,%d,%ld\n", m_id, dest, m_pathType, 
                m_queueLoss[m_pathType][dest], m_bufferLoss[m_pathType][dest], m_teleSend[m_pathType][dest], m_paths.size());
        }
        fflush(fout);
        fclose(fout);
        
        out_file = output_file + ".switch.util";
        fout = fopen(out_file.c_str(), "a");
        for(uint32_t dest = 0;dest < m_collector;++dest){
            fprintf(fout, "%d,%d,%d,%d,%d,%d,%ld\n", m_id, dest, m_portType, 
                m_queueLoss[m_portType][dest], m_bufferLoss[m_portType][dest], m_teleSend[m_portType][dest], m_paths.size());
        }
        fflush(fout);
        fclose(fout);

        out_file = output_file + ".switch.drop";
        fout = fopen(out_file.c_str(), "a");
        for(uint32_t dest = 0;dest < m_collector;++dest){
            fprintf(fout, "%d,%d,%d,%d,%d,%d,%ld\n", m_id, dest, m_dropType, 
                m_queueLoss[m_dropType][dest], m_bufferLoss[m_dropType][dest], m_teleSend[m_dropType][dest], m_paths.size());
        }
        fflush(fout);
        fclose(fout);
        
        out_file = output_file + ".switch.count";
        fout = fopen(out_file.c_str(), "a");
        for(uint32_t dest = 0;dest < m_collector;++dest){
            fprintf(fout, "%d,%d,%d,%d,%d,%d,%ld\n", m_id, dest, m_countType, 
                m_queueLoss[m_countType][dest], m_bufferLoss[m_countType][dest], m_teleSend[m_countType][dest], m_paths.size());
        }
        fflush(fout);
        fclose(fout);
    }

    if(m_record){
        if(m_path){
            FILE* fout = fopen((output_file + ".switch.path.data").c_str(), "a");
            for(auto path : m_paths){
                fprintf(fout, "%d %d ", path.GetSrcIP(), path.GetDstIP());
                fprintf(fout, "%d %d ", path.GetSrcPort(), path.GetDstPort());
                fprintf(fout, "%d %d ", path.GetNodeId(), (int)path.GetProtocol());
                fprintf(fout, "%d\n", (int)path.GetTTL());
                fflush(fout);
            }
            fclose(fout);
        }
        if(m_port){
            FILE* fout = fopen((output_file + ".switch.util.data").c_str(), "a");
            for(auto util : m_utils){
                fprintf(fout, "%d %d ", util.GetNodeId(), util.GetPortId());
                fprintf(fout, "%d %d\n", util.GetTime(), util.GetByte());
                fflush(fout);
            }
            fclose(fout);
        }
        if(m_drop){
            FILE* fout = fopen((output_file + ".switch.drop.data").c_str(), "a");
            for(auto drop : m_drops){
                fprintf(fout, "%d %d ", drop.GetSrcIP(), drop.GetDstIP());
                fprintf(fout, "%d %d ", drop.GetSrcPort(), drop.GetDstPort());
                fprintf(fout, "%d %d ", drop.GetNodeId(), (int)drop.GetProtocol());
                fprintf(fout, "%d\n", drop.GetTime());
                fflush(fout);
            }
            fclose(fout);
        }
        if(m_count){
            FILE* fout = fopen((output_file + ".switch.count.sketch").c_str(), "a");
            for(int i = 0;i < OURS_SKETCH_HASH;++i){
                fprintf(fout, "%d", m_id);
                for(int j = 0;j < OURS_SKETCH_LENGTH;++j){
                    fprintf(fout, " %d", m_sketch.values[i][j]);
                }
                fprintf(fout, "\n");
                fflush(fout);
            }
            fclose(fout);

            fout = fopen((output_file + ".switch.count.old").c_str(), "a");
            for(int i = 0;i < OURS_SKETCH_HASH;++i){
                fprintf(fout, "%d", m_id);
                for(int j = 0;j < OURS_SKETCH_LENGTH;++j){
                    fprintf(fout, " %d", m_send_sketch.values[i][j]);
                }
                fprintf(fout, "\n");
                fflush(fout);
            }
            fclose(fout);

            fout = fopen((output_file + ".switch.count.data").c_str(), "a");
            for(auto it = m_counts.begin();it != m_counts.end();++it){
                fprintf(fout, "%d %d %d ", m_id, it->first.m_srcIP, it->first.m_dstIP);
                fprintf(fout, "%d %d ", it->first.m_srcPort, it->first.m_dstPort);
                fprintf(fout, "%d\n", it->second);
                fflush(fout);
            }
            fclose(fout);
        }
    }
}

void 
SwitchNode::Init(){
    Simulator::Schedule(Seconds(m_measureStart - 1.000002), &SwitchNode::GeneratePacket, this);
    Simulator::Schedule(Seconds(m_measureStart - 1), &SwitchNode::StartMeasure, this);
    Simulator::Schedule(Seconds(m_measureStart - 1), &SwitchNode::RecordUtil, this);
    Simulator::Schedule(Seconds(m_measureStart - 1), &SwitchNode::GenerateUtil, this);
    Simulator::Schedule(Seconds(m_measureEnd - 1), &SwitchNode::EndMeasure, this);
}

void 
SwitchNode::StartMeasure(){
    m_measure = true;
}
    
void 
SwitchNode::EndMeasure(){
    m_measure = false;
}

uint32_t 
SwitchNode::GetBufferSize(){
    uint32_t bufferSize = m_teleQueue.size[0] + m_teleQueue.size[1];
    
    for(uint32_t i = 0;i < m_devices.size();++i){
        auto dev = DynamicCast<PointToPointNetDevice>(m_devices[i]);
        if(dev != nullptr)
            bufferSize += dev->GetQueue()->GetNBytes();
    }

    return bufferSize;
}

void
SwitchNode::SetOrbWeaver(uint32_t OrbWeaver){
    m_orbweaver = ((OrbWeaver & 0x1) == 0x1);
    m_postcard = (OrbWeaver == 0x2);
    m_basic = ((OrbWeaver & 0x3) == 0x3);
    m_pull = ((OrbWeaver & 0x9) == 0x9);
    m_final = ((OrbWeaver & 0x11) == 0x11);
    m_push = ((OrbWeaver & 0x21) == 0x21);
}

void 
SwitchNode::SetPath(int8_t pathType = 1){
    m_path = true;
    m_pathType = pathType;
}

void 
SwitchNode::SetPort(int8_t portType = 2){
    m_port = true;
    m_portType = portType;
}

void 
SwitchNode::SetDrop(int8_t dropType = 4){
    m_drop = true;
    m_dropType = dropType;
}

void 
SwitchNode::SetCount(int8_t countType = 8){
    m_count = true;
    m_countType = countType;
}

void 
SwitchNode::SetGenerate(int64_t bandwidth){
    m_generate = true;
    m_generateGap = ((double)1e9) / bandwidth * 1024.0;
}

void 
SwitchNode::SetThd(double thd){
    m_thd = thd;
}

void 
SwitchNode::SetTeleThd(uint32_t teleThd){
    m_teleThd = teleThd;
}

void 
SwitchNode::SetUtilGap(uint32_t utilGap){
    m_utilGap = utilGap;
}

void
SwitchNode::SetHashSeed(uint32_t hashSeed){
    m_hashSeed = hashSeed;
}

void 
SwitchNode::SetEcmp(uint32_t ecmp){
    m_ecmp = ecmp;
}

void 
SwitchNode::SetRecord(uint32_t record){
    m_record = record;
}

void 
SwitchNode::SetCollector(uint32_t number){
    m_collector = number;
}

void 
SwitchNode::SetTime(double measureStart, double measureEnd){
    m_measureStart = measureStart;
    m_measureEnd = measureEnd;
}

void 
SwitchNode::SetOutput(std::string output){
    output_file = output;
    FILE* fout;
    if(m_record){
        if(m_path){
            std::string out_file = output_file + ".switch.path";
            fout = fopen(out_file.c_str(), "w");
            fclose(fout);
            fout = fopen((out_file + ".data").c_str(), "w");
            fclose(fout);
        }
        if(m_port){
            std::string out_file = output_file + ".switch.util";
            fout = fopen(out_file.c_str(), "w");
            fclose(fout);
            fout = fopen((out_file + ".data").c_str(), "w");
            fclose(fout);
        }
        if(m_drop){
            std::string out_file = output_file + ".switch.drop";
            fout = fopen(out_file.c_str(), "w");
            fclose(fout);
            fout = fopen((out_file + ".data").c_str(), "w");
            fclose(fout);
        }
        if(m_count){
            std::string out_file = output_file + ".switch.count";
            fout = fopen(out_file.c_str(), "w");
            fclose(fout);
            fout = fopen((out_file + ".data").c_str(), "w");
            fclose(fout);
            fout = fopen((out_file + ".sketch").c_str(), "w");
            fclose(fout);
            fout = fopen((out_file + ".old").c_str(), "w");
            fclose(fout);
        }
    }
}

void
SwitchNode::AddHostRouteTo(Ipv4Address dest, uint32_t devId)
{
    m_routeForward[dest.Get()].push_back(devId);
    m_deviceMap[m_devices[devId]].devId = devId;
    m_bytes[devId] = 0;
}

void 
SwitchNode::AddTeleRouteTo(uint8_t dest, uint32_t devId)
{
    m_teleForward[dest].push_back(devId);

    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Unknown devId " << devId << " in deviceMap for collector" << std::endl;
    else{
        if(std::find(m_deviceMap[m_devices[devId]].collectorDst.begin(), 
        m_deviceMap[m_devices[devId]].collectorDst.end(), dest) != m_deviceMap[m_devices[devId]].collectorDst.end())
            std::cout << "Error in AddTeleRouteTo" << std::endl;
        m_deviceMap[m_devices[devId]].collectorDst.push_back(dest);
    }
}

void 
SwitchNode::SetDeviceGenerateGap(uint32_t devId, uint32_t generateGap){
    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Cannot find " << devId << std::endl;
    else if(m_deviceMap[m_devices[devId]].devId != devId)
        std::cout << "Mismatch " << devId << "," << m_deviceMap[m_devices[devId]].devId << std::endl;
    m_deviceMap[m_devices[devId]].devId = devId;
    m_deviceMap[m_devices[devId]].generateGap = generateGap;
}

void
SwitchNode::SetDeviceUpperPull(uint8_t dest, uint32_t devId)
{
    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Unknown devId " << devId << " in deviceMap for upper pull" << std::endl;
    else{
        if(std::find(m_deviceMap[m_devices[devId]].collectorDst.begin(), 
        m_deviceMap[m_devices[devId]].collectorDst.end(), dest) != m_deviceMap[m_devices[devId]].collectorDst.end())
            std::cout << "Error in SetDeviceUpperPull" << std::endl;
        m_deviceMap[m_devices[devId]].collectorDst.push_back(dest);
        m_deviceMap[m_devices[devId]].isUpperPull[dest] = true;
    }
}

void
SwitchNode::SetDeviceLowerPull(uint8_t dest, uint32_t devId)
{
    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Unknown devId " << devId << " in deviceMap for lower pull" << std::endl;
    else{
        if(std::find(m_deviceMap[m_devices[devId]].collectorDst.begin(), 
        m_deviceMap[m_devices[devId]].collectorDst.end(), dest) != m_deviceMap[m_devices[devId]].collectorDst.end())
            std::cout << "Error in SetDeviceLowerPull" << std::endl;
        m_deviceMap[m_devices[devId]].collectorDst.push_back(dest);
        m_deviceMap[m_devices[devId]].isLowerPull[dest] = true;
    }
}

uint32_t
SwitchNode::AddDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    uint32_t index = m_devices.size();
    m_devices.push_back(device);
    device->SetNode(this);
    device->SetIfIndex(index);
    device->SetReceiveCallback(MakeCallback(&SwitchNode::ReceiveFromDevice, this));
    Simulator::ScheduleWithContext(GetId(), Seconds(0.0), &NetDevice::Initialize, device);
    NotifyDeviceAdded(device);
    return index;
}

Ptr<Packet> 
SwitchNode::CreatePacket(uint8_t priority)
{
    Ptr<Packet> packet = Create<Packet>();
    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(priority);
    packet->ReplacePacketTag(priorityTag);
    TimestampTag timestamp;
    timestamp.SetTimestamp(Simulator::Now());
    packet->ReplacePacketTag(timestamp);
    return packet;
}

bool
SwitchNode::BatchPath(PathHeader path, uint8_t dest){
    m_teleQueue.pathBatch[dest].push_back(path);
    if(m_teleQueue.pathBatch[dest].size() % batchSize == 0){
        m_teleSend[m_pathType][dest] += batchSize;

        Ptr<Packet> packet = CreatePacket(0);
        for(uint32_t i = 0;i < batchSize;++i){
            if(m_record && m_paths.find(m_teleQueue.pathBatch[dest][i]) == m_paths.end())
                m_paths.insert(m_teleQueue.pathBatch[dest][i]);   
            packet->AddHeader(m_teleQueue.pathBatch[dest][i]);
        }
        m_teleQueue.pathBatch[dest].clear();

        TeleHeader teleHeader;
        teleHeader.SetType(m_pathType);
        teleHeader.SetDest(dest);
        teleHeader.SetSize(batchSize);
        packet->AddHeader(teleHeader);

        if(m_teleQueue.size[dest] + packet->GetSize() > m_teleThd){
            m_bufferLoss[m_pathType][dest] += batchSize;
            if(m_bufferLoss[m_pathType][dest] % 100 == 0){
                std::cout << "BatchPath Buffer loss 100 in " << m_id << std::endl;
            }
            return false;
        }
        m_teleQueue.packets[dest].push(packet);
        m_teleQueue.size[dest] += packet->GetSize();
        return true;
    }

    return false;
}

bool
SwitchNode::BatchUtil(UtilHeader util, uint8_t dest){
    m_teleQueue.utilBatch[dest].push_back(util);
    if(m_teleQueue.utilBatch[dest].size() % batchSize == 0){
        m_teleSend[m_portType][dest] += batchSize;

        Ptr<Packet> packet = CreatePacket(0);
        for(uint32_t i = 0;i < batchSize;++i)
            packet->AddHeader(m_teleQueue.utilBatch[dest][i]);
        m_teleQueue.utilBatch[dest].clear();

        TeleHeader teleHeader;
        teleHeader.SetType(m_portType);
        teleHeader.SetDest(dest);
        teleHeader.SetSize(batchSize);
        packet->AddHeader(teleHeader);

        if(m_teleQueue.size[dest] + packet->GetSize() > m_teleThd){
            m_bufferLoss[m_portType][dest] += batchSize;
            if(m_bufferLoss[m_portType][dest] % 100 == 0){
                std::cout << "BatchUtil Buffer loss 100 in " << m_id << std::endl;
            }
            return false;
        }
        m_teleQueue.packets[dest].push(packet);
        m_teleQueue.size[dest] += packet->GetSize();
        return true;
    }

    return false;
}

bool
SwitchNode::BatchDrop(DropHeader drop, uint8_t dest){
    m_teleQueue.dropBatch[dest].push_back(drop);
    if(m_teleQueue.dropBatch[dest].size() % batchSize == 0){
        m_teleSend[m_dropType][dest] += batchSize;

        Ptr<Packet> packet = CreatePacket(0);
        for(uint32_t i = 0;i < batchSize;++i){
            if(m_record && m_drops.find(m_teleQueue.dropBatch[dest][i]) == m_drops.end())
                m_drops.insert(m_teleQueue.dropBatch[dest][i]);   
            packet->AddHeader(m_teleQueue.dropBatch[dest][i]);
        }
        m_teleQueue.dropBatch[dest].clear();

        TeleHeader teleHeader;
        teleHeader.SetType(m_dropType);
        teleHeader.SetDest(dest);
        teleHeader.SetSize(batchSize);
        packet->AddHeader(teleHeader);

        if(m_teleQueue.size[dest] + packet->GetSize() > m_teleThd){
            m_bufferLoss[m_dropType][dest] += batchSize;
            if(m_bufferLoss[m_dropType][dest] % 100 == 0){
                std::cout << "BatchDrop Buffer loss 100 in " << m_id << std::endl;
            }
            return false;
        }
        m_teleQueue.packets[dest].push(packet);
        m_teleQueue.size[dest] += packet->GetSize();
        return true;
    }

    return false;
}

bool
SwitchNode::BatchCount(CountHeader count, uint8_t dest){
    m_teleQueue.countBatch[dest].push_back(count);
    if(m_teleQueue.countBatch[dest].size() % batchSize == 0){
        m_teleSend[m_countType][dest] += batchSize;

        Ptr<Packet> packet = CreatePacket(0);
        for(uint32_t i = 0;i < batchSize;++i){
            packet->AddHeader(m_teleQueue.countBatch[dest][i]);

            CountHeader countHeader = m_teleQueue.countBatch[dest][i];
            uint32_t position = countHeader.GetPosition();

            uint32_t row = position / OURS_SKETCH_LENGTH;
            uint32_t column = position % OURS_SKETCH_LENGTH;

            m_send_sketch.values[row][column] = std::max(countHeader.GetCount(),
                        m_send_sketch.values[row][column]); 
        }
        m_teleQueue.countBatch[dest].clear();

        TeleHeader teleHeader;
        teleHeader.SetType(m_countType);
        teleHeader.SetDest(dest);
        teleHeader.SetSize(batchSize);
        packet->AddHeader(teleHeader);

        if(m_teleQueue.size[dest] + packet->GetSize() > m_teleThd){
            m_bufferLoss[m_countType][dest] += batchSize;
            if(m_bufferLoss[m_countType][dest] % 100 == 0){
                std::cout << "BatchCount Buffer loss 100 in " << m_id << std::endl;
            }
            return false;
        }
        m_teleQueue.packets[dest].push(packet);
        m_teleQueue.size[dest] += packet->GetSize();
        return true;
    }

    return false;
}

Ptr<Packet> 
SwitchNode::GetTelePacket(uint32_t priority, uint8_t dest)
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

void 
SwitchNode::SendPostcard(uint8_t dest){
    Ptr<Packet> packet = GetTelePacket(0, dest);

    if(packet != nullptr){
        auto vec = m_teleForward[dest];
        uint32_t devId = vec[rand() % vec.size()];
        Ptr<NetDevice> dev = m_devices[devId];
        if(m_teleQueue.size[dest] + packet->GetSize() <= m_teleThd){
            m_teleQueue.size[dest] += packet->GetSize();
            dev->Send(packet, dev->GetBroadcast(), 0x0171);
        }
        else{
            TeleHeader teleHeader;
            packet->RemoveHeader(teleHeader);
            m_queueLoss[teleHeader.GetType()][teleHeader.GetDest()] += batchSize;
            if(m_queueLoss[teleHeader.GetType()][teleHeader.GetDest()] % 100 == 0)
                std::cout << "Send postcard loss 100 in " << m_id << std::endl;
        }   
    }
}

void 
SwitchNode::GenerateUtil(){
    if(!m_generate)
        return;
    
    if(m_orbweaver || m_postcard){
        for(int i = 0;i < 64;++i){
            UtilHeader utilHeader;
            uint8_t dest = Hash32((char*)&m_id, sizeof(m_id)) % m_collector;
            if(BatchUtil(utilHeader, dest) && m_postcard)
                SendPostcard(dest);
        }

        Simulator::Schedule(NanoSeconds(m_generateGap), &SwitchNode::GenerateUtil, this);
    }
}

void 
SwitchNode::RecordUtil(){
    if(!m_port)
        return;

    if(m_orbweaver || m_postcard){
        Simulator::Schedule(NanoSeconds(m_utilGap), &SwitchNode::RecordUtil, this);
        if(!m_measure)
            return;

        for(auto it = m_bytes.begin();it != m_bytes.end();++it){
            UtilHeader utilHeader;
            utilHeader.SetNodeId(m_id);
            utilHeader.SetPortId(it->first);
            utilHeader.SetTime(Simulator::Now().GetMicroSeconds());
            utilHeader.SetByte(it->second);
            if(m_utils.find(utilHeader) == m_utils.end())
                m_utils.insert(utilHeader);

            uint8_t dest = Hash32((char*)&m_id, sizeof(m_id)) % m_collector;
            if(BatchUtil(utilHeader, dest) && m_postcard)
                SendPostcard(dest);

            it->second = 0;
        }
    }           
}

void
SwitchNode::GeneratePacket(){
    if(m_orbweaver){
        int64_t nsNow = Simulator::Now().GetNanoSeconds();
        int64_t nextTime = 0xffffffffffffL;

        for(auto it = m_deviceMap.begin();it != m_deviceMap.end();++it){
            if(it->second.generateGap > 0){
                if(nsNow >= it->second.m_lastTime + it->second.generateGap){
                    it->second.m_lastTime = nsNow;
                    Ptr<Packet> packet = CreatePacket(3);
                    if(packet != nullptr){
                        (it->first)->Send(packet, (it->first)->GetBroadcast(), 0x0170);
                    }
                }
                nextTime = std::min(nextTime, it->second.m_lastTime + it->second.generateGap);
            }
        }
        Simulator::Schedule(NanoSeconds(nextTime - nsNow), &SwitchNode::GeneratePacket, this);
    }
}

void 
SwitchNode::BufferData(Ptr<Packet> packet){
    if(m_basic){
        std::cout << "Basic in Buffer?" << std::endl;
        return;
    }

    TeleHeader teleHeader;
    packet->PeekHeader(teleHeader);

    if(m_teleQueue.size[teleHeader.GetDest()] + packet->GetSize() > m_teleThd){
        m_bufferLoss[teleHeader.GetType()][teleHeader.GetDest()] += batchSize;
        if(m_bufferLoss[teleHeader.GetType()][teleHeader.GetDest()] % 100 == 0){
            std::cout << "BufferData Buffer loss 100 in " << m_id << std::endl;
        }
    }
    else{
        m_teleQueue.packets[teleHeader.GetDest()].push(packet);
        m_teleQueue.size[teleHeader.GetDest()] += packet->GetSize();
    }
}

bool
SwitchNode::IngressPipelineUser(Ptr<Packet> packet)
{
    // Start Parse Header
    Ipv4Header ipHeader;
    TcpHeader tcpHeader;

    packet->RemoveHeader(ipHeader);
    packet->RemoveHeader(tcpHeader);
    // End Parse Header

    uint8_t proto = ipHeader.GetProtocol();
    uint8_t ttl = ipHeader.GetTtl();
    uint32_t src = ipHeader.GetSource().Get();
    uint32_t dst = ipHeader.GetDestination().Get();

    uint16_t srcPort = tcpHeader.GetSourcePort();
    uint16_t dstPort = tcpHeader.GetDestinationPort();

    auto vec = m_routeForward[dst];

    if(vec.size() <= 0){
        std::cout << "Unknown Destination for Routing" << std::endl;
        return false;
    }
    if(ttl == 0)
        return false;
    if(proto != 6){
        std::cout << "Unknown Protocol" << std::endl;
        return false;
    }

    ipHeader.SetTtl(ttl - 1);

    // Start Deparse Header
    packet->AddHeader(tcpHeader);
    packet->AddHeader(ipHeader);
    // End Deparse Header

    uint32_t devId = -1;
    uint32_t hashValue = 0;

    MyFlowId flowId;

    flowId.m_srcIP = src;
    flowId.m_dstIP = dst;
    flowId.m_srcPort = srcPort;
    flowId.m_dstPort = dstPort;

    if(m_ecmp == 1){
        dst = dst + m_hashSeed;
        hashValue = Hash32((char*)&dst, sizeof(dst));
    }
    else{
        hashValue = hash(flowId, m_hashSeed);
    }

    devId = vec[hashValue % vec.size()];
    Ptr<NetDevice> dev = m_devices[devId];

    if(m_userSize + packet->GetSize() <= m_userThd){
        m_userSize += packet->GetSize();
        return dev->Send(packet, dev->GetBroadcast(), 0x0800);
    }

    //std::cout << "Drop in switch" << std::endl;
    if(m_measure && m_drop && (m_orbweaver || m_postcard)){
        DropHeader dropHeader;
        
        dropHeader.SetSrcIP(src);
        dropHeader.SetDstIP(dst);
        dropHeader.SetSrcPort(srcPort);
        dropHeader.SetDstPort(dstPort);
        dropHeader.SetProtocol(proto);
        dropHeader.SetNodeId(m_id);
        dropHeader.SetTime(Simulator::Now().GetMicroSeconds());

        uint8_t dest = Hash32((char*)&m_id, sizeof(m_id)) % m_collector;
        if(BatchDrop(dropHeader, dest) && m_postcard)
            Simulator::Schedule(NanoSeconds(1), &SwitchNode::SendPostcard, this, dest);
    }      

    return false;
}

bool 
SwitchNode::EgressPipelineUser(Ptr<Packet> packet){
    //Only count data packets (no SYN/ACK)
    if(!m_measure || packet->GetSize() <= 56)
        return true;

    if(!m_orbweaver && !m_postcard)
        return true;
    
    Ipv4Header ipHeader;
    TcpHeader tcpHeader;

    MyFlowId flowId;
    uint8_t proto, ttl;

    if(m_path || m_count){
        // Start Parse Header
        packet->RemoveHeader(ipHeader);
        packet->RemoveHeader(tcpHeader);
        // End Parse Header

        proto = ipHeader.GetProtocol();
        ttl = ipHeader.GetTtl();

        flowId.m_srcIP = ipHeader.GetSource().Get();
        flowId.m_dstIP = ipHeader.GetDestination().Get();

        flowId.m_srcPort = tcpHeader.GetSourcePort();
        flowId.m_dstPort = tcpHeader.GetDestinationPort();

        // Start Deparse Header
        packet->AddHeader(tcpHeader);
        packet->AddHeader(ipHeader);
        // End Deparse Header
    }

    if(m_path){
        PathHeader pathHeader;
        memset((char*)(&pathHeader), 0, sizeof(PathHeader));

        pathHeader.SetSrcIP(flowId.m_srcIP);
        pathHeader.SetDstIP(flowId.m_dstIP);
        pathHeader.SetSrcPort(flowId.m_srcPort);
        pathHeader.SetDstPort(flowId.m_dstPort);
        pathHeader.SetProtocol(proto);

        pathHeader.SetNodeId(m_id);
        pathHeader.SetTTL(ttl);
        
        uint32_t arrIndex = pathHeader.Hash() % m_table.size();
        if(m_table[arrIndex].Empty() || !(m_table[arrIndex] == pathHeader)){
            m_table[arrIndex] = pathHeader;

            uint8_t dest = Hash32((char*)(&flowId.m_dstIP), sizeof(flowId.m_dstIP)) % m_collector;
            if(BatchPath(pathHeader, dest) && m_postcard)
                Simulator::Schedule(NanoSeconds(1), &SwitchNode::SendPostcard, this, dest);
        }
    }

    if(m_count){
        m_counts[flowId] += 1;

        /* Sketch algorithm */
        for(uint32_t hashPos = 0;hashPos < OURS_SKETCH_HASH;++hashPos){
            uint32_t pos = hash(flowId, hashPos) % OURS_SKETCH_LENGTH;
            m_sketch.values[hashPos][pos] += 1;

            if(m_sketch.values[hashPos][pos] > m_old.values[hashPos][pos] * (1.0 + m_thd)){
                CountHeader countHeader;
                countHeader.SetNodeId(m_id);
                countHeader.SetPosition(hashPos * OURS_SKETCH_LENGTH + pos);
                countHeader.SetCount(m_sketch.values[hashPos][pos]);

                uint8_t dest = Hash32((char*)&m_id, sizeof(m_id)) % m_collector;
                if(BatchCount(countHeader, dest) && m_postcard)
                    Simulator::Schedule(NanoSeconds(1), &SwitchNode::SendPostcard, this, dest);
                    
                m_old.values[hashPos][pos] = m_sketch.values[hashPos][pos];
            }
        }
    }

    return true;
}

bool 
SwitchNode::IngressPipelinePush(Ptr<Packet> packet, Ptr<NetDevice> dev){
    TeleHeader teleHeader;
    packet->PeekHeader(teleHeader);

    if(m_basic){
        auto vec = m_teleForward[teleHeader.GetDest()];
        uint32_t devId = vec[rand() % vec.size()];
        dev = m_devices[devId];

        if(m_teleQueue.size[teleHeader.GetDest()] + packet->GetSize() <= m_teleThd){
            m_teleQueue.size[teleHeader.GetDest()] += packet->GetSize();
            return dev->Send(packet, dev->GetBroadcast(), 0x0171);
        }

        m_queueLoss[teleHeader.GetType()][teleHeader.GetDest()] += batchSize;
        if(m_queueLoss[teleHeader.GetType()][teleHeader.GetDest()] % 100 == 0)
            std::cout << "IngressPipelinePush loss 100 in " << m_id << std::endl;
        return false;
    }
    else{
        dev = m_devices[m_devices.size() - 1];
        return dev->Send(packet, dev->GetBroadcast(), 0x0171);
    }

    return false;
}

bool 
SwitchNode::IngressPipelinePull(Ptr<Packet> packet, Ptr<NetDevice> dev){
    if(m_basic){
        std::cout << "Unknown config (pull in basic/push)" << std::endl;
        return false;
    }
    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(2);
    packet->ReplacePacketTag(priorityTag);
    return dev->Send(packet, dev->GetBroadcast(), 0x0172);
}

bool 
SwitchNode::IngressPipelinePostcard(Ptr<Packet> packet, Ptr<NetDevice> dev){
    TeleHeader teleHeader;
    packet->PeekHeader(teleHeader);

    auto vec = m_teleForward[teleHeader.GetDest()];
    uint32_t devId = vec[rand() % vec.size()];
    dev = m_devices[devId];

    if(m_teleQueue.size[teleHeader.GetDest()] + packet->GetSize() <= m_teleThd){
        m_teleQueue.size[teleHeader.GetDest()] += packet->GetSize();
        return dev->Send(packet, dev->GetBroadcast(), 0x0171);
    }

    m_queueLoss[teleHeader.GetType()][teleHeader.GetDest()] += batchSize;
    if(m_queueLoss[teleHeader.GetType()][teleHeader.GetDest()] % 100 == 0)
        std::cout << "IngressPipeline postcard loss 100 in " << m_id << std::endl;
    return false;
}

Ptr<Packet>
SwitchNode::EgressPipelineSeed(Ptr<Packet> packet, Ptr<NetDevice> dev){
    DeviceProperty property = m_deviceMap[dev];

    PppHeader ppp;
    packet->RemoveHeader(ppp);
    if(m_basic){
        for(auto dest : property.collectorDst){
            if(!m_teleQueue.packets[dest].empty()){
                packet = GetTelePacket(1, dest);
                if(packet != nullptr){
                    ppp.SetProtocol(0x171);
                    packet->AddHeader(ppp);
                }
                return packet;
            }
        }
    }
    else{
        uint8_t dest = property.collectorDst[rand() % property.collectorDst.size()];
        
        if(m_pull && property.isLowerPull[dest]){
            if(m_teleQueue.size[dest] < m_teleThd * 0.5){
                TeleHeader teleHeader;
                teleHeader.SetDest(dest);
                teleHeader.SetSize(0);
                packet->AddHeader(teleHeader);

                ppp.SetProtocol(0x172);
                packet->AddHeader(ppp);
                return packet;
            }
            return nullptr;
        }
        else if((m_final || m_push) && (property.isUpperPull[dest] || property.isLowerPull[dest])){
            int32_t upperBound = m_teleThd * 0.95;

            if(m_push && m_teleQueue.size[dest] > upperBound){
                packet = GetTelePacket(1, dest);
                if(packet != nullptr){
                    ppp.SetProtocol(0x171);
                    packet->AddHeader(ppp);
                }
                return packet;
            }
            else{
                bool send = true;

                if(m_push){
                    send = (m_teleQueue.size[dest] <= (m_rng() % upperBound));
                }

                if(send){
                    TeleHeader teleHeader;
                    teleHeader.SetDest(dest);
                    teleHeader.SetSize(m_teleQueue.size[dest]);
                    packet->AddHeader(teleHeader);

                    ppp.SetProtocol(0x172);
                    packet->AddHeader(ppp);
                    return packet;
                }
                else{
                    return nullptr;
                }
            }
        }
    }
    return nullptr;
}

Ptr<Packet>
SwitchNode::EgressPipelinePush(Ptr<Packet> packet, Ptr<NetDevice> dev){
    if(m_basic){
        return packet;
    }
    else{
        PppHeader ppp;
        packet->RemoveHeader(ppp);
        BufferData(packet);
        return nullptr;
    }
}

Ptr<Packet>
SwitchNode::EgressPipelinePull(Ptr<Packet> packet, Ptr<NetDevice> dev){
    PppHeader ppp;
    packet->RemoveHeader(ppp);

    TeleHeader teleHeader;
    packet->RemoveHeader(teleHeader);

    uint8_t dest = teleHeader.GetDest();
    uint32_t size = teleHeader.GetSize();

    if(m_final || m_push){
        bool isUpper = false, isLower = false;
        uint32_t bufferSize = 0;

        if(m_deviceMap.find(dev) != m_deviceMap.end()){
            DeviceProperty property = m_deviceMap[dev];
            isUpper = property.isUpperPull[dest];
            isLower = property.isLowerPull[dest];
            bufferSize = m_teleQueue.size[dest];
        }
        else{
            std::cout << "Cannot find dev in EgressPipelinePull" << std::endl;
        }

        if(isUpper){
            if(size < bufferSize){
                packet = GetTelePacket(1, dest);
                if(packet != nullptr){
                    ppp.SetProtocol(0x171);
                    packet->AddHeader(ppp);
                }
                return packet;
            }
            else if(size == 0 && bufferSize == 0){
                //Speedup simulation
                return nullptr;
            }
            else{
                teleHeader.SetSize(bufferSize);
                packet->AddHeader(teleHeader);

                ppp.SetProtocol(0x172);
                packet->AddHeader(ppp);
                return packet;
            }
        }
        else if(isLower){
            if(size + 1500 < bufferSize){
                packet = GetTelePacket(1, dest);
                if(packet != nullptr){
                    ppp.SetProtocol(0x171);
                    packet->AddHeader(ppp);
                }
                return packet;
            }
            else if(size == 0 && bufferSize == 0){
                //Speedup simulation
                return nullptr;
            }
            else{
                teleHeader.SetSize(bufferSize);
                packet->AddHeader(teleHeader);

                ppp.SetProtocol(0x172);
                packet->AddHeader(ppp);
                return packet;
            }
        }

        packet = GetTelePacket(1, dest);
        if(packet != nullptr){
            ppp.SetProtocol(0x171);
            packet->AddHeader(ppp);
        }
        return packet;
    }
    else if(m_pull){
        packet = GetTelePacket(1, dest);
        if(packet != nullptr){
            ppp.SetProtocol(0x171);
            packet->AddHeader(ppp);
        }
        return packet;
    }
    else
        std::cout << "EgressPipelinePull for other protocol?" << std::endl;
       
    return nullptr;
}

Ptr<Packet>
SwitchNode::EgressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev){
    PppHeader ppp;
    packet->RemoveHeader(ppp);

    if(priority == 0){
        if(protocol == 0x0171){
            TeleHeader teleHeader;
            packet->PeekHeader(teleHeader);

            m_teleQueue.size[teleHeader.GetDest()] -= packet->GetSize();
            if(m_teleQueue.size[teleHeader.GetDest()] < 0)
                std::cout << "Error for teleSize" << std::endl;

            packet->AddHeader(ppp);
            return packet;
        }
        else{
            m_userSize -= packet->GetSize();
            if(m_userSize < 0)
                std::cout << "Error for userSize" << std::endl;

            if(m_deviceMap.find(dev) == m_deviceMap.end())
                std::cout << "Error in find dev" << std::endl;
            m_bytes[m_deviceMap[dev].devId] += packet->GetSize();

            EgressPipelineUser(packet);

            packet->AddHeader(ppp);
            return packet;
        }
    }
    else{
        if(protocol == 0x0171 && m_basic){
            TeleHeader teleHeader;
            packet->PeekHeader(teleHeader);

            m_teleQueue.size[teleHeader.GetDest()] -= packet->GetSize();
            if(m_teleQueue.size[teleHeader.GetDest()] < 0)
                std::cout << "Error for queueSize" << std::endl;
        }
        packet->AddHeader(ppp);

        if(protocol == 0x0170)
            return EgressPipelineSeed(packet, dev);
        else if(protocol == 0x0171)
            return EgressPipelinePush(packet, dev);
        else if(protocol == 0x0172)
            return EgressPipelinePull(packet, dev);
        else{
            std::cout << "Unknown Protocol for EgressPipeline" << std::endl;
            return nullptr;
        }
    }
}

bool
SwitchNode::IngressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev){
    if(priority == 0){
        if(protocol == 0x0171)
            return IngressPipelinePostcard(packet, dev);
        return IngressPipelineUser(packet);
    }
    else{
        if(protocol == 0x0171)
            return IngressPipelinePush(packet, dev);
        else if(protocol == 0x0172)
            return IngressPipelinePull(packet, dev);
        else{
            std::cout << "Unknown Protocol for IngressPipeline" << std::endl;
            return false;
        }
    }
}

bool
SwitchNode::ReceiveFromDevice(Ptr<NetDevice> device,
                                  Ptr<const Packet> p,
                                  uint16_t protocol,
                                  const Address& from)
{
    Ptr<Packet> packet = p->Copy();

    uint32_t priority = 0;
    SocketPriorityTag priorityTag;
    if(packet->PeekPacketTag(priorityTag))
        priority = priorityTag.GetPriority();

    return IngressPipeline(packet, priority, protocol, device);
}

/* Hash function */
uint32_t
SwitchNode::rotateLeft(uint32_t x, unsigned char bits)
{
    return (x << bits) | (x >> (32 - bits));
}

uint32_t
SwitchNode::inhash(const uint8_t* data, uint64_t length, uint32_t seed)
{
    seed = prime[seed];
    uint32_t state[4] = {seed + Prime[0] + Prime[1],
                        seed + Prime[1], seed, seed - Prime[0]};
    uint32_t result = length + state[2] + Prime[4];

    // point beyond last byte
    const uint8_t* stop = data + length;

    // at least 4 bytes left ? => eat 4 bytes per step
    for (; data + 4 <= stop; data += 4)
        result = rotateLeft(result + *(uint32_t*)data * Prime[2], 17) * Prime[3];

    // take care of remaining 0..3 bytes, eat 1 byte per step
    while (data != stop)
        result = rotateLeft(result + (*data++) * Prime[4], 11) * Prime[0];

    // mix bits
    result ^= result >> 15;
    result *= Prime[1];
    result ^= result >> 13;
    result *= Prime[2];
    result ^= result >> 16;
    return result;
}

template<typename T>
uint32_t
SwitchNode::hash(const T& data, uint32_t seed){
    return inhash((uint8_t*)&data, sizeof(T), seed);
}


} // namespace ns3