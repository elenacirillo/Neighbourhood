#include "LocalSearchGPU.hpp"

// constructor
LocalSearchGPU::LocalSearchGPU(const std::string& args, const std::string& d,
                        const std::string& file_jobs, const std::string& file_times,
                        const std::string& file_nodes):
  LocalSearch(args, d, file_jobs, file_times, file_nodes)
{}

// update the data structure node_jobs acordingly to local_best_schedule
void
LocalSearchGPU::update_node_jobs(void)
{
  node_jobs.clear();

  // cycle over the initial schedule checking if it's empty
  for(auto js: local_best_schedule)
  {
    const Job & job = js.first;
    Schedule & sch = js.second;
    if(!sch.isEmpty())
    {
      unsigned node_idx = sch.get_node_idx();

      // insert the job in the unordered_set associated with the node idx
      node_jobs[node_idx].insert(job);
    }

  }
}

// returns the indexes of the neigh_size nodes with the highest number of jobs in tardiness;
std::unordered_set<unsigned> 
LocalSearchGPU::get_tardiness_nodes(unsigned top)
{
  std::uniform_int_distribution<unsigned> distribution(0,last_node_idx-1);

  // Cycle over the schedule to create the multimap of nodes having at least one job in tardiness
  std::map<unsigned, unsigned> multi_indices;
  for(auto js: local_best_schedule)
  {
    const Job & j = js.first;
    const Schedule & sch = js.second;
    if(!sch.isEmpty())
    {
      // If the job is in tardiness..
      if(j.get_deadline() < current_time)
      {
        // ..add its node index to the list of indices
        multi_indices[sch.get_node_idx()]++;
      }
    }
  }
  
  std::vector<unsigned> indices_vec;
  
  // if the multimap is not empty, remove dupicates and order by # of jobs in tardiness
  if( !multi_indices.empty())
  {
    indices_vec.assign(multi_indices.begin(),multi_indices.end());
    auto it=std::unique(indices_vec.begin(), indices_vec.end());
    indices_vec.resize(static_cast<int>(std::distance(indices_vec.begin(),it)));
    std::sort(indices_vec.begin(),indices_vec.end(),[&] ( const unsigned &u1,const unsigned &u2 )
      {
        return multi_indices.count(u1) > multi_indices.count(u2);
        }
      );
  }
  // If there are more than neigh_size nodes with jobs in tardiness
  if (indices_vec.size()>neigh_size)
  {
    indices_vec.resize(neigh_size);
  }
  // I put everything in the final set and fill potential gaps.
  std::unordered_set<unsigned> indices(indices_vec.begin(),indices_vec.end());
  
  // While the number of jobs in tardiness is less then neigh_size
  
  while(indices.size() < std::min(neigh_size, last_node_idx))
  {
    // I add open nodes choosen at random..
    unsigned idx = distribution(generator);
    // ..if they are not already present
      indices.insert(idx);
  }
  return indices;
  
}
