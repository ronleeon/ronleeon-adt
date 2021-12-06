#include <iostream>
#include "comphsics/tree/b_tree.h"

void testBTree() {
	auto tree=comphsics::tree::b_tree<int>::create_tree_l();
	tree.level_order(tree.get_root(),std::cout);
}