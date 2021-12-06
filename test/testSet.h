#include <iostream>
#include "comphsics/tree/tree_set.h"

void testSet() {
	comphsics::tree::tree_set<std::string> set{
		"sd","sd","sss"
	};
	auto It=set.iter();
	while(It){
		std::cout<<*It<<'\n';
		++It;
	}
	set.clear();
	std::cout<<set.size()<<'\n';
}