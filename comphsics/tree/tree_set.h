// provides a set containers implemented by bs_tree.
#ifndef COMPHSICS_ADT_TREE_SET_H
#define COMPHSICS_ADT_TREE_SET_H

#include "comphsics/tree/bs_tree.h"
#include "comphsics/tree/avl_tree.h"
#include "comphsics/tree/rb_tree.h"
#include <functional>
#include <exception>

namespace comphsics {
	namespace tree{

		// Provide Java style Iterator.
		template<typename NodeType,typename ValueType,typename Tree>
		struct tree_set_iterator;
		template<typename NodeType,typename ValueType,typename Tree>
		struct tree_set_iterator<NodeType*,ValueType,Tree>
		{

			typedef NodeType*  type;
			typedef NodeType*& reference;
			typedef NodeType** pointer;
			typedef ValueType value_type;

			type node;
			tree_set_iterator() : node() { }

			explicit
			tree_set_iterator(type x) :node(x) { }

			reference get_node_ptr()const{
				return static_cast<reference>(node);
			}

			ValueType operator*() const 
			{ return node->data;}

			ValueType* operator->() const
			{ return &(node->data); }


			bool has_next(){
				tree_set_iterator Tmp(this->node);
				Tree::tree_set_increment(Tmp);
				return Tmp.is_null()?false:true;
			}

			bool is_null() const {
				return node?false:true;
			}
			

			tree_set_iterator& operator++() 
			{	
				node=Tree::tree_set_increment(node);
				return *this;
			}

			tree_set_iterator operator++(int) 
			{	
				tree_set_iterator Tmp(node);
				node=Tree::tree_set_increment(node);
				return Tmp;
			}

			tree_set_iterator& operator--()
			{
				node=Tree::tree_set_decrement(node);
				return *this;
			}

			tree_set_iterator operator--(int)
			{
				tree_set_iterator Tmp(node);
				node=Tree::tree_set_decrement(node);
				return Tmp;
			}
			
			operator bool()
			{
				return !is_null();
			}

			friend bool operator==(const tree_set_iterator& x, const tree_set_iterator& y)
			{ 
				return x.node=y.node; 
			}	
		};


		// Alternatively, you can implements you own tree , such as bs_tree, avl_tree
		//, and rb_tree.(pure bs_tree is not recommended)

		template <typename NodeValue,typename Compare=std::less<NodeValue>,
			typename Tree=rb_tree<NodeValue,Compare>>
		class tree_set{
		public:
			typedef tree_set_iterator<typename Tree::node_pointer,NodeValue,tree_set> iterator;
		private:
			Tree tree;
		public:
			tree_set():tree(){ 
			}

			tree_set(const tree_set&) = delete;

			tree_set(tree_set&&) = delete;

			tree_set(std::initializer_list<NodeValue> l)
			: tree_set()
			{ 
				insert(l); 
			}


			~tree_set() = default;

			tree_set& operator=(const tree_set&)=delete;

			tree_set& operator=(tree_set&&) = delete;

			size_t size() const {
				return tree.size();
			}

			static typename Tree::node_pointer tree_set_increment(typename Tree::node_pointer value){
				return Tree::increment(value);
			}


			static typename Tree::node_pointer tree_set_decrement(typename Tree::node_pointer value){
				return Tree::decrement(value);
			}


			// Return an iterator of the first element or the last element.
			
			iterator iter() { 
				return iterator(tree.left_most(tree.get_root())); 
			}

			iterator iter_last(){
				return iterator(tree.right_most(tree.get_root())); 
			}

			std::pair<iterator, bool> insert(const NodeValue& x)
			{ 
				std::pair<iterator,bool> Ret;
				auto InsertResult=tree.insert(x);
				if(InsertResult.second){
					Ret.first=iterator(InsertResult.first);
					Ret.second=true;
				}else{
					Ret.first=iterator(nullptr);
					Ret.second=false;
				}
				return Ret;
			}

			std::pair<iterator, bool> insert(NodeValue&& x)
			{ 	
				std::pair<iterator,bool> Ret;
				auto InsertResult=tree.insert(std::move(x));
				if(InsertResult.second){
					Ret.first=iterator(InsertResult.first);
					Ret.second=true;
				}else{
					Ret.first=iterator(nullptr);
					Ret.second=false;
				}
				return Ret; 
			}



			void insert(std::initializer_list<NodeValue> list)
			{ 
				insert(list.begin(), list.end()); 
			}

			template<typename InputIterator>
			void insert(InputIterator first, InputIterator last)
			{ 
				for(auto &It=first;It!=last;++It){
					insert(*It);
				}	
			}


			iterator erase(iterator position)
			{ 
				auto EraseResult=tree.erase(position.get_node_ptr());
				if(EraseResult){
					return iterator(nullptr);
				}else{
					return iterator(EraseResult);
				}
			}
			

			void erase(const NodeValue& x)
			{ 
				tree.erase(x);
			}


			void clear() { tree.destroy(); }



			iterator find(const NodeValue& x)
			{ 
				auto FindResult= tree.find(x); 
				if(FindResult.second){
					return iterator(FindResult.first);
				}else{
					return iterator(nullptr);
				}
			} 

			int count(const NodeValue& x) const
			{ return find(x) == iterator(nullptr) ? 0 : 1; }

			bool contains(const NodeValue& x) const
			{ return find(x) != iterator(nullptr); }



		};

	}
}


#endif