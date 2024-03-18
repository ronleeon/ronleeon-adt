// CRTP-mixin.



#ifndef RONLEEON_ADT_ABSTRACT_TREE_H
#define RONLEEON_ADT_ABSTRACT_TREE_H

#include "ronleeon/tree/node.h"
#include <cassert>
#include <iostream>
#include <ostream>
#include <istream>
#include <string>
#include <utility>
#include <stack>
#include <queue>
#include <cstring>


// macros defines signals for an empty node
// end of a node.
#define EMPTY_NODE_INDICATOR '#'

namespace ronleeon::tree {

		template<typename NodeType>
		class default_node_trait{
		public:
			using node_type = NodeType;
			using node_pointer = NodeType*;
			using node_type_reference = NodeType&;
			using const_node_type = const NodeType;
			using const_node_pointer = const NodeType*;
			using const_node_type_reference = const NodeType&;
		};

		template<typename NodeType>
		class m_node_print_trait{
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
			}
		};

		template<typename DataType,size_t Size,typename NodeType,typename TreeType, typename NodePrintTrait = m_node_print_trait<NodeType>>
		class abstract_tree{
		public:
			using node_type = NodeType;
			using node_pointer = NodeType*;
			using node_type_reference = NodeType&;
			using const_node_type = const NodeType;
			using const_node_pointer = const NodeType*;
			using const_node_type_reference = const NodeType&;

			using PrintTrait = NodePrintTrait;
		private:
			void set_m(size_t m) {
				_m=m;
				assert(m >= 0 && "The tree must at least have one child");
			}
			void set_root(node_pointer t) {
				if (!t) {
					_root = nullptr;
					return;
				}
				assert(t->child_size != 0 && "Cannot convert non empty node to be the root node!");
				_root = t;
			}
		protected:

			explicit abstract_tree(std::nullptr_t) {
				set_m(Size);
				set_root(nullptr);
			}

			
		protected:
			node_pointer _root;
			size_t _m{};
			
			// when create tree or modify it, change it height recursively,
			// this request all subtree of node(except node itself) are height-corrective.

			// Call this method only when node is modified 
			void shift_height(node_pointer node){
				while(node){
					size_t height=0;
					for(auto Child=node->child_begin(),End=node->child_end();Child!=End;++Child){
						if(*Child){
							height=std::max(height,(*Child)->height+1);
						}
					}
					if(height==node->height){
						break;
					}else{
						node->height=height;
						node=node->parent;
					}
				}
			}


			node_pointer create_node_impl(std::istream&in=std::cin){
				node_pointer node=nullptr;
				char Indicator;
				DataType Data;
				// !!! allowed to read a char of indicator.
				re_input:
					if(in.good()){
						Indicator=in.peek();
					}else{
						Indicator=EMPTY_NODE_INDICATOR;
					}
				if(Indicator=='\n'||Indicator==' '||Indicator=='\t'){
					in.get();
					goto re_input;
				}else if (Indicator==EMPTY_NODE_INDICATOR || Indicator == EOF) {
					node=nullptr;
					in.get();
					return node;
				}else {
					++num_of_nodes;
					in>> Data;
					node=new node_type();
					node->data = Data;
				}
				for(auto It = node->child_begin();It != node->child_end();++It){
					auto NewNode = create_node_impl(in);
					*It=NewNode;
					if(NewNode){
						++node->child_size;
						node->is_leaf=false;
						NewNode->parent=node;
					}
				}
				shift_height(node);
				return node;
			}

			// levelly input.
			static TreeType create_tree_l(std::istream &in=std::cin){
				TreeType Ret;
				char Indicator;
				size_t size=0;
				DataType Data;
				node_pointer pop;
				std::queue<node_pointer> tmp_queue;
				// !!! allowed to read a char of indicator.
				root_input:
				if(in.good()){
					Indicator=in.peek();
				}else{
					Indicator=EMPTY_NODE_INDICATOR;
				}
				if(Indicator=='\n'){
					in.get();
					goto root_input;
				}else if (Indicator==EMPTY_NODE_INDICATOR || Indicator == EOF) {
					in.get();
					Ret._root=nullptr;
					return Ret;
				}else if(Indicator==' '||Indicator=='\t'){
					in.get();
					// read next input.
					return Ret;
				}else {
					// take back to stream.
					in>> Data;
					node_pointer node=new node_type();
					node->data = Data;
					Ret._root=node;
					tmp_queue.push(Ret._root);
					++Ret.num_of_nodes;
				}
				while (!tmp_queue.empty()){
					pop = tmp_queue.front();
					tmp_queue.pop();
					for(auto It = pop->child_begin();It != pop->child_end();++It){
						// !!! allowed to read a char of indicator.
						re_input:
							if(in.good()){
								Indicator=in.peek();
							}else{
								Indicator=EMPTY_NODE_INDICATOR;
							}
						if(Indicator=='\n'||Indicator==' '||Indicator=='\t'){
							in.get();
							goto re_input;
						}else if (Indicator==EMPTY_NODE_INDICATOR || Indicator == EOF) {
							*It = nullptr;
							in.get();
							continue;
						}else {
							in>> Data;
							node_pointer node=new node_type();
							node->data = Data;
							*It = node;
							++pop->child_size;
							pop->is_leaf=false;
							node->parent=pop;
							++Ret.num_of_nodes;
							tmp_queue.push(node);
						}
					}
					Ret.shift_height(pop);
				}
				return Ret;
			}

			// recursively input.
			static TreeType create_tree_r(std::istream &in=std::cin){
				TreeType Ret;
				Ret._root=Ret.create_node_impl(in);
				return Ret;
			}


			
		public:


			// statistics.
			size_t num_of_nodes=0;		

			abstract_tree(const abstract_tree&) = delete;
			abstract_tree():abstract_tree(nullptr){}
            // OK,move semantics sets nullptr to temp tree.
			abstract_tree(abstract_tree && tree) noexcept {
				_m=Size;
				_root=tree._root;
				tree._root=nullptr;
				num_of_nodes=tree.num_of_nodes;
			}
			const_node_pointer get_root()const {
				return _root;
			}
			
			virtual void destroy() {
				delete _root;
				_root=nullptr;
				num_of_nodes=0;
			}

			[[nodiscard]] size_t size() const {
				return num_of_nodes;
			}


			virtual ~abstract_tree(){
				destroy();
			}
			// constructors.

			
			[[nodiscard("Unused Tree")]] static TreeType create_empty_tree() {
				TreeType Ret;
				return Ret;
			}


			size_t get_height(const_node_pointer node) const {
				if(node){
					return node->height;
				}
				return 0;
			}

            const_node_pointer get_root(const_node_pointer node) const {
                if(!node){
                    return nullptr;
                }
                node_pointer cur = node;
                node_pointer parent = nullptr;
                do{
                    cur = parent;
                    parent = get_parent(node);
                }while(parent != nullptr);
                return cur;
            }

            node_pointer get_root(const_node_pointer node){
                if(!node){
                    return nullptr;
                }
                node_pointer cur = node;
                node_pointer parent = nullptr;
                do{
                    cur = parent;
                    parent = get_parent(node);
                }while(parent != nullptr);
                return cur;
            }

			const_node_pointer get_parent(const_node_pointer node)const{
				if(node){
					return node->parent;
				}
				return nullptr;
			}
			
			node_pointer get_parent(const_node_pointer node){
				if(node){
					return node->parent;
				}
				return nullptr;
			}

			// visiting.
			template <bool Echo = true,bool ShowNullNode=false>
			void pre_order_r(const_node_pointer start,std::ostream &out)const{
				if (is_empty()) {
					out << "Empty Tree!\n";
					return;
				}
				if (const_node_pointer tmp = start; Echo&&(tmp||ShowNullNode)) {
					PrintTrait::print_visiting_node(tmp,out);
					out << '\n';
				}
				if(start){
					for (auto It=start->child_begin(),End=start->child_end();It!=End;++It) {
						pre_order_r<Echo,ShowNullNode>(*It,out);
					}
				}
			}
			template<bool Echo = true,bool ShowNullNode=false>
			void post_order_r(const_node_pointer start,std::ostream &out)const{
				if (is_empty()) {
					out << "Empty Tree!\n";
					return;
				}
				if(start){
					for (auto It=start->child_begin(),End=start->child_end();It!=End;++It) {
						post_order_r<Echo,ShowNullNode>(*It,out);
					}
				}
				const_node_pointer tmp = start;
				if (Echo&&(tmp||ShowNullNode)) {
					PrintTrait::print_visiting_node(tmp,out);
					out << '\n';
				}
			}
			template <bool Echo = true,bool ShowNullNode=false>
			void pre_order(const_node_pointer start,std::ostream &out)const{
				if (is_empty()) {
					out << "Empty Tree!\n";
					return;
				}
				std::stack<const_node_pointer> s;
				s.push(start);
				const_node_pointer t;
				while (!s.empty()) {
					t = s.top();
					s.pop();
					if (Echo&&(t||ShowNullNode)) {
						PrintTrait::print_visiting_node(  t, out);
						out << '\n';
					}
					if (!t) {
						continue;
					}
					for (auto rit = t->child_rbegin(); rit != t->child_rend(); rit++) {
						s.push(*rit);
					}
				}
			}
			template <bool Echo = true,bool ShowNullNode=false>
			void post_order(const_node_pointer start,std::ostream &out)const{
				if (is_empty()) {
					out << "Empty Tree!\n";
					return;
				}
				std::stack<std::pair<const_node_pointer, bool>*> s;
				s.push(new std::pair<const_node_pointer, bool>(std::make_pair(start, false)));
				std::pair<const_node_pointer, bool>* t;
				while (!s.empty()) {
					t = s.top();
					if (!t->second) {
						t->second = true;
						if (!t->first) {
							continue;
						}
						for (auto rit = t->first->children.rbegin(); rit != t->first->children.rend(); rit++) {
							s.push(new std::pair<const_node_pointer, bool>(std::make_pair(*rit, false)));
						}
					}else {
						if (Echo&&(t->first||ShowNullNode)) {
							PrintTrait::print_visiting_node(t->first,out);
							out << '\n';
						}
						s.pop();
					}
				}
			}
			template<bool Echo = true,bool ShowNullNode=false>
			void level_order(const_node_pointer start,std::ostream &out)const{
				if (is_empty()) {
					out << "Empty Tree!\n";
				}
				// store the current level also.
				std::queue<std::pair<const_node_pointer, int>*> q;
				size_t cur_h = 0;
				bool showLine=true;
				std::pair<const_node_pointer, int>* t;
				q.push(new std::pair<const_node_pointer, int>(std::make_pair(start, 0)));
				while (!q.empty()) {
					t = q.front();
					q.pop();
					if (t->second != cur_h) {
						cur_h = t->second;
						showLine=true;
					}
					if (Echo&&(t->first||ShowNullNode)) {
						if (cur_h == 0||showLine){
							showLine=false;
							out << "Level:" << t->second << '\n';
						}
						PrintTrait::print_visiting_node(t->first,out);
						out << '\n';
					}
					if(!t->first){
						continue;
					}
					for (auto it = t->first->child_begin(); it != t->first->child_end(); it++) {
						q.push(new std::pair<const_node_pointer, int>(std::make_pair(*it, t->second + 1)));
					}
				}
			}


            // virtual methods.
			[[nodiscard]] virtual std::string to_string()const {
				return "<-" + std::to_string(_m) + " Order Tree->";
			}

			virtual void dump_tree(const_node_pointer start, std::ostream& out)const{
				/* how to dump tree like graph?*/
			}

			[[nodiscard]] inline bool is_empty()const {
				return _root == nullptr;
			}

			// friend functions.
			inline friend bool operator==(const TreeType& t1, TreeType& t2) {
				return t1.get_root() == t2.get_root();
			}
			inline friend std::ostream& operator<<(std::ostream& out, const TreeType& t) {
				t.dump_tree(t.get_root(), out);
				return out;
			}

		};

		template<typename NodeType>
		class b_node_print_trait{
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
			}
		};
		template<typename DataType,typename NodeType,typename TreeType, typename NodePrintTrait = b_node_print_trait<NodeType>>
		class abstract_b_tree:public abstract_tree<DataType,2,NodeType,TreeType,NodePrintTrait>{
		
			using basic_type=abstract_tree<DataType,2,NodeType,TreeType, NodePrintTrait>;
		public:
			using node_type = NodeType;
			using node_pointer = NodeType*;
			using node_type_reference = NodeType&;
			using const_node_type = const NodeType;
			using const_node_pointer = const NodeType*;
			using const_node_type_reference = const NodeType&;

			using PrintTrait = typename basic_type::PrintTrait;
		protected:

			explicit abstract_b_tree(std::nullptr_t):abstract_tree<DataType,2,NodeType
				,TreeType, NodePrintTrait>(nullptr){}
			
		public:
			abstract_b_tree(const abstract_b_tree&) = delete;
			abstract_b_tree():abstract_b_tree(nullptr){}
			abstract_b_tree(abstract_b_tree && tree) noexcept :abstract_tree<DataType,2,NodeType
				,TreeType, NodePrintTrait>(std::move(tree)) {}

			[[nodiscard]] std::string to_string()const override {
				return "<-Binary tree->";
			}

			bool is_left_child(const_node_pointer child,const_node_pointer parent) const{
				if(child&&parent){
					if(child==parent->left_child){
						return true;
					}
				}
				return false;
			}

			bool is_right_child(const_node_pointer child,const_node_pointer parent) const{
				if(child&&parent){
					if(child==parent->right_child){
						return true;
					}
				}
				return false;
			}

			template <bool Echo = true,bool ShowNullNode=false>
			void in_order_r(const_node_pointer n,std::ostream &out)const{
				if (basic_type::is_empty()) {
					out << "Empty Tree!\n";
					return;
				}
				if(n){
					in_order_r<Echo,ShowNullNode>(n->left_child,out);
				}
				if(Echo&&(n||ShowNullNode)){
					PrintTrait::isiting_node(n,out);
					out<<'\n';
				}
				if(n){
					in_order_r<Echo,ShowNullNode>(n->right_child,out);
				}
			}
			template <bool Echo = true,bool ShowNullNode=false>
			void in_order(const_node_pointer n,std::ostream &out)const{
				if (basic_type::is_empty()) {
					out << "Empty Tree!\n";
					return;
				}
				std::stack<const_node_pointer> s;
				s.push(n);
				const_node_pointer top;
				while(!s.empty()){
					top=s.top();
					while(top){
						top=top->left_child;
						s.push(top);
					}
					s.pop();
					// consider empty node.
					if(Echo&&(top||ShowNullNode)){
						PrintTrait::print_visiting_node(top,out);
						out<<'\n';
					}
					if(!s.empty()){
						top=s.top();
						if(Echo&&(top||ShowNullNode)){
							PrintTrait::print_visiting_node(top,out);
							out<<'\n';
						}
						s.pop();
						if(top){
							s.push(top->right_child);
						}
					}
				}
			}

		};

		// Guarantee all NodeType data are not equal,otherwise the latter 
		// will be ignored.

		// C++ style compare,not java compare style.
		template<typename DataType,typename Compare,typename NodeType,typename TreeType,typename NodePrintTrait = b_node_print_trait<NodeType>>
		class abstract_bs_tree:public abstract_b_tree<DataType,NodeType,TreeType, NodePrintTrait>{
			using basic_type=abstract_b_tree<DataType,NodeType,TreeType, NodePrintTrait>;
			// prohibit all create functions.
			using basic_type::create_tree_l;
			using basic_type::create_tree_r;
		public:
			using node_type = NodeType;
			using node_pointer = NodeType*;
			using node_type_reference = NodeType&;
			using const_node_type = const NodeType;
			using const_node_pointer = const NodeType*;
			using const_node_type_reference = const NodeType&;

			using PrintTrait = typename basic_type::PrintTrait;
		protected:
			Compare comp;

			// helper nodes , for map/set containers to define their iterators,
			// every sub-class must update min_node and max_node when updating the tree
			// empty tree: they are all nullptr.
			const_node_pointer min_node = nullptr; //< min_node points left most node of the root.
			const_node_pointer max_node = nullptr; //< max_node points right most node of the root.
		public:
			static const_node_pointer left_most(const_node_pointer node){
				if(!node){
					return nullptr;
				}
				const_node_pointer tmp=node;
				while(tmp->left_child){
					tmp=tmp->left_child;
				}
				return tmp;
			}
			static const_node_pointer right_most(const_node_pointer node){
				if(!node){
					return nullptr;
				}
				const_node_pointer tmp=node;
				while(tmp->right_child){
					tmp=tmp->right_child;
				}
				return tmp;
			}
			// Get the next node in in-order.
			static const_node_pointer increment(const_node_pointer node)  {
				if(!node){
					return nullptr;
				}
				const_node_pointer tmp=node->right_child;
				if(tmp) {
					return left_most(tmp);
				}else{
					const_node_pointer parent=node->parent;
					const_node_pointer cur=node;
					while(parent!=nullptr&&cur==parent->right_child){
						cur=parent;
						parent=cur->parent;
					}
					return parent;
				}
			}

			// Get the pre node in in-order.
			static const_node_pointer decrement(const_node_pointer node){
				if(!node){
					return nullptr;
				}
				const_node_pointer tmp=node->left_child;
				if(tmp) {
					return right_most(tmp);
				}else{
					const_node_pointer parent=node->parent;
					const_node_pointer cur=node;
					while(parent!=nullptr&&cur==parent->left_child){
						cur=parent;
						parent=cur->parent;
					}
					return parent;
				}
			}
		protected:
			explicit abstract_bs_tree(std::nullptr_t, Compare comp_ =  Compare{}):basic_type(nullptr), comp(comp_){}
		public:
			abstract_bs_tree(Compare comp_ =  Compare{}):abstract_bs_tree(nullptr, comp_){};
			abstract_bs_tree(const abstract_bs_tree&)=delete;
			abstract_bs_tree(const DataType data[],size_t Size,Compare comp_ =  Compare{}):basic_type(nullptr, comp_){
				for(size_t Index=0;Index<Size;++Index){	
					insert(data[Index]);
				}
			}
			abstract_bs_tree(abstract_bs_tree && tree):basic_type(std::move(tree)), comp(std::move(tree.comp)) {}
			std::string to_string()const override {
				return "<-Binary Sort Tree->";
			}
			static TreeType create_tree(std::istream &in=std::cin) {
				TreeType tree; 
				char Indicator;
				DataType Data;
				// !!! allowed to read a char of indicator.
				while(true){
					root_input:
						if(in.good()){
							Indicator=in.peek();
						}else{
							Indicator=EMPTY_NODE_INDICATOR;
						}
					if(Indicator=='\n'){
						in.get();
						goto root_input;
					}else if(Indicator==' '||Indicator=='\t'){
						in.get();
						continue;
					}else if(Indicator==EMPTY_NODE_INDICATOR || Indicator == EOF ){
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
			std::pair<const_node_pointer,bool> find(const DataType& data)const{
				if(basic_type::is_empty()){
					return std::make_pair(nullptr,false);
				}
				const_node_pointer start=basic_type::get_root();
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

			
			// insert the data if find it, ignored!
			// the second returns whether insert operation is successful.
			virtual std::pair<const_node_pointer,bool> insert(const DataType& data){
				auto find_result=find(data);
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
					min_node = max_node = basic_type::_root;
					return find_result;
				}
				auto node=const_cast<node_pointer>(find_result.first);
				// not equal
				if(comp(data,node->data)){
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
				if(min_node&& (min_node->left_child == node)){
					min_node = node;
				}
				if(max_node&&(max_node->right_child == node)){
					max_node = node;
				}
				basic_type::shift_height(node);
				return find_result;
			}

			
			// erase has two replacement.
			// returns the next node.
			// when erased node has two children,
			// we can replace it with node of its left tree.
			//  or  node of its right tree.

			virtual const_node_pointer erase(node_pointer node,bool left=true){
				if(!node){
					return nullptr;
				}
				--basic_type::num_of_nodes;
				const_node_pointer Ret=increment(node);
				// get the next node.
				if(node->left_child&&node->right_child){
					if(left){
						auto left=left_most(node->left_child);
						// cannot be empty.
						node->data=left->data;
						// now left do not have right_child.
						node=const_cast<node_pointer>(left);
					}else{
						auto right=right_most(node->right_child);
						// cannot be empty.
						node->data=right->data;
						// now right do not have right_child.
						node=const_cast<node_pointer>(right);
					}
				}
				// now node cannot have two childs.
				node_pointer parent=node->parent;
				node_pointer* point_to_node;
				// newParent records the new min node when node is min_node
				// records the new max node when node is max_node
				const_node_pointer newParent = nullptr;
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
						basic_type::shift_height(node->parent);
					}else{
						basic_type::_root=nullptr;
					}
					newParent = node->parent;
					node->parent=nullptr;
				}else if(node->left_child){
					if(point_to_node){
						node->left_child->parent=parent;
						*point_to_node=node->left_child;
						--parent->child_size;
						if(parent->child_size==0){
							parent->is_leaf=true;
						}
						basic_type::shift_height(node->parent);
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
						--parent->child_size;
						if(parent->child_size==0){
							parent->is_leaf=true;
						}
						basic_type::shift_height(node->parent);
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
					min_node = max_node = nullptr;
				}else{
					if(node == min_node){
						min_node = newParent;
					}
					if(node == max_node){
						max_node = newParent;
					}
				}
				
				delete node;
				return Ret;
			}

			const_node_pointer erase(const_node_pointer node, bool left = true){
				return erase(const_cast<node_pointer>(node), left);
			}


			void erase(const DataType& data,bool left=true){
				auto find_result=find(data);
				if(!find_result.second){
					return;
				}
				erase(find_result.first,left);
			}

			// find the min and the max data. 
			DataType min()const{
				const auto find_result=left_most(basic_type::_root);
				assert(find_result&&"Empty tree!");
				return find_result->data;
			}
			DataType max()const{
				const auto find_result=right_most(basic_type::_root);
				assert(find_result&&"Empty tree!");
				return find_result->data;
			}

			const_node_pointer start(){
				return min_node;
			}

			const_node_pointer last(){
				return max_node;
			}

			static bool is_binary_search_tree(const_node_pointer root){
				if(!root){
					return true;
				}
				if(root->left_child&&root->right_child){
					if(Compare(root->left_child->data,root->data)&&Compare(root->data,root->right_child->data)){
						return is_binary_search_tree(root->left_child)&&is_binary_search_tree(root->right_child);
					}
				}else if(root->left_child){
					if(Compare(root->left_child->data,root->data)){
						return is_binary_search_tree(root->left_child);
					}
				}else if(root->right_child){
					if(Compare(root->data,root->right_child->data)){
						return is_binary_search_tree(root->right_child);
					}
				}else{
					return true;
				}
				return false;
			}


			void destroy() override {
				basic_type::destroy();
				min_node = max_node = nullptr;
			}

		};
	}

#endif