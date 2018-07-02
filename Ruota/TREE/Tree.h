#ifndef TREE_H
#define TREE_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <sstream>
#include "../Ruota.h"

SP_MEMORY mparse(SP_MEMORY parent, boost::property_tree::ptree pt);
std::vector<SP_MEMORY> __json_parse(std::vector<SP_MEMORY> args);

#endif