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
              [&](auto self, pmm const &a) {
                int x = 0;
                std::vector<value> v;

                return std::make_tuple(x, v);
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
}