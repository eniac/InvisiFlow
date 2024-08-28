#undef PGO_TRAINING
#define PATH_TO_PGO_CONFIG "path_to_pgo_config"

#include <iostream>
#include <iomanip> 
#include <fstream>
#include <unordered_map>
#include <chrono>

#include "schedule.h"

NS_LOG_COMPONENT_DEFINE("Telemetry");


int main(int argc, char *argv[])
{
	std::string flow_file = "Hadoop_142_0.5_25G_0.2";

	CommandLine cmd(__FILE__);
	cmd.AddValue("time", "the total run time (s), by default 0.5", duration);
	cmd.AddValue("flow", "the flow file", flow_file);
	cmd.AddValue("fctRecord", "record fct", fct_record);
	cmd.AddValue("record", "record telemetry", recordConfig);
	cmd.AddValue("INT", "the size of INT header", intSize);
	cmd.AddValue("taskId", "ID of task: (1) path tracing (2) port utilization (3) packet generator (4) packet drop (7) all tasks", taskId);
	cmd.AddValue("utilGap", "Generate gap of port utilization", utilGap);
	cmd.AddValue("Threshold", "Threshold of the flow accounting", thd);
	cmd.AddValue("OrbWeaver", "Start OrbWeaver", OrbWeaver);
	cmd.AddValue("ECMP", "configuration of ecmp: (0) 5-tuple (1) dst IP", ecmpConfig);
	cmd.AddValue("Topology", "configuration of topology: (0) leaf-spine (1) fat-tree", topology);
	cmd.AddValue("Failure", "Link failure", failConfig);

	cmd.AddValue("teleThd", "Buffer size for telemetry data", teleThd);
	cmd.AddValue("priority", "Threshold for Temp Storage", priority);
	cmd.AddValue("GenerateBps", "Telemetry data generated", generateBps);
	cmd.AddValue("hG", "100Gbps", hG);

	cmd.AddValue("Temp", "Temp Storage", tempConfig);
	cmd.AddValue("Store", "Additional Storage", storeConfig);

    cmd.Parse(argc, argv);

	measureStart = start_time + 0.2 * duration;
	measureEnd = start_time + 0.8 * duration;
	
	std::cout << "Run Telemetry." << std::endl;

	file_name = "scratch/" + flow_file + "s_Topo" + std::to_string(topology) + 
			"_ECMP" + std::to_string(ecmpConfig) + "_Gap" + std::to_string(utilGap);

	if(teleThd != 172000)
		file_name += "_Buffer" + std::to_string(teleThd);

	if(storeConfig)
		file_name += "_Store2";
	else if(tempConfig)
		file_name += "_Store1_" + std::to_string(priority);

	if(failConfig)
		file_name += "_Fail";
	if(intSize > 0)
		file_name += "_INT" + std::to_string(intSize);
	else if(OrbWeaver > 0)
		file_name += "_Orb" + std::to_string(OrbWeaver);

	build_dctcp();


	if(topology == 1)
		build_fat_tree();
	else
		std::cout << "Unknown Topology." << std::endl;

	start_sink_app();
	std::cout << "Finish Topology." << std::endl;

	auto start = std::chrono::system_clock::now();

	schedule_flow(flow_file);

	Simulator::Stop(Seconds(start_time + duration + 5));
	Simulator::Run();
	Simulator::Destroy();

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	std::cout << "Used time: " << diff.count() << "s." << std::endl;

	flow_input.close();
	if(fct_record)
		fclose(fct_output);
}