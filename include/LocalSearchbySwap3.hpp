#ifndef LOCAL_SEARCH_BY_SWAP3_HH
#define LOCAL_SEARCH_BY_SWAP3_HH

#include "LocalSearchbySwap.hpp"

class LocalSearchbySwap3 : public LocalSearchbySwap
{

private:
    void fill_swapping_sets() override;
    job_schedule_t perform_swap(const std::vector<int>& swap_indices) override;
    double evaluate_objective(job_schedule_t&);

public:
    LocalSearchbySwap3(const std::string&, const std::string&, const std::string&,
	                   const std::string&, const std::string&);

	virtual ~LocalSearchbySwap3 (void) = default;
};


#endif /* LOCAL_SEARCH_BY_SWAP3_HH */