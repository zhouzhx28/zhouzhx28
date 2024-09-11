#include<stdio.h>
#include<hls_stream.h>

#define parallel_points 8
#define buf_points 5

struct data_t{
	int s;
	int p;
};

void load(int mem_stream[100000],int mem_state[100000],int mem_input[100000],
		int len,int l,
			hls::stream<data_t> out[parallel_points])
{
	int b;
	int j[parallel_points];
	int p[parallel_points];
	int a[parallel_points];
	int temp[parallel_points];
	data_t c[parallel_points];

	b=mem_stream[len];
	for(int i=0;i<parallel_points;i++){
#pragma HLS unroll
		for(j[i]=0;j[i]<buf_points;j[i]++){
			p[i]=l+i*buf_points;
			temp[i]=mem_input[p[i]+j[i]];
			a[i]=mem_state[temp[i]];
			if((a[i]==128&&b!=32)||(a[i]==b)){
				c[i].s=temp[i];
				c[i].p=len;
				out[i].write(c[i]);
			}
		}
	}
}

void select(hls::stream<data_t> in0[parallel_points],
		hls::stream<data_t> in1[parallel_points],
		hls::stream<data_t> out[parallel_points])
{
	data_t temp[parallel_points];
	for(int i=0;i<parallel_points;i++){
#pragma HLS unroll
		while(!in0[i].empty()){
			temp[i]=in0[i].read();
			out[i].write(temp[i]);
		}
		while(!in1[i].empty()){
			temp[i]=in1[i].read();
			out[i].write(temp[i]);
		}
	}
}

void produce(int mem_l[100000],int mem_degree[100000],int mem_csr[100000],
			int mem_state[100000],int mem_stream[100000],int mem_report[100000],
			hls::stream<data_t> in[parallel_points],
			hls::stream<data_t> out[parallel_points],
			hls::stream<data_t> out_ans[parallel_points])
{
	int l[parallel_points],num[parallel_points],j[parallel_points];
	int p[parallel_points],child[parallel_points],a[parallel_points],b[parallel_points];

	data_t temp[parallel_points],c[parallel_points];

	for(int i=0;i<parallel_points;i++){
#pragma HLS unroll
		while(!in[i].empty()){
			temp[i]=in[i].read();
			//if(i==0)
				//printf("%d %d  ",temp[i].s,temp[i].p);
			if(mem_report[temp[i].s]){
				//printf("%d\n",temp[i].s);
				out_ans[i].write(temp[i]);
			}
			l[i]=mem_l[temp[i].s];
			p[i]=temp[i].p+1;
			num[i]=mem_degree[temp[i].s];
			for(j[i]=0;j[i]<num[i];j[i]++){
				child[i]=mem_csr[l[i]+j[i]];
				a[i]=mem_state[child[i]];
				b[i]=mem_stream[p[i]];
				if((a[i]==128&&b[i]!=32)||(a[i]==b[i])){
					c[i].s=child[i];
					c[i].p=p[i];
					out[i].write(c[i]);
				}
			}
		}
		//if(i==0)
			//printf("\n");
	}
	//printf("\n");
}

void store(hls::stream<data_t> in[parallel_points],
				int &cnt,
				int ans[100000]){
	data_t temp[parallel_points];

	for(int i=0;i<parallel_points;i++){
#pragma HLS unroll
		while(!in[i].empty()){
			temp[i]=in[i].read();
			//printf("%d\n",temp[i].p);
			ans[++cnt]=temp[i].p;
		}
	}
}

extern "C"{
	void test(
			int mem_state[100000], int mem_state_1[100000],
			int mem_stream[100000], int mem_stream_1[100000],
			int mem_input[100000], int mem_l[100000], int mem_degree[100000], int mem_csr[100000],int mem_report[100000],
			int ans[100000],
			int len, int num_input
			){

		#pragma HLS INTERFACE m_axi port=mem_state 	bundle=aximm1
		#pragma HLS INTERFACE m_axi port=mem_state_1 	bundle=aximm2
		#pragma HLS INTERFACE m_axi port=mem_stream 	bundle=aximm1
		#pragma HLS INTERFACE m_axi port=mem_stream_1 bundle=aximm2
		#pragma HLS INTERFACE m_axi port=mem_input 	bundle=aximm1
		#pragma HLS INTERFACE m_axi port=mem_l 	bundle=aximm1
		#pragma HLS INTERFACE m_axi port=mem_degree 	bundle=aximm1
		#pragma HLS INTERFACE m_axi port=mem_csr 	bundle=aximm1
		#pragma HLS INTERFACE m_axi port=mem_report 	bundle=aximm1
		#pragma HLS INTERFACE m_axi port=ans 		bundle=aximm1
		
		hls::stream<data_t> flow[parallel_points],flow_1[parallel_points],flow_2[parallel_points];
		hls::stream<data_t> ans_flow[parallel_points];

		#pragma HLS stream depth=10000 variable=flow
		#pragma HLS stream depth=10000 variable=flow_1
		#pragma HLS stream depth=10000 variable=flow_2
		#pragma HLS stream depth=10000 variable=ans_flow

		int cnt=0;
		for(int i=0;i<len;i++)
			for(int j=0;j<num_input;j+=parallel_points*buf_points){
				load(mem_stream,mem_state,mem_input,i,j,flow);
				select(flow_1,flow,flow_2);
				produce(mem_l,mem_degree,mem_csr,mem_state,mem_stream,mem_report,flow_2,flow_1,ans_flow);
				store(ans_flow,cnt,ans);
			}

		ans[0]=cnt;

	}
}
