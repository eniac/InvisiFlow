#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "topology.h"

FILE *fct_output;
std::ifstream flow_input;

struct Flows{
    uint32_t index, total_size;
    uint32_t src, dst, bytes, start_time;
};
Flows flow = {0};

void record_fct(FILE* fout, uint64_t app_id, int64_t size, int64_t fct, int64_t end_time){
	if(fct_record){
		if(end_time - fct > measureStart * 1e9 && end_time - fct < measureEnd * 1e9){
			fprintf(fout, "%ld %ld %ld %ld\n", app_id, size, fct, end_time);
    		fflush(fout);
		}
	}
}

void ReadFlowInput(){
	if(flow.index < flow.total_size){
		flow_input >> flow.src >> flow.dst >> flow.bytes >> flow.start_time;
	}
}

void OutputTime(){
	int64_t nsNow = Simulator::Now().GetNanoSeconds();
	std::cout << "Now: " << nsNow << std::endl;

	/*
	for(auto node : switches){
		if(node != nullptr){
			if(node->GetBufferSize() > 0){
				Simulator::Schedule(NanoSeconds(10000000), OutputTime);
				return;
			}
		}
	}
	*/

	for(auto node : switches)
		node->m_orbweaver = false;
	std::cout << "Stop here" << std::endl;
}

void ScheduleFlowInputs(){
	while(flow.index < flow.total_size && NanoSeconds(flow.start_time) == Simulator::Now()){
		BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress(serverAddress[flow.dst], DEFAULT_PORT));
		source.SetAttribute("MaxBytes", UintegerValue(flow.bytes));
		source.SetAttribute("ApplicationID", UintegerValue(flow.index));

		ApplicationContainer sourceApps = source.Install(servers[flow.src - 1]);
		sourceApps.Get(0)->TraceConnectWithoutContext("BulkEnd", MakeBoundCallback(record_fct, fct_output));
		sourceApps.Start(Time(0));
		
		flow.index++;
		ReadFlowInput();
	}

	if (flow.index < flow.total_size)
		Simulator::Schedule(NanoSeconds(flow.start_time) - Simulator::Now(), ScheduleFlowInputs);
	else
		flow_input.close();
}

void schedule_flow(std::string flow_file){
	if(fct_record)
		fct_output = fopen((file_name + ".fct").c_str(), "w");
    	
	flow_input.open("scratch/" + flow_file + ".tr");
	if(!flow_input.is_open()){
		std::cout << "Cannot open flow file" << "scratch/" + flow_file + ".tr" << std::endl;
		exit(1);
	}

	flow_input >> flow.total_size;
	std::cout << "Total flow number: " << flow.total_size << std::endl;

	if(flow.total_size > 0){
		ReadFlowInput();
		Simulator::Schedule(NanoSeconds(flow.start_time), ScheduleFlowInputs);
		Simulator::Schedule(NanoSeconds(flow.start_time + duration * 1e9), OutputTime);
	}
}


#endif 