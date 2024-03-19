#include <iostream>
#include "ronleeon/tree/avl_tree.h"
#include "ronleeon/tree/bs_tree.h"
#include "ronleeon/tree/tree_map.h"
#include "ronleeon/tree/tree_set.h"

void testSet() {
	ronleeon::tree::tree_set<std::string, std::less<std::string>,ronleeon::tree::avl_tree<std::string>> set{
		"sss","sd","sd","asss"
	};
	for(auto It = set.begin();It != set.end(); ++It){
		std::cout<<*It<<'\n';
	}
	for(auto It = set.rbegin();It != set.rend(); ++It){
		std::cout<<*It<<'\n';
	}
	//set.clear();
	ronleeon::tree::tree_map<int,int,ronleeon::tree::less<ronleeon::tree::pair<int,int>>,ronleeon::tree::rb_tree<ronleeon::tree::pair<int,int>
	,ronleeon::tree::less<ronleeon::tree::pair<int,int>>>> m;
	m.insert(1,2);
	m.insert(2,3);
	m.insert_or_assign(1,3);
	std::cout<<m.size()<<'\n';
	std::cout<<(m.begin()==m.end())<<"\n";
	for(auto It = m.begin();It != m.end(); ++It){
		std::cout<<It->first<<","<<It->second<<'\n';
	}
	for(auto It = m.rbegin();It != m.rend(); ++It){
		std::cout<<It->first<<","<<It->second<<'\n';
	}
	std::cout<<*(++set.rend())<<'\n';
	std::cout<<(--m.end())->first<<","<<(--m.end())->second<<'\n';
}