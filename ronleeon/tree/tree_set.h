// provides a set containers implemented by bs_tree.
#ifndef RONLEEON_ADT_TREE_SET_H
#define RONLEEON_ADT_TREE_SET_H

#include "ronleeon/tree/rb_tree.h"
#include <functional>
#include <exception>
#include <iterator>
#include <utility>

namespace ronleeon::tree{

	template<typename NodeType,typename ValueType,typename Container>
	struct tree_set_iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = const ValueType;
		using pointer           = const ValueType*; 
		using reference         = const ValueType&;  

		const NodeType* node;

		const Container& tree;

		tree_set_iterator(const Container& container) : tree(container), node(nullptr) {}

		explicit
		tree_set_iterator(const NodeType* x,const Container& container) :node(x),  tree(container){ }

		const NodeType* get_node_ptr()const{
			return node;
		}

		reference operator*() const
		{ 
			return node->data;
		}

		pointer operator->() const
		{
			return &(node->data); 
		}


		tree_set_iterator& operator++()
		{
			node=tree.tree_set_increment(node);
			return *this;
		}

		tree_set_iterator operator++(int)
		{
			tree_set_iterator Tmp(node);
			node=tree.tree_set_increment(node);
			return Tmp;
		}

		tree_set_iterator& operator--()
		{
			node=tree.tree_set_decrement(node);
			return *this;
		}

        tree_set_iterator operator--(int)
		{
			tree_set_iterator Tmp(node);
			node=tree.tree_set_decrement(node);
			return Tmp;
		}

		friend bool operator==(const tree_set_iterator& x, const tree_set_iterator& y)
		{
			return x.node==y.node;
		}

		friend bool operator!=(const tree_set_iterator& x, const tree_set_iterator& y)
		{
			return x.node!=y.node;
		}
	};


	// Alternatively, you can implement you own tree , such as bs_tree, avl_tree
	//, and rb_tree.(pure bs_tree is not recommended)

	template <typename NodeValue,typename Compare=std::less<NodeValue>,
		typename Tree=rb_tree<NodeValue,Compare>>
	class tree_set{
	public:
		typedef tree_set_iterator<typename Tree::node_type,NodeValue,tree_set> iterator;
		typedef iterator const_iterator;
	private:
		Tree tree;

		// First and Last children
		typename Tree::const_node_pointer start;

		typename Tree::const_node_pointer last;


		// use in end iterator, implement as `this` ptr.
		typename Tree::const_node_pointer this_end;

	public:
		tree_set(Compare comp_ = Compare{} ):tree(comp_), last(reinterpret_cast<typename Tree::const_node_pointer>(this)), start(reinterpret_cast<typename Tree::const_node_pointer>(this)),this_end(reinterpret_cast<typename Tree::const_node_pointer>(this)){}

		tree_set(const tree_set&) = delete;

		tree_set(tree_set&&) = delete;

		tree_set(std::initializer_list<NodeValue> l,Compare comp_ = Compare{})
		: tree_set(comp_)
		{
			insert(l);
		}


		~tree_set() = default;

		tree_set& operator=(const tree_set&)=delete;

		tree_set& operator=(tree_set&&) = delete;

		size_t size() const {
			return tree.size();
		}

		typename Tree::const_node_pointer tree_set_increment(typename Tree::const_node_pointer value) const {
			if(value == this_end){
				return start;
			}
			auto Next = Tree::increment(value);
			if(!Next){
				return this_end;
			}
			return Next;
		}


		typename Tree::const_node_pointer tree_set_decrement(typename Tree::const_node_pointer value) const {
			if(value == this_end){
				return last;
			}
			auto Pre = Tree::decrement(value);
			if(!Pre){
				return this_end;
			}
			return Pre;
		}


		std::pair<iterator, bool> insert(const NodeValue& x)
		{
			auto InsertResult=tree.insert(x);
			if(InsertResult.second){
				start = tree.start();
				last = tree.last();
				return std::make_pair<iterator, bool>(iterator(InsertResult.first, *this), true);
			}else{
				// end iterator
				return std::make_pair<iterator, bool>(end(),false);
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


		iterator erase(iterator position)
		{
			auto EraseResult=tree.erase(position.get_node_ptr());
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


		void erase(const NodeValue& x)
		{
			tree.erase(x);
		}


		void clear() { 
			tree.destroy();
			start = last = this_end;
		}

		iterator find(const NodeValue& x)
		{
			auto FindResult= tree.find(x);
			if(FindResult.second){
				return iterator(FindResult.first, *this);
			}else{
				return end();
			}
		}

		int count(const NodeValue& x) const
		{ return find(x) == end()? 0 : 1; }

		bool contains(const NodeValue& x) const
		{ return find(x) != end(); }



		iterator begin(){
			return iterator(start, *this);
		}
		iterator end(){
			return const_iterator(this_end, *this);
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



#endif