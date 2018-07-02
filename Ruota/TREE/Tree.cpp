#include "Tree.h"
/*
std::string indent(int level) {
  std::string s; 
  for (int i=0; i<level; i++) s += "  ";
  return s; 
} 

void printTree (boost::property_tree::ptree &pt, int level) {
  if (pt.empty()) {
    std::cout << "\""<< pt.data()<< "\"";
  }

  else {
    if (level) std::cout << std::endl; 

    std::cout << indent(level) << "{" << std::endl;     

    for (boost::property_tree::ptree::iterator pos = pt.begin(); pos != pt.end();) {
      std::cout << indent(level+1) << "\"" << pos->first << "\": "; 

      printTree(pos->second, level + 1); 
      ++pos; 
      if (pos != pt.end()) {
        std::cout << ","; 
      }
      std::cout << std::endl;
    } 

   std::cout << indent(level) << " }";     
  }

  return; 
}
*/

SP_MEMORY mparse(SP_MEMORY parent, boost::property_tree::ptree pt) {
	VEC_Memory tree_base;

	for (auto &pos : pt) {
		VEC_Memory m;
		m.push_back(NEW_MEMORY(std::string(pos.first)));
		if (pos.second.empty()){
			m.push_back(NEW_MEMORY(pos.second.data()));
		} else {
			SP_MEMORY tree = NEW_MEMORY();
			tree->setType(ARR);
			m.push_back(mparse(tree, pos.second));
		}
		tree_base.push_back(NEW_MEMORY(m));
    }

	return NEW_MEMORY(tree_base);
}

std::vector<SP_MEMORY> __json_parse(std::vector<SP_MEMORY> args) {
	std::string json = args[0]->toString();
	std::stringstream ss;
	ss << json;
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(ss, pt);

	SP_MEMORY p = NEW_MEMORY();
	p->setType(ARR);
	VEC_Memory v;
	v.push_back(mparse(p, pt));

	return v;
}