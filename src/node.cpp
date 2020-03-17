#include "node.hpp"

const std::string&
Node::get_VMtype (void) const
{
  assert(isOpen);
  return c.get_VMtype();
}
  
const std::string&
Node::get_GPUtype (void) const 
{
  assert(isOpen);
  return c.get_GPUtype();
}

double
Node::get_cost (void) const
{
  assert(isOpen);
  return c.get_cost();
}

unsigned
Node::get_usedGPUs (void) const
{
  assert(isOpen);
  return c.get_usedGPUs();
}

unsigned
Node::get_remainingGPUs (void) const
{
  assert(isOpen);
  return c.get_remainingGPUs();
}

void 
Node::set_remainingGPUs (unsigned g)
{
  c.update_n_GPUs(g);
}

void
Node::change_setup (const Setup& stp)
{
  c.set_configuration(stp);
}

void
Node::open_node (const Setup& stp)
{
  isOpen = true;
  c.set_configuration(stp);
}

void
Node::close_node (void)
{
  isOpen = false;
  c.delete_configuration();
}

void
Node::print_names (std::ostream& ofs, char endline)
{
  ofs << "Nodes" << endline;
}

void
Node::print (std::ostream& ofs, char endline) const
{
  ofs << ID << endline;
}

void
Node::print_open_node (std::ostream& ofs) const
{
  assert(isOpen);
  Node::print_names(ofs,',');
  Configuration::print_names(ofs);
  
  print(ofs,',');
  c.print(ofs);
}

bool
operator== (const Node& n1, const Node& n2)
{
  return (n1.ID == n2.ID);
}