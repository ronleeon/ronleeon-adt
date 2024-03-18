// The file defines two kinds of basic nodes using in tree,
// one is for binary tree,
// the others are for arbitrary tree.


#ifndef RONLEEON_ADT_NODE_H
#define RONLEEON_ADT_NODE_H

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <list>
#include <istream>
#include <ostream>
#include <iterator>
#include <map>
#include <utility>
#include <tuple>
namespace ronleeon::tree::node{


			// for m -node use an array to store all children.
			// its iterator must provide to access tyo its m-children(include empty node).
			template <typename NodeType,size_t Size>
			struct m_child_storage{
				// default storing pointers.
				std::array<NodeType*,Size> children;
				// in B/Bplus tree, all non-empty children must be near each other.
				size_t child_size;// non-empty child size
				explicit m_child_storage() {
					child_size = 0;
					for(size_t I = 0; I <Size;++ I){
						children[I]=nullptr;
					}
				}
				m_child_storage(const m_child_storage&)=delete;
				m_child_storage(m_child_storage&& rhs) noexcept: children(std::move(rhs.children)),child_size(rhs.child_size) {}
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

				virtual ~m_child_storage(){
					for(size_t Index = 0; Index < child_size; ++Index){
						if(children[Index]){
							delete children[Index];
						}
					}
				}
			};

			
			// this data storage is used in node that just need one data field.
			template <typename DataType>
			struct single_data_storage{
				DataType data;
				explicit single_data_storage(const DataType &Data):data(Data){}

				single_data_storage(const single_data_storage&)=delete;
				single_data_storage()=default;
				single_data_storage(single_data_storage&& rhs) noexcept: data(std::move(rhs.data)) {}
			};

            template<typename NodeType,typename DataType,size_t Size
				,typename data_storage=single_data_storage<DataType>
				,typename child_storage=m_child_storage<NodeType,Size>
			>
			struct abstract_node:child_storage,data_storage{
				
				bool is_leaf;
				NodeType* parent;
                // In these trees: height field is useless(null).
				// 1. B_tree/Bplus_tree(maintain exactly height for each node is hard when inserting causes the root node splitting)
				size_t height;
				explicit abstract_node():data_storage()
					,child_storage(),is_leaf(true),parent(nullptr),height(0){}

				abstract_node(const abstract_node&)=delete;
				abstract_node(abstract_node&& rhs) noexcept :child_storage(std::move(rhs))
					,data_storage(std::move(rhs)){
					is_leaf=rhs.is_leaf;
					parent=rhs.parent;
					rhs.parent=nullptr;
					height=rhs.height;
				}
			};

			
			// m- node with an array of children and a single data field.
			// base class of many concrete nodes.(bs_node, h_node...)
			template<typename DataType,size_t Size>
			struct m_node final:abstract_node<m_node<DataType,Size>,DataType,Size,
			single_data_storage<DataType>, m_child_storage<m_node<DataType,Size>, Size>>{
			
				explicit m_node()
					:abstract_node<m_node<DataType,Size>,DataType,Size>(){}
			};
			
			// binary node can default children storage(left and right children)
			template <typename NodeType,typename DataType>
			struct abstract_b_node :abstract_node<NodeType,DataType,2>{
				// use these two field to bind children[0],and children[1]
				NodeType* &left_child;
				NodeType* &right_child;
				explicit abstract_b_node()
					:abstract_node<NodeType,DataType,2>()
					,left_child(*abstract_node<NodeType,DataType,2>::child_begin()),
					right_child(*abstract_node<NodeType,DataType,2>::child_rbegin()){
					// only has two children.
				}

			};

			template<typename DataType>
			struct b_node final:abstract_b_node<b_node<DataType>,DataType>{};
			
			template <typename DataType>
			struct h_node final :abstract_b_node<h_node<DataType>,DataType>{
				explicit h_node()
					:abstract_b_node<h_node<DataType>,DataType>(){
					code=0;
					wpl=0;
				}

				long long code;
				// weighted path length. One leaf(not node).
				long long wpl;
			};


			// Effective C++:
			// constructor and destructor can not call virtual methods!

			// So when destroy all nodes, thread tree must be un-threaded, 
			template <typename DataType>
			struct t_node final:abstract_b_node<h_node<DataType>,DataType>{
				explicit t_node()
				:abstract_b_node<t_node<DataType>,DataType>(){
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
			struct abstract_bs_node :abstract_b_node<NodeType,DataType>{};

			template <typename DataType>
			struct bs_node final:abstract_bs_node<bs_node<DataType>,DataType>{};

			template <typename DataType>
			struct avl_node final :abstract_bs_node<avl_node<DataType>,DataType>{
				explicit avl_node()
					:abstract_bs_node<avl_node<DataType>,DataType>(){
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
				explicit rb_node()
					:abstract_bs_node<rb_node<DataType>,DataType>(){
					color=COLOR::RED;
				}

				COLOR color;
			};


			// used for B tree , B+ tree, a node can store an array of data.
			template <typename DataType,size_t Size>
			struct B_data_storage{
				std::array<DataType,Size> data;
				size_t data_size;// data size <= Size
				explicit B_data_storage() {
					data_size = 0;
				}
				B_data_storage(const B_data_storage&)=delete;
				B_data_storage(B_data_storage&& rhs) noexcept: data(std::move(rhs.data)),data_size(rhs.data_size){}
				using const_data_iterator=typename std::array<DataType,Size>::const_iterator;
				using data_iterator=typename std::array<DataType,Size>::iterator;
				using reverse_data_iterator=typename std::array<DataType,Size>::reverse_iterator;
				using const_reverse_data_iterator=typename std::array<DataType,Size>::const_reverse_iterator;
				data_iterator data_begin(){
					return data.begin();
				}
				data_iterator data_end(){
					return data.end();
				};
				const_data_iterator data_cbegin()const{
					return data.cbegin();
				}
				const_data_iterator data_cend()const{
					return data.cend();
				}
				reverse_data_iterator data_rbegin(){
					return data.rbegin();
				}
				reverse_data_iterator data_rend(){
					return data.rend();
				}
				const_reverse_data_iterator data_crbegin()const{
					return data.crbegin();
				}
				const_reverse_data_iterator data_crend()const{
					return data.crend();
				}
				const_data_iterator data_begin()const{
					return data.cbegin();
				}
				const_data_iterator data_end()const{
					return data.cend();
				}

				const_reverse_data_iterator data_rbegin() const {
					return data.crbegin();
				}
				const_reverse_data_iterator data_rend() const {
					return data.crend();
				}
			};
			template <typename DataType,size_t Size>
			struct Bplus_data_storage: public B_data_storage<DataType,  Size>{};
            // B tree node.
            // A B node having Size children must have Size-1 data.(Size>0)
            template <typename DataType, size_t Size>
            struct B_node final:abstract_node<B_node<DataType,Size>,DataType,Size
                    ,B_data_storage<DataType, Size - 1>,m_child_storage<B_node<DataType, Size>, Size>>{
				static_assert(Size >= 2, "A B node must have at least 2 children");
            };

			// Their exists another B+ tree definition, which requires the number of keys and childrens are the same,
			// the operations of find,insertion and erase are the same procedures, here only provides the same definition of
			// B+ tree as B tree.

			// B+tree node.
			// Unlike B tree node, B+ internal nodes only store indices , and only leaves store the data.
            // A B+ node having Size children must have Size-1 data.(Size>0)
            template <typename DataType, size_t Size>
            struct Bplus_node final:abstract_node<Bplus_node<DataType,Size>,DataType,Size
                    ,Bplus_data_storage<DataType, Size-1>,m_child_storage<Bplus_node<DataType, Size>, Size>>{
				static_assert(Size >= 2, "A Bplus node must have at least 2 children");

				// Points to the pre node and next node.
				Bplus_node* Pre;
				Bplus_node* Next;

            };

}

namespace ronleeon::heap::node {
	// node classes for binary heap node or d-ary heap node or fibonacci heap node.
}

#endif