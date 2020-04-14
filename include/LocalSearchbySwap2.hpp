#ifndef LOCAL_SEARCH_BY_SWAP2_HH
#define LOCAL_SEARCH_BY_SWAP2_HH

#include "LocalSearchbySwap.hpp"

class LocalSearchbySwap2 : public LocalSearchbySwap
{

private:
    void fill_swapping_sets() override;
    job_schedule_t perform_swap(const std::vector<int>& swap_indices) override;
    
public:
    LocalSearchbySwap2(const std::string&, const std::string&, const std::string&,
	                   const std::string&, const std::string&);

	virtual ~LocalSearchbySwap2 (void) = default;
};


#endif /* LOCAL_SEARCH_BY_SWAP2_HH */
