#ifndef LOCAL_SEARCH_BY_SWAP1_HH
#define LOCAL_SEARCH_BY_SWAP1_HH

#include "LocalSearchbySwap.hpp"

class LocalSearchbySwap1 : public LocalSearchbySwap
{

private:
    void fill_swapping_sets() override;
    job_schedule_t perform_swap(const std::vector<int>& swap_indices) override;
    
public:
    LocalSearchbySwap1(const std::string&, const std::string&, const std::string&,
	                   const std::string&, const std::string&);

	virtual ~LocalSearchbySwap1 (void) = default;
};

#endif /* LOCAL_SEARCH_BY_SWAP1_HH */
