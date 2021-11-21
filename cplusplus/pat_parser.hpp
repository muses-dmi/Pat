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

namespace pat {
    enum class TOKEN {
      LIST_OPEN,
      LIST_CLOSE,
      PRM,
      PMM,
      VARIABLE,
      SLIENT,
    };

    struct Token {
        std::string value_;
        TOKEN type_;
    
        std::string toString() {
            switch (type_) {
                case TOKEN::LIST_OPEN: {
                    return "[";
                }
                case TOKEN::LIST_CLOSE: {
                    return "]";
                }
                case TOKEN::PMM: {
                    return "|+|";
                }
                case TOKEN::PRM: {
                    return "|:|";
                }
                case TOKEN::VARIABLE: {
                    return value_;
                }
                case TOKEN::SLIENT: {
                  return "-";
                }
                default:
                    break;
            }

            return "";
        }
    };

    class Tokenizer {
      std::istream& file_;
      size_t prevPos_;

    public:
      Tokenizer(std::istream& s) : file_{s} { }

      char getWithoutWhiteSpace() {
        char c = ' ';
        // std::cout << "e" << file_.eof() << std::endl;
        while ((c == ' ' || c == '\n')) {
          file_.get(c); // check

          // std::cout << "**" << c << "**" << file_.eof() << std::endl;
          if ((c == ' ' || c == '\n') && !file_.good()) {
            // std::cout << file.eof() << " " << file.fail() << std::endl;
            throw std::logic_error("Ran out of tokens");
          } else if (!file_.good()) {
            return c;
          }
        }

        return c;
      }

      Token getToken() {
        char c;
        if (file_.eof()) {
          std::cout << "Exhaused tokens" << std::endl;
          // throw std::exception("Exhausted tokens");
        }
        prevPos_ = file_.tellg();
        c = getWithoutWhiteSpace();

        struct Token token;
        if (c == '[') {
          token.type_ = TOKEN::LIST_OPEN;
        } else if (c == ']') {
          token.type_ = TOKEN::LIST_CLOSE;
        } else if (c == '|') { // handle PRM or PMM operator
          file_.get(c);
          if (c == ':') {
            token.type_ = TOKEN::PRM;
          } else if (c == '+') {
            token.type_ = TOKEN::PMM;
          } else {
            throw std::logic_error("Expected : or +");
          }
          file_.get(c);
          if (c != '|') {
            throw std::logic_error("Expected |");
          }
        } else if (c == '-') {
          token.type_ = TOKEN::SLIENT;
        } else if (isalpha(c)) {
          token.type_ = TOKEN::VARIABLE;
          token.value_ = "";
          token.value_.push_back(c);
          std::streampos prevCharPos = file_.tellg();
          // file_.get(c);
          while (isalpha(c)) {
            prevCharPos = file_.tellg();
            file_.get(c);

            if (file_.eof()) {
              break;
            } else {
              if (isalpha(c)) {
                token.value_.push_back(c);
              } else {
                file_.seekg(prevCharPos);
              }
            }
          }
        }

        return token;
      }

      bool hasMoreTokens() const {
        size_t prevPos = file_.tellg();
        char c;
        file_.get(c);
        if (file_.eof()) {
          return false;
        }
        file_.seekg(prevPos);
        return true;
      }

      void rollBackToken() const {
        if (file_.eof()) {
          file_.clear();
        }
        file_.seekg(prevPos_);
      }
    };

    class Parser {
    private:
        Tokenizer tokenizer_;
          
        value root_;
        //value current_;
        std::vector<value> current_;

        void parseMany() {
          while (tokenizer_.hasMoreTokens()) {
            value node = parseNode();
            current_.push_back(node);
          }
        }

      public:
        Parser(std::istream &s) : tokenizer_{s} {}

        value parse() {
          parseMany();
          top tt;
          
          for (auto &v : current_) {
            tt.push_back(v);
          }
          
          return value{tt};
        }

        value parseNode() { 
            Token token = tokenizer_.getToken();

            // std::cout << token.toString() << "\n";
            if (token.type_ == TOKEN::VARIABLE) {
              return value{token.value_};
            }
            else if (token.type_ == TOKEN::SLIENT) {
              return value{token.toString()};
            }
            else if (token.type_ == TOKEN::LIST_OPEN) {
              return parseList();
            }
            else if (token.type_ == TOKEN::PRM) {
                std::vector<value> left{current_};
                current_.clear();
                parseMany();
                std::vector<value> right{current_};
                current_.clear();

                top l;
                top r;

                for (auto &v: left) {
                  l.push_back(v);
                }

                for (auto &v : right) {
                  r.push_back(v);
                }

                return value{prm{l, r}};
            } 
            else if (token.type_ == TOKEN::PMM) {
              std::vector<value> left{current_};
              current_.clear();
              parseMany();
              std::vector<value> right{current_};
              current_.clear();

              seq l;
              seq r;

              for (auto &v : left) {
                l.push_back(v);
              }

              for (auto &v : right) {
                r.push_back(v);
              }

              return value{pmm{l, r}};
            }

          throw std::logic_error("Unexpected token");
        }

        value parseList() { 
            seq list;
            bool hasCompleted = false;
            while (!hasCompleted) {
                if (tokenizer_.hasMoreTokens()) {
                    Token nextToken = tokenizer_.getToken();  // lookahead
                    if (nextToken.type_ == TOKEN::LIST_CLOSE) {
                      hasCompleted = true;
                    }
                    else {
                      tokenizer_.rollBackToken();
                      value node = parseNode();
                      list.push_back(node);
                    }
                } else {
                  throw std::logic_error("No more tokens");
                }
            }

            return value{list};
        }
    };
}