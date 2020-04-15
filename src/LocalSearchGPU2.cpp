#include "LocalSearchGPU2.hpp"

//________________________________________________________________________________________________________________________________

// constructor
LocalSearchGPU2::LocalSearchGPU2(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  LocalSearchGPU(args, d, file_jobs, file_times, file_nodes)
{}

//________________________________________________________________________________________________________________________________-
// look for a better schedule by visiting the neighborhood of initial schedule
bool
LocalSearchGPU2::visit_neighbor()
{
  // true if the function finds a better schedule and updates it
  bool changed = false;

  // update the data structure that links the nodes to the jobs ("change_GPU" needs it updated)
  update_node_jobs();

  // neighborhood to be explored
  std::unordered_set<unsigned> neighborhood = generate_neighborhood();
  //TO REMOVE
  std::cout<< "ci sono " << neighborhood.size() << " nodi dove posso raddoppiare la capacità "<<std::endl;
  // visit the neighbourhood
  for(auto neighbor: neighborhood)
  {
    unsigned node_idx = neighbor;

    // TODO: rimuovere
    //std::cout << std::endl << "# Sto visitando il nodo: " << node_idx << std::endl;
    //Node & nn = nodes[node_idx];
    //std::cout << "# old_stp: " << nn.get_VMtype() << "," << nn.get_GPUtype() << "," << nn.get_usedGPUs() << "," << nn.get_cost() << std::endl;
    //std::cout << "# new_stp: "; new_stp.print(std::cout);

    // candidate schedule, obtained by modifying the initial schedule accordingly to neighbor infos
    job_schedule_t candidate_schedule = change_nGPU(node_idx);

    //std::cout << std::endl << "change_GPU candidate_schedule:\n";
    //printer(candidate_schedule);

    // evaluate the objective in this point of the neighborhood
    double candidate_value = evaluate_objective(candidate_schedule);

    //std::cout << "Obj function candidate_value: " << candidate_value << std::endl;

    // if this schedule improves the objective function
    if(candidate_value < best_schedule_value_t)
    {
        // update the best schedule and the best value
        local_best_schedule = candidate_schedule;
        best_schedule_value_t = candidate_value;
        changed = true;

        //printer(local_best_schedule);

        // update the configuration of the node accordingly:

        //I am using twice as many GPUs
        unsigned used_GPUs = nodes[node_idx].get_usedGPUs();
        nodes[node_idx].set_remainingGPUs(used_GPUs); // I double the number of used GPUs

        //std::cout <<std::endl << "Ho aggiornato la config del nodo" << std::endl;
        //Node & nn = nodes[node_idx];
        //std::cout << "new_stp: " << nn.get_VMtype() << "," << nn.get_GPUtype() << "," << nn.get_usedGPUs() << "," << nn.get_cost() << "\n\n";  

        // if I'm not performing a best improvement search
        if(!best_fit)
        {
            break;
        }
    } // end if

  } // end for
  return changed;

}

//________________________________________________________________________________________________________________________________

// create a new schedule by changing the type of GPU on node node_idx and updating the schedules of the jobs on that node
// input: new_stp, va modificato per ogni job aggiornando nGPUs
job_schedule_t
LocalSearchGPU2::change_nGPU(unsigned node_idx)
{
  // new schedule to modify
  job_schedule_t new_schedule = local_best_schedule;

  // get the jobs in execution on the node "node_idx"
  std::unordered_set<Job> jobs_to_modify = node_jobs[node_idx];

  //std::cout << std::endl << "Scorro i jobs sul nodo" << std::endl << std::endl;

  // cycle over these jobs
  for(Job j: jobs_to_modify)
  {
    //std::cout << j.get_ID() << std::endl; // TODO: remove

    // schedule of job j
    Schedule & sch = new_schedule[j];

    // If the shcedule is not empty
    if(!sch.isEmpty())
    {
        // old setup
        const Setup & old_stp = sch.get_setup();

        //std::cout << "old_stp: "; old_stp.print(std::cout);

        // old number of GPU used
        unsigned nGPUs = old_stp.get_nGPUs(); // questa info è diversa per ogni job!
        unsigned new_nGPUs =nGPUs*2;        

        // row to initialize a new setup
        row_t build_stp;
        build_stp.push_back(old_stp.get_VMtype());
        build_stp.push_back(old_stp.get_GPUtype());
        build_stp.push_back(std::to_string(new_nGPUs));
        build_stp.push_back(std::to_string(old_stp.get_maxnGPUs()));
        build_stp.push_back(std::to_string(old_stp.get_cost()));

        // new setup
        Setup job_stp(build_stp); // cambio il nome in job_stp

        //std::cout << "job_stp: "; new_stp.print(std::cout);

        // get the range of setup_time_t with this setup (same VMtype and GPUtype)
        auto range = ttime[j.get_ID()].equal_range(job_stp);
        setup_time_t::const_iterator start = range.first;
        setup_time_t::const_iterator finish = range.second;

        setup_time_t::const_iterator c_newsetup;

        // I'm looking for the setup with exactly this number of GPUs
        for(auto it=start; it!=finish; ++it)
        {
            const Setup & stp = it->first;
            if(stp.get_nGPUs() == new_nGPUs)
            {
                c_newsetup = it;
            }
        }

    // modify the schedule of job j according to the new setup
    sch.change_setup(c_newsetup);
    }
}
return new_schedule;
}

//________________________________________________________________________________________________________________________________

// generate the neighborhood of local_best_schedule
std::unordered_set<unsigned> 
LocalSearchGPU2::generate_neighborhood(void)
{
  // neighbourhood
  std::unordered_set<unsigned> neighbourhood;

  // nodes with the highest number of jobs in tardiness
  std::unordered_set<unsigned> tochange = get_tardiness_nodes(neigh_size);

  // TODO: INSERT IT IN get_tardiness_nodes and randomize the selection
  // If no nodes have jobs in tardiness
  /*if (tochange.empty())
  {
    // I take the first neigh_size nodes // TODO: randomize this selection
    for(int i=0; i < std::min(last_node_idx,neigh_size); ++i)
    {
      tochange.insert(i);
    }
  }
  */
  // build the neighbourhood of each node
  for(unsigned n: tochange)
  {
    // max num of GPU for the current node
    unsigned max_nGPUs = nodes[n].get_usedGPUs() + nodes[n].get_remainingGPUs();
    unsigned used_nGPUs = nodes[n].get_usedGPUs();
    unsigned  new_nGPUs = used_nGPUs*2;
    if (!(max_nGPUs < new_nGPUs))
    {
        neighbourhood.insert(n);
    }  
  }
  return neighbourhood;
}

//________________________________________________________________________________________________________________________________
