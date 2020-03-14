#ifndef BUILDERS_HH
#define BUILDERS_HH

#include <memory>

/* BaseBuilder
*     abstract builder base class to be used with object factories. It relies
*     on a constructor that takes five strings as arguments; it is intended 
*     to be used with Heuristic class
*/
template <typename BaseClass>
class BaseBuilder {

public:
  virtual std::unique_ptr<BaseClass> create (const std::string&, 
                                             const std::string&, 
                                             const std::string&, 
                                             const std::string&, 
                                             const std::string&) = 0;
  virtual ~BaseBuilder (void) = default;

};


/* Builder
*     builder class to be used with object factories. It relies on
*     on a constructor that takes five strings as arguments; it is intended 
*     to be used with classes deriving from Heuristic
*/
template <typename DerivedClass, typename BaseClass>
class Builder: public BaseBuilder<BaseClass> {

public:
  std::unique_ptr<BaseClass> create (const std::string& s1, 
                                     const std::string& s2, 
                                     const std::string& s3, 
                                     const std::string& s4, 
                                     const std::string& s5)
  {
    return std::unique_ptr<BaseClass> (new DerivedClass(s1,s2,s3,s4,s5));
  }

};

#endif /* BUILDERS_HH */