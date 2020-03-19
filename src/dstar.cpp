#include "dstar.hpp"

#include <iostream>

Dstar::Dstar (const Job& j, const setup_time_t& tjvg, double ct)
{
  setup_time_t::const_iterator cit;
  for (cit = tjvg.cbegin(); cit != tjvg.cend(); ++cit)
  {
    const Setup& stp = cit->first;
    double t = cit->second;
    if (t + ct <= j.get_deadline())
    {
      double cost = stp.get_cost() * t;
      D_star_j.insert({cost, cit});
    }
    else
      D_star_j_C.insert({t, cit});
  }
}

setup_time_t::const_iterator
Dstar::get_best_setup (std::default_random_engine& generator)
{
  assert(! is_end());

  setup_time_t::const_iterator cit;

  if (! D_star_j.empty())
    cit = random_select(D_star_j, generator, alpha);
  else
    cit = random_select(D_star_j_C, generator, alpha);
  
  return cit;
}

bool
Dstar::is_end (void) const
{
  return (D_star_j.empty() && D_star_j_C.empty());
}

void
Dstar::print (std::ostream& ofs) const
{
  ofs << "D_star_j" << std::endl;
  ofs << "cost,";
  Setup::print_names(ofs,',');
  ofs << "time" << std::endl;
  for (const_iterator cit = D_star_j.cbegin(); cit != D_star_j.cend(); ++cit)
  {
    ofs << cit->first << ",";
    setup_time_t::const_iterator cit2 = cit->second;
    (cit2->first).print(ofs,',');
    ofs << cit2->second << std::endl;
  }

  ofs << "\nD_star_j_C" << std::endl;
  ofs << "cost,";
  Setup::print_names(ofs);
  for (const_iterator cit = D_star_j_C.cbegin(); 
       cit != D_star_j_C.cend(); ++cit)
  {
    ofs << cit->first << ",";
    setup_time_t::const_iterator cit2 = cit->second;
    (cit2->first).print(ofs,',');
    ofs << cit2->second << std::endl;
  }
}