#include "LocalSearchGPU.hpp"

//________________________________________________________________________________________________________________________________

// constructor
LocalSearchGPU::LocalSearchGPU(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  LocalSearch(args, d, file_jobs, file_times, file_nodes)
{

  // initialize vm_gpus starting from ttime
  initialize_vm_gpus();

}

//________________________________________________________________________________________________________________________________-

// initialize the map GPU-VM starting from ttime (ttime is a protected memeber of the parent class Heuristic)
void
LocalSearchGPU::initialize_vm_gpus(void)
{
  if(! ttime.empty())
  {
    // consider the setups related to the first job
    // Here  I am assuming that all jobs work with all setups hence it is enough to cycle over the first jobś setups
    setup_time_t & setup_time = ttime.begin()->second;
    // cycle over all its setups. TODO Here I want types to be ordered by type???
    // namely the most powerful first
    for (auto stp_t: setup_time)
    {
      vm_gpus[stp_t.first.get_VMtype()] = stp_t.first.get_GPUtype();
    }
  }
}

//________________________________________________________________________________________________________________________________

// update the data structure node_jobs acordingly to initial_schedule
void
LocalSearchGPU::update_node_jobs(void)
{
  node_jobs.clear();

  // cycle over the initial schedule
  for(auto pair: initial_schedule)
  {
    Job & job = pair.first;
    Schedule & sch = pair.second;

    unsigned node_idx = sch.get_node_idx();

    // insert the job in the unordered_set associated with the node idx
    node_jobs[sch.get_node_idx()].insert(job);
  }
}

//________________________________________________________________________________________________________________________________

// look for a better schedule by visiting the neighborhood of initial schedule
bool
LocalSearchGPU::visit_neighbor(bool best_fit) override
{
  // true if the function finds a better schedule and updates it
  bool changed = false;

  // update the data structure that links the nodes to the jobs (change_GPU needs it updated)
  update_node_jobs();

  // neighborhood to be explored
  neighborhood_t neighborhood = generate_neighborhood();

  // visit the neighbourhood
  for(auto neighbor: neighborhood)
  {
    // a neighbor is made of a node and a new GPU_type
    unsigned node_idx = neighbor.first;
    std::string GPU_type = neighbor.second;

    // candidate schedule, obtained by modifying the initial schedule accordingly to neighbor infos
    job_schedule_t candidate_schedule = change_GPU(node_idx, GPU_type).first;
    setup new_stp=change_GPU(node_idx, GPU_type).second;
    // evaluate the objective in this point of the neighborhood
    double candidate_value = evaluate_objective(candidate_schedule);

    // if this schedule improves the objective function
    if (candidate_value < best_schedule_value)
    {
      // update the best schedule and the best value
      best_schedule = candidate_schedule;
      best_schedule_value = candidate_value;
      changed = true;

      // update the configuration of the node accordingly
      nodes[node_idx].change_setup(new_stp);
      nodes[node_idx].set_remainingGPUs(new_stp.get_maxnGPUs()-new_stp.get_nGPUs());

      // if I'm not performing a best improvement search
      if(!best_fit)
      {
        break;
      }
    }

  }

  return changed;

}


//________________________________________________________________________________________________________________________________

// create a new schedule by changing the type of GPU on node node_idx and updating the schedules of the jobs on that node
std::pair<job_schedule_t, setup>
LocalSearchGPU::change_GPU(unsigned node_idx, std::string GPU_type)
{
  // new schedule to modify
  job_schedule_t new_schedule = initial_schedule;

  // get the jobs in execution on the node "node_idx"
  std::unordered_set<Job> jobs_to_modify = node_jobs[node_idx];

  if(!jobs_to_modify.empty())
  {
    // consider one of these jobs
    Job & j = *jobs_to_modify.begin();
    Schedule & sch = new_schedule[j];
    // old setup of the node
    stp = sch.get_setup();
    // new setup
    //get information from existing setup to build setup
    std::string VM_type = stp.get_VMtype();
    unsigned nGPUs=stp.get_usedGPUs();
    unsigned max_nGPUs=stp.get_maxnGPUs();
    double cost=get_cost();
    row_t build_stp;
    build_stp.push_back(VM_type);
    build_stp.push_back(GPU_type);
    build_stp.push_back(std::to_string(nGPUs));
    build_stp.push_back(std::to_string(max_nGPUs));
    build_stp.push_back(std::to_string(cost));
    setup new_stp(build_stp);
    setup_time_t new_stp_time_t;
    new_stp_time_t[new_stp]=sch.get_selectedTime();
    setup_time_t::const_iterator c_newsetup= new_stp_time_t.cbegin();

    for(Job j: jobs_to_modify)
    {
      // schedule of job j
      Schedule & sch = new_schedule[j];
      sch.change_setup(c_newsetup);
    }
  return std::make_pair(new_schedule,new_stp);
  }
}

//________________________________________________________________________________________________________________________________

// generate the neighborhood of initial_schedule
neighborhood_t
LocalSearchGPU::generate_neighborhood(void)
{
  // neighbourhood
  neighborhood_t neighbourhood;
  unsigned neigh_size=5; // number of nodes to change in the neighbourhood
  std::set<unsigned> tochange=FindTopNodes(neigh_size);
  if (!tochange.empty())
  {
    for(auto n: tochange)
    {
      std::string VM=nodes[n].get_VMtype();
      std::list<std::string> possible_gpus= vm_gpus[VM];
      //here I assume that the GPUs are ordered by power
      neighbourhood[n]=possible_gpus.front();
    }
  return neighbourhood;
  }
  // I take the first nodes and in this case I take a less powerful GPU
  else
  {
    // I count the number of open nodes
    for(auto i=0;i<neigh_size;i++)
    {
      if (nodes[i].get_usedGPUs()>0)
      {
        tochange.insert(i);
      }
    }
    for(auto n: tochange)
    {
      std::string VM=nodes[n].get_VMtype();
      std::list<std::string> possible_gpus= vm_gpus[VM];
      //here I assume that the GPUs are ordered by power
      auto pos=possible_gpus.find(nodes[n].get_GPUtype());
      if (pos!= possible_gpus.end())
      {
        neighbourhood[n]= *(pos+1);
      }
      else
      {
        neighbourhood[n]=possible_gpus.back();
      }
    }
  return neighbourhood;
  }
}

// returns the indexes of the 5 nodes with the highest number of jobs in tardiness;
std::set<unsigned> LocalSearchGPU::FindTopNodes(unsigned top)
{
  // TODO here I should add a comparator ot order by number of jobs in tardiness
  std::map<unsigned,unsigned> topnodes;
  //count the jobs in each node that have tardiness>0
  for (auto js:initial_schedule)
  {
    if (js.second.get_tardiness>0.)
    topnodes[js.second.get_node_idx]++;
  }
  std::set<unsigned> result;
  for(auto node_count:topnodes)
  {
    result.insert(node_count.first);
  }
  return result;
}
//________________________________________________________________________________________________________________________________
