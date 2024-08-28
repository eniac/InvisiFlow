import argparse
import json
import sys, os, time

sys.path.append(os.path.dirname(os.path.realpath(__file__))+"/libs")
from mgr import *


def config():
    print("PID: " + str(os.getpid())) 
    m = Manager(p4name="main")

    data = {}
    with open('libs/servers.json', 'r') as f:
        servers_json=json.loads(f.read())
    server_ports = []
    for val in servers_json.values():
        server_ports.append(val['port_id'])

    # 1/0 <-> 2/0
    multicast_ports = [140, 132, 141, 133, 142, 134, 143, 135]
    recirculate_ports = [160, 161, 162, 163]

    print("--- Set up ports (physically occupied for successful enabling) ---")
    m.enab_ports_sym_bw(multicast_ports, "25G")
    m.enab_ports_sym_bw(recirculate_ports, "25G")

    m.create_mc_grp(201, multicast_ports)

    print("--- Configure DP states ---")

    LEAF_NUM = 4
    SPINE_NUM = 2
    
    for server in servers_json.values():
        print(server)

        groupId = server['switch_id'] % LEAF_NUM
        for i in range(len(multicast_ports)):
            m.add_rule_exact_read_ternary_read_action_arg(
                        tbl_name="ti_forward_user",
                        action_name='ai_forward_user',
                        match_arg_exact_name="ig_intr_md.ingress_port",
                        match_arg_exact=multicast_ports[i],
                        match_arg_ternary_name="hdr.ipv4.dst_addr",
                        match_arg_ternary=server['ip_addr'],
                        match_arg_ternary_mask="255.255.255.255",
                        match_arg_ternary_annotation="ipv4",
                        priority=0x2,
                        action_arg_name="egress_port",
                        action_arg=multicast_ports[groupId*SPINE_NUM+i%SPINE_NUM])

    for i in range(len(multicast_ports)):
        m.add_rule_exact_read_action_arg(
                tbl_name="table_switch_id_", 
                action_name="action_switch_id_",
                match_arg_name="eg_intr_md.egress_port",
                match_arg=multicast_ports[i],
                action_arg_name="switch_id", 
                action_arg=i%SPINE_NUM)
        
        m.add_rule_exact_read_action_arg(
                tbl_name="ti_forward_push", 
                action_name="ai_forward_push",
                match_arg_name="ig_intr_md.ingress_port",
                match_arg=multicast_ports[i],
                action_arg_name="egress_port", 
                action_arg=recirculate_ports[i%SPINE_NUM])
    
    for i in range(SPINE_NUM):
        m.add_rule_exact_read_action_arg(
                tbl_name="table_switch_id_", 
                action_name="action_switch_id_",
                match_arg_name="eg_intr_md.egress_port",
                match_arg=recirculate_ports[i],
                action_arg_name="switch_id", 
                action_arg=i)

    m.disconnect()


def debug(log_time, log_dir):

    print("PID: " + str(os.getpid()))
    m = Manager(p4name="main")

    presize = [0, 0, 0, 0]
    predrop = [0, 0, 0, 0]
    pregen = [0, 0, 0, 0]
    preuser = [0, 0, 0, 0]
    while True:
        time.sleep(log_time)

        for switchId in range(2):
            size = m.read_reg_element_for_pipe("buffer_size_", switchId, pipeid=1)
            if size != presize[switchId]:
                print(str(switchId) + " Size: " + str(size))
                presize[switchId] = size
            
            drop = m.read_reg_element_for_pipe("reg_tele_drop_", switchId, pipeid=1)
            if drop != predrop[switchId]:
                print(str(switchId) + " Drop: " + str(drop))
                predrop[switchId] = drop

            gen = m.read_reg_element_for_pipe("reg_tele_gen_", switchId, pipeid=1)
            if gen != pregen[switchId]:
                print(str(switchId) + " Gen: " + str(gen))
                pregen[switchId] = gen
        
            user = m.read_reg_element_for_pipe("reg_user_num_", switchId, pipeid=1)
            if user != preuser[switchId]:
                print(str(switchId) + " User: " + str(user))
                preuser[switchId] = user
                
        '''
        diff_str = ""

        for index in range(131000):
            diff =  m.read_reg_element_for_pipe("reg_diff_order_", index, pipeid=1)
            if index % 1000 == 999:
                print(index)
                print(diff)
            if diff == 0:
                break
            diff_str += (str(diff) + "\n")
        
        diff_file = open('diff_order.csv', 'w')
        diff_file.write(diff_str)
        diff_file.close()
        '''


    m.disconnect()


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    subcmds = parser.add_subparsers(dest="cmd")

    cmd_config = subcmds.add_parser("config")

    cmd_debug = subcmds.add_parser("debug")
    cmd_debug.add_argument("-t", "--time", type=float, required=False, default=0.1, help="Number of seconds to record the debugging stats")
    cmd_debug.add_argument("-d", "--dir", type=str, required=False, default="logs", help="Name of the directory to hold the debugging stats")

    args = parser.parse_args(['config'] if len(sys.argv)==1 else None)

    if args.cmd == "config":
        config()
    elif args.cmd == "debug":
        debug(args.time, args.dir)
    else:
        print("ERR")