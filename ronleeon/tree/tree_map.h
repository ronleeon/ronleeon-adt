// provides a map containers implemented by bs_tree.
#ifndef RONLEEON_ADT_TREE_MAP_H
#define RONLEEON_ADT_TREE_MAP_H

#include "ronleeon/tree/rb_tree.h"
#include <cstdint>
#include <functional>
#include <exception>
#include <type_traits>
#include <utility>
#include "ronleeon/tree/bs_tree.h"


namespace ronleeon{
	namespace tree{

		// Compare partially template function.
		template<typename Type>
		struct less
		{
			bool
			operator()(const Type& x, const Type& y) const
			{ return x < y; }
		};

		// provide default NodeType for tree.
		template <typename Key,typename Value>
		struct pair{
			pair(const Key& k,const Value& v):first(k),second(v){
			}
			pair() = default;
			pair(Key&& k,Value&& v):first(std::move(k)),second(std::move(v)){
			}
			pair(pair&& pair):first(std::move(pair.first)),second(std::move(pair.second)){}

			pair(const pair& pair):first(pair.first),second(pair.second){}

			pair& operator=(const pair& pair){
				first=pair.first;
				second=pair.second;
				return *this;
			}
			
			pair& operator=(pair&& pair){
				first=std::move(pair.first);
				second=std::move(pair.second);
				return *this;
			}

			Key first;
			Value second;

			// provided default tree::pair input and output.
			friend std::ostream& operator<<(std::ostream& out, const pair<Key,Value> Pair) {
				out<<'('<<Pair.first<<','<<Pair.second<<')';
				return out;
			}

			friend std::istream& operator<<(std::istream& in, const pair<Key,Value>& Pair) {
				in>>Pair.first;
				in>>Pair.second;
				return in;
			}
		};

		template<typename Key,typename Value>
		struct less<pair<Key,Value>>
		{
			bool
			operator()(const pair<Key,Value>& x, const pair<Key,Value>& y) const
			{ return x.first < y.first; }
		};
		
		
		template<typename NodeType,typename ValueType,typename Tree>
		struct tree_map_iterator
		{

			using iterator_category = std::forward_iterator_tag;
			using difference_type   = std::ptrdiff_t;
			using value_type        = const ValueType;
			using pointer           = const ValueType*; 
			using reference         = const ValueType&;  

			const NodeType* node;
			const Tree& tree;

			tree_map_iterator(const Tree& container) : tree(container), node(nullptr) { }

			explicit
			tree_map_iterator(const NodeType* x,const Tree& container) :tree(container),node(x) { }

			const NodeType* get_node_ptr()const{
				return node;
			}

			reference operator*() const 
			{ return node->data;}

			pointer operator->() const
			{ return &(node->data); }



			tree_map_iterator& operator++() 
			{	
				node=tree.tree_map_increment(node);
				return *this;
			}

			tree_map_iterator operator++(int) 
			{	
				tree_map_iterator Tmp(node);
				node=tree.tree_map_increment(node);
				return Tmp;
			}

			tree_map_iterator& operator--()
			{
				node=tree.tree_map_decrement(node);
				return *this;
			}

			tree_map_iterator operator--(int)
			{
				tree_map_iterator Tmp(node);
				node=tree.tree_map_decrement(node);
				return Tmp;
			}
			

			friend bool operator==(const tree_map_iterator& x, const tree_map_iterator& y)
			{ 
				return x.node==y.node; 
			}

			friend bool operator!=(const tree_map_iterator& x, const tree_map_iterator& y)
			{ 
				return x.node!=y.node; 
			}		
		};


		// map have a key and value, so we use tree::pair for a node value.
		// Alternatively, you can implement you own tree , such as bs_tree, avl_tree
		//, and rb_tree.(pure bs_tree is not recommended)

		template <typename Key,typename Value,typename Compare=tree::less<tree::pair<Key,Value>>,
			typename Tree=rb_tree<tree::pair<Key,Value>,Compare>>
		class tree_map{

			using NodeValue=tree::pair<Key,Value>;
		public:
			typedef tree_map_iterator<typename Tree::node_type,NodeValue,tree_map> iterator;
			typedef iterator const_iterator;
		private:


			// First and Last children
			typename Tree::const_node_pointer start;

			typename Tree::const_node_pointer last;
			// use in end iterator, implement as `this` ptr.
			typename Tree::const_node_pointer this_end;

			Tree tree;
		public:
			tree_map(Compare comp_ = Compare{} ):tree(comp_),last(reinterpret_cast<typename Tree::const_node_pointer>(this)), start(reinterpret_cast<typename Tree::const_node_pointer>(this)),this_end(reinterpret_cast<typename Tree::const_node_pointer>(this)){}

			tree_map(const tree_map&) = delete;

			tree_map(tree_map&&) = delete;

			tree_map(std::initializer_list<NodeValue> l,Compare comp_ = Compare{} )
			: tree_map(comp_)
			{ 
				insert(l); 
			}


			~tree_map() = default;

			tree_map& operator=(const tree_map&)=delete;

			tree_map& operator=(tree_map&&) = delete;

			size_t size() const {
				return tree.size();
			}

			typename Tree::const_node_pointer tree_map_increment(typename Tree::const_node_pointer value) const {
				auto Next = Tree::increment(value);
				if(!Next){
					return this_end;
				}
				return Next;
			}


			typename Tree::const_node_pointer tree_map_decrement(typename Tree::const_node_pointer value) const {
				if(value == reinterpret_cast<typename Tree::const_node_pointer>(this)){
					return last;
				}
				auto Pre = Tree::decrement(value);
				if(!Pre){
					return this_end;
				}
				return Pre;
			}

			Value&
			operator[](const Key& k){
				const auto Find=tree.find(NodeValue(k,Value()));// construct an empty Value
				if(Find.second){
					return Find.first->data.second;
				}
				throw "Invalid Key";
			}

			Value&
			operator[](Key&& k)
			{
				const auto Find=tree.find(NodeValue(k,Value()));
				if(Find.second){
					return Find.first->data.second;
				}
				throw "Invalid Key";
			}

			Value& at(const Key& k){
				const auto Find=tree.find(NodeValue(k,Value()));
				if(Find.second){
					return Find.first->data.second;
				}
				throw "Invalid Key";
			}

			Value& at(Key&& k)
			{
				const auto Find=tree.find(NodeValue(k,Value()));
				if(Find.second){
					return Find.first->data.second;
				}
				throw "Invalid Key";
			}

			std::pair<iterator, bool> insert(const NodeValue& x)
			{ 
				std::pair<iterator,bool> Ret;
				if(auto InsertResult=tree.insert(x); InsertResult.second){
					start = tree.start();
					last = tree.last();
					return std::make_pair(iterator(InsertResult.first, *this), true);
				}else{
					return std::make_pair(end(), false);
				}
			}

			std::pair<iterator,bool> insert(const Key& k,const Value &v)
			{ 
				if(auto InsertResult=tree.insert(NodeValue(k,v)); InsertResult.second){
					start = tree.start();
					last = tree.last();
					return std::make_pair(iterator(InsertResult.first, *this), true);
				}else{
					return std::make_pair(end(), false);
				}
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

			iterator insert_or_assign(const Key& k, const Value& v){
				if(auto InsertResult=tree.insert(NodeValue(k,v)); InsertResult.second){
					start = tree.start();
					last = tree.last();
					return iterator(InsertResult.first, *this);
				}else{
					// HACK: insert before delete
					erase(k);
					InsertResult = tree.insert(NodeValue(k,v));
					start = tree.start();
					last = tree.last();
					return iterator(InsertResult.first, *this);
				}
			}

			iterator erase(iterator position)
			{ 
				auto Node = position.get_node();
				auto EraseResult=tree.erase(Node);
				if(EraseResult){
					return end();
				}else{
					start = tree.start();
					last = tree.last();
					if(!start){
						start = this_end;
					}
					if(!last){
						last = this_end;
					}
					return iterator(EraseResult, *this);
				}
			}
			

			void erase(const Key& x)
			{
				tree.erase(NodeValue(x,Value()));
			}


			void clear() { 
				tree.destroy();
				start = last = this_end;
			}



			const_iterator find(const Key& x)
			{ 
				auto FindResult= tree.find(NodeValue(x,Value())); 
				if(FindResult.second){
					return iterator(FindResult.first, *this);
				}else{
					return end();
				}
			} 


			int count(const Key& x) const
			{ return find(x) == end() ? 0 : 1; }

			bool contains(const Key& x) const
			{ return find(x) != end(); }


		iterator begin(){
			return iterator(start, *this);
		}
		iterator end(){
			return iterator(this_end, *this);
		}

		const_iterator begin() const {
			return const_iterator(start);
		}
		const_iterator end() const {
			return const_iterator(this_end, *this);
		}

		const_iterator cbegin() const {
			return const_iterator(start);
		}
		const_iterator cend() const {
			return const_iterator(this_end, *this);
		}

		std::reverse_iterator<iterator> rbegin() {
			return std::reverse_iterator(end());
		}

		std::reverse_iterator<iterator> rend() {
			return std::reverse_iterator(begin());
		}

		std::reverse_iterator<const_iterator> rbegin() const {
			return std::reverse_iterator(end());
		}

		std::reverse_iterator<const_iterator> rend() const {
			return std::reverse_iterator(begin());
		}
		std::reverse_iterator<const_iterator> crbegin() {
			return std::reverse_iterator(cend());
		}

		std::reverse_iterator<const_iterator> crend() {
			return std::reverse_iterator(cbegin());
		}

		};

	}
}


#endif