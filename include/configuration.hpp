#ifndef CONFIGURATION_HH
#define CONFIGURATION_HH

#include "setup.hpp"

class Configuration {

private:
  std::string VMtype = "";
  std::string GPUtype = "";
  double cost = 0.;
  unsigned used_GPUs = 0;
  unsigned remaining_GPUs = 0;

public:
  /*  constructors
  *
  *   Input(1):  void              default
  */
  Configuration (void) = default;

  // getters
  const std::string& get_VMtype (void) const {return VMtype;}
  const std::string& get_GPUtype (void) const {return GPUtype;}
  double get_cost (void) const {return cost;}
  unsigned get_usedGPUs (void) const {return used_GPUs;}
  unsigned get_remainingGPUs (void) const {return remaining_GPUs;}

  // setters
  void set_configuration (const Setup&);
  void delete_configuration (void);
  void update_n_GPUs (unsigned);

  /*  print_names (static)
  *
  *   Input:  std::ostream&       where to print names of fields stored in
  *                               the class    
  *           char endline='\n'   last character to be printed (default \n)
  */
  static void print_names (std::ostream&, char endline='\n');

  /*  print
  *
  *   Input:  std::ostream&       where to print elements stored in the class
  *           char endline='\n'   last character to be printed (default \n)
  */
  void print (std::ostream&, char endline='\n') const;

};

#endif /* CONFIGURATION_HH */