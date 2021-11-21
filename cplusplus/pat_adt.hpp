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
 * Copyright: Benedict R. Gaster (2021)
 */
#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <fstream>

// #include <immer/vector.hpp>
#include <vector>

namespace impl {
  // a small set of utility templates to support using lambdas in std::visit,
  // in combination with recursion.
  // the implementation is based on ideas in excellent blog:
  //     https://jgreitemann.github.io/2019/02/03/recursive-visitors-from-fixed-point-combinators/
  template <typename... Bases> struct visitor : Bases... {
    using Bases::operator()...;
  };

    template <typename... Bases> visitor(Bases...) -> visitor<Bases...>;

    template <typename G> struct Y {
      template <typename... X> decltype(auto) operator()(X &&...x) const & {
        return g(*this, std::forward<X>(x)...);
      }
      G g;
    };

    template <typename G> Y(G) -> Y<G>;
}

namespace pat {
    // The pat grammar itself is very simple
    //
    //  pat ::= name | [pat*] | pat |:| pat | pat |+| pat
    //
    //  name ::= [a-zA-Z]+

    // The internal ADT is extended to support notes that happen in parallel, 
    // which is expressed in pat with the polyrythmic or polymetric merge 
    // operators, but once fully flattened is expressed by lists of "parallel" 
    // notes.

    struct top; // forward declaration
    struct seq; // forward declaration
    struct seq; // forward declaration
    struct par; // forward declaration
    struct prm; // forward declaration
    struct pmm; // forward declaration

    // variant (sum) type for patterns
    using value = std::variant<std::string, top, seq, par, prm, pmm>;

    struct top : std::vector<value> {
      using std::vector<value>::vector;
    };

    struct seq : std::vector<value> {
        using std::vector<value>::vector;
    };

    struct par : std::vector<value> {
        using std::vector<value>::vector;
    };

    // invariant must be two elements long, this is used rather 
    // than tuple to enable explicit allocation for recursion
    struct prm : std::vector<value> {
        using std::vector<value>::vector;
    };

    // invariant must be two elements long, this is used rather
    // than tuple to enable explicit allocation for recursion
    struct pmm : std::vector<seq> {
      using std::vector<seq>::vector;
    };

    // simple pretty printer for patterns
    struct printer {
    private:
      std::ostream &os_;
    public:
      printer(std::ostream &os) : os_{os} {}

      std::ostream &operator()(std::string const &s) const {
        return os_ << s;
      }
      
      std::ostream &operator()(top const &a) const {

        for (int i = 0; i < a.size(); i++) {
          if (i != a.size() - 1) {
            std::visit(*this, a[i]) << ' ';
          } else {
            std::visit(*this, a[i]);
          }
        }
        // for (auto const &v : a)

        return os_;
      }
      
      std::ostream &operator()(seq const &a) const {
        os_ << '[';
                
        for (int i = 0; i < a.size(); i++) {
          if (i != a.size() - 1) {
            std::visit(*this, a[i]) << ' ';
          }
          else {
            std::visit(*this, a[i]);
          }
        }
          //for (auto const &v : a)
            
        return os_ << ']';
      }
      std::ostream &operator()(par const &a) const {
        os_ << '[';
        for (auto const &v : a)
          std::visit(*this, v) << ' ';
        return os_ << ']';
      }
      std::ostream &operator()(prm const &a) const {
        std::visit(*this, a[0]) << " |:| ";
        std::visit(*this, a[1]);
        return os_;
      }
      std::ostream &operator()(pmm const &a) const {
        std::visit(*this, value{a[0]}) << " |+| ";
        std::visit(*this, value{a[1]});
        return os_;
      }
    };

    std::ostream &operator<<(std::ostream &os, value const &val) {
      return std::visit(printer{os}, val);
    }
} // namespace pat
