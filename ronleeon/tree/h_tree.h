#ifndef RONLEEON_ADT_H_TREE_H
#define RONLEEON_ADT_H_TREE_H

#include "ronleeon/tree/abstract_tree.h"
#include <istream>
namespace ronleeon{
	namespace tree{
		template<size_t Size>
		struct compute_node_size{
			// for size data,
			// data+(data-1)=2*data-1.
			static constexpr size_t value=2*Size-1;
		};
		// NOTICE:DataType must be comparable.
		// DataType must support '>','<','=','>=','<=' and '+' traits.
		// to support weight, must override '*' operator.
		// specified type can override these operators.
		template<typename DataType,size_t Size,typename NodeType=node::h_node<DataType>,typename NodePrintTrait = b_node_print_trait<NodeType>>
		class h_tree:public abstract_b_tree<DataType,NodeType
			,h_tree<DataType,Size,NodeType,NodePrintTrait>, NodePrintTrait>{
			using basic_type=abstract_b_tree<DataType,NodeType,h_tree<DataType,Size,NodeType,NodePrintTrait>,NodePrintTrait>;
			// prohibit all create functions.
			using basic_type::create_tree_l;
			using basic_type::create_tree_r;
			using basic_type::create_empty_tree;
		
		public:
			using node_type = NodeType;
			using node_pointer = NodeType*;
			using node_type_reference = NodeType&;
			using const_node_type = const NodeType;
			using const_node_pointer = const NodeType*;
			using const_node_type_reference = const NodeType&;

			using PrintTrait = typename basic_type::PrintTrait;
		private:
			// preallocated nodes size within data sizes to be compared.

			node_pointer _ele[compute_node_size<Size>::value];
			// wpl: weighted path length
			long long _wpl;
			
			void shift(){
				if(Size<1){
					return;
				}
				bool _sorted[compute_node_size<Size>::value];
				for(int i=0;i<basic_type::num_of_nodes;i++){
					_sorted[i]=false;
				}
				size_t min_index1=0;
				size_t min_index2=0;
				// min_data1 is always <= min_data2
				DataType* min_data1=&(_ele[0]->data);
				DataType* min_data2=&(_ele[0]->data);
				// first data sign. 
				bool _find=true;
				size_t find_edge=Size;
				while(true) {
					// find two mimimum data.
					_find=true;
					for (int index = 0; index < find_edge; index++) {
						if (_sorted[index] == true) {
							continue;
						} else if (_find) {
							// set both.
							min_index1 = index;
							min_data1 = &(_ele[index]->data);
							min_index2 = index;
							min_data2 = &(_ele[index]->data);
							_find = false;
						}
						if(min_index1==min_index2){
							if (*min_data1 > _ele[index]->data) {
								min_index1 = index;
								min_data1 = &(_ele[index]->data);
							}else{
								min_index2 = index;
								min_data2 = &(_ele[index]->data);
							}
						}else{
							if (*min_data1 > _ele[index]->data) {
								min_index2=min_index1;
								min_data2=min_data1;
								min_index1 = index;
								min_data1 = &(_ele[index]->data);
							}else{
								if(*min_data2> _ele[index]->data){
									min_index2 = index;
									min_data2 = &(_ele[index]->data);
								}
							}
						}
					}
					_find=true;
					_sorted[min_index1] = true;
					_sorted[min_index2]=true;
					
					auto new_node = new NodeType();
					new_node->data = *min_data1 + *min_data2;
					// binary node
					new_node->left_child=_ele[min_index1];
					new_node->right_child=_ele[min_index2];
					_sorted[find_edge]=false;
					_ele[find_edge]=new_node;
					new_node->is_leaf=false;
					_ele[min_index1]->parent=new_node;
					_ele[min_index2]->parent=new_node;
					++new_node->child_size;
					basic_type::shift_height(new_node);
					++find_edge;
					if(find_edge==basic_type::num_of_nodes){
						break;
					}
				}
				basic_type::_root=_ele[basic_type::num_of_nodes-1];
			}

			explicit h_tree(std::nullptr_t):basic_type(nullptr){}

			void compute_code_and_wpl() {
				long long code=0;
				size_t index_bit=0;
				size_t path=0;
				long long wpl=0;
				const_node_pointer node=nullptr;
				const_node_pointer parent=nullptr;
				for(size_t Index=0;Index<Size;++Index){
					code=0;
					index_bit=0;
					path=0;
					const_node_pointer node=_ele[Index];
					const_node_pointer parent=node->parent;
					while(parent){
						if(node==parent->right_child){
							code|=(1<<index_bit);
						}
						++index_bit;
						++path;
						node=parent;
						parent= parent=node->parent;
					}
					_ele[Index]->code=code;
					code=(_ele[Index]->data)*path;
					_ele[Index]->wpl=code;
					wpl+=code;
				}
				_wpl=wpl;
				for(size_t Index=Size;Index< compute_node_size<Size>::value;++Index){
					// non leaves do not have code.
					_ele[Index]->code=0;
					_ele[Index]->wpl=0;
				}
			}
		public:
			h_tree():h_tree(nullptr){}
			h_tree(const h_tree&t)=delete;
			h_tree(const DataType data[]){
				for(size_t Index=0;Index<Size;++Index){	
					_ele[Index]=new NodeType();
					_ele[Index]->data = data[Index];
				}
				for(size_t Index=Size;Index< compute_node_size<Size>::value;++Index){
					_ele[Index]=nullptr;
				}
				shift();
				// compute all codes and wpl(leaves).
				compute_code_and_wpl();
				basic_type::number_of_codes=compute_node_size<Size>::value;
			}

			h_tree(h_tree && tree):basic_type(std::move(tree)) {
				_wpl=tree._wpl;
				_ele=tree._ele;
			}

			std::string to_string()const override {
				return "<-Huffman tree->";
			}
			long long get_huffman_weight(size_t Index) const {
				assert(Index<Size&&"Invalid index!");
				return _ele[Index]->wpl;
			}
			long long get_weighted_path_length()const{
				return _wpl;
			}
			
			long long get_huffman_code(size_t Index)const{
				assert(Index<Size&&"Invalid index!");
				return _ele[Index]->code;
			}
			static h_tree create_huffman_tree(std::istream &in){
				DataType data[Size];
				for(size_t Index=0;Index<Size;++Index){	
					in>>data[Index];
				}
				h_tree<DataType,Size,NodeType> tree(data); 
				return tree;
			}
		};
	}
}
#endif