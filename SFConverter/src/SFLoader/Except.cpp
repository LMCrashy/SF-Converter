#include <string>
#include "Except.h"

Except::Except(const char* what) : str(what)
{}
Except::Except(const std::string &what) : str(what)
{}
Except::~Except() throw()
{}

