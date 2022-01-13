#ifndef COMPHSICS_ADT_RB_TREE_H
#define COMPHSICS_ADT_RB_TREE_H

#include "comphsics/tree/bs_tree.h"
#include <istream>
namespace comphsics{
	namespace tree{


		// Guarantee all NodeType data are not equal,otherwise the latter 
		// will be ignored.
		template<typename DataType,typename Compare=std::less<DataType>,typename NodeType=node::rb_node<DataType>>
		class rb_tree:public bs_tree<DataType,Compare,NodeType>{
			using basic_type=bs_tree<DataType,Compare,NodeType>;
			// prohibit all create functions.
			using basic_type::create_binary_sort_tree;
			using basic_type::shift_height;
	
	
			TREE_TRAITS(NodeType)
		private:
			explicit rb_tree(node_pointer t)
				:bs_tree<DataType,Compare,NodeType>(t){}
			explicit rb_tree(std::nullptr_t):bs_tree<DataType,Compare,NodeType>(nullptr){}


			void shift_rb_node(node_pointer node){
				while(node){
					size_t left_height=0,right_height=0;
					if(node->left_child&&node->right_child){
						left_height=node->left_child->height;
						right_height=node->right_child->height;
						node->height=std::max(left_height,right_height)+1;
						node->is_leaf=false;
						node->child_size=2;
					}else if(node->left_child){
						node->height=node->left_child->height+1;
						node->is_leaf=false;
						node->child_size=1;
					}else if(node->right_child){
						node->height=node->right_child->height+1;
						node->is_leaf=false;
						node->child_size=1;
					}else{
						node->is_leaf=true;
						node->height=0;
						node->child_size=0;
					}
					node=node->parent;
				}

			}

			
			void fix_after_insert(node_pointer node){
				if(node==basic_type::_root){
					node->color=NodeType::COLOR::BLACK;
					return;
				}
				// node is not the root.
				node_pointer parent=node->parent;
				if(parent->color==NodeType::COLOR::BLACK){
					// node is red and its parent is black, not violate
					// any rules of rb tree.
					return;
				}
				//now its parent is red, that means it must have a grandparent.
				node_pointer gparent=parent->parent;
				node_pointer uncle=nullptr;
				if(parent==gparent->right_child){
					uncle=gparent->left_child;
					if(node==parent->left_child){
						right_rotation(parent);
						// update indices.
						parent=node;
						node=parent->right_child;
					}
					if(uncle&&uncle->color==NodeType::COLOR::RED){
						parent->color=NodeType::COLOR::BLACK;
						gparent->color=NodeType::COLOR::RED;
						uncle->color=NodeType::COLOR::BLACK;
						// this situation we need recursively re-modify because now gparent
						// is red which may be conflict with its parent.
						fix_after_insert(gparent);
					}else{
						left_rotation(gparent);
						gparent->color=NodeType::COLOR::RED;
						parent->color=NodeType::COLOR::BLACK;
					}
				}else{
					uncle=gparent->right_child;
					if(node==parent->right_child){
						left_rotation(parent);
						// update indices.
						parent=node;
						node=parent->left_child;
					}
					if(uncle&&uncle->color==NodeType::COLOR::RED){
						parent->color=NodeType::COLOR::BLACK;
						gparent->color=NodeType::COLOR::RED;
						uncle->color=NodeType::COLOR::BLACK;
						// this situation we need recursively re-modify because now gparent
						// is red which may be conflict with its parent.
						fix_after_insert(gparent);
					}else{
						right_rotation(gparent);
						gparent->color=NodeType::COLOR::RED;
						parent->color=NodeType::COLOR::BLACK;
					}
				}

			}

			// property: every path within current node may have one less black.
			void fix_after_erase(node_pointer node){
				assert(node->is_leaf);
				// There are much complex cases.
				if(node->color==NodeType::COLOR::RED){
					// red leaf ,can safely delete ok!
					return;
				}
				node_pointer P;//parent
				node_pointer S;// sibling
				node_pointer C;// close newphew
				node_pointer D;// distant newphew
				while((P=node->parent)!=nullptr){
					// because node is black, so its sibling must be not null.
					if(node==P->left_child){
						S=P->right_child;
						if(S->color==NodeType::COLOR::RED){
							left_rotation(P);
							P->color=NodeType::COLOR::RED;
							S->color=NodeType::COLOR::BLACK;
							S=C;
						}
						D=S->right_child;
						if(D&&D->color==NodeType::COLOR::RED){
							left_rotation(P);
							S->color=P->color;
							P->color=NodeType::COLOR::BLACK;
							D->color=NodeType::COLOR::BLACK;
							return;//complete
						}
						C=S->left_child;
						if(C&&C->color==NodeType::COLOR::RED){
							right_rotation(S);
							S->color=NodeType::COLOR::RED;
							C->color=NodeType::COLOR::BLACK;
							D=S;
							S=C;
						}
						if(P->color==NodeType::COLOR::RED){
							S->color=NodeType::COLOR::RED;
							P->color=NodeType::COLOR::BLACK;
							return;// complete
						}
						// all black.
						S->color=NodeType::COLOR::RED;
						node=P;
					}else{
						// symmetric
						S=P->left_child;
						if(S->color==NodeType::COLOR::RED){
							right_rotation(P);
							P->color=NodeType::COLOR::RED;
							S->color=NodeType::COLOR::BLACK;
							S=C;
						}
						D=S->left_child;
						if(D&&D->color==NodeType::COLOR::RED){
							left_rotation(P);
							S->color=P->color;
							P->color=NodeType::COLOR::BLACK;
							D->color=NodeType::COLOR::BLACK;
							return;//complete
						}
						C=S->right_child;
						if(C&&C->color==NodeType::COLOR::RED){
							left_rotation(S);
							S->color=NodeType::COLOR::RED;
							C->color=NodeType::COLOR::BLACK;
							D=S;
							S=C;
						}
						if(P->color==NodeType::COLOR::RED){
							S->color=NodeType::COLOR::RED;
							P->color=NodeType::COLOR::BLACK;
							return;// complete
						}
						// all black.
						S->color=NodeType::COLOR::RED;
						node=P;
					}
				
					node->color=NodeType::COLOR::BLACK;// node may be escaped but still be black.
				}
			}


			void left_rotation(node_pointer node){
				node_pointer right=node->right_child;
				node_pointer left=(right)->left_child;
				node_pointer parent=node->parent;
				node_pointer* point_to_node=nullptr;
				if(parent){
					if(node==(parent)->left_child){
						point_to_node=(node_pointer*)&(parent->left_child);
					}else{
						point_to_node=(node_pointer*)&(parent->right_child);
					}
				}
				if(point_to_node){
					*point_to_node=right;
					right->parent=parent;
				}else{
					// node is the root.
					basic_type::_root=right;
					right->parent=nullptr;
				}
				node->right_child=left;
				if(left){
					left->parent=node;
				}
				right->left_child=node;
				node->parent=right;
				// others.
				shift_rb_node(node);
				shift_rb_node(right);

			}
			void right_rotation(node_pointer node){
				node_pointer left=node->left_child;
				node_pointer right=left->right_child;
				node_pointer parent=node->parent;
				node_pointer* point_to_node=nullptr;
				if(parent){
					if(node==parent->left_child){
						point_to_node=(node_pointer*)&(parent->left_child);
					}else{
						point_to_node=(node_pointer*)&(parent->right_child);
					}
				}
				if(point_to_node){
					*point_to_node=left;
					left->parent=parent;
				}else{
					// node is the root.
					basic_type::_root=left;
					left->parent=nullptr;
				}
				node->left_child=right;
				if(right){
					right->parent=node;
				}
				left->right_child=node;
				node->parent=left;
				// others.
				shift_rb_node(node);
				shift_rb_node(left);

			}

		public:
			rb_tree():rb_tree(nullptr){};
			rb_tree(const rb_tree&)=delete;
			rb_tree(const DataType data[],size_t Size):bs_tree<DataType,Compare,NodeType>(nullptr){
				for(size_t Index=0;Index<Size;++Index){	
					insert(data[Index]);
				}
			}
			rb_tree(rb_tree && tree):bs_tree<DataType,Compare,NodeType>((bs_tree<DataType,Compare,NodeType>&&)tree) {}
			// return a shared tree holding the same nodes.
			[[nodiscard("Unused Tree")]] rb_tree make_shared() {
				rb_tree<DataType,Compare,NodeType> Ret(basic_type::_root);
				Ret._owned=false;
				Ret.num_of_nodes=basic_type::num_of_nodes;
				return Ret;
			}
			std::string to_string()const override {
				return "<-RB(Red,Black) Tree->";
			}
			// NOTICE: EMPTY_NODE_INDICATOR is used to denote the end of input.
			static rb_tree create_rb_tree(std::istream &in=std::cin) {
				rb_tree<DataType,Compare,NodeType> tree(nullptr); 
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


			// insert the data if find it, ignored!
			// the second returns whether insert operation is successful.
			std::pair<node_pointer,bool> insert(const DataType& data){
				std::pair<node_pointer,bool> find_result=basic_type::find(data);
				node_pointer node=find_result.first;
				if(find_result.second){
					find_result.second=false;
					return find_result;
				}
				++basic_type::num_of_nodes;
				if(!find_result.first){
					// root.
					basic_type::_root=new NodeType(data);
					find_result.first=basic_type::_root;
					node=basic_type::_root;
					find_result.second=true;
				}else{
					node=find_result.first;
					node_pointer child=new NodeType(data);
					if(basic_type::comp(data,node->data)){
						assert(!node->left_child);
						node->left_child=child;
						find_result.first=node;
						find_result.second=true;
						node->is_leaf=false;
						child->parent=node;
						++node->child_size;
					}else{
						assert(!node->right_child);
						node->right_child=child;
						find_result.first=node;
						find_result.second=true;
						node->is_leaf=false;
						child->parent=node;
						++node->child_size;
					}
					node=child;
				}
				shift_rb_node(node);
				fix_after_insert(node);
				return find_result;
			}

			std::pair<node_pointer,bool> insert(DataType&& data){
				return insert(data);
			}

			node_pointer erase(node_pointer node,bool left=true) override {
				if(!node){
					return nullptr;
				}
				--basic_type::num_of_nodes;
				node_pointer Ret=basic_type::increment(node);
				if(node->left_child&&node->right_child){
					if(left){
						auto left=basic_type::left_most(node->left_child);
						// cannot be empty.
						node->data=left->data;
						// now left do not have right_child.
						node=left;
					}else{
						auto right=basic_type::right_most(node->right_child);
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
					point_to_node=(node_pointer*)&((parent->left_child));
				}else{
					point_to_node=(node_pointer*)&((parent->right_child));
				}
				if(node->is_leaf){
					// delete directly.
					if(point_to_node){
						fix_after_erase(node);
						*point_to_node=nullptr;
						--parent->child_size;
						if(parent->child_size==0){
							parent->is_leaf=true;
						}
					}else{
						basic_type::_root=nullptr;
					}
					node->parent=nullptr;
					shift_rb_node(parent);
					delete node;
				}else if(node->left_child){
					// if node has left child, then its left child color must be red.
					// so node color must be black.
					if(point_to_node){
						node->left_child->parent=parent;
						*point_to_node=node->left_child;
						// color its child to black.
						(node->left_child)->color=NodeType::COLOR::BLACK;
					}else{
						basic_type::_root=node->left_child;
						node->left_child->parent=nullptr;
						// root must be black.
						(node->left_child)->color=NodeType::COLOR::BLACK;
					}
					node->parent=nullptr;
					node->left_child=nullptr;
					shift_rb_node(parent);
					delete node;
				}else if(node->right_child){
					// if node has right child, then its right child color must be red.
					// so node color must be black.
					if(point_to_node){
						node->right_child->parent=parent;
						*point_to_node=node->right_child;
						// color its child to black.
						(node->right_child)->color=NodeType::COLOR::BLACK;
					}else{
						basic_type::_root=node->right_child;
						node->right_child->parent=nullptr;
						// root must be black.
						(node->right_child)->color=NodeType::COLOR::BLACK;
					}
					node->parent=nullptr;
					node->right_child=nullptr;
					shift_rb_node(parent);
					delete node;
				}
				return Ret;
			}

			void erase(const DataType& data,bool left=true){
				auto find_result=basic_type::find(data);
				if(!find_result.second){
					return;
				}
				erase(find_result.first,left);
			}

			void erase(DataType&& data,bool left=true){
				erase(data,left);
			}

			// NOTE: null node is also black.
			bool is_black(const_node_pointer node) const {
				if(!node||node->color==NodeType::COLOR::BLACK){
					return true;
				}
				return false;
			}
			bool is_red(const_node_pointer node) const {
				return !is_black(node);
			}

			[[nodiscard("Unused Tree")]] static rb_tree create_empty_tree() {
				rb_tree Ret(nullptr);
				return Ret;
			}

			void print_visiting_node(const_node_pointer t,std::ostream& out)const override{
				basic_type::print_visiting_node(t,out);
				if(t){
					if(t->color==NodeType::COLOR::RED){
						out<<" , color:red";
					}else{
						out<<" , color:black";
					}
				}
			}
		};

	}
}
#endif