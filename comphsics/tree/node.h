// The file defines two kinds of basic nodes using in tree,
// one is for binary tree,
// the others are for arbitrary tree.

#ifndef COMPHSICS_ADT_NODE_H
#define COMPHSICS_ADT_NODE_H

#include <array>
#include <cassert>
#include <list>
#include <istream>
#include <ostream>
#include <iterator>
#include <map>
namespace comphsics{
	namespace tree {
		namespace node {

			// for arbitrary children nodes, you can specify how to store them .
			// for m node(a node have arbitrary children), we can define some traits of the storage
			// of its children.
			// every storage must at least implement:
			// iterator
			// reverse_iterator
			// const_iterator
			// const_reverse_iterator
			// ++,--(of every iterator)
			// begin(),end(),rbegin(),rend(),cbegin(),cend(),crbegin(),crend()
			// default:using a std::array to store all children.
			template<typename NodeType,typename DataType,size_t Size>
			struct abstract_node{
				// record data, multi-data can be zipped into one type.
				DataType data;
				size_t size;
				size_t child_size;// real child size
				bool is_leaf;
				NodeType* parent;
				size_t height;
				explicit abstract_node(const DataType &Data):data(Data),child_size(0),size(Size)
					,is_leaf(true),parent(nullptr),height(0){}
				explicit abstract_node(const DataType &&Data):data(Data),child_size(0),size(Size)
					,is_leaf(true),parent(nullptr),height(0){}
				abstract_node(const abstract_node&)=delete;
				abstract_node()=delete;
				abstract_node(abstract_node&& rhs){
					data=std::move(rhs.data);
					child_size=rhs.child_size;
					size=rhs.size;
					is_leaf=rhs.is_leaf;
					parent=rhs.parent;
					rhs.parent=nullptr;
					height=rhs.height;
				}

				
				~abstract_node(){
					for(auto Child=child_begin(),End=child_end();Child!=End;++Child){
						if(*Child){
							delete *Child;
						}
					}
				}
				// default storing pointers.
				std::array<NodeType*,Size> children;
				
				using const_child_iterator=typename std::array<NodeType*,Size>::const_iterator;
				using child_iterator=typename std::array<NodeType*,Size>::iterator;
				using reverse_child_iterator=typename std::array<NodeType*,Size>::reverse_iterator;
				using const_reverse_child_iterator=typename std::array<NodeType*,Size>::const_reverse_iterator;
				child_iterator child_begin(){
					return children.begin();
				}
				child_iterator child_end(){
					return children.end();
				};
				const_child_iterator child_cbegin()const{
					return children.cbegin();
				}
				const_child_iterator child_cend()const{
					return children.cend();
				}
				reverse_child_iterator child_rbegin(){
					return children.rbegin();
				}
				reverse_child_iterator child_rend(){
					return children.rend();
				}
				const_reverse_child_iterator child_crbegin()const{
					return children.crbegin();
				}
				const_reverse_child_iterator child_crend()const{
					return children.crend();
				}
				const_child_iterator child_begin()const{
					return children.cbegin();
				}
				const_child_iterator child_end()const{
					return children.cend();
				}
				const_reverse_child_iterator child_rbegin() const {
					return children.crbegin();
				}
				const_reverse_child_iterator child_rend() const {
					return children.crend();
				}
			};
			
			template<typename DataType,size_t Size>
			struct m_node final:abstract_node<m_node<DataType,Size>,DataType,Size>{
			
				explicit m_node(const DataType &Data)
					:abstract_node<m_node<DataType,Size>,DataType,Size>(Data){}
				explicit m_node(const DataType &&Data)
					:abstract_node<m_node<DataType,Size>,DataType,Size>(std::move(Data)){}
			};

			// binary node can default children storage(left and right children)
			template <typename NodeType,typename DataType>
			struct abstract_b_node :abstract_node<NodeType,DataType,2>{
				// use these two field to bind children[0],and children[1]
				NodeType* &left_child;
				NodeType* &right_child;
				explicit abstract_b_node(const DataType &data)
					:abstract_node<NodeType,DataType,2>(data)
					,left_child(*abstract_node<NodeType,DataType,2>::child_begin()),
					right_child(*abstract_node<NodeType,DataType,2>::child_rbegin()){
					// only has two children.
				}
				explicit abstract_b_node(const DataType &&data)
					:abstract_node<NodeType,DataType,2>(std::move(data))
					,left_child(*abstract_node<NodeType,DataType,2>::child_begin()),
					right_child(*abstract_node<NodeType,DataType,2>::child_rbegin()){
					// only has two children.
				}
			};

			template<typename DataType>
			struct b_node final:abstract_b_node<b_node<DataType>,DataType>{
			
				explicit b_node(const DataType &data)
					:abstract_b_node<b_node<DataType>,DataType>(data){
				}
				explicit b_node(const DataType &&data)
					:abstract_b_node<b_node<DataType>,DataType>(std::move(data)){
				}
			};
			
			template <typename DataType>
			struct h_node final :abstract_b_node<h_node<DataType>,DataType>{
				explicit h_node(const DataType &data)
					:abstract_b_node<h_node<DataType>,DataType>(data){
					code=0;
					wpl=0;
				}
				explicit h_node(const DataType &&data)
					:abstract_b_node<h_node<DataType>,DataType>(std::move(data)){
					code=0;
					wpl=0;
				}
				long long code;
				// weighted path length. One leaf(not node).
				long long wpl;
			};


			// Effective C++:
			// constructor and deconstructor can not call virtual methods!

			// So when destroy all nodes, thread tree must be un-threaded, 
			template <typename DataType>
			struct t_node final:abstract_b_node<h_node<DataType>,DataType>{
				explicit t_node(const DataType &data)
				:abstract_b_node<t_node<DataType>,DataType>(data){
					left_is_thread=false;
					right_is_thread=false;
				}
				explicit t_node(const DataType &&data)
					:abstract_b_node<h_node<DataType>,DataType>(std::move(data)){
					left_is_thread=false;
					right_is_thread=false;
				}
				bool left_is_thread;
				bool right_is_thread;
				~t_node(){
					assert(!left_is_thread&&!right_is_thread);
				}
			};

			template <typename NodeType,typename DataType>
			struct abstract_bs_node :abstract_b_node<NodeType,DataType>{
				explicit abstract_bs_node(const DataType &data)
					:abstract_b_node<NodeType,DataType>(data){}
				explicit abstract_bs_node(const DataType &&data)
					:abstract_b_node<NodeType,DataType>(std::move(data)){}
			};

			template <typename DataType>
			struct bs_node final:abstract_bs_node<bs_node<DataType>,DataType>{
			
				explicit bs_node(const DataType &data)
					:abstract_bs_node<bs_node<DataType>,DataType>(data){}
				explicit bs_node(const DataType &&data)
					:abstract_bs_node<bs_node<DataType>,DataType>(std::move(data)){}
			};

			template <typename DataType>
			struct avl_node final :abstract_bs_node<avl_node<DataType>,DataType>{
				explicit avl_node(const DataType &data)
					:abstract_bs_node<avl_node<DataType>,DataType>(data){
					balance_factor=0;
				}
				explicit avl_node(const DataType &&data)
					:abstract_bs_node<avl_node<DataType>,DataType>(std::move(data)){
					balance_factor=0;
				}

				// only valid in -1,0,1
				short balance_factor;// left_child->height-right_child->height
			};

			// all leaves are created to red by default.
			template <typename DataType>
			struct rb_node final:abstract_bs_node<rb_node<DataType>,DataType>{
				enum class COLOR{
					RED,BLACK
				};
				explicit rb_node(const DataType &data)
					:abstract_bs_node<rb_node<DataType>,DataType>(data){
					color=COLOR::RED;
				}
				explicit rb_node(const DataType &&data)
					:abstract_bs_node<rb_node<DataType>,DataType>(std::move(data)){
					color=COLOR::RED;
				}

				COLOR color;
			};

			// B-tree node


			// B+tree

		}
	}
}

#endif