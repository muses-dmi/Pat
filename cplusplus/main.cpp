#include <pat.hpp>
#include <pat_parser.hpp>
#include <pat_flatten.hpp>
#include <sstream>

//---------------------------------------------------------------------------

int main() {
  pat::value val = pat::top {
      pat::prm{ pat::seq{"bd", "sd"}, pat::seq{"bd", "sd"} }
  };

  std::cout << val << std::endl;

  

  std::stringstream sss("bd [sd hh] |:| - sd");

  std::stringstream ex1("bd sd bd [sd sd]");
  std::stringstream ex2("bd - bd - |:| hh - sn");

  pat::Parser parser{ex2};
  auto v = parser.parse();

  auto tp = pat::subdiv(v);

  // pat::top tp;
  // for (auto &h: std::get<1>(t)) {
  //   tp.push_back(h);
  // }

  std::cout << "\n" << v;

  std::cout << "\n" << tp;

  return 0;
}