#ifndef RONLEEON_ADT_BS_TREE_H
#define RONLEEON_ADT_BS_TREE_H

#include "ronleeon/tree/abstract_tree.h"
#include <istream>
namespace ronleeon{
	namespace tree{

		// Guarantee all NodeType data are not equal,otherwise the latter 
		// will be ignored.

		// C++ style compare,not java compare style.
		template<typename DataType,typename Compare=std::less<DataType>,typename NodeType=node::bs_node<DataType>,typename NodePrintTrait = b_node_print_trait<NodeType>>
		class bs_tree:public abstract_bs_tree<DataType,Compare,NodeType
			,bs_tree<DataType,Compare,NodeType,NodePrintTrait>, NodePrintTrait>{
			using basic_type=abstract_bs_tree<DataType,Compare,NodeType
				,bs_tree<DataType,Compare,NodeType,NodePrintTrait>, NodePrintTrait>;
		
		protected:

			explicit bs_tree(std::nullptr_t,Compare comp_ = Compare{} ):basic_type(nullptr,comp_){}

		public:
			using node_type = NodeType;
			using node_pointer = NodeType*;
			using node_type_reference = NodeType&;
			using const_node_type = const NodeType;
			using const_node_pointer = const NodeType*;
			using const_node_type_reference = const NodeType&;

			using PrintTrait = typename basic_type::PrintTrait;
			bs_tree(Compare comp_ = Compare{} ):bs_tree(nullptr,comp_){};
			bs_tree(const bs_tree&)=delete;
			bs_tree(const DataType data[],size_t Size,Compare comp_ = Compare{} ):basic_type(data,Size,comp_){}
			bs_tree(bs_tree && tree):basic_type(std::move(tree)) {}
		};

	}
}
#endif