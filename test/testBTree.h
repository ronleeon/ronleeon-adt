#include <iostream>
#include "ronleeon/tree/b_tree.h"
#include <sstream>
void testBTree() {
	std::stringstream s("123#12");
	auto tree=ronleeon::tree::b_tree<int>::create_tree_l(s);
	tree.level_order(tree.get_root(),std::cout);
}