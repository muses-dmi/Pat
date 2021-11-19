#include <pat.hpp>
#include <sstream>

//---------------------------------------------------------------------------

int main() {
  pat::value val = pat::top {
      pat::prm{ pat::seq{"bd", "sd"}, pat::seq{"bd", "sd"} }
  };

  std::cout << val << std::endl;

  std::stringstream ss("bd sd |+| [bd sd]");
  std::stringstream sss("bd [sd hh] |:| - sd");

  pat::Tokenizer tz{ss};

  while (tz.hasMoreTokens()) {
    std::cout << tz.getToken().toString() << " ";
  }

  pat::Parser parser{sss};

  auto v = parser.parse();

  std::cout << "\n" << v;

  return 0;
}