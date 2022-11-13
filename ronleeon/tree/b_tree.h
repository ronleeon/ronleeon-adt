#ifndef RONLEEON_ADT_B_TREE_H
#define RONLEEON_ADT_B_TREE_H


#include "ronleeon/tree/node.h"
#include "ronleeon/tree/abstract_tree.h"

namespace ronleeon{
	namespace tree{
		template<typename DataType,typename NodeType=node::b_node<DataType>>
		class b_tree final:public abstract_b_tree<DataType,NodeType,b_tree<DataType,NodeType>>{

		protected:

			explicit b_tree(std::nullptr_t):abstract_b_tree<DataType,NodeType
				,b_tree<DataType,NodeType>>(nullptr){}
			   
		public:
			b_tree(const b_tree&) = delete;
			b_tree():b_tree(nullptr){}
			b_tree(b_tree && tree):abstract_b_tree<DataType,NodeType
				,b_tree<DataType,NodeType>>(std::move(tree)) {}
		};
	}
}

#endif