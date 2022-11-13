#ifndef RONLEEON_ADT_BS_TREE_H
#define RONLEEON_ADT_BS_TREE_H

#include "ronleeon/tree/abstract_tree.h"
#include <istream>
namespace ronleeon{
	namespace tree{

		// Guarantee all NodeType data are not equal,otherwise the latter 
		// will be ignored.

		// C++ style compare,not java compare style.
		template<typename DataType,typename Compare=std::less<DataType>,typename NodeType=node::bs_node<DataType>>
		class bs_tree:public abstract_bs_tree<DataType,Compare,NodeType
			,bs_tree<DataType,Compare,NodeType>>{
			using basic_type=abstract_bs_tree<DataType,Compare,NodeType
				,bs_tree<DataType,Compare,NodeType>>;
		
		protected:

			explicit bs_tree(std::nullptr_t):basic_type(nullptr){}

		public:
			bs_tree():bs_tree(nullptr){};
			bs_tree(const bs_tree&)=delete;
			bs_tree(const DataType data[],size_t Size):basic_type(data,Size){}
			bs_tree(bs_tree && tree):basic_type(std::move(tree)) {}
		};

	}
}
#endif