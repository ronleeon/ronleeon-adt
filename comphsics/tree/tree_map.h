// provides a map containers implemented by bs_tree.
#ifndef COMPHSICS_ADT_TREE_MAP_H
#define COMPHSICS_ADT_TREE_MAP_H

#include "comphsics/tree/bs_tree.h"
#include "comphsics/tree/avl_tree.h"
#include "comphsics/tree/rb_tree.h"
#include <functional>
#include <exception>


namespace comphsics{
	namespace tree{

		// Compare partially template function.
		template<typename Type>
		struct less
		{
			bool
			operator()(const Type& x, const Type& y) const
			{ return x < y; }
		};

		// provid default NodeType for tree.
		template <typename Key,typename Value>
		struct pair{
			pair(const Key& k,const Value& v):first(k),second(v){
			}
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
		
		
		// Provide Java style Iterator.
		template<typename NodeType,typename ValueType,typename Tree>
		struct tree_map_iterator;
		template<typename NodeType,typename ValueType,typename Tree>
		struct tree_map_iterator<NodeType*,ValueType,Tree>
		{

			typedef NodeType*  type;
			typedef NodeType*& reference;
			typedef NodeType** pointer;
			typedef ValueType value_type;

			type node;
			tree_map_iterator() : node() { }

			explicit
			tree_map_iterator(type x) :node(x) { }

			reference get_node_ptr()const{
				return static_cast<reference>(node);
			}

			ValueType operator*() const 
			{ return node->data;}

			ValueType* operator->() const
			{ return &(node->data); }


			bool has_next(){
				tree_map_iterator Tmp(this->node);
				Tree::tree_map_increment(Tmp);
				return Tmp.is_null()?false:true;
			}

			bool is_null() const {
				return node?false:true;
			}
			

			tree_map_iterator& operator++() 
			{	
				node=Tree::tree_map_increment(node);
				return *this;
			}

			tree_map_iterator operator++(int) 
			{	
				tree_map_iterator Tmp(node);
				node=Tree::tree_map_increment(node);
				return Tmp;
			}

			tree_map_iterator& operator--()
			{
				node=Tree::tree_map_decrement(node);
				return *this;
			}

			tree_map_iterator operator--(int)
			{
				tree_map_iterator Tmp(node);
				node=Tree::tree_map_decrement(node);
				return Tmp;
			}
			
			operator bool()
			{
				return !is_null();
			}

			friend bool operator==(const tree_map_iterator& x, const tree_map_iterator& y)
			{ 
				return x.node=y.node; 
			}	
		};


		// map have a key and value, so we use tree::pair for a node value.
		// Alternatively, you can implements you own tree , such as bs_tree, avl_tree
		//, and rb_tree.(pure bs_tree is not recommended)

		template <typename Key,typename Value,typename NodeValue=tree::pair<Key,Value>,typename Compare=tree::less<NodeValue>,
			typename Tree=rb_tree<NodeValue,Compare>>
		class tree_map{
		public:
			typedef tree_map_iterator<typename Tree::node_pointer,NodeValue,tree_map> iterator;
		private:
			Tree tree;
		public:
			tree_map():tree(){ 
			}

			tree_map(const tree_map&) = delete;

			tree_map(tree_map&&) = delete;

			tree_map(std::initializer_list<NodeValue> l)
			: tree_map()
			{ 
				insert(l); 
			}


			~tree_map() = default;

			tree_map& operator=(const tree_map&)=delete;

			tree_map& operator=(tree_map&&) = delete;

			size_t size() const {
				return tree.size();
			}

			static typename Tree::node_pointer tree_map_increment(typename Tree::node_pointer value){
				return Tree::increment(value);
			}


			static typename Tree::node_pointer tree_map_decrement(typename Tree::node_pointer value){
				return Tree::decrement(value);
			}


			// Return an iterator of the first element or the last element.
			
			iterator iter() { 
				return iterator(tree.left_most(tree.get_root())); 
			}

			iterator iter_last(){
				return iterator(tree.right_most(tree.get_root())); 
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

			std::pair<iterator,bool> insert(const Key& k,const Value &v)
			{ 
				std::pair<iterator,bool> Ret;
				auto InsertResult=tree.insert(NodeValue(k,v));
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


			std::pair<iterator, bool> insert_or_assign(const Key k, const Value v){
				std::pair<iterator,bool> Ret;
				auto InsertResult=tree.insert(NodeValue(k,v));
				if(InsertResult.second){
					Ret.second=true;
					Ret.first=iterator(InsertResult.first);
				}else{
					Ret.first=iterator(InsertResult.first);
					InsertResult.first->data=NodeValue(k,v);//assign
					Ret.second=false;
				}
				return Ret; 
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
			

			void erase(const Key& x)
			{ 
				tree.erase(NodeValue(x,Value()));
			}


			void clear() { tree.destroy(); }



			iterator find(const Key& x)
			{ 
				auto FindResult= tree.find(NodeValue(x,Value())); 
				if(FindResult.second){
					return iterator(FindResult.first);
				}else{
					return iterator(nullptr);
				}
			} 

			iterator find(Key&& x)
			{ 
				auto FindResult= tree.find(NodeValue(std::move(x),Value())); 
				if(FindResult.second){
					return iterator(FindResult.first);
				}else{
					return iterator(nullptr);
				}
			} 

			int count(const Key& x) const
			{ return find(x) == iterator(nullptr) ? 0 : 1; }

			bool contains(const Key& x) const
			{ return find(x) != iterator(nullptr); }



		};

	}
}


#endif