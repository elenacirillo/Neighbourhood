#include "LocalSearchGPU2.hpp"

// constructor
LocalSearchGPU2::LocalSearchGPU2(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  LocalSearchGPU(args, d, file_jobs, file_times, file_nodes)
{}
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
 
 // visit the neighbourhood
  for(auto neighbor: neighborhood)
  {
    unsigned node_idx = neighbor;

    // candidate schedule, obtained by modifying the initial schedule accordingly to neighbor infos
    job_schedule_t candidate_schedule = change_nGPU(node_idx);

    // evaluate the objective in this point of the neighborhood
    double candidate_value = evaluate_objective(candidate_schedule);

    // if this schedule improves the objective function
    if(candidate_value < best_schedule_value_t)
    {
        // update the best schedule and the best value
        local_best_schedule = candidate_schedule;
        best_schedule_value_t = candidate_value;
        changed = true;

        // update the configuration of the node accordingly:
        // I am using twice as many GPUs
        unsigned used_GPUs = nodes[node_idx].get_usedGPUs();
        nodes[node_idx].set_remainingGPUs(used_GPUs);

        // if I'm not performing a best improvement search
        if(!best_fit)
        {
            break;
        }
    }
  }
  return changed;
}

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
    // schedule of job j
    Schedule & sch = new_schedule[j];

    // If the shcedule is not empty
    if(!sch.isEmpty())
    {
        // old setup
        const Setup & old_stp = sch.get_setup();

        // old number of GPU used
        unsigned nGPUs = old_stp.get_nGPUs();
        unsigned new_nGPUs =nGPUs*2;        

        // row to initialize a new setup
        row_t build_stp;
        build_stp.push_back(old_stp.get_VMtype());
        build_stp.push_back(old_stp.get_GPUtype());
        build_stp.push_back(std::to_string(new_nGPUs));
        build_stp.push_back(std::to_string(old_stp.get_maxnGPUs()));
        build_stp.push_back(std::to_string(old_stp.get_cost()));

        // new setup
        Setup job_stp(build_stp);

        // get the range of setup_time_t with this setup (same VMtype and GPUtype)
        auto range = ttime[j.get_ID()].equal_range(job_stp);
        setup_time_t::const_iterator start = range.first;
        setup_time_t::const_iterator finish = range.second;

        setup_time_t::const_iterator c_newsetup;

        // Looking for the setup with exactly this number of GPUs
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

// generate the neighborhood of local_best_schedule
std::unordered_set<unsigned> 
LocalSearchGPU2::generate_neighborhood(void)
{
  // neighbourhood
  std::unordered_set<unsigned> neighbourhood;

  // nodes with the highest number of jobs in tardiness
  std::unordered_set<unsigned> tochange = get_tardiness_nodes(neigh_size);

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