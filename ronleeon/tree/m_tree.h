#ifndef RONLEEON_ADT_M_TREE_H
#define RONLEEON_ADT_M_TREE_H

#include "ronleeon/tree/abstract_tree.h"
#include <cassert>
#include <ostream>
#include <istream>
#include <string>
#include <utility>
#include <stack>
#include <queue>
#include <cstring>


namespace ronleeon {
	namespace tree {
		// using default m_node.
		template<typename DataType,size_t Size,typename NodeType=node::m_node<DataType,Size>>
		class m_tree final:public abstract_tree<DataType,Size,NodeType,m_tree<DataType,Size,NodeType>>{
		
		protected:

			explicit m_tree(std::nullptr_t):abstract_tree<DataType,Size,NodeType
				,m_tree<DataType,Size,NodeType>>(nullptr){}
			
		public:
			m_tree(const m_tree&) = delete;
			m_tree():m_tree(nullptr){}
			m_tree(m_tree && tree):abstract_tree<DataType,Size,NodeType
				,m_tree<DataType,Size,NodeType>>(std::move(tree)) {}
		};
	}
}
#endif