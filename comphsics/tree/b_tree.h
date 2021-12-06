#ifndef COMPHSICS_ADT_B_TREE_H
#define COMPHSICS_ADT_B_TREE_H


#include "comphsics/tree/node.h"
#include "comphsics/tree/m_tree.h"

namespace comphsics{
	namespace tree{
		template<typename DataType,typename NodeType=node::b_node<DataType>>
		class b_tree:public m_tree<DataType,2,NodeType>{
			using basic_type=m_tree<DataType,2,NodeType>;
		public:
			using node_type = NodeType;
			using node_pointer = NodeType*; 
			using node_iterator = NodeType*; 
			using node_type_reference = NodeType&; 
			using const_node_type = const NodeType; 
			using const_node_pointer = const NodeType*; 
			using const_node_iterator = const NodeType*; 
			using const_node_type_reference = const NodeType&; 
		protected:

			explicit b_tree(std::nullptr_t):m_tree<DataType,2,NodeType>(nullptr){}
			
			explicit b_tree(node_pointer t)
				:m_tree<DataType,2,NodeType>(t){}
			
		public:
			b_tree(const b_tree&) = delete;
			b_tree():b_tree(nullptr){}
			b_tree(b_tree && tree):m_tree<DataType,2,NodeType>((m_tree<DataType,2,NodeType>&&)tree) {}

			
			// return a shared tree holding the same nodes.
			[[nodiscard("Unused Tree")]] b_tree make_shared() {
				b_tree<DataType,NodeType> Ret(basic_type::_root);
				Ret._owned=false;
				Ret.num_of_nodes=basic_type::num_of_nodes;
				return Ret;
			}

			std::string to_string()const override {
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
					print_visiting_node(n,out);
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
						print_visiting_node(top,out);
						out<<'\n';
					}
					if(!s.empty()){
						top=s.top();
						if(Echo&&(top||ShowNullNode)){
							print_visiting_node(top,out);
							out<<'\n';
						}
						s.pop();
						if(top){
							s.push(top->right_child);
						}
					}
				}
			}
			void virtual print_visiting_node(const_node_pointer t,std::ostream& out)const override{
				basic_type::print_visiting_node(t,out);
				if(t){
					if(t->left_child&&!t->right_child){
						out<<" has left child";
					}else if(t->left_child&&t->right_child){
						out<<" has left and right child";
					}else if(!t->left_child&&t->right_child){
						out<<" has right child";
					}
				}
			}
			[[nodiscard("Unused Tree")]] static b_tree create_empty_tree() {
				b_tree Ret(nullptr);
				return Ret;
			}
			// levelly input.
			[[nodiscard("Unused Tree")]] static b_tree create_tree_l(std::istream &in=std::cin){
				b_tree Ret(nullptr);
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
					++Ret.num_of_nodes;
					tmp_queue.push(Ret._root);
				}
				while (!tmp_queue.empty()){
					pop = tmp_queue.front();
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
							++Ret.num_of_nodes;
							pop->is_leaf=false;
							node->parent=pop;
							++pop->child_size;
							tmp_queue.push(node);		
						}
					}
					Ret.shift_height(pop);
					tmp_queue.pop();
				}
				return Ret;
			}
			// recursively input.
			[[nodiscard("Unused Tree")]] static b_tree<DataType,NodeType> create_tree_r(std::istream &in=std::cin){
				b_tree<DataType,NodeType> Ret(nullptr);
				Ret._root=Ret.create_node_impl(in);
				return Ret;
			}

		};
	}
}

#endif