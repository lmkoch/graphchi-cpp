
/**
 * @file
 * @author  Aapo Kyrola <akyrola@cs.cmu.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * Copyright [2012] [Aapo Kyrola, Guy Blelloch, Carlos Guestrin / Carnegie Mellon University]
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 
 *
 * @section DESCRIPTION
 *
 * Template for GraphChi applications. To create a new application, duplicate
 * this template.
 */



#include <string>

#include "graphchi_basic_includes.hpp"

#include "../collaborative_filtering/common.hpp"
#include "../collaborative_filtering/eigen_wrapper.hpp"
#include "../collaborative_filtering/io.hpp"

using namespace graphchi;

/**
  * Type definitions. Remember to create suitable graph shards using the
  * Sharder-program. 
  */

struct custom_vogsl_edge {
	float weight;
	bool isActive;

	custom_vogsl_edge() {
		weight = 0;
		isActive = false;
	}

	custom_vogsl_edge(float x) {
		weight = x;
		isActive = false;
	}

	bool is_active(){
		return isActive;
	}
};

struct custom_vogsl_vertex {

	vec pvec;
	float confidence;
	bool seed;

	custom_vogsl_vertex() {
		pvec = zeros(D);
		confidence = 0;
		seed = false;
	}
	//this function is only called for seed nodes
	void set_p(int index, float val){
		pvec[index] = val;
		seed = true;
	}
	//this function is only called for seed nodes
	void set_c(float val){
		confidence = val;
		seed = true;
	}
	float get_p(int index){
		return pvec[index];
	}
	float get_c(){
		return confidence;
	}
};

typedef custom_vogsl_vertex VertexDataType;
typedef custom_vogsl_edge EdgeDataType;


std::vector<VertexDataType> latent_factors_inmem;

/**
  * GraphChi programs need to subclass GraphChiProgram<vertex-type, edge-type> 
  * class. The main logic is usually in the update function.
  */
struct MyGraphChiProgram : public GraphChiProgram<VertexDataType, EdgeDataType> {
    
 
    /**
     *  Vertex update function.
     */
    void update(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext) {

        if (gcontext.iteration == 0) {
            /* On first iteration, initialize vertex (and its edges). This is usually required, because
               on each run, GraphChi will modify the data files. To start from scratch, it is easiest
               do initialize the program in code. Alternatively, you can keep a copy of initial data files. */
            // vertex.set_data(init_value);
        
        } else {
            /* Do computation */ 

        	VertexDataType & vdata = latent_factors_inmem[vertex.id()];
        	if (vdata.seed || vertex.num_outedges() == 0) //if this is a seed node, don't do anything
        		return;

            /* Loop over in-edges */
        	// Find valuable neighbours and adaptive h
            for(int i=0; i < vertex.num_inedges(); i++) {

            	EdgeDataType edata = vertex.edge(i)->get_data();

            	float weight = edata.weight;
            	assert(weight != 0);
            	VertexDataType & nbr_latent = latent_factors_inmem[vertex.edge(i)->vertex_id()];

            	if ( nbr_latent.get_c() * weight > vdata.get_c() || edata.isActive )
            	{
            		// make edge active

            		// check for adaptive h
            	}

            }

            /* Loop over in-edges */
        	// Do propagation
            for(int i=0; i < vertex.num_inedges(); i++) {

            	EdgeDataType edata = vertex.edge(i)->get_data();

            	if (!edata.isActive)
            	{
            		continue;
            	}

            	float weight = edata.weight;
            	assert(weight != 0);
            	VertexDataType & nbr_latent = latent_factors_inmem[vertex.edge(i)->vertex_id()];

            	// get exponential weights

            	// sum up

            }
            
           	// get new confidence and probabilities

            /* Loop over out-edges (example) */
            for(int i=0; i < vertex.num_outedges(); i++) {
                // Do something
                // vertex.outedge(i).set_data(x)
            }
            
            /* Loop over all edges (ignore direction) */
            for(int i=0; i < vertex.num_edges(); i++) {
                // vertex.edge(i).get_data() 
            }
            
            // v.set_data(new_value);
        }
    }
    
    /**
     * Called before an iteration starts.
     */
    void before_iteration(int iteration, graphchi_context &gcontext) {
    }
    
    /**
     * Called after an iteration has finished.
     */
    void after_iteration(int iteration, graphchi_context &gcontext) {
    }
    
    /**
     * Called before an execution interval is started.
     */
    void before_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &gcontext) {        
    }
    
    /**
     * Called after an execution interval has finished.
     */
    void after_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &gcontext) {        
    }
    
};

int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line 
       arguments and the configuration file. */
    graphchi_init(argc, argv);
    
    /* Metrics object for keeping track of performance counters
       and other information. Currently required. */
    metrics m("my-application-name");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file");  // Base filename
    int niters           = get_option_int("niters", 4); // Number of iterations
    bool scheduler       = get_option_int("scheduler", 0); // Whether to use selective scheduling
    
    /* Detect the number of shards or preprocess an input to create them */
    int nshards          = convert_if_notexists<EdgeDataType>(filename, 
                                                            get_option_string("nshards", "auto"));
    
    /* Run */
    MyGraphChiProgram program;
    graphchi_engine<VertexDataType, EdgeDataType> engine(filename, nshards, scheduler, m); 
    engine.run(program, niters);
    
    /* Report execution metrics */
    metrics_report(m);
    return 0;
}
