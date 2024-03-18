#ifndef RONLEEON_ADT_B_TREE_H
#define RONLEEON_ADT_B_TREE_H


#include "ronleeon/tree/node.h"
#include "ronleeon/tree/abstract_tree.h"

namespace ronleeon::tree{
	template<typename DataType,typename NodeType=node::b_node<DataType>,typename NodePrintTrait = b_node_print_trait<NodeType>>
	class b_tree final:public abstract_b_tree<DataType,NodeType,b_tree<DataType,NodeType,NodePrintTrait>,NodePrintTrait>{

        using basic_type = abstract_b_tree<DataType,NodeType,b_tree<DataType,NodeType,NodePrintTrait>,NodePrintTrait>;

	protected:
		explicit b_tree(std::nullptr_t):basic_type(nullptr){}
    public:
            using node_type = NodeType;
			using node_pointer = NodeType*;
			using node_type_reference = NodeType&;
			using const_node_type = const NodeType;
			using const_node_pointer = const NodeType*;
			using const_node_type_reference = const NodeType&;

			using PrintTrait = typename basic_type::PrintTrait;

        b_tree(const b_tree&) = delete;
        b_tree():b_tree(nullptr){}
        b_tree(b_tree && tree) noexcept :basic_type(std::move(tree)) {}
        static b_tree create_tree_l(std::istream &in=std::cin){
            return abstract_b_tree<DataType,NodeType,b_tree<DataType,NodeType>>::create_tree_l(in);
        }
        static b_tree create_tree_r(std::istream &in=std::cin){
            return abstract_b_tree<DataType,NodeType,b_tree<DataType,NodeType>>::create_tree_r(in);
        }
    };
}

#endif