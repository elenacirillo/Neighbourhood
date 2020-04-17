#ifndef LOCAL_SEARCH_BY_SWAP_HH
#define LOCAL_SEARCH_BY_SWAP_HH

#include "LocalSearch.hpp"

class LocalSearchbySwap : public LocalSearch
{
public:
    typedef std::multimap<double, Job> map_value_j;
    typedef map_value_j::iterator map_it;
    typedef std::vector<Job> row_j;
    typedef std::vector<std::vector<int>> vecvec;
    
    enum neigh_type {
        neigh_1,
        neigh_2,
        };

private:
    // Neighbourhood definition
    neigh_type neigh_def = neigh_2;
    // sets of jobs to be swapped
    row_j A_job_ids;
    row_j B_job_ids;
    // vector of vectors of possible swappings
    vecvec possible_swap_indices;

    // functions to fill A_jobs_ids and B_jobs_ids
    row_j get_top(const map_value_j&) ;
    row_j top_tardiness_jobs() ;
    row_j top_cost_jobs() ;
    row_j top_margin_jobs() ;
    void fill_swapping_sets();
    // function to generate possible_swap_indices
    vecvec get_base_possible_swaps() const;
    vecvec permute_unique(std::vector<int> vec) const;
    vecvec get_possible_swaps() const;

    // function of the local search
    bool visit_neighbor() override;
    void erase_job_from_node(Schedule &);
    job_schedule_t perform_swap(const std::vector<int>& swap_indices);

public:
    LocalSearchbySwap(const std::string&, const std::string&, const std::string&,
	                   const std::string&, const std::string&);

	virtual ~LocalSearchbySwap (void) = default;
};


#endif /* LOCAL_SEARCH_BY_SWAP_HH */
