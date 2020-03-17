#ifndef SETUP_HH
#define SETUP_HH

#include <ostream>
#include <vector>

#include "utilities.hpp"

class Setup {

public:
  // friend operator== and operator!=
  friend bool operator== (const Setup&, const Setup&);
  friend bool operator!= (const Setup&, const Setup&);

private:
  std::string VMtype = "";
  std::string GPUtype = "";
  unsigned nGPUs = 0;
  unsigned max_nGPUs = 0;
  double cost = 0.;

public:
  /*  constructors
  *
  *   Input(1):  void                 default
  *
  *   Input(2):  const row_t&         list of elements used to initialize all 
  *                                   the class members (see utilities.hpp to 
  *                                   inspect row_t type)
  *
  *   Input(3):  const std::string&   type of VM
  *              const std::string&   type of GPU
  */
  Setup (void) = default;
  Setup (const row_t& info);
  Setup (const std::string&, const std::string&);

  // getters
  const std::string& get_VMtype (void) const {return VMtype;}
  const std::string& get_GPUtype (void) const {return GPUtype;}
  unsigned get_nGPUs (void) const {return nGPUs;}
  unsigned get_maxnGPUs (void) const {return max_nGPUs;}
  double get_cost (void) const {return cost;}

  /*  print_names (static)
  *
  *   Input:  std::ostream&       where to print names of fields stored in the 
  *                               class
  *           char endline='\n'   last character to be printed (default \n)
  */
  static void print_names (std::ostream&, char endline='\n');

  /*  print
  *
  *   Input:  std::ostream&       where to print information stored in the 
  *                               class
  *           char endline='\n'   last character to be printed (default \n)
  */
  void print (std::ostream&, char endline = '\n') const;

};

// operator==   two setups are equal if they have the same VMtype and GPUtype
bool operator== (const Setup&, const Setup&);
bool operator!= (const Setup&, const Setup&);

// specialization of the hash function
namespace std {    
  template<>
  struct hash<Setup> {
    std::size_t operator() (const Setup& s) const {
        std::size_t h1 = std::hash<std::string>() (s.get_VMtype());
        std::size_t h2 = std::hash<std::string>() (s.get_GPUtype());
        return (h1 ^ (h2 << 1));
    }
  };
}

#endif /* SETUP_HH */