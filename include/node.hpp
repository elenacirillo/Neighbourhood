#ifndef NODE_HH
#define NODE_HH

#include <ostream>
#include <cassert>

#include "utilities.hpp"
#include "setup.hpp"
#include "configuration.hpp"

class Node {

public:
  // friend operator==
  friend bool operator== (const Node&, const Node&);

private:
  std::string ID = "";

  bool isOpen = false;
  Configuration c;

public:
  /*  constructor
  *
  *   Input(1):  void              default
  *
  *   Input(2):  const row_t&      list of elements used to initialize all the
  *                                class members (see utilities.hpp to inspect
  *                                row_t type)
  */
  Node (void) = default;
  Node (const row_t& id): ID(*(id.cbegin())) {}

  // getters
  const std::string& get_ID (void) const {return ID;}
  const std::string& get_VMtype (void) const;
  const std::string& get_GPUtype (void) const;
  double get_cost (void) const; 
  unsigned get_usedGPUs (void) const;
  unsigned get_remainingGPUs (void) const;

  // setters
  void set_remainingGPUs (unsigned);
  void change_setup (const Setup&);

  /*  open_node
  *     changes state of node and assigns the given configuration
  *
  *   Input:  const Setup&        Setup to be assigned to node
  */
  void open_node (const Setup&);

  /*  close_node
  *     changes the state of node and deletes the current configuration
  */
  void close_node (void);

  /*  print_names (static)
  *
  *   Input:  std::ostream&       where to print names (namely "Nodes") of 
  *                               fields stored in the class
  *           char endline='\n'   last character to be printed (default \n)
  */
  static void print_names (std::ostream&, char endline='\n');

  /*  print
  *
  *   Input:  std::ostream&       where to print node ID
  *           char endline='\n'   last character to be printed (default \n)
  */
  void print (std::ostream&, char endline='\n') const;

  /*  print_open_node
  *
  *   Input:  std::ostream&       where to print node ID and current
  *                               configuration
  */
  void print_open_node (std::ostream&) const;
};

// operator==   two nodes are equal if they have the same ID
bool operator== (const Node&, const Node&);

// specialization of the hash function
namespace std {    
  template<>
  struct hash<Node> {
    std::size_t operator() (const Node& n) const {
        return std::hash<std::string>() (n.get_ID());
    }
  };
}

#endif /* NODE_HH */