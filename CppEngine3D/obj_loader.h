#ifndef LOAD_OBJ_H
#define LOAD_OBJ_H

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <strstream>

#include "engine.h"

#define DELIM 'e'


std::string _strip(std::string s);

std::vector<std::string> _split(std::string s);

std::vector<triangle> load_obj_from_fname(std::string fname);


#endif