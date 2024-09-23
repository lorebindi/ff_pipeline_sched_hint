#include <string>
#include <iostream>
#include <thread>
#include <barrier>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include <ff/ff.hpp>
#include <ff/node.hpp>
#include "manager.hpp"


using namespace ff;
using namespace std;

barrier bar{2};
atomic_bool managerstop{false};

struct Source: ff_node_t<long> {

	const int ntasks;
	cpu_set_t cpuset;

    Source(const int ntasks, const cpu_set_t cpu_set):ntasks(ntasks), cpuset(cpu_set) {}

	int svc_init() {
    	// // set the affinity for the current thread
    	if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
    		cout << "Errore durante l'impostazione dell'affinità della CPU." << endl;
    	}
		bar.arrive_and_wait();
		return 0;
	}
	long* svc(long*) {
    	assert(sched_getcpu()== 0 || sched_getcpu()== 16);
        for(long i=1;i<=ntasks;++i) {

			ticks_wait(1000);
            ff_send_out((long*)i);
        }
        return EOS;
    }

};

struct Stage: ff_node_t<long> {

	long workload;
	cpu_set_t cpuset;

	Stage(long workload, const cpu_set_t cpu_set):workload(workload), cpuset(cpu_set) {}

	int svc_init() {
		// set the affinity for the current thread
		if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
			cout << "Errore durante l'impostazione dell'affinità della CPU." << endl;
		}
		return 0;
	}

    long* svc(long*in) {
		assert(sched_getcpu()== 0 || sched_getcpu()== 16);
		ticks_wait(workload);
        return in;
    }

};

struct Sink: ff_node_t<long> {

	size_t counter=0;
	cpu_set_t cpuset;

	Sink(cpu_set_t cpu_set): cpuset(cpu_set){}

	int svc_init() {
		// set the affinity for the current thread
		if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
			cout << "Errore durante l'impostazione dell'affinità della CPU." << endl;
		}
		return 0;
	}

    long* svc(long*) {
		assert(sched_getcpu()== 0 || sched_getcpu()== 16);
		ticks_wait(1000);
        ++counter;
        return GO_ON;
    }

	void svc_end() {
		std::printf("Sink finished\n");
		managerstop=true;
	}
};

int main(int argc, char* argv[]) {

    // default arguments
    size_t ntasks = 10000;
    size_t nnodes = 2;
	int interval_ms = 1; // Default interval (100 ms)

	if (argc > 1) {
		if (argc < 3) {
			error("use: %s ntasks nnodes interval_ms\n", argv[0]);
			return -1;
		}
		if(argc == 3) {
			ntasks = std::stol(argv[1]);
			nnodes = std::stol(argv[2]);
		}
		if(argc == 4) {
			ntasks = std::stol(argv[1]);
			nnodes = std::stol(argv[2]);
			interval_ms = std::stoi(argv[3]);
		}

	}

	/* Definition of cpuset. Used for restrict the cpus on which
	 * the threads of the pipeline can be scheduled. */
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);
	CPU_SET(16, &cpuset);
	//CPU_SET(32, &cpuset);
	//CPU_SET(48, &cpuset);

    Source first(ntasks, cpuset);
    Sink   last(cpuset);

	ff_pipeline pipe;
	pipe.add_stage(&first);
	for(size_t i=1;i<=nnodes;++i)
		pipe.add_stage(new Stage(2000*i, cpuset), true);
	pipe.add_stage(&last);

	// set all queues to bounded of capacity 10
	// pipe.setXNodeInputQueueLength(10, true);

	// lancio il thread manager
	manager manager(managerstop, bar, pipe, std::chrono::milliseconds(interval_ms));
	std::thread managerThread([&manager]() { manager.run(); });


	//std::thread th(manager, std::ref(pipe));

	// executing the pipe
    if (pipe.run_and_wait_end()<0) {
        error("running pipeline\n");
        return -1;
    }
	std::printf("pipe done\n");
	managerThread.join();
	std::printf("manager done\n");
	cout<< "Pipeline execution time: " << pipe.ffTime() << " ms"<< endl;

	return 0;
}

