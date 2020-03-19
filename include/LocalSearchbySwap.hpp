#ifndef LOCAL_SEARCH_BY_SWAP_HH
#define LOCAL_SEARCH_BY_SWAP_HH

#include "LocalSearch.hpp"


    typedef std::multimap<double, Job> map_value_j;
    typedef map_value_j::iterator map_it;
    typedef std::vector<Job> row_j;
    
    enum neigh_type {
        neigh_1,
        neigh_2,
        neigh_3
        // ...
        //TODO cambiare i nomi dei neig_type con qualcosa di sensato
        };

class LocalSearchbySwap : public LocalSearch
{

private:
    // Neighbourhood definition
    neigh_type neigh_def = neigh_1;
    row_j A_job_ids;
    row_j B_job_ids;
    std::vector< std::vector<int> > possible_swap_indices;
    job_schedule_t perform_swap(const std::vector<int>& swap_indices);
    row_j get_top(const map_value_j&) ;
    row_j top_tardiness_jobs() ;
    row_j top_cost_jobs() ;
    row_j top_margin_jobs() ;
    void fill_swapping_sets();
    std::vector< std::vector<int> > get_base_possible_swaps() const;
    std::vector< std::vector<int> > permute_unique(std::vector<int> vec) const;
    std::vector< std::vector<int> > get_possible_swaps() const;

    bool visit_neighbor() override;

public:
    LocalSearchbySwap(const std::string&, const std::string&, const std::string&,
	                   const std::string&, const std::string&);

	virtual ~LocalSearchbySwap (void) = default;
};

#endif /* LOCAL_SEARCH_BY_SWAP_HH */
