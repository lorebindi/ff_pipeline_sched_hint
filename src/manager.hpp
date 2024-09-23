#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <ff/pipeline.hpp>
#include <chrono>
#include <fstream>
#include <condition_variable>
#include <vector>
#include <pthread.h>
#include <iostream>
#include <cassert>
#include <sched.h>
#include <cstring>
#include <cerrno>
#include <barrier>

using namespace ff;
using namespace std;


class manager {

private:
    atomic_bool& managerstop;
    barrier<>& bar;
    ff_pipeline& pipe;
    chrono::milliseconds interval;
    ofstream outFile;
    condition_variable cv;
    mutex cv_m;

    /**
     *
     * @return the pipeline operators' output queue length.
     */
    vector<unsigned long>* get_operators_queues_lengths () {
        const svector<ff_node*> nodes = pipe.get_pipeline_nodes();
        vector<unsigned long>* queue_lenghts = new vector<unsigned long> (nodes.size()-1);

        std::unique_lock<std::mutex> lk(cv_m);
        cv.wait_for(lk, interval, [this] { return managerstop.load(); });

        if (managerstop.load()) {
            delete queue_lenghts;
            return nullptr;
        }

        for(size_t i=0;i<(nodes.size()-1); ++i) {
            svector<ff_node*> in;
            nodes[i]->get_out_nodes(in);  // retrieve of the output nodes of the current node
            (*queue_lenghts)[i] = in[0]->get_out_buffer()->length();
            outFile << "node" << (i + 1) << " qlen=" << in[0]->get_out_buffer()->length() << std::endl;
        }
        outFile << "-------" << std::endl;

        return queue_lenghts;
    }

    /**
     *
     * @param queue_lengths pipeline operators' output queue length.
     * @return the logical priority for each operator.
     */
    const vector<double>* get_logical_priority(vector<unsigned long>* queue_lengths) {
        vector<double>* logical_priorities = new vector<double>(queue_lengths->size());

        /*
         * Stuff
         *
         */

    	// delete queue_lengths;
        return logical_priorities;
    }

	const vector<int>* translation (vector<double>* logical_priority) {
    	vector<int>* concrete_priorities = new vector<int>(logical_priority->size());

    	/*
		 * Stuff
		 *
		 */

    	//delete logical_priority;
    	return concrete_priorities;
    }

    /**
	* Note from Linux Man: "for threads scheduled under one of the normal scheduling policies
	* (SCHED_OTHER, SCHED_IDLE, SCHED_BATCH), sched_priority is not used in scheduling
	* decisions (it must be specified as 0). Processes scheduled under one of the real-time
	* policies (SCHED_FIFO, SCHED_RR) have a sched_priority value in the range 1 (low) to 99 (high)."
    *
    * @param thread thread whose priority we want to change.
    * @param new_priority the new value of priority
    * @return 0 on success, -1 on failure
    */
    int update_thread_priorities(pthread_t thread, int new_priority) {
    	int policy;
    	struct sched_param param;

    	// get the scheduling policy and parameters for the specified thread
    	if (pthread_getschedparam(thread, &policy, &param) != 0) {
    		std::cerr << "Error getting scheduling parameters: " << std::strerror(errno) << std::endl;
    		return -1;
    	}

    	// Checking the value of new_priority
    	int min_priority = sched_get_priority_min(policy);
    	int max_priority = sched_get_priority_max(policy);
    	if (new_priority > max_priority || new_priority < min_priority) {
    		std::cerr << "New priority is out of range (" << min_priority << " to " << max_priority << ")." << std::endl;
    		return -1;
    	}

    	// set the new priority
    	param.sched_priority = new_priority;
    	if (pthread_setschedparam(thread, policy, &param) != 0) {
    		std::cerr << "Error setting scheduling parameters: " << std::strerror(errno) << std::endl;
    		return -1;
    	}

    	return 0;
    }

	void apply_priorities(vector<int>* concrete_priorities) {

		svector<ff_node*> pipe_nodes = pipe.get_pipeline_nodes();

    	assert (pipe_nodes.size() == concrete_priorities->size());

	    for(size_t i = 0; i < concrete_priorities->size()-1; i++) {
		    this->update_thread_priorities(pipe_nodes[i]->get_handle(), (*concrete_priorities)[i]);
	    }

    	//delete concrete_priorities;
    }

public:
    manager(atomic_bool& _managerstop, barrier<>& _bar, ff_pipeline& _p, std::chrono::milliseconds _i)
        : managerstop(_managerstop), bar(_bar),pipe(_p), interval(_i), outFile("logs/out.log") {
        if (!outFile.is_open()) {
            cerr << "Error opening output file" << endl;
            exit(EXIT_FAILURE);
        }
    }

    void run() {
        bar.arrive_and_wait();
        std::printf("manager started\n");

        while(!managerstop.load()) {
            // Retrieves of the operators' queues lengths.
            vector<unsigned long>* queue_lenghts = this->get_operators_queues_lengths();

            if(managerstop) {
                std::printf("manager completed\n");
                delete queue_lenghts;
                return;
            }

        	/*
        	 * Call to:
        	 *
        	 * get_logical_priority
        	 *
        	 * translation
        	 *
        	 * apply priorities
        	 *
        	 */

        }

        std::printf("manager completed\n");
    }



};



#endif