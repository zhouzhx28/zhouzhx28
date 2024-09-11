#include <iostream>
#include <cstring>

#include "xrt/xrt_bo.h"
#include <experimental/xrt_xclbin.h>
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"

#include "net_2.h"

#define SIZE 100000

unsigned int cnt;

int main(int argc, char** argv){
    std::cout << "argc = " << argc << std::endl;
	for(int i=0; i < argc; i++){
	    std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
	    }
	 
	std::string binaryFile = "./test.xclbin";
    int device_index = 0;

    std::cout << "Open the device " << device_index << std::endl;
    auto device = xrt::device(device_index);
    std::cout << "Load the xclbin " << binaryFile << std::endl;
    auto uuid = device.load_xclbin("./test.xclbin");	
	
	auto krnl = xrt::kernel(device, uuid, "test", xrt::kernel::cu_access_mode::exclusive);
	
	size_t size_bytes=sizeof(int)*SIZE;
	
	auto bo0 = xrt::bo(device, size_bytes, krnl.group_id(0));//mem_start_set
	auto bo1 = xrt::bo(device, size_bytes, krnl.group_id(1));//mem_l
	auto bo2 = xrt::bo(device, size_bytes, krnl.group_id(2));//mem_r
	auto bo3 = xrt::bo(device, size_bytes, krnl.group_id(3));//mem_indice
	auto bo4 = xrt::bo(device, size_bytes, krnl.group_id(4));//mem_state
	auto bo5 = xrt::bo(device, size_bytes, krnl.group_id(5));//mem_state_1
	auto bo6 = xrt::bo(device, size_bytes, krnl.group_id(6));//mem_symbol
	auto bo7 = xrt::bo(device, size_bytes, krnl.group_id(7));//mem_instream
	auto bo8 = xrt::bo(device, size_bytes, krnl.group_id(8));//mem_instream_1
	auto bo9 = xrt::bo(device, size_bytes, krnl.group_id(9));//mem_b
		
	cnt=0;
	
	auto bo0_map = bo0.map<int*>();
	auto bo1_map = bo1.map<int*>();
	auto bo2_map = bo2.map<int*>();
	auto bo3_map = bo3.map<int*>();
	auto bo4_map = bo4.map<int*>();
	auto bo5_map = bo5.map<int*>();
	auto bo6_map = bo6.map<int*>();
	auto bo7_map = bo7.map<int*>();
	auto bo8_map = bo8.map<int*>();
	auto bo9_map = bo9.map<int*>();
	
	for(int i=0;i<SIZE;i++){
		bo0_map[i]=a_symbol[i];
	}
	
	for(int i=0;i<SIZE;i++){
		bo1_map[i]=a_symbol[i];
	}
	
	for(int i=0;i<SIZE;i++){
		bo2_map[i]=a_instream[i];
	}
	
	for(int i=0;i<SIZE;i++){
		bo3_map[i]=a_instream[i];
	}
	
	for(int i=0;i<SIZE;i++){
		bo4_map[i]=a_input[i];
	}
	
	for(int i=0;i<SIZE;i++){
		bo5_map[i]=a_l[i];
	}
	
	for(int i=0;i<SIZE;i++){
		bo6_map[i]=a_mem[i];
	}
	
	for(int i=0;i<SIZE;i++){
		bo7_map[i]=a_indice[i];
	}
	
	for(int i=0;i<SIZE;i++){
		bo8_map[i]=b_report[i];
	}
	
	
	std::fill(bo9_map, bo9_map + SIZE, 0);
	std::cout << "synchronize input buffer data to device global memory\n";
	
    bo0.sync(XCL_BO_SYNC_BO_TO_DEVICE);//start_set
    bo1.sync(XCL_BO_SYNC_BO_TO_DEVICE);//mem_l
	bo2.sync(XCL_BO_SYNC_BO_TO_DEVICE);//mem_r
	bo3.sync(XCL_BO_SYNC_BO_TO_DEVICE);//indice
	bo4.sync(XCL_BO_SYNC_BO_TO_DEVICE);//state
	bo5.sync(XCL_BO_SYNC_BO_TO_DEVICE);//state
	bo6.sync(XCL_BO_SYNC_BO_TO_DEVICE);//state
	bo7.sync(XCL_BO_SYNC_BO_TO_DEVICE);//instream
	bo8.sync(XCL_BO_SYNC_BO_TO_DEVICE);//instream

	cnt=0;
	
    std::cout << "Execution of the kernel\n";	
	auto run = krnl(bo0,bo1,bo2,bo3,bo4,bo5,bo6,bo7,bo8,bo9,40,400);
	run.wait();
	
	//std::cout << cnt << std::endl;
	
	bo9.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
	
	for(int i=1;i<bo9_map[0];i++){
		std::cout << bo9_map[i];
		std::cout << " ";
	}
	std::cout << "\n";
	
	std::cout << "TEST FINISHED\n";
	return 0;
}
