#include <iostream>
#include "ronleeon/tree/B_tree.h"
#include <sstream>

void testKruthBbTree1(){
    auto t = ronleeon::tree::B_tree_Kruth<char,6>::create_empty_tree();
    t.insert('A');
	t.insert('C');
	t.insert('G');
	t.insert('J');
	t.insert('K');
	t.insert('M');
	t.insert('N');
	t.insert('O');
	t.insert('P');
	t.insert('R');
	t.insert('S');
	t.insert('X');
	t.insert('Y');
	t.insert('Z');
	t.insert('T');
	t.insert('U');
	t.insert('V');
	t.insert('D');
	t.insert('E');
	t.insert('B');
	t.insert('Q');
	t.insert('L');
	t.insert('F');
	//t.pre_order(t.get_root(), std::cout);
	t.erase('F');
	t.erase('M');
	t.erase('G',true,false, false);
	t.erase('D',true,false, false);
	t.erase('Z');
	t.erase('X');
	t.erase('Y');
	t.erase('V');
	t.pre_order(t.get_root(), std::cout);
	std::cout<<t.min()<<","<<t.max()<<'\n';
}
void testKruthBbTree2(){
	auto t = ronleeon::tree::B_tree_Kruth<char,6>::create_empty_tree();
    t.insert('A');
	t.insert('C');
	t.insert('G');
	t.insert('J');
	t.insert('K');
	t.insert('M');
	t.pre_order(t.get_root(), std::cout);
	std::cout<<t.min()<<","<<t.max()<<'\n';
	t.erase('A');
	t.erase('C');
	t.erase('G');
	t.erase('J');
	t.erase('K');
	t.pre_order(t.get_root(), std::cout);
	std::cout<<"height:"<<t.get_height()<<'\n';

}

void testCormenBbTree1(){
	auto t = ronleeon::tree::B_tree_Cormen<char,3>::create_empty_tree();
    t.insert('A');
	t.insert('C');
	t.insert('G');
	t.insert('J');
	t.insert('K');
	t.insert('M');
	t.insert('N');
	t.insert('O');
	t.insert('P');
	t.insert('R');
	t.insert('S');
	t.insert('X');
	t.insert('Y');
	t.insert('Z');
	t.insert('T');
	t.insert('U');
	t.insert('V');
	t.insert('D');
	t.insert('E');
	t.insert('B');
	t.insert('Q');
	t.insert('L');
	t.insert('F');
	t.erase('F');
	t.erase('M');
	t.erase('G',true,false, false);
	t.erase('D',true,false, false);
	t.erase('V');
	t.pre_order(t.get_root(), std::cout);
	std::cout<<t.min()<<","<<t.max()<<'\n';
	std::cout<<"height:"<<t.get_height()<<'\n';
}

void testCormenBbTree2(){
	auto t = ronleeon::tree::B_tree_Cormen<char,3>::create_empty_tree();
    t.insert('A');
	t.insert('C');
	t.insert('G');
	t.insert('J');
	t.insert('K');
	t.insert('M');
	t.pre_order(t.get_root(), std::cout);
	std::cout<<t.min()<<","<<t.max()<<'\n';
	t.erase('A');
	t.erase('C');
	t.erase('G');
	t.erase('J');
	t.erase('K');
	t.pre_order(t.get_root(), std::cout);

}


void testBbTree() {
	testKruthBbTree2();
	testCormenBbTree1();
}
