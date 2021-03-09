#ifndef LOAD_OBJ_H
#define LOAD_OBJ_H

#include "engine.h"

#define DELIM 'e'

std::vector<std::string> _split(const std::string s);

std::vector<triangle> load_obj_mtl_fname(std::string obj, std::string mtl);


#endif