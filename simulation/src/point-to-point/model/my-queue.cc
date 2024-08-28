#include "my-queue.h"

#include "ns3/abort.h"
#include "ns3/enum.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

#include "ns3/socket.h"
#include "ns3/ipv4-header.h"
#include "ns3/ppp-header.h"

#include "point-to-point-net-device.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("MyQueue");

NS_OBJECT_ENSURE_REGISTERED(MyQueue);

TypeId
MyQueue::GetTypeId()
{
    static TypeId tid = 
        TypeId("ns3::MyQueue")
            .SetParent<Queue<Packet>>()
            .SetGroupName("PointToPoint")
            .AddConstructor<MyQueue>()
            .AddAttribute(
                "ECNThreshold",
                "Threshold for ECN",
                UintegerValue(65),
                MakeUintegerAccessor(&MyQueue::m_ecnThreshold),
                MakeUintegerChecker<uint32_t>());
    return tid;
}

MyQueue::MyQueue()
{
    m_queues.push_back(CreateObject<DropTailQueue<Packet>>());
    m_queues.push_back(CreateObject<DropTailQueue<Packet>>());
    m_queues.push_back(CreateObject<DropTailQueue<Packet>>());
    m_queues.push_back(CreateObject<DropTailQueue<Packet>>());
    m_queues[0]->SetMaxSize(QueueSize("16MiB"));
    m_queues[1]->SetMaxSize(QueueSize("16MiB"));
    m_queues[2]->SetMaxSize(QueueSize("16MiB"));
    m_queues[3]->SetMaxSize(QueueSize("16MiB"));
}

MyQueue::~MyQueue() {}

bool
MyQueue::Enqueue(Ptr<Packet> item)
{
    PppHeader ppp;
    item->RemoveHeader(ppp);

    uint16_t proto = ppp.GetProtocol();

    Ipv4Header ipHeader;
    if(proto == 0x0021)
        item->RemoveHeader(ipHeader);
    
    uint32_t priority = 0;
    SocketPriorityTag priorityTag;
    if(item->PeekPacketTag(priorityTag))
        priority = priorityTag.GetPriority();

    switch(priority){
        case 0 : 
            if(proto == 0x0021 && m_queues[0]->GetNPackets() > m_ecnThreshold){
                if(ipHeader.GetEcn() == Ipv4Header::ECN_ECT1 || 
                    ipHeader.GetEcn() == Ipv4Header::ECN_ECT0)
                    ipHeader.SetEcn(Ipv4Header::ECN_CE);
            }
            break;
        case 1 : break;
        case 2 : 
            if(m_queues[2]->GetNBytes() > 256)
                return false;
            break;
        case 3 : 
            if(m_queues[3]->GetNBytes() > 256)
                return false;
            break;
        default : std::cout << "Unknown priority for queue" << std::endl; return false;
    }

    if(proto == 0x0021)
        item->AddHeader(ipHeader);
    item->AddHeader(ppp);

    bool ret = m_queues[priority]->Enqueue(item);
    if(!ret){
        std::cout << "Error in buffer " << priority << std::endl;
        std::cout << "Buffer size " << m_queues[priority]->GetNBytes() << std::endl;
    }

    return ret;
}

Ptr<Packet>
MyQueue::Dequeue()
{
    Ptr<Packet> ret = nullptr;
    for(auto queue : m_queues){
        ret = queue->Dequeue();
        if(ret != nullptr)
            return ret;
    }
    return nullptr;
}

Ptr<Packet>
MyQueue::Remove()
{
    std::cout << "Remove in MyQueue is not implemented now." << std::endl;
    return nullptr;
}

Ptr<const Packet>
MyQueue::Peek() const
{
    std::cout << "Peek in MyQueue is not implemented now." << std::endl;
    return nullptr;
}

bool 
MyQueue::IsEmpty() const
{
    return (m_queues[0]->IsEmpty() && m_queues[1]->IsEmpty());
}

uint32_t
MyQueue::GetNBytes() const
{
    return (m_queues[0]->GetNBytes() + m_queues[1]->GetNBytes());
}

} // namespace ns3