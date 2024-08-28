#include <stdint.h>

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_ether.h>

#include <pthread.h>
#include <string.h>


#define RX_RING_SIZE 128
#define TX_RING_SIZE 256

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 500

#define RX_BURST_SIZE 32
#define TX_BURST_SIZE 16

static struct rte_eth_conf port_conf = {
	.rxmode = {
		.split_hdr_size = 0,
	},
	.txmode = {
		.mq_mode = RTE_ETH_MQ_TX_NONE,
	},
};

static struct rte_mempool *mbuf_pool;
static uint32_t num_of_telemetry = 0;

static inline int
port_init(struct rte_mempool *mbuf_pool)
{
	const uint16_t rx_rings = 1, tx_rings = 1;
	int retval;
	uint16_t q;

	retval = rte_eth_dev_configure(0, rx_rings, tx_rings, &port_conf);
	if (retval != 0)
		return retval;

	/* Allocate and set up 1 RX queue per Ethernet port. */
	for (q = 0; q < rx_rings; q++) {
		retval = rte_eth_rx_queue_setup(0, q, RX_RING_SIZE,
				rte_eth_dev_socket_id(0), NULL, mbuf_pool);
		if (retval < 0)
			return retval;
	}

	/* Allocate and set up 1 TX queue per Ethernet port. */
	for (q = 0; q < tx_rings; q++) {
		retval = rte_eth_tx_queue_setup(0, q, TX_RING_SIZE,
				rte_eth_dev_socket_id(0), NULL);
		if (retval < 0)
			return retval;
	}

	struct rte_ether_addr mac_addr;
	retval = rte_eth_macaddr_get(0, &mac_addr);
	if (retval < 0)
		return retval;
	printf("MAC address: " RTE_ETHER_ADDR_PRT_FMT "\n\n",
		RTE_ETHER_ADDR_BYTES(&mac_addr));

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(0);
	if (retval < 0)
		return retval;

	return 0;
}


static struct rte_mbuf* send_pkt[TX_BURST_SIZE] = {0};
static struct rte_mbuf* receive_pkt[RX_BURST_SIZE] = {0};

void send_pull_request(){
    struct Message {
		uint16_t data;
	};
	struct rte_ether_hdr *eth_hdr;
	struct Message obj = {0};
	struct Message *msg;

	// Please modify it based on your need
	struct rte_ether_addr src_addr = {{0x1c,0x34,0xda,0x6a,0x2f,0x52}};
	struct rte_ether_addr dst_addr = {{0x1c,0x34,0xda,0x6a,0x2f,0x52}}; 	
		
	if(rte_pktmbuf_alloc_bulk(mbuf_pool, send_pkt, TX_BURST_SIZE) < 0)
		return;

	for(int i = 0;i < TX_BURST_SIZE;i++) {
		eth_hdr = rte_pktmbuf_mtod(send_pkt[i], struct rte_ether_hdr*);
		eth_hdr->dst_addr = dst_addr;
		eth_hdr->src_addr = src_addr;
		// The ether_type for the pull request is 0x0172 in our setting
		// Please modify it based on your need
		eth_hdr->ether_type = 0x0172;
		msg = (struct Message*) (rte_pktmbuf_mtod(send_pkt[i],char*) + sizeof(struct rte_ether_hdr));
		*msg = obj;
		int pkt_size = sizeof(struct Message) + sizeof(struct rte_ether_hdr);
		send_pkt[i]->data_len = pkt_size;
		send_pkt[i]->pkt_len = pkt_size;
	}

	uint16_t nb_tx = rte_eth_tx_burst(0, 0, send_pkt, TX_BURST_SIZE);
	// printf("Send %d pull requests\n", nb_tx);
	
	for(int i = nb_tx;i < TX_BURST_SIZE;++i)
		rte_pktmbuf_free(send_pkt[i]);
}

void receive_telemetry(){
	uint16_t nb_rx = rte_eth_rx_burst(0, 0, receive_pkt, RX_BURST_SIZE);
	if(nb_rx == 0)
		return;

	//p rintf("Receive %d telemetry packets\n", nb_rx);
	
    char * msg;
	struct rte_ether_hdr * eth_hdr;

	for(int i = 0;i < nb_rx;i++){
		eth_hdr = rte_pktmbuf_mtod(receive_pkt[i], struct rte_ether_hdr*);
		// msg = ((rte_pktmbuf_mtod(receive_pkt[i], char*)) + sizeof(struct rte_ether_hdr));

		// The ether_type for the telemetry packet is 0x0171 in our setting
		// Please modify it based on your need
		if(eth_hdr->ether_type == 0x171){
			// Print packets if you want
			// for(int j = 0;j < 4;++j){
			//	 printf("%d,", rte_be_to_cpu_32(((int*)msg)[j]));
			// }
			// printf("\n");
			num_of_telemetry += 1;
		}
		rte_pktmbuf_free(receive_pkt[i]);
	}
}

int main(int argc, char *argv[])
{
	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "initlize fail!");

	argc -= ret;
	argv += ret;

	uint16_t nb_ports = rte_eth_dev_count_avail();
	if (nb_ports == 0)
		rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");

	/* Creates a new mempool in memory to hold the mbufs. */
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS,
		MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	if (port_init(mbuf_pool) != 0)
		rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu8 "\n", 0);

	uint64_t intervalUs, last_time, t = rte_get_tsc_cycles();

	while(true){
		num_of_telemetry = 0;
		last_time = t;

		while(true){
			t = rte_get_tsc_cycles();
			intervalUs = 1000000 * (t - last_time) / (rte_get_tsc_hz());
			if(intervalUs >= 1000000)
				break;
	        
			for(int i = 0;i < 100;++i){
				send_pull_request();
				receive_telemetry();
			}
		}

		if(num_of_telemetry != 0)
			printf("Receive %d telemetry packets\n", num_of_telemetry);
	}

	return 0;
}