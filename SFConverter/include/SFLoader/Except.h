#pragma once

struct Except : public std::exception
{
   std::string str;
   Except(const char* what);
   Except(const std::string &what);
   ~Except() throw();
   const char* what() const throw() { return str.c_str(); }
};
