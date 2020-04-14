#ifndef LOCAL_SEARCH_BY_SWAP_HH
#define LOCAL_SEARCH_BY_SWAP_HH

#include "LocalSearch.hpp"


    typedef std::multimap<double, Job> map_value_j;
    typedef map_value_j::iterator map_it;
    typedef std::vector<Job> row_j;
    typedef std::vector<std::vector<int>> vecvec;
    

class LocalSearchbySwap : public LocalSearch
{

protected:
    row_j A_job_ids;
    row_j B_job_ids;
    vecvec possible_swap_indices;
    virtual job_schedule_t perform_swap(const std::vector<int>& swap_indices)=0;
    row_j get_top(const map_value_j&) ;
    row_j top_tardiness_jobs() ;
    row_j top_cost_jobs() ;
    row_j top_margin_jobs() ;
    row_j top_tardiness_waiting_jobs();
    virtual void fill_swapping_sets()=0;
    vecvec get_base_possible_swaps() const;
    vecvec permute_unique(std::vector<int> vec) const;
    vecvec get_possible_swaps() const;
    bool visit_neighbor() override;
    void erase_job_from_node(Schedule &);

public:
    LocalSearchbySwap(const std::string&, const std::string&, const std::string&,
	                   const std::string&, const std::string&);

	virtual ~LocalSearchbySwap (void) = default;
};


#endif /* LOCAL_SEARCH_BY_SWAP_HH */
