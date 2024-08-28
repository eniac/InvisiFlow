#ifndef SWITCH_NODE_H
#define SWITCH_NODE_H

#include "ns3/node.h"
#include "ns3/ipv4-header.h"
#include "ns3/path-header.h"
#include "ns3/util-header.h"
#include "ns3/drop-header.h"
#include "ns3/count-header.h"

#include <random>
#include <queue>
#include <unordered_map>

namespace ns3
{

class Application;
class Packet;
class Address;
class Time;


class SwitchNode : public Node
{
    
    public:

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    SwitchNode();
    virtual ~SwitchNode();

    /**
     * \brief Associate a NetDevice to this node.
     *
     * \param device NetDevice to associate to this node.
     * \returns the index of the NetDevice into the Node's list of
     *          NetDevice.
     */
    uint32_t AddDevice(Ptr<NetDevice> device) override;

    /**
     * \brief Receive a packet from a device.
     * \param device the device
     * \param packet the packet
     * \param protocol the protocol
     * \param from the sender
     * \returns true if the packet has been delivered to a protocol handler.
     */
    bool ReceiveFromDevice(Ptr<NetDevice> device,
                                Ptr<const Packet> packet,
                                uint16_t protocol,
                                const Address& from);

    void AddHostRouteTo(Ipv4Address dest, uint32_t devId);
    void AddTeleRouteTo(uint8_t dest, uint32_t devId);

    struct DeviceProperty{
      std::vector<uint8_t> collectorDst;

      std::unordered_map<uint8_t, bool> isUpperPull;
      std::unordered_map<uint8_t, bool> isLowerPull;

      uint32_t devId;
      uint32_t generateGap;

      int64_t m_lastTime;

      DeviceProperty(){
        devId = 0;
        generateGap = 0;
        m_lastTime = 0;
      }
    };

    uint32_t GetBufferSize();

    void SetDeviceGenerateGap(uint32_t devId, uint32_t generateGap);

    void SetDeviceUpperPull(uint8_t dest, uint32_t devId);
    void SetDeviceLowerPull(uint8_t dest, uint32_t devId);

    void SetThd(double thd);
    void SetTeleThd(uint32_t teleThd);
    void SetUtilGap(uint32_t utilGap);
    void SetHashSeed(uint32_t hashSeed);
    void SetEcmp(uint32_t ecmp);
    void SetRecord(uint32_t record);
    void SetOrbWeaver(uint32_t OrbWeaver);
    void SetOutput(std::string output);

    void SetCollector(uint32_t number);

    void SetPath(int8_t pathType);
    void SetPort(int8_t portType);
    void SetDrop(int8_t dropType);
    void SetCount(int8_t countType);
    void SetGenerate(int64_t bandwidth);

    void SetTime(double measureStart, double measureEnd);

    bool IngressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev);
    Ptr<Packet> EgressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev);

    bool m_orbweaver = false;

    struct TeleQueue{
      std::vector<PathHeader> pathBatch[3];
      std::vector<UtilHeader> utilBatch[3];
      std::vector<DropHeader> dropBatch[3];
      std::vector<CountHeader> countBatch[3];
      std::queue<Ptr<Packet>> packets[3];
      int32_t size[3] = {0, 0, 0};
    };

    protected:

    std::mt19937 m_rng;
    
    TeleQueue m_teleQueue;

    bool BatchPath(PathHeader path, uint8_t dest);
    bool BatchUtil(UtilHeader util, uint8_t dest);
    bool BatchDrop(DropHeader drop, uint8_t dest);
    bool BatchCount(CountHeader count, uint8_t dest);

    Ptr<Packet> GetTelePacket(uint32_t priority, uint8_t dest);

    int64_t m_lastTime = 2200000000;

    const uint32_t batchSize = 8;
    const uint32_t arrSize = 65537;

    uint32_t m_userThd = 2064000;
    uint32_t m_teleThd = 172000;

    int32_t m_userSize = 0;

    std::unordered_map<uint8_t, uint32_t> m_teleSend[9];
    std::unordered_map<uint8_t, uint32_t> m_bufferLoss[9];
    std::unordered_map<uint8_t, uint32_t> m_queueLoss[9];

    std::string output_file;

    uint32_t m_collector = 1;
    int m_ecmp;

    double m_thd = 0.5;
    uint32_t m_utilGap = 10000;
    uint32_t m_hashSeed = 0;

    bool m_measure = false;
    bool m_path = false;
    bool m_port = false;
    bool m_drop = false;
    bool m_count = false;
    bool m_generate = false;
    double m_generateGap = 1e5;

    int8_t m_pathType = 1;
    int8_t m_portType = 2;
    int8_t m_dropType = 4;
    int8_t m_countType = 8;

    bool m_record = false;
    double m_measureStart = 2.1;
    double m_measureEnd = 2.4;
    
    bool m_postcard = false;
    bool m_basic = false;
    bool m_pull = false;
    bool m_final = false;
    bool m_push = false;

    std::unordered_map<uint32_t, uint32_t> m_bytes;

    std::unordered_map<uint8_t, std::vector<uint32_t>> m_teleForward;
    std::unordered_map<uint32_t, std::vector<uint32_t>> m_routeForward;

    std::unordered_map<Ptr<NetDevice>, DeviceProperty> m_deviceMap;

    std::vector<PathHeader> m_table;
    std::set<PathHeader> m_paths;
    std::set<DropHeader> m_drops;
    std::set<UtilHeader> m_utils;
	  std::map<MyFlowId, int32_t> m_counts;

	CMSketch m_sketch;
  CMSketch m_old;
  CMSketch m_send_sketch;

    void Init();

    void StartMeasure();
    void EndMeasure();

    void GeneratePacket();
    void RecordUtil();
    void GenerateUtil();

    Ptr<Packet> CreatePacket(uint8_t priority);
    void SendPostcard(uint8_t dest);

    void BufferData(Ptr<Packet> packet);

    bool IngressPipelineUser(Ptr<Packet> packet);
    bool EgressPipelineUser(Ptr<Packet> packet);

    bool IngressPipelinePostcard(Ptr<Packet> packet, Ptr<NetDevice> dev);

    bool IngressPipelinePush(Ptr<Packet> packet, Ptr<NetDevice> dev);
    bool IngressPipelinePull(Ptr<Packet> packet, Ptr<NetDevice> dev);

    Ptr<Packet> EgressPipelineSeed(Ptr<Packet> packet, Ptr<NetDevice> dev);
    Ptr<Packet> EgressPipelinePush(Ptr<Packet> packet, Ptr<NetDevice> dev);
    Ptr<Packet> EgressPipelinePull(Ptr<Packet> packet, Ptr<NetDevice> dev);

	/* Hash function */
	uint32_t rotateLeft(uint32_t x, unsigned char bits);

	uint32_t inhash(const uint8_t* data, uint64_t length, uint32_t seed = 0);

	template<typename T>
	uint32_t hash(const T& data, uint32_t seed = 0);

	const uint32_t Prime[5] = {2654435761U,246822519U,3266489917U,668265263U,374761393U};

	const uint32_t prime[1229] = {
        181, 5197, 1151, 137, 5569, 7699, 2887, 8753, 9323, 8963, 6053, 8893, 9377, 6577, 733, 3527, 3881,
        6857, 9203, 3733, 3061, 8713, 1321, 887, 1913, 487, 1831, 257, 1459, 3833, 6397, 89, 3391,
        4759, 1579, 8731, 3517, 8467, 307, 3049, 3259, 4391, 6451, 3323, 7703, 2707, 2273, 9857, 5479, 7621,
        4463, 5381, 6173, 2551, 4139, 661, 9187, 5927, 103, 4219, 347, 4519, 1879, 4507, 2287, 6367, 3793,
        2803, 3217, 9221, 4261, 8111, 4441, 1637, 2591, 8311, 8747, 9679, 2243, 5563, 1901, 2137, 5939, 233,
        5, 2843, 3571, 3467, 211, 1669, 4597, 4799, 821, 6607, 6211, 4099, 9257, 2689, 7523, 967, 4253, 523,
        3253, 8663, 7489, 1697, 577, 3407, 8971, 4729, 701, 7129, 7639, 6343, 1091, 3863, 4721, 7927, 4051,
        1231, 1481, 1123, 2011, 6353, 2333, 7853, 2081, 9293, 7591, 6379, 3167, 7673, 2647, 5233, 6151, 1787,
        5387, 9949, 5419, 4003, 1213, 2027, 1619, 9341, 6763, 1789, 2477, 3329, 5641, 7013, 7001, 2203,
        2963, 1259, 1319, 4273, 6791, 4909, 2281, 4049, 2389, 9239, 7919, 3257, 613, 5867, 9629, 7103, 5009,
        2063, 11, 9151, 7069, 8369, 251, 5861, 4493, 5743, 557, 7213, 839, 2441, 4423, 9661, 6073, 3359, 7121,
        6067, 1543, 3581, 1559, 7351, 3919, 5059, 409, 9497, 7283, 1531, 5689, 5591, 6317, 6337, 8191, 2417,
        379, 4591, 9437, 3361, 7177, 751, 3851, 9337, 9157, 7669, 659, 6217, 1747, 569, 9041, 6361, 7759, 953,
        881, 5843, 5471, 3187, 1429, 1051, 2693, 4969, 3769, 4013, 9733, 3083, 991, 3533, 7823, 197, 1327,
        6007, 8831, 1291, 6619, 8287, 941, 6841, 823, 241, 607, 8513, 9439, 8941, 9619, 3631, 3163, 449, 5737,
        3209, 5297, 2371, 2797, 2, 1283, 3457, 7243, 9209, 9833, 4451, 9199, 2957, 3847, 4159, 337, 4831, 331,
        757, 1999, 5393, 1511, 2969, 1237, 4937, 1181, 8627, 6473, 5717, 71, 727, 9181, 2017, 983, 7079, 4337,
        8009, 9749, 4643, 7951, 2687, 7127, 4447, 5441, 9043, 1949, 7459, 3449, 3251, 8629, 7321, 4583, 6037,
        617, 4703, 7757, 3461, 4663, 223, 2437, 2339, 47, 7789, 6977, 7411, 6043, 2381, 373, 61, 2539, 9433,
        4999, 5507, 5399, 53, 8573, 317, 8123, 3541, 5869, 8233, 619, 127, 829, 4129, 2099, 3853, 1297, 3319,
        4793, 8887, 7481, 1609, 8231, 997, 2087, 9649, 3389, 4957, 1801, 2711,
        1987, 4021, 5647, 9007, 59, 4079, 5881, 7793, 1907, 1583, 5711, 7027, 1487,
        599, 9539, 3271, 7727, 1663, 811, 1049, 1493, 5531, 7741, 5849, 3877, 3347,
        7039, 8101, 1433, 8221, 2309, 2267, 5261, 2801, 8599, 4871, 7681, 7753, 4723,
        3637, 8929, 349, 8219, 421, 7541, 5413, 9767, 6529, 5779, 683, 2459, 4157,
        151, 6899, 271, 5651, 631, 5657, 1709, 4649, 1933, 5209, 97, 9623, 1439, 3767,
        2549, 5437, 2879, 9739, 3343, 7219, 8803, 2239, 1171, 1381, 809, 787, 5519,
        7433, 5309, 2341, 7649, 2713, 2671, 3931, 2767, 9343, 8699, 2683, 7607, 6959,
        8783, 2729, 5741, 9319, 9283, 3719, 5557, 673, 1823, 1667, 5189, 8623, 6829,
        2857, 2213, 1039, 2423, 7867, 9067, 1499, 3643, 7879, 2833, 3617, 863, 101,
        4201, 5503, 4817, 7877, 73, 2777, 3671, 4127, 5821, 9811, 929, 4231, 9137, 3121,
        5501, 5827, 9059, 6287, 5431, 1993, 6967, 1163, 1783, 6599, 2753, 4177, 883, 5801,
        5483, 23, 113, 2657, 1289, 6803, 3803, 8093, 1103, 6833, 3433, 5087, 2897, 8527,
        743, 7193, 7309, 9311, 919, 5119, 9013, 1489, 2953, 8171, 8689, 2311, 283, 7019,
        2141, 199, 2131, 3697, 5897, 7369, 9397, 419, 4783, 5147, 8053, 1303, 7829, 1549,
        2377, 5701, 3547, 4861, 3607, 2207, 3917, 229, 3191, 8243, 8861, 7583, 1361, 4057,
        8779, 4657, 4639, 281, 2003, 1277, 1117, 3677, 4933, 4877, 3137, 2293, 3911, 3691,
        9241, 83, 9697, 239, 6389, 7907, 269, 9029, 1721, 2791, 8597, 3907, 9391, 109, 6673,
        2347, 1877, 4903, 4483, 587, 3821, 9931, 4211, 8293, 6571, 9011, 1753, 6961, 79, 8387,
        971, 1657, 5783, 9901, 5351, 8443, 3529, 5813, 2467, 8669, 4073, 7417, 4289, 9871,
        6793, 463, 6863, 4271, 6299, 709, 677, 9133, 8167, 5987, 5113, 2917, 5669, 8521,
        3739, 9403, 3511, 9001, 3889, 2179, 4931, 3539, 859, 311, 107, 359, 5077, 7331, 5279,
        6197, 3701, 6521, 9883, 857, 8837, 907, 1409, 2521, 761, 9521, 3469, 7333, 4363, 3557,
        8423, 563, 5023, 3019, 8147, 1699, 5581, 1367, 2351, 1873, 5179, 2411, 277, 9547, 3203,
        3221, 2531, 9769, 3089, 9049, 4243, 8329, 8363, 1187, 7993, 2129, 2719, 7517, 6047, 7247,
        2927, 1217, 911, 4919, 8609, 5659, 5903, 6581, 1867, 6661, 1423, 5011, 5171, 2741, 6311,
        4789, 8867, 9281, 5237, 2593, 9277, 4327, 8273, 9791, 5153, 9551, 7547, 739, 8059, 2749,
        6883, 8431, 5839, 9631, 647, 4567, 3299, 4523, 3929, 1373, 461, 8819, 7577, 5683, 6271,
        431, 2837, 6079, 9161, 6011, 227, 6911, 5227, 1451, 8923, 313, 9419, 3373, 6733, 6703,
        7043, 6689, 4787, 8209, 1693, 2221, 9721, 433, 6269, 6719, 1871, 5281, 1031, 797, 6553,
        3989, 2663, 571, 2659, 7457, 5407, 8353, 653, 1301, 7, 4093, 521, 9887, 131, 6491, 6779,
        1109, 6247, 6569, 769, 5879, 3371, 6359, 2083, 4027, 4217, 4513, 7883, 8419, 8461, 5273,
        4679, 7559, 7901, 2851, 8539, 3023, 4561, 1447, 2297, 43, 9907, 947, 7297, 3943, 8821,
        2819, 2383, 6709, 1723, 9689, 2909, 2617, 1571, 3967, 5101, 1223, 9643, 5303, 6551, 8011,
        4007, 6871, 5573, 8581, 19, 6029, 9973, 4651, 1249, 1613, 6163, 9227, 2161, 9941, 827, 3623,
        853, 9587, 2039, 3727, 3301, 8537, 6203, 8291, 7211, 1153, 937, 3229, 9719, 6997, 4603, 9851,
        3011, 2237, 8719, 499, 8849, 9461, 1567, 5039, 2029, 5099, 3331, 4889, 8069, 4421, 3067, 5347,
        6781, 1861, 167, 2789, 7873, 9929, 6737, 8501, 5003, 8543, 8237, 439, 7307, 3499, 2609, 5527,
        509, 17, 1093, 2269, 7159, 8933, 8317, 8377, 9511, 3583, 7717, 8117, 4481, 263, 7237, 9923,
        9859, 1019, 3413, 389, 1931, 3041, 2251, 6659, 9533, 8951, 6133, 9473, 4621, 1307, 2053, 5081,
        9839, 467, 8863, 2089, 5923, 7451, 2633, 4019, 6143, 6449, 7937, 6869, 5231, 1471, 5857, 7573,
        7393, 5449, 5981, 179, 4111, 8269, 8681, 2113, 1399, 2731, 2677, 9817, 6991, 7187, 9829, 4943,
        9613, 191, 383, 6089, 1063, 2971, 6547, 8081, 6263, 8179, 6221, 3, 7603, 8087, 5851, 9467, 6229,
        37, 8017, 6827, 6469, 401, 877, 5107, 31, 6983, 2903, 8563, 8707, 2999, 8693, 7687, 1627, 2473,
        4357, 7691, 3001, 4751, 5639, 7537, 773, 641, 3823, 9967, 1453, 6823, 593, 1777, 7349, 601,
        1009, 8839, 6701, 6421, 4297, 4133, 4373, 7949, 6323, 719, 643, 6257, 7507, 9103, 4259,
        3491, 2579, 6373, 8161, 1553, 6637, 2393, 3119, 139, 8039, 8999, 1997, 9091, 6947, 9173,
        4517, 5477, 5521, 367, 503, 9127, 6199, 5693, 5417, 193, 5791, 4409, 7529, 4987, 67, 6917,
        4229, 2621, 5653, 8807, 7109, 4283, 4967, 1069, 1279, 5623, 1201, 1193, 1229, 13, 7723, 9431,
        1951, 6131, 691, 6761, 6653, 3593, 4549, 2699, 8389, 5807, 4153, 9349, 2557, 173, 3779, 1601,
        4547, 8969, 1811, 9421, 5051, 7643, 1061, 8737, 3169, 4973, 7207, 2069, 7253, 5323, 9677,
        1033, 7817, 2447, 7477, 7963, 6971, 4691, 3797, 5443, 7841, 6329, 4349, 7589, 1847, 9479,
        1483, 2153, 443, 1597, 3109, 6301, 491, 1759, 8429, 8297, 9787, 541, 4813, 6691, 4241,
        8677, 6427, 479, 4993, 3559, 3947, 9743, 7487, 8641, 4001, 3709, 8447, 6121, 9463, 9491,
        6563, 1621, 5167, 9413, 163, 7561, 3079, 8647, 4673, 3463, 3037, 9601, 3659, 7057, 293,
        2503, 4637, 1741, 1979, 3761, 457, 1021, 3613, 7499, 3307, 3923, 9109, 3673, 1523, 1607,
        1889, 6113, 1097, 4339, 6101, 8089, 977, 6679, 9803, 7229, 3313, 29, 2111, 1733, 9781,
        2357, 5953, 5021, 8263, 2861, 8761, 7549, 8741, 1973, 9371, 2399, 397, 4801, 4733, 4397,
        157, 149, 1427, 4951, 2939, 6481, 5333, 7933, 1087, 6277, 2143, 41, 2543, 3181, 5749,
        4091, 1013, 547, 6907, 7151, 353, 4457, 6091, 6949, 1129
	};
	
};

} // namespace ns3

#endif /* SWITCH_NODE_H */