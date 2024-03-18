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
#include <iostream>

namespace ronleeon {
	namespace tree {
		// using default m_node.
		template<typename DataType,size_t Size,typename NodeType=node::m_node<DataType,Size>,typename NodePrintTrait = m_node_print_trait<NodeType>>
		class m_tree final:public abstract_tree<DataType,Size,NodeType,m_tree<DataType,Size,NodeType,NodePrintTrait>, NodePrintTrait>{
			
			using basic_type=abstract_tree<DataType,Size,NodeType,m_tree<DataType,Size,NodeType, NodePrintTrait>, NodePrintTrait>;

		protected:

			explicit m_tree(std::nullptr_t):abstract_tree<DataType,Size,NodeType
				,m_tree<DataType,Size,NodeType>>(nullptr){}
			
		public:
			using node_type = NodeType;
			using node_pointer = NodeType*;
			using node_type_reference = NodeType&;
			using const_node_type = const NodeType;
			using const_node_pointer = const NodeType*;
			using const_node_type_reference = const NodeType&;

			using PrintTrait = typename basic_type::PrintTrait;
			m_tree(const m_tree&) = delete;
			m_tree():m_tree(nullptr){}
			m_tree(m_tree && tree) noexcept : basic_type(std::move(tree)) {}

            static m_tree create_tree_l(std::istream &in=std::cin){
                return abstract_tree<DataType,Size,NodeType,m_tree<DataType,Size,NodeType>>::create_tree_l(in);
            }
            static m_tree create_tree_r(std::istream &in=std::cin){
                return abstract_tree<DataType,Size,NodeType,m_tree<DataType,Size,NodeType>>::create_tree_r(in);
            }
		};
	}
}
#endif