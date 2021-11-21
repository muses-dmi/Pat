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

#include "pat_adt.hpp"

#include <tuple>
#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>

namespace impl {
    std::vector<pat::value> extend(int n, int ln,
                                    std::vector<std::vector<pat::value>> vvs) {
    std::vector<pat::value> v;
        for (auto &sub : vvs) {
            int l = (n / ln) - sub.size();
            for (auto &e : sub) {
                v.push_back(e);
            }
            for (int i = 0; i < l; i++) {
                v.push_back(pat::value{"-"});
            }
        }
        return v;
    }

    std::vector<pat::value> expand(int lcm, int n, std::vector<pat::value> vs) {
        std::vector<pat::value> result;
        for (auto &v : vs) {
            result.push_back(v);
            for (int i = 0; i < (lcm / n) - 1; i++) {
                result.push_back(pat::value{"-"});
            }
        }
        return result;
    }
} // namespace impl

namespace pat {
    
    std::tuple<int, std::vector<value>> subdivAux(value p) {
      return std::visit(
          impl::Y{impl::visitor{
              [&](auto self, std::string const &s) {
                return std::make_tuple(1, std::vector<value>{value{s}});
              },
              [&](auto self, top const &a) {
                std::vector<std::vector<value>> vvs;
                std::vector<int> is;

                for (auto &s : a) {
                  auto t = subdivAux(s);
                  vvs.push_back(std::get<1>(t));
                  is.push_back(std::get<0>(t));
                }

                // calulate LCM of all sub lengths
                int n = is[0];
                std::for_each(is.begin() + 1, is.end(),
                              [&](int i) { n = std::lcm(i, n); });
                n = n * a.size();

                // extend each sub pattern to shared length
                auto v = impl::extend(n, a.size(), vvs);

                return std::make_tuple(n, v);
              },
              [&](auto self, seq const &a) {
                std::vector<std::vector<value>> vvs;
                std::vector<int> is;
                for (auto &s : a) {
                  auto t = subdivAux(s);
                  vvs.push_back(std::get<1>(t));
                  is.push_back(std::get<0>(t));
                }

                // calulate LCM of all lengths
                int n = is[0];
                std::for_each(is.begin() + 1, is.end(),
                              [&](int i) { n = std::lcm(i, n); });
                n = n * a.size();

                // extend each sub pattern to shared length
                auto v = impl::extend(n, a.size(), vvs);

                return std::make_tuple(n, v);
              },
              [&](auto self, par const &a) {
                return std::make_tuple(1, std::vector<value>{value{a}});
              },
              [&](auto self, prm const &a) {
                auto l = subdivAux(a[0]);
                auto r = subdivAux(a[1]);
                int lcm = std::lcm(std::get<0>(l), std::get<0>(r));

                auto el = impl::expand(lcm, std::get<0>(l), std::get<1>(l));
                auto er = impl::expand(lcm, std::get<0>(r), std::get<1>(r));

                seq v;
                for (int i = 0; i < el.size(); i++) {
                  v.push_back(value{par{el[i], er[i]}});
                }

                return subdivAux(v);
              },

              //               pm :: Event a b => Pat a -> Pat a -> Pat a
              // pm (PPSeq l)  (PPSeq r) =
              //   let l' = PPSeq (concat (replicate (length r) l))
              //       r' = PPSeq (concat (replicate (length l) r))
              //   in  PPSeq $ map PPPar $ zipWith (\x y -> [x,y]) (subdiv l')
              //   (subdiv r')
              // pm _          _         = error "pm missing cases"

              [&](auto self, pmm const &a) {
                auto l = a[0];
                auto r = a[1];
                seq ll;
                for (int i = 0; i < r.size(); i++) {
                  for (auto &v: l) {
                    ll.push_back(v);
                  }
                }
                auto ls = std::get<1>(subdivAux(ll));

                seq rr;
                for (int i = 0; i < l.size(); i++) {
                  for (auto &v : r) {
                    rr.push_back(v);
                  }
                }
                auto rs = std::get<1>(subdivAux(rr));

                seq v;
                for (int i = 0; i < ls.size(); i++) {
                  v.push_back(value{par{ls[i], rs[i]}});
                }

                return subdivAux(v);
              }}},
          p);
    }

  value subdiv(value p) {
    auto s = std::get<1>(subdivAux(p));

    pat::top tp;
    for (auto &h : s) {
      tp.push_back(h);
    }

    return tp;
  }

  // Convert a flattened pattern to a Sequence for use in the Muses Pat Max external...
  // A flattened pattern is limited as follows:
  //
  //  pat        ::= flattened*
  //  flattened :: = par | string
  //
  //
  std::vector<std::vector<std::string>> to_seq(std::vector<value> p) {
    std::vector<std::vector<std::string>> result;
    
    for (auto &v: p) {
      if (std::holds_alternative<par>(v)) { // par case
        std::vector<value> pa = std::get<par>(v);
        std::vector<std::string> current;
        for (auto s: pa) {
          if (std::holds_alternative<std::string> (s)) {
            // prepend text, making it easier too map in Max messages
            current.push_back("text " + std::get<std::string>(s));
          }
        }
        result.push_back(current);
      } else if (std::holds_alternative<std::string>(v)) { // lit case
        result.push_back(std::vector<std::string>{"text " + std::get<std::string>(v)});
      }
    }
    return result;
  }
}