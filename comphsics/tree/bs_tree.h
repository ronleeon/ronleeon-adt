#ifndef COMPHSICS_ADT_BS_TREE_H
#define COMPHSICS_ADT_BS_TREE_H

#include "comphsics/tree/b_tree.h"
#include <istream>
namespace comphsics{
	namespace tree{

		// Guarantee all NodeType data are not equal,otherwise the latter 
		// will be ignored.

		// C++ style compare,not java compare style.
		template<typename DataType,typename Compare=std::less<DataType>,typename NodeType=node::bs_node<DataType>>
		class bs_tree:public b_tree<DataType,NodeType>{
			using basic_type=b_tree<DataType,NodeType>;
			// prohibit all create functions.
			using basic_type::create_tree_l;
			using basic_type::create_tree_r;
		public:
			using node_type = NodeType;
			using node_pointer = NodeType*; 
			using node_iterator = NodeType*; 
			using node_type_reference = NodeType&; 
			using const_node_type = const NodeType; 
			using const_node_pointer = const NodeType*; 
			using const_node_iterator = const NodeType*; 
			using const_node_type_reference = const NodeType&; 
		public:
			static node_pointer left_most(node_pointer node){
				if(!node){
					return nullptr;
				}
				node_pointer tmp=node;
				while(tmp->left_child){
					tmp=tmp->left_child;
				}
				return tmp;
			}
			static node_pointer right_most(node_pointer node){
				if(!node){
					return nullptr;
				}
				node_pointer tmp=node;
				while(tmp->right_child){
					tmp=tmp->right_child;
				}
				return tmp;
			}
			// Get the next node in in-order.
			static node_pointer increment(node_pointer node)  {
				if(!node){
					return nullptr;
				}
				node_pointer tmp=node->right_child;
				if(tmp) {
					return left_most(tmp);
				}else{
					node_pointer parent=node->parent;
					node_pointer cur=node;
					while(parent!=nullptr&&cur==parent->right_child){
						cur=parent;
						parent=cur->parent;
					}
					return parent;
				}
			}

			// Get the pre node in in-order.
			static node_pointer decrement(node_pointer node){
				if(!node){
					return nullptr;
				}
				node_pointer tmp=node->left_child;
				if(tmp) {
					return right_most(tmp);
				}else{
					node_pointer parent=node->parent;
					node_pointer cur=node;
					while(parent!=nullptr&&cur==parent->left_child){
						cur=parent;
						parent=cur->parent;
					}
					return parent;
				}
			}
		protected:
			explicit bs_tree(node_pointer t)
				:bs_tree<DataType,Compare,NodeType>(t){}
			explicit bs_tree(std::nullptr_t):b_tree<DataType,NodeType>(nullptr){}
			Compare comp;
		public:
			bs_tree():bs_tree(nullptr){};
			bs_tree(const bs_tree&)=delete;
			bs_tree(const DataType data[],size_t Size):b_tree<DataType,NodeType>(nullptr){
				for(size_t Index=0;Index<Size;++Index){	
					insert(data[Index]);
				}
			}
			bs_tree(bs_tree && tree):b_tree<DataType,NodeType>((b_tree<DataType,NodeType>&&)tree) {}
			// return a shared tree holding the same nodes.
			[[nodiscard("Unused Tree")]] bs_tree make_shared() {
				bs_tree<DataType,Compare,NodeType> Ret(basic_type::_root);
				Ret._owned=false;
				Ret.num_of_nodes=basic_type::num_of_nodes;
				return Ret;
			}
			std::string to_string()const override {
				return "<-Binary Sort Tree->";
			}
			// NOTICE: EMPTY_NODE_INDICATOR is used to denote the end of input.
			static bs_tree create_binary_sort_tree(std::istream &in=std::cin) {
				bs_tree<DataType,Compare,NodeType> tree(nullptr); 
				char Indicator;
				DataType Data;
				// !!! allowed to read a char of indicator.
				while(true){
					root_input:Indicator=in.peek();
					if(Indicator=='\n'){
						in.get();
						goto root_input;
					}else if(Indicator==' '||Indicator=='\t'){
						in.get();
						continue;
					}else if(Indicator==EMPTY_NODE_INDICATOR){
						break;
					}else {
						// take back to stream.
						in>> Data;
						tree.insert(Data);
					}
				}
				return tree;
			}

			// return the result ,
			// if bool is true, then the first is the result node.
			// if bool is false, then the first is the node.(also may be null,empty tree)
			// whose child(left or right) is the inserted position. 
			std::pair<node_pointer,bool> find(const DataType& data)const{
				if(basic_type::is_empty()){
					return std::make_pair(nullptr,false);
				}
				node_pointer start=basic_type::get_root();
				while(true){
					if(comp(data,start->data)) {
						if (start->left_child) {
							start=start->left_child;
						} else {
							return std::make_pair(start,false);
						}
					}else if(comp(start->data,data)) {
						if(start->right_child){
							start=start->right_child;
						}else {
							return std::make_pair(start,false);
						}
					}else{
						return std::make_pair(start,true);
					}
				}
			}

			std::pair<node_pointer,bool> find(const DataType&& data)const{
				return find(data);
			}
			
			// insert the data if find it, ignored!
			// the second returns whether insert operation is successful.
			virtual std::pair<node_pointer,bool> insert(const DataType& data){
				auto find_result=find(data);
				if(find_result.second){
					find_result.second=false;
					return find_result;
				}
				++basic_type::num_of_nodes;
				if(!find_result.first){
					// root.
					basic_type::_root=new NodeType(data);
					find_result.first=basic_type::_root;
					find_result.second=true;
					return find_result;
				}
				auto& node=find_result.first;
				// not equal
				if(comp(data,node->data)){
					assert(!node->left_child);
					node->left_child=new NodeType(data);
					find_result.first=node;
					find_result.second=true;
					node->is_leaf=false;
					node->left_child->parent=node;
					++node->child_size;
				}else{
					assert(!node->right_child);
					node->right_child=new NodeType(data);
					find_result.first=node;
					find_result.second=true;
					node->is_leaf=false;
					node->right_child->parent=node;
					++node->child_size;
				}
				basic_type::shift_height(node);
				return find_result;
			}

			std::pair<node_pointer,bool> insert(DataType&& data){
				return insert(data);
			}

			
			// erase has two replacement.
			// returns the next node.
			// when erased node has two children,
			// we can replace it with node of its left tree.
			//  or  node of its right tree.

			virtual node_pointer erase(node_pointer node,bool left=true){
				if(!node){
					return nullptr;
				}
				--basic_type::num_of_nodes;
				node_pointer Ret=increment(node);
				// get the next node.
				if(node->left_child&&node->right_child){
					if(left){
						auto left=left_most(node->left_child);
						// cannot be empty.
						node->data=left->data;
						// now left do not have right_child.
						node=left;
					}else{
						auto right=right_most(node->right_child);
						// cannot be empty.
						node->data=right->data;
						// now right do not have right_child.
						node=right;
					}
				}
				// now node cannot have two childs.
				node_pointer parent=node->parent;
				node_pointer* point_to_node;
				if(!parent){
					// root.
					point_to_node=nullptr;
				}else if(node==parent->left_child){
					point_to_node=(node_pointer*)&(parent->left_child);
				}else{
					point_to_node=(node_pointer*)&((parent->right_child));
				}
				if(node->is_leaf){
					// delete directly.
					if(point_to_node){
						*point_to_node=nullptr;
						--parent->child_size;
						if(parent->child_size==0){
							parent->is_leaf=true;
						}
						node->parent=nullptr;
						basic_type::shift_height(node->parent);
					}else{
						basic_type::_root=nullptr;
					}
					node->parent=nullptr;
					delete node;
				}else if(node->left_child){
					if(point_to_node){
						node->left_child->parent=parent;
						*point_to_node=node->left_child;
						--parent->child_size;
						if(parent->child_size==0){
							parent->is_leaf=true;
						}
						node->parent=nullptr;
						basic_type::shift_height(node->parent);
					}else{
						basic_type::_root=node->left_child;
						node->left_child->parent=nullptr;
					}
					node->parent=nullptr;
					node->left_child=nullptr;
					delete node;
				}else if(node->right_child){
					if(point_to_node){
						node->right_child->parent=parent;
						*point_to_node=node->right_child;
						--parent->child_size;
						if(parent->child_size==0){
							parent->is_leaf=true;
						}
						node->parent=nullptr;
						basic_type::shift_height(node->parent);
					}else{
						basic_type::_root=node->right_child;
						node->right_child->parent=nullptr;
					}
					node->parent=nullptr;
					node->right_child=nullptr;
					delete node;
				}
				return Ret;
			}


			void erase(const DataType& data,bool left=true){
				auto find_result=find(data);
				if(!find_result.second){
					return;
				}
				erase(find_result.first,left);
			}

			void erase(DataType&& data,bool left=true){
				erase(data,left);
			}

			// find the left and the right data. 
			DataType left()const{
				const auto find_result=left_most(basic_type::_root);
				assert(find_result&&"Empty tree!");
				return find_result->data;
			}
			DataType right()const{
				const auto find_result=right_most(basic_type::_root);
				assert(find_result&&"Empty tree!");
				return find_result->data;
			}

			static bool is_binary_search_tree(node::b_node<DataType>* root){
				Compare comp;
				if(!root){
					return true;
				}
				if(root->left_child&&root->right_child){
					if(comp(root->left_child->data,root->data)&&comp(root->data,root->right_child->data)){
						return is_binary_search_tree(root->left_child)&&is_binary_search_tree(root->right_child);
					}
				}else if(root->left_child){
					if(comp(root->left_child->data,root->data)){
						return is_binary_search_tree(root->left_child);
					}
				}else if(root->right_child){
					if(comp(root->data,root->right_child->data)){
						return is_binary_search_tree(root->right_child);
					}
				}else{
					return true;
				}
				return false;
			}

			[[nodiscard("Unused Tree")]] static bs_tree create_empty_tree() {
				bs_tree Ret(nullptr);
				return Ret;
			}

			void virtual print_visiting_node(const_node_pointer t,std::ostream& out)const override{
				basic_type::print_visiting_node(t,out);
			}
		};

	}
}
#endif