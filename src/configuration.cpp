#include "configuration.hpp"

void
Configuration::set_configuration (const Setup& stp)
{
  VMtype = stp.get_VMtype();
  GPUtype = stp.get_GPUtype();
  cost = stp.get_cost();
  used_GPUs = 0;
  remaining_GPUs = stp.get_maxnGPUs();
}

void
Configuration::update_n_GPUs (unsigned g)
{
  unsigned max_GPUs = used_GPUs + remaining_GPUs;
  if (remaining_GPUs >= g)
  {
    remaining_GPUs -= g;
    used_GPUs += g;
  }
  else
  {
    std::cout << "ERROR: trying to occupy to more than the num of remaining gpus." << std::endl;
    remaining_GPUs = 0;
    used_GPUs = max_GPUs;
  }
}

void
Configuration::delete_configuration (void)
{
  VMtype = "";
  GPUtype = "";
  cost = 0.;
  used_GPUs = 0;
  remaining_GPUs = 0;
}

void
Configuration::print_names (std::ostream& ofs, char endline)
{
  ofs << "VMtype,GPUtype,cost,used_GPUs,remaining_GPUs" << endline;
}

void
Configuration::print (std::ostream& ofs, char endline) const
{
  ofs << VMtype << "," << GPUtype << "," << cost << "," << used_GPUs << ","
      << remaining_GPUs << endline;
}
