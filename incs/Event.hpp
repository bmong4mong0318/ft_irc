#pragma once

#include <iostream>
#include <map>
#include <utility>
#include <sys/_types/_uintptr_t.h>

typedef std::map<uintptr_t, std::string> event_map;
typedef std::pair<uintptr_t, std::string> event_pair;
typedef std::map<uintptr_t, std::string>::iterator event_map_iter;
