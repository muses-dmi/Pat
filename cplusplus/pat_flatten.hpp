#pragma once

#include "pat.hpp"

#include <tuple>
#include <vector>
#include <algorithm>
#include <numeric>

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

              std::vector<value> v;
              for (auto &sub : vvs) {
                int l = (n / a.size()) - sub.size();
                for (auto &e : sub) {
                  v.push_back(e);
                }
                for (int i = 0; i < l; i++) {
                    v.push_back(value{"-"});
                }
              }

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
              std::cout << "n: " << n << "\n";

              std::vector<value> v;
              for (auto &sub : vvs) {
                int l = (n / a.size()) - sub.size();
                for (auto &e : sub) {
                  v.push_back(e);
                }
                for (int i = 0; i < l; i++) {
                  v.push_back(value{"-"});
                }
              }

              return std::make_tuple(n, v);
            },
            [&](auto self, par const &a) {
              return std::make_tuple(1, std::vector<value>{value{a}});
            },
            [&](auto self, prm const &a) {
              int x = 0;
              std::vector<value> v;

              return std::make_tuple(x, v);
            },
            [&](auto self, pmm const &a) {
              int x = 0;
              std::vector<value> v;

              return std::make_tuple(x, v);
            }}},
        p);
  }
}