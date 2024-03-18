
#ifndef RONLEEON_ADT_AVL_TREE_H
#define RONLEEON_ADT_AVL_TREE_H

#include "ronleeon/tree/abstract_tree.h"
#include <istream>
namespace ronleeon::tree{

	template<typename NodeType>
	class avl_node_print_trait{
	public:
		static void print_visiting_node(const NodeType* t, std::ostream& os){
			os<< "visiting node:[";
			if (!t) {
				os << "null]";
				return;
			}
			else {
				os << t->data;
			}
			os << "]  ";
			os<<"[Height:"<<t->height<<"] ";
			if (t->is_leaf) {
				os << "Leaf";
			}
			else {
				os << "Node";
			}
			if(t->left_child&&!t->right_child){
				os<<" has left child";
			}else if(t->left_child&&t->right_child){
				os<<" has left and right child";
			}else if(!t->left_child&&t->right_child){
				os<<" has right child";
			}
			os<<" ,balance factor:"<<t->balance_factor;
		}
	};
	// NOTICE:DataType must be comparable.
	// DataType must support '>','<','=','>=','<='traits.
	// specified type can override these operators.


	// Guarantee all NodeType data are not equal,otherwise the latter
	// will be ignored.
	template<typename DataType,typename Compare=std::less<DataType>,typename NodeType=node::avl_node<DataType>,typename NodePrintTrait = avl_node_print_trait<NodeType>>
	class avl_tree:public abstract_bs_tree<DataType,Compare,NodeType
		,avl_tree<DataType,Compare,NodeType,NodePrintTrait>, NodePrintTrait>{
		using basic_type=abstract_bs_tree<DataType,Compare,NodeType
			,avl_tree<DataType,Compare,NodeType,NodePrintTrait>, NodePrintTrait>;
		// prohibit all create functions.
		using basic_type::shift_height;

	public:
		using node_type = NodeType;
		using node_pointer = NodeType*;
		using node_type_reference = NodeType&;
		using const_node_type = const NodeType;
		using const_node_pointer = const NodeType*;
		using const_node_type_reference = const NodeType&;

		using PrintTrait = typename basic_type::PrintTrait;
	private:

		explicit avl_tree(std::nullptr_t,Compare comp_ = Compare{} ):basic_type(nullptr, comp_){}

		void shift_avl_node(node_pointer node){
			while(node){
				size_t left_height=0,right_height=0;
				if(node->left_child&&node->right_child){
					left_height=node->left_child->height;
					right_height=node->right_child->height;
					node->height=std::max(left_height,right_height)+1;
					node->balance_factor=left_height-right_height;
					node->is_leaf=false;
					node->child_size=2;
				}else if(node->left_child){
					node->height=node->left_child->height+1;
					node->balance_factor=node->height;
					node->is_leaf=false;
					node->child_size=1;
				}else if(node->right_child){
					node->height=node->right_child->height+1;
					node->balance_factor=-node->height;
					node->is_leaf=false;
					node->child_size=1;
				}else{
					node->is_leaf=true;
					node->height=0;
					node->child_size=0;
					node->balance_factor=0;
				}
				node=node->parent;
			}

		}
		// four rotate operations.


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
			shift_avl_node(node);
			shift_avl_node(right);
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
			shift_avl_node(node);
			shift_avl_node(left);

		}
		void left_right_rotation(node_pointer node){
			left_rotation(node->left_child);
			right_rotation(node);
		}
		void right_left_rotation(node_pointer node){
			right_rotation(node->right_child);
			left_rotation(node);
		}

		void rebalance(node_pointer node){
			while(node){
				if(node->balance_factor< -1){
					if((node->right_child)->balance_factor<0){
						left_rotation(node);
					}else if((node->right_child)->balance_factor>0){
						right_left_rotation(node);
					}
				}else if(node->balance_factor>1){
					if((node->left_child)->balance_factor>0){
						right_rotation(node);
					}else if((node->left_child)->balance_factor<0 ){
						left_right_rotation(node);
					}
				}
				node=node->parent;
			}
		}
	public:
		avl_tree(Compare comp_ = Compare{} ):avl_tree(nullptr, comp_){};
		avl_tree(const avl_tree&)=delete;
		avl_tree(const DataType data[],size_t Size,Compare comp_ = Compare{} ):basic_type(data,Size,comp_){
		}
		avl_tree(avl_tree && tree):basic_type(std::move(tree)) {}

		std::string to_string()const override {
			return "<-AVL(Adelson,Velsky,Landis) Tree->";
		}

		static bool is_balanced(const_node_pointer root){
			if(!root){
				return true;
			}
			if(root->balance_factor>-2&&root->balance_factor<2){
				return is_balanced(root->left_child)&&is_balanced(root->right_child);
			}else{
				return false;
			}
		}

		// insert the data if find it, ignored!
		// the second returns whether insert operation is successful.
		std::pair<const_node_pointer,bool> insert(const DataType& data) override {
			auto find_result=basic_type::find(data);
			if(find_result.second){
				find_result.second=false;
				return find_result;
			}
			++basic_type::num_of_nodes;
			if(!find_result.first){
				// root.
				basic_type::_root=new NodeType();
				basic_type::_root->data = data;
				find_result.first=basic_type::_root;
				find_result.second=true;
				basic_type::min_node = basic_type::max_node = basic_type::_root;
				return find_result;
			}
			auto node=const_cast<node_pointer>(find_result.first);
			if(basic_type::comp(data,node->data)){
				assert(!node->left_child);
				node->left_child=new NodeType();
				node->left_child->data = data;
				find_result.first=node->left_child;
				find_result.second=true;
				node->is_leaf=false;
				node->left_child->parent=node;
				++node->child_size;
				node = node->left_child;
			}else{
				assert(!node->right_child);
				node->right_child=new NodeType();
				node->right_child->data = data;
				find_result.first=node->right_child;
				find_result.second=true;
				node->is_leaf=false;
				node->right_child->parent=node;
				++node->child_size;
				node = node->right_child;
			}
			// update min_node and max_node.
			if(basic_type::min_node&& (basic_type::min_node->left_child == node)){
				basic_type::min_node = node;
			}
			if(basic_type::max_node&&(basic_type::max_node->right_child == node)){
				basic_type::max_node = node;
			}
			shift_avl_node(node);
			rebalance(node);
			return find_result;
		}

		const_node_pointer erase(const_node_pointer node, bool left = true){
            return erase(const_cast<node_pointer>(node), left);
        }

		const_node_pointer erase(node_pointer node,bool left=true) override {
			if(!node){
				return nullptr;
			}
			--basic_type::num_of_nodes;
			const_node_pointer Ret=basic_type::increment(node);
			if(node->left_child&&node->right_child){
				if(left){
					auto left=basic_type::left_most(node->left_child);
					// cannot be empty.
					node->data=left->data;
					// now left do not have right_child.
					node=const_cast<node_pointer>(left);
				}else{
					auto right=basic_type::right_most(node->right_child);
					// cannot be empty.
					node->data=right->data;
					// now right do not have right_child.
					node=const_cast<node_pointer>(right);
				}
			}
			// now node can not have two childs.
			node_pointer parent=node->parent;
			node_pointer* point_to_node;
			// newParent records the new min node when node is min_node
            // records the new max node when node is max_node
            const_node_pointer newParent = nullptr;
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
					*point_to_node=nullptr;
					--parent->child_size;
					if(parent->child_size==0){
						parent->is_leaf=true;
					}
					shift_avl_node(parent);
					rebalance(parent);
				}else{
					basic_type::_root=nullptr;
				}
				newParent = node->parent;
				node->parent=nullptr;
			}else if(node->left_child){
				if(point_to_node){
					node->left_child->parent=parent;
					*point_to_node=node->left_child;
					shift_avl_node(parent);
					rebalance(parent);
				}else{
					basic_type::_root=node->left_child;
					node->left_child->parent=nullptr;
				}
				newParent = node->left_child;
				node->parent=nullptr;
				node->left_child=nullptr;
			}else if(node->right_child){
				if(point_to_node){
					node->right_child->parent=parent;
					*point_to_node=node->right_child;
					shift_avl_node(parent);
					rebalance(parent);
				}else{
					basic_type::_root=node->right_child;
					node->right_child->parent=nullptr;
				}
				newParent = node->right_child;
				node->parent=nullptr;
				node->right_child=nullptr;
			}
			if(basic_type::_root == nullptr){
				// empty
				basic_type::min_node = basic_type::max_node = nullptr;
			}else{
				if(node == basic_type::min_node){
					basic_type::min_node = newParent;
				}
				if(node == basic_type::max_node){
					basic_type::max_node = newParent;
				}
			}
			delete node;
			return Ret;
		}

		void erase(const DataType& data,bool left=true){
            auto find_result=basic_type::find(data);
            if(!find_result.second){
                return;
            }
            erase(find_result.first,left);
        }

	};

}

#endif