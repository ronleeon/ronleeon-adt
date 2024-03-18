#include <iostream>
#include "ronleeon/tree/b_tree.h"
#include "ronleeon/tree/rb_tree.h"
#include <sstream>
void testBTree() {
	std::stringstream s("123#12");
	auto tree=ronleeon::tree::b_tree<int>::create_tree_l(s);
	tree.level_order(tree.get_root(),std::cout);
	auto t2 = ronleeon::tree::rb_tree<int>::create_empty_tree(); // should be ERROR
	//t2.erase(t2.get_root());
}