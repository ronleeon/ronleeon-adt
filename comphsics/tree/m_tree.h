#ifndef COMPHSICS_ADT_M_TREE_H
#define COMPHSICS_ADT_M_TREE_H

#include "comphsics/tree/node.h"
#include <cassert>
#include <ostream>
#include <istream>
#include <string>
#include <utility>
#include <stack>
#include <queue>
#include <cstring>


#define TREE_TRAITS(NODE_TYPE)   \
		public:using node_type = NODE_TYPE;\
		using node_pointer = NODE_TYPE*;\
		using node_iterator = NODE_TYPE*;\
		using node_type_reference = NODE_TYPE&;\
		using const_node_type = const NODE_TYPE;\
		using const_node_pointer = const NODE_TYPE*;\
		using const_node_iterator = const NODE_TYPE*;\
		using const_node_type_reference = const NODE_TYPE&; 


// macros defines signals for an empty node
// end of a node.
#define EMPTY_NODE_INDICATOR '#'

namespace comphsics {
	namespace tree {
		// using default m_node.
		template<typename DataType,size_t Size,typename NodeType=node::m_node<DataType,Size>>
		class m_tree{
			TREE_TRAITS(NodeType)

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
			explicit m_tree(node_pointer t) {
				set_m(Size);
				set_root(t);
				_owned=true;
			}
			explicit m_tree(std::nullptr_t) {
				set_m(Size);
				set_root(nullptr);
				_owned=true;
			}

			bool _owned;

			
		protected:
			node_pointer _root;
			size_t _m;
			
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
				re_input:Indicator=in.peek();
				if(Indicator=='\n'){
					in.get();
					goto re_input;
				}else if (Indicator==EMPTY_NODE_INDICATOR) {
					node=nullptr;
					in.get();
					return node;
				}else if(Indicator==' '||Indicator=='\t'){
					in.get();
					return node;
				}else {
					++num_of_nodes;
					in>> Data;
					node=new node_type(Data);
				}
				for(auto It=node->child_begin();It!=node->child_end();++It){
					*It=create_node_impl(in);
					if(*It){
						node->is_leaf=false;
						(*It)->parent=node;
						++node->child_size;
					}
				}
				shift_height(node);
				return node;
			}
			
		public:


			// statictics.
			size_t num_of_nodes=0;		

			m_tree(const m_tree&) = delete;
			m_tree():m_tree(nullptr){}
			m_tree(m_tree && tree) noexcept {
				_m=Size;
				_root=tree._root;
				tree._root=nullptr;
				_owned=tree._owned;
				num_of_nodes=tree.num_of_nodes;
			}
			node_pointer get_root()const {
				return _root;
			}
			void destroy() {
				if(_owned){
					delete _root;
					_root=nullptr;
					num_of_nodes=0;
				}
			}

			size_t size() const {
				return num_of_nodes;
			}


			~m_tree(){
				destroy();
			}
			// constructors.
			
			// return a shared tree holding the same nodes.
			[[nodiscard("Unused Tree")]] m_tree< DataType,Size,NodeType> make_shared() {
				m_tree< DataType,Size,NodeType> Ret(_root);
				Ret._owned=false;
				Ret.num_of_nodes=num_of_nodes;
				return Ret;
			}
			
			[[nodiscard("Unused Tree")]] static m_tree<DataType,Size,NodeType> create_empty_tree() {
				m_tree<DataType,Size,NodeType> Ret(nullptr);
				return Ret;
			}


			size_t get_height(const_node_pointer node) const {
				if(node){
					return node->height;
				}
				return 0;
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

			// levelly input.
			static m_tree<DataType,Size,NodeType> create_tree_l(std::istream &in=std::cin){
				m_tree<DataType,Size,NodeType> Ret(nullptr);
				char Indicator;
				size_t size=0;
				DataType Data;
				node_pointer pop;
				std::queue<node_pointer> tmp_queue;
				// !!! allowed to read a char of indicator.
				root_input:Indicator=in.peek();
				if(Indicator=='\n'){
					in.get();
					goto root_input;
				}else if (Indicator==EMPTY_NODE_INDICATOR) {
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
					node_pointer node=new node_type(Data);
					Ret._root=node;
					tmp_queue.push(Ret._root);
					++Ret.num_of_nodes;
				}
				while (!tmp_queue.empty()){
					pop = tmp_queue.front();
					tmp_queue.pop();
					for(auto It=pop->child_begin();It!=pop->child_end();++It){
						// !!! allowed to read a char of indicator.
						re_input:Indicator=in.peek();
						if(Indicator=='\n'){
							in.get();
							goto re_input;
						}else if (Indicator==EMPTY_NODE_INDICATOR) {
							*It=nullptr;
							in.get();
							continue;
						}else if(Indicator==' '||Indicator=='\t'){
							in.get();
							// read next input.
							continue;
						}else {
							in>> Data;
							node_pointer node=new node_type(Data);
							*It=node;
							pop->is_leaf=false;
							node->parent=pop;
							++pop->child_size;
							++Ret.num_of_nodes;
							tmp_queue.push(node);
						}
					}
					Ret.shift_height(pop);
				}
				return Ret;
			}
			// recursively input.

			static m_tree<DataType,Size,NodeType> create_tree_r(std::istream &in=std::cin){
				m_tree<DataType,Size,NodeType> Ret(nullptr);
				Ret._root=Ret.create_node_impl(in);
				return Ret;
			}

			// visiting.
			template <bool Echo = true,bool ShowNullNode=false>
			void pre_order_r(const_node_pointer start,std::ostream &out)const{
				if (is_empty()) {
					out << "Empty Tree!\n";
					return;
				}
				const_node_iterator tmp = start;
				if (Echo&&(tmp||ShowNullNode)) {
					print_visiting_node(tmp,out);
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
				const_node_iterator tmp = start;
				if (Echo&&(tmp||ShowNullNode)) {
					print_visiting_node(tmp,out);
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
				const_node_iterator t;
				while (!s.empty()) {
					t = s.top();
					s.pop();
					if (Echo&&(t||ShowNullNode)) {
						print_visiting_node(  t, out);
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
							print_visiting_node(t->first,out);
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
						print_visiting_node(t->first,out);
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
			virtual std::string to_string()const {
				return "<-" + std::to_string(_m) + " tree->";
			}
			virtual void print_visiting_node(const_node_pointer t, std::ostream& os)const{
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

			virtual void dump_tree(const_node_pointer start, std::ostream& out)const{
				/* how to dump tree like graph?*/
			}

			inline bool is_empty()const {
				return _root == nullptr;
			}

			// friend functions.
			friend bool operator==(const m_tree<DataType,Size,const NodeType>& t1, m_tree<DataType,Size,NodeType>& t2) {
				return t1.get_root() == t2.get_root();
			}
			friend std::ostream& operator<<(std::ostream& out, const m_tree<DataType,Size,NodeType>& t) {
				t.dump_tree(t.get_root(), out);
				return out;
			}
		};
	}
}
#endif