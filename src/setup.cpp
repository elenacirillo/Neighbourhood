#include "setup.hpp"

Setup::Setup (const row_t& info)
{
  typename row_t::const_iterator it = info.cbegin();
  VMtype = *it++;
  GPUtype = *it++;
  nGPUs = std::stoi(*it++);
  max_nGPUs = std::stoi(*it++);
  cost = std::stod(*it);
}

Setup::Setup (const std::string& vm, const std::string& gpu):
  VMtype(vm), GPUtype(gpu)
{}

void
Setup::print_names (std::ostream& ofs, char endline)
{
  ofs << "VMType,GpuType,GpuNumber,cost" << endline;
}

void
Setup::print (std::ostream& ofs, char endline) const
{
  ofs << VMtype << "," << GPUtype << "," << nGPUs << "," 
      << cost << endline;
}

bool
operator== (const Setup& s1, const Setup& s2)
{
  return (s1.VMtype == s2.VMtype && s1.GPUtype == s2.GPUtype);
}

bool operator!= (const Setup& s1, const Setup& s2)
{
  return ! (s1 == s2);
}