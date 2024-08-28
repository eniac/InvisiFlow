#include <core.p4>
#include <tna.p4>

#define ETHERTYPE_IPV4 0x0800

#define ETHERTYPE_SEED 0x7001
#define ETHERTYPE_PUSH 0x7101
#define ETHERTYPE_PULL 0x7201

#define TABLE_SIZE 512
#define BUFFER_SIZE 1024

#define SWITCH_ID 31
#define SWITCH_NUM 4

#define COLLECTOR_PORT 201

#define USER_SHIFT 4

/*==============================
=            Header            =
==============================*/

header ethernet_h {
    bit<48> dst_addr;
    bit<48> src_addr;
    bit<16> ether_type;
}

header ipv4_h {
    bit<4> version;
    bit<4> ihl;
    bit<8> tos;
    bit<16> total_len;
    bit<16> identification;
    bit<3> flags;
    bit<13> frag_offset;
    bit<8> ttl;
    bit<8> protocol;
    bit<16> hdr_checksum;
    bit<32> src_addr;
    bit<32> dst_addr;
}

header buffer_h{
    bit<16> size;
}

header entry_h{
    bit<32> nodeId;
    bit<32> portId;
    bit<32> timeNs;
    bit<32> utils;
}

header time_h{
    bit<32> time;
}

struct header_t {
    ethernet_h ethernet;
    time_h time;
    buffer_h buffer;
    ipv4_h ipv4;
    entry_h batches;
}

/*================================
=            Metadata            =
================================*/

struct entry_t{
    bit<32> nodeId;
    bit<32> portId;
    bit<32> timeNs;
    bit<32> utils;
}

struct metadata_t {
    bit<8> evict_;
    bit<16> size_;
    bit<16> index_;
    bit<32> diff_;
    bit<32> user_num_;
    bit<32> switch_;
    bit<32> timeReport_;
    entry_t entry_;
}

/*========================================
=            Ingress parsing             =
========================================*/


parser EthIngressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out ingress_intrinsic_metadata_t ig_intr_md){

    state start {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            ETHERTYPE_PUSH : parse_push;
            ETHERTYPE_PULL : parse_pull;
            ETHERTYPE_SEED : parse_seed;
            default : accept;
        }
    }
    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition accept;
    }
    state parse_pull {
        pkt.extract(hdr.buffer);
        transition accept;
    }
    state parse_seed {
        pkt.extract(hdr.buffer);
        pkt.extract(hdr.ipv4);
        transition accept;
    }
    state parse_push {
        pkt.extract(hdr.batches);
        transition accept;
    }
}


parser IngressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out ingress_intrinsic_metadata_t ig_intr_md) {
    state start {
        pkt.extract(ig_intr_md);
        pkt.advance(64);
        EthIngressParser.apply(pkt, hdr, md, ig_intr_md);
        transition accept;
    }
}

control IngressDeparser(
        packet_out pkt, 
        inout header_t hdr,
        in metadata_t md,
        in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md) {
    apply {
        pkt.emit(hdr);
    }
}

/*========================================
=            Egress parsing             =
========================================*/


parser EthEgressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out egress_intrinsic_metadata_t eg_intr_md){
    
    state start {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            ETHERTYPE_PUSH : parse_push;
            ETHERTYPE_PULL : parse_pull;
            ETHERTYPE_SEED : parse_pull;
            default : accept;
        }
    }
    state parse_ipv4 {
        pkt.extract(hdr.time);
        pkt.extract(hdr.ipv4);
        transition accept;
    }
    state parse_pull {
        pkt.extract(hdr.buffer);
        transition accept;
    }
    state parse_push {
        pkt.extract(hdr.batches);
        transition accept;
    }
}

parser EgressParser(
        packet_in pkt,
        out header_t hdr,
        out metadata_t md,
        out egress_intrinsic_metadata_t eg_intr_md) {
    state start {
        pkt.extract(eg_intr_md);
        EthEgressParser.apply(pkt, hdr, md, eg_intr_md);
        transition accept;
    }
}


control EgressDeparser(
        packet_out pkt, 
        inout header_t hdr,
        in metadata_t md,
        in egress_intrinsic_metadata_for_deparser_t eg_dprsr_md) {
    apply {
        pkt.emit(hdr);
    }
}

/*===========================================
=            Ingress match-action           =
===========================================*/

control Ingress(
        inout header_t hdr, 
        inout metadata_t md,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

    
    action ai_drop() {
        ig_dprsr_md.drop_ctl = 0x1;
    }
    action ai_forward_user(bit<9> egress_port){
        ig_tm_md.ucast_egress_port = egress_port;
    }
    table ti_forward_user {
        key = {
            ig_intr_md.ingress_port : exact;
	        hdr.ipv4.dst_addr : ternary;
        }
        actions = {
            ai_forward_user;
            ai_drop;
        }
        default_action = ai_drop;
    }

    action ai_forward_push(bit<9> egress_port){
        ig_tm_md.ucast_egress_port = egress_port;
    }
    table ti_forward_push {
        key = {
            ig_intr_md.ingress_port : exact;
        }
        actions = {
            ai_forward_push;
            ai_drop;
        }
        default_action = ai_drop;
    }
    
    apply {
        if(hdr.ethernet.ether_type == ETHERTYPE_IPV4) {
            ti_forward_user.apply();
            ig_tm_md.qid = 0x0;
            hdr.time.setValid();
            hdr.time.time = ig_prsr_md.global_tstamp[31:0];
	    }
        else if(hdr.ethernet.ether_type == ETHERTYPE_PUSH) {
            ti_forward_push.apply();
            ig_tm_md.qid = 0x6;
	    }
        else if(hdr.ethernet.ether_type == ETHERTYPE_PULL) {
            ig_tm_md.ucast_egress_port = ig_intr_md.ingress_port;
            ig_tm_md.qid = 0x7;
	    } 
        else if(hdr.ethernet.ether_type == ETHERTYPE_SEED) {
            ig_tm_md.mcast_grp_a = COLLECTOR_PORT;
	        ig_tm_md.qid = 0x7;
            hdr.ipv4.setInvalid();
	    }
        else
            ig_dprsr_md.drop_ctl = 0x1;
    }
}

/*===========================================
=            Egress match-action           =
===========================================*/

control Egress(
        inout header_t hdr, 
        inout metadata_t md,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_oport_md){
    
    
    Register<bit<32>, _>(TABLE_SIZE) reg_time_;
    RegisterAction<bit<32>, _, bit<8>>(reg_time_) action_reg_time_ = {
        void apply(inout bit<32> val, out bit<8> ret) {
            if(md.timeReport_ != val){
                val = md.timeReport_;
                ret = 1;
            }
            else{
                ret = 0;
            }
        }
    };
    action action_time_() {
        md.evict_ = action_reg_time_.execute(eg_intr_md.egress_port);
    }
    table table_time_ {
        actions = { action_time_; }
	    const default_action = action_time_();
    }

    Register<bit<32>, _>(TABLE_SIZE) reg_util_;
    RegisterAction<bit<32>, _, bit<32>>(reg_util_) action_reg_util_clear_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val + (bit<32>)(hdr.ipv4.total_len); 
            val = 0;
        }
    };
    action action_util_clear_() {
        md.entry_.utils = action_reg_util_clear_.execute(eg_intr_md.egress_port);
    }
    table table_util_clear_ {
        actions = { action_util_clear_; }
	    const default_action = action_util_clear_();
    }
    RegisterAction<bit<32>, _, bit<32>>(reg_util_) action_reg_util_add_ = {
        void apply(inout bit<32> val) {
            val = val + (bit<32>)(hdr.ipv4.total_len);
        }
    };
    action action_util_add_() {
        action_reg_util_add_.execute(eg_intr_md.egress_port);
    }
    table table_util_add_ {
        actions = { action_util_add_; }
	    const default_action = action_util_add_();
    }


    action action_switch_id_(bit<32> switch_id){
        md.switch_ = switch_id;
    }
    table table_switch_id_ {
        key = {
            eg_intr_md.egress_port : exact;
        }
        actions = {
            action_switch_id_;
        }
        default_action = action_switch_id_(0);
    }

    Register<bit<16>, _>(SWITCH_NUM) buffer_size_;
    RegisterAction<bit<16>, _, bit<16>>(buffer_size_) action_buffer_size_push_ = {
        void apply(inout bit<16> val, out bit<16> ret) {
            ret = val;
            if(val < BUFFER_SIZE)
                val = val + 1;
        }
    };
    action action_buf_push_() {
        md.size_ = action_buffer_size_push_.execute(md.switch_);
    }
    table table_buf_push_ {
        actions = { action_buf_push_; }
	    const default_action = action_buf_push_();
    }
    RegisterAction<bit<16>, _, bit<16>>(buffer_size_) action_buffer_size_pop_ = {
        void apply(inout bit<16> val, out bit<16> ret) {
            if(hdr.buffer.size < val){
                ret = val - 1;
                val = val - 1;
            }
            else{
                ret = 65535;
            }
        }
    };
    action action_buf_pop_() {
        md.size_ = action_buffer_size_pop_.execute(md.switch_);
    }
    table table_buf_pop_ {
        actions = { action_buf_pop_; }
	    const default_action = action_buf_pop_();
    }
    RegisterAction<bit<16>, _, bit<16>>(buffer_size_) action_buffer_size_get_ = {
        void apply(inout bit<16> val, out bit<16> ret) {
            ret = val;
        }
    };
    action action_buf_get_() {
        md.size_ = action_buffer_size_get_.execute(md.switch_);
    }
    table table_buf_get_ {
        actions = { action_buf_get_; }
	    const default_action = action_buf_get_();
    }


    Register<bit<32>, _>(BUFFER_SIZE*SWITCH_NUM) buf_nodeId_;
    RegisterAction<bit<32>, _, bit<32>>(buf_nodeId_) action_buffer_push_nodeId_ = {
        void apply(inout bit<32> val) {
            val = md.entry_.nodeId;
        }
    };
    action action_buf_push_nodeId_() {
        action_buffer_push_nodeId_.execute(md.index_);
    }
    table table_buf_push_nodeId_ {
        actions = { action_buf_push_nodeId_; }
	    const default_action = action_buf_push_nodeId_();
    }
    RegisterAction<bit<32>, _, bit<32>>(buf_nodeId_) action_buffer_pop_nodeId_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val;
        }
    };
    action action_buf_pop_nodeId_() {
        hdr.batches.nodeId = 
            action_buffer_pop_nodeId_.execute(md.index_);
    }
    table table_buf_pop_nodeId_ {
        actions = { action_buf_pop_nodeId_; }
	    const default_action = action_buf_pop_nodeId_();
    }


    Register<bit<32>, _>(BUFFER_SIZE*SWITCH_NUM) buf_portId_;
    RegisterAction<bit<32>, _, bit<32>>(buf_portId_) action_buffer_push_portId_ = {
        void apply(inout bit<32> val) {
            val = md.entry_.portId;
        }
    };
    action action_buf_push_portId_() {
        action_buffer_push_portId_.execute(md.index_);
    }
    table table_buf_push_portId_ {
        actions = { action_buf_push_portId_; }
	    const default_action = action_buf_push_portId_();
    }
    RegisterAction<bit<32>, _, bit<32>>(buf_portId_) action_buffer_pop_portId_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val;
        }
    };
    action action_buf_pop_portId_() {
        hdr.batches.portId = 
            action_buffer_pop_portId_.execute(md.index_);
    }
    table table_buf_pop_portId_ {
        actions = { action_buf_pop_portId_; }
	    const default_action = action_buf_pop_portId_();
    }


    Register<bit<32>, _>(BUFFER_SIZE*SWITCH_NUM) buf_timeNs_;
    RegisterAction<bit<32>, _, bit<32>>(buf_timeNs_) action_buffer_push_timeNs_ = {
        void apply(inout bit<32> val) {
            val = md.entry_.timeNs;
        }
    };
    action action_buf_push_timeNs_() {
        action_buffer_push_timeNs_.execute(md.index_);
    }
    table table_buf_push_timeNs_ {
        actions = { action_buf_push_timeNs_; }
	    const default_action = action_buf_push_timeNs_();
    }
    RegisterAction<bit<32>, _, bit<32>>(buf_timeNs_) action_buffer_pop_timeNs_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val;
        }
    };
    action action_buf_pop_timeNs_() {
        hdr.batches.timeNs = 
            action_buffer_pop_timeNs_.execute(md.index_);
    }
    table table_buf_pop_timeNs_ {
        actions = { action_buf_pop_timeNs_; }
	    const default_action = action_buf_pop_timeNs_();
    }


    Register<bit<32>, _>(BUFFER_SIZE*SWITCH_NUM) buf_utils_;
    RegisterAction<bit<32>, _, bit<32>>(buf_utils_) action_buffer_push_utils_ = {
        void apply(inout bit<32> val) {
            val = md.entry_.utils;
        }
    };
    action action_buf_push_utils_() {
        action_buffer_push_utils_.execute(md.index_);
    }
    table table_buf_push_utils_ {
        actions = { action_buf_push_utils_; }
	    const default_action = action_buf_push_utils_();
    }
    RegisterAction<bit<32>, _, bit<32>>(buf_utils_) action_buffer_pop_utils_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val;
        }
    };
    action action_buf_pop_utils_() {
        hdr.batches.utils = 
            action_buffer_pop_utils_.execute(md.index_);
    }
    table table_buf_pop_utils_ {
        actions = { action_buf_pop_utils_; }
	    const default_action = action_buf_pop_utils_();
    }


    Register<bit<32>, _>(SWITCH_NUM) reg_tele_drop_;
    RegisterAction<bit<32>, _, bit<32>>(reg_tele_drop_) action_tele_drop_ = {
        void apply(inout bit<32> val) {
            val = val + 1;
        }
    };
    action action_tele_drops_() {
        action_tele_drop_.execute(md.switch_);
    }
    table table_tele_drop_ {
        actions = { action_tele_drops_; }
	    const default_action = action_tele_drops_();
    }

    Register<bit<32>, _>(SWITCH_NUM) reg_tele_gen_;
    RegisterAction<bit<32>, _, bit<32>>(reg_tele_gen_) action_tele_gen_ = {
        void apply(inout bit<32> val) {
            val = val + 1;
        }
    };
    action action_tele_gens_() {
        action_tele_gen_.execute(md.switch_);
    }
    table table_tele_gen_ {
        actions = { action_tele_gens_; }
	    const default_action = action_tele_gens_();
    }

    Register<bit<32>, _>(SWITCH_NUM) reg_user_num_;
    RegisterAction<bit<32>, _, bit<32>>(reg_user_num_) action_user_num_ = {
        void apply(inout bit<32> val, out bit<32> ret) {
            ret = val;
            val = val + 1;
        }
    };
    action action_user_nums_() {
        md.user_num_ = action_user_num_.execute(md.switch_);
    }
    table table_user_num_ {
        actions = { action_user_nums_; }
	    const default_action = action_user_nums_();
    }

    action action_diff_shift_() {
        md.user_num_ = md.user_num_ >> USER_SHIFT;
    }
    table table_diff_shift_ {
        actions = { action_diff_shift_; }
	    const default_action = action_diff_shift_();
    }

    Register<bit<32>, _>(131072) reg_diff_order_;
    RegisterAction<bit<32>, _, bit<8>>(reg_diff_order_) action_diff_order_ = {
        void apply(inout bit<32> val) {
            val = md.diff_;
            //val = (bit<32>)eg_intr_md.enq_qdepth;
        }
    };
    action action_diff_orders_() {
        action_diff_order_.execute(md.user_num_);
    }
    table table_diff_order_ {
        actions = { action_diff_orders_; }
	    const default_action = action_diff_orders_();
    }

    apply {
        md.evict_ = 0;
        table_switch_id_.apply();

        md.entry_.timeNs = eg_prsr_md.global_tstamp[31:0];
        md.timeReport_ = md.entry_.timeNs >> 10;
        md.diff_ = eg_prsr_md.global_tstamp[31:0] - hdr.time.time;
        
        if(hdr.ethernet.ether_type == ETHERTYPE_IPV4){
            table_user_num_.apply();

            table_time_.apply();
            if(md.evict_ == 1){
                table_tele_gen_.apply();
                table_util_clear_.apply();

                md.entry_.nodeId = md.switch_ + SWITCH_ID;
                md.entry_.portId = (bit<32>)eg_intr_md.egress_port;
            }
            else
                table_util_add_.apply();

            table_diff_shift_.apply();
            hdr.time.setInvalid();
            table_diff_order_.apply();
	    }
        else if(hdr.ethernet.ether_type == ETHERTYPE_PUSH) {
            md.evict_ = 1;

            md.entry_.nodeId = hdr.batches.nodeId;
            md.entry_.portId = hdr.batches.portId;
            md.entry_.timeNs = hdr.batches.timeNs;
            md.entry_.utils = hdr.batches.utils;

            eg_dprsr_md.drop_ctl = 0x1;
	    }

        if(md.evict_ == 1) {
            table_buf_push_.apply();
            md.index_ = md.size_ + (bit<16>)(md.switch_ << 10);
            if(md.size_ != BUFFER_SIZE){
                table_buf_push_nodeId_.apply();
                table_buf_push_portId_.apply();
                table_buf_push_timeNs_.apply();
                table_buf_push_utils_.apply();
            }
            else
                table_tele_drop_.apply();
        }
        else if(hdr.ethernet.ether_type == ETHERTYPE_PULL){
            table_buf_pop_.apply();
            md.index_ = md.size_ + (bit<16>)(md.switch_ << 10);
            if(md.size_ != 65535){
                hdr.ethernet.ether_type = ETHERTYPE_PUSH;
                hdr.buffer.setInvalid();
                hdr.batches.setValid();
                table_buf_pop_nodeId_.apply();
                table_buf_pop_portId_.apply();
                table_buf_pop_timeNs_.apply();
                table_buf_pop_utils_.apply();
            }
            else
                eg_dprsr_md.drop_ctl = 0x1;
        }
        else if(hdr.ethernet.ether_type == ETHERTYPE_SEED) {
            table_buf_get_.apply();
            hdr.buffer.size = md.size_;
            hdr.ethernet.ether_type = ETHERTYPE_PULL;
	    }
    }
}

/*==============================================
=            The switch's pipeline             =
==============================================*/
Pipeline(
    IngressParser(), Ingress(), IngressDeparser(),
    EgressParser(), Egress(), EgressDeparser()) pipe;

Switch(pipe) main;