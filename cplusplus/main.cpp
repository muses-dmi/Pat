#include <pat.hpp>
#include <sstream>

//---------------------------------------------------------------------------

int main() {
  pat::value val = pat::top {
      pat::prm{ pat::seq{"bd", "sd"}, pat::seq{"bd", "sd"} }
  };

  std::cout << val << std::endl;

  

  std::stringstream ex1("bd sd bd [sd sd]");
  std::stringstream ex2("bd - bd - |:| hh - sn");
  std::stringstream ex3("bd - bd - |+| hh - sd");

  pat::Parser parser{ex3};
  auto v = parser.parse();

  auto tp = pat::subdiv(v);

  std::cout << "\n" << v;

  std::cout << "\n" << tp;

  return 0;
}