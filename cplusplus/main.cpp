/*
 * C++ header only implementation of Pat, a simple language for expressing
 * poly rythms. Details of Pat can be found in the paper:
 *
 *   Nathan Renney and Benedict R. Gaster. (2019) Digital Expression and
 *   Representation of Rhythm. AM'19: Proceedings of the 14th International
 *   Audio Mostly Conference on Augmented and Participatory Sound and Music
 *   Experiences.
 *
 *   https://uwe-repository.worktribe.com/output/2569484
 *
 * This is a simple example of its general use.
 * 
 * Copyright: Benedict R. Gaster (2021)
 */
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

  auto stp = pat::subdivAux(v);

  auto sv = pat::to_seq(std::get<1>(stp));

  std::cout << "\n" << v;

  std::cout << "\n" << tp << "\n\n";

  for (auto &p: sv) {
    std::cout << "[";
    for (auto s : p) {
      std::cout << s << " ";
    }
    std::cout << "]";
  }

  return 0;
}