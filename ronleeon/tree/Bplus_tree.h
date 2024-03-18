#ifndef RONLEEON_ADT_Bplus_TREE_H
#define RONLEEON_ADT_Bplus_TREE_H


#include "ronleeon/tree/node.h"
#include "ronleeon/tree/abstract_tree.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple> 
#include <cmath>

/// @file The file describes two kinds of B+ tree implementation.
/// @details 
///    there are various specifications about how B+ tree should work,
///         1. B+tree key/child number, whether key number should be the same as the child number or key number is one small than its child number;
///         2. the internal node key stores the min data or max data of one of its child as the index;
///         3. the internal node key stores the index of its left child or right child.
/// The file implements: the key number is one small than its child number.
/// The file implements: the internal node key stores the min data.
/// The file implements: the internal node key stores the index of its right child.
/// This B+ tree internal node stores [0,...,n] indices, n < ceil(m/2) - 1, each index i store the minimum data of its child i+1.
/// By the above specifications,  those properties must be hold.
/// 1. for each internal node, k is its key array, c is its child array
///      min(c[i].k)<k[i]=min(c[i+1].k) = c[i+1].k[0] < c[i+1].k[j](j>)
/// @note insertion tricky:
///    if the inserted key can be inserted into a left node last position(push back) or a right node first position.
///    we always insert the key into the left node last position, because this will need not be update the parent index.
/// @note it may be confusing that unlike B tree, @see B_tree_Cormen, here does not implement this kind of tree,(m,2m) children,
///       this kinds of tree facilitates the insertion and deletion, it just needs one traversal(mostly),
///      but in B+tree, because internal nodes store indices, if an index is to be erased, we still need one more traversal to update the parent index.
namespace ronleeon::tree{
    template<typename NodeType>
    class Bplus_node_print_trait{
    public:
        static void print_visiting_node(const NodeType* t, std::ostream& os){
            os<< "visiting node:[";
            if (!t) {
                os << "null]";
                return;
            }
            else {
                size_t Index = 0;
                for(auto It = t->data_begin(); It != t->data_end();++It){
                    if(Index == t->data_size){
                        break;
                    }
                    if(It!=t->data_begin()){
                        os<<",";
                    }
                    ++Index;
                    os<<*It;
                }
            }
            os << "]  ";
            if (t->is_leaf) {
                os << "Leaf";
            }
            else {
                os << "Node";
            }
        }
    };

    // m-order B+ tree definition by Kruth 
    // node child number except root node/leaf nodes is in [ceil(m/2.0), m]
    // key number is in [ceil(m/2.0) - 1, m - 1]
    // Thus we can maintain these properties when merging and splitting,
    // Root child number is in [2,m]
    // All leaves are in the same level.
	template<typename DataType,size_t Size, typename Compare = std::less<DataType>, typename NodeType=node::Bplus_node<DataType, Size>
        , typename NodePrintTrait = Bplus_node_print_trait<NodeType>>
	class Bplus_tree_Kruth final:public abstract_tree<DataType,Size,NodeType,Bplus_tree_Kruth<DataType,Size,Compare,NodeType,NodePrintTrait>, NodePrintTrait>{
    
    private:    
        // minimum size 3.
        // child [2,3]
        // key [1,2]
        static_assert(Size >= 3);

        size_t height = 0;// Tree height.

        using basic_type=abstract_tree<DataType,Size,NodeType,Bplus_tree_Kruth<DataType,Size,Compare,NodeType,NodePrintTrait>,NodePrintTrait>;
        // prohibit all create functions.
        using basic_type::create_tree_l;
        using basic_type::create_tree_r;
        using basic_type::shift_height;
    public:
        using node_type = NodeType;
        using node_pointer = NodeType*;
        using node_type_reference = NodeType&;
        using const_node_type = const NodeType;
        using const_node_pointer = const NodeType*;
        using const_node_type_reference = const NodeType&;

        using PrintTrait = typename basic_type::PrintTrait;

    private:

        explicit Bplus_tree_Kruth(std::nullptr_t, Compare comp_ = Compare{}):basic_type(nullptr), comp(comp_){}

        Compare comp;

        /**
         * @brief get the left most node of the tree.
         * 
         * @param node 
         * @return the left most node
         */
        static const_node_pointer left_most(const_node_pointer node){
            if(!node){
                return nullptr;
            }
            const_node_pointer tmp=node;
            // see node.h child_begin() and child_end()
            while(tmp->child_size != 0){
                tmp = *(tmp->child_begin());
            }
            return tmp;
        }

        /**
         * @brief get the right most node of the tree.
         * 
         * @param node 
         * @return the right most node
         */
        static const_node_pointer right_most(const_node_pointer node) {
            if(!node){
                return nullptr;
            }
            const_node_pointer tmp=node;
            // cannot use child_rend() here
            while(tmp->child_size != 0){
                tmp = tmp->children[tmp->child_size-1];
            }
            return tmp;
        }

        
        // Find a data in one node.
        // Using binary search.
        //  the first offset denotes the position offset(and the original data will be moved to the right)
        // Otherwise, return the offset , the second is true.
        // Notice that : offset may be the size which means Data should be pushed back.
        std::pair<size_t,bool> find_in_node(const_node_pointer node,const DataType& Data) const {
            if(!node){
                return {0,false};
            }
            if(node->data_size == 0){
                return {0,false};
            }
            size_t Left = 0;
            size_t Right = node->data_size - 1; // Right >= Left
            size_t Middle = 0;
            while(Left<=Right){
                Middle = Left + (Right - Left) / 2; // avoid overflow
                if(comp(node->data[Middle],Data)){
                    // overflow.
                    if(Middle == std::numeric_limits<size_t>::max()){
                        break;
                    }
                    Left = Middle + 1;
                }else if(comp(Data,node->data[Middle])){
                    if(Middle == std::numeric_limits<size_t>::min()){
                        break;
                    }
                    Right = Middle - 1;
                }else{
                    return {Middle, true};
                }
            }
            if(comp(node->data[Middle],Data)){
                return {Middle + 1,false};
            }else {
                // comp(Data,data[Middle]) == true.
                return {Middle,false};
            }
        }

        /**
         * @brief 
         * if the node is not full, insert the data to the node
         */
        void insert_not_full(node_pointer node, const DataType& Data, size_t InsertedPosition,node_pointer LChild, node_pointer RChild){
            // if node is full,do nothing.
            if(!node  || InsertedPosition < 0 || InsertedPosition > node->data_size || node->data_size >= Size - 1){
                return;
            }
            // If the node with extra data would not be full, just insert the data.
            // Avoiding -1. Each index + 1
            for(size_t Index = node->data_size; Index > InsertedPosition; --Index){
                size_t ActualIndex = Index - 1;
                node->children[ActualIndex + 2]= node->children[ActualIndex + 1];
                node->data[ActualIndex + 1] = node->data[ActualIndex];
            }
            // insert the data.
            node->data[InsertedPosition] = Data;
            node->children[InsertedPosition + 1] = RChild;
            if(RChild){
                RChild->parent = node;
            }
            node->children[InsertedPosition] = LChild;
            if(LChild){
                LChild->parent = node;
            }
            ++node->data_size;
            if(!node->is_leaf){
                ++node->child_size;
            }
        }

        /**
         * @brief split the full node with m - 1 keys with one inserted data into two nodes: the left node has ceil(m/2.0) - 1 keys
         * , the right node has m - ceil(m/2.0) keys,and one middle data(merged into the node parent).
         * @pre                                                                                                                                                                                            
         * 1:an internal node must be a full node(it has m children and m - 1 keys).
         * 2:an leaf node must be a full leaf(it has non children and m - 1keys).
         * 3: Data must be the splitting result the InsertedPosition-th child of this node, thus the splitting result is the tuple (left, right, middle data)
         *      which is also the returned type of splitNode
         * 4:if node is leaf, LChild and RChild is null otherwise they cannot be null.
         * 5:the original InsertedPosition-th child of node now was splitted before calling splitNode, so the original children[InsertedPosition] must be set to
         *    null before calling this function, as the function would override it.
         * 6:like 5, child_size of node must be updated before calling this function as it lost InsertedPosition-th child.
         */
        [[nodiscard("allocate a new node")]] std::tuple<node_pointer, node_pointer, DataType> insert_full(node_pointer node, 
            const DataType& Data, size_t InsertedPosition,node_pointer LChild, node_pointer RChild){
            const int UpperCeil = std::ceil(Size / 2.0);
            // if node cannot be splitted, do nothing.
            if(!node  || InsertedPosition < 0 || InsertedPosition > node->data_size || node->data_size != Size - 1){
                return {nullptr, nullptr,DataType{}};
            }
            // else, split the node.
            // node->data_size == Size - 1.
            // range: [0,...,ceil(Size/2) - 2] [ceil(Size/2) - 1] [ceil(Size/2),... Size - 1]
            // Size >= 3 (at least 3 keys to be splitted)
            // Compute the middle position.
            size_t Middle = UpperCeil - 1;// >= 1
            node_pointer RightNode = new node_type();
            ++basic_type::num_of_nodes;
            // firstly fill RightNode.
            // RightNode has Size - Middle keys.
            // Avoiding -1. ProcessIndex1InNode is the right of processing node.
            size_t ProcessIndex1InNode = Size - 1;
            bool allocateRChild = false;
            // Avoiding -1. Each index + 1
            for(size_t Index = Size - Middle - 1;Index > 0;--Index){
                size_t ActualIndex = Index - 1;
                // InsertedPosition may be the size of node(that means appending to the last).
                if(ProcessIndex1InNode == InsertedPosition){
                    RightNode->data[ActualIndex] = Data;
                    if(RChild){
                        RightNode->children[ActualIndex + 1]  = RChild;
                        RChild->parent = RightNode;
                    }
                    allocateRChild = true;
                    InsertedPosition = Size; // set it to the max, so that this branch will not be executed again.
                }else{
                    RightNode->data[ActualIndex] = node->data[ProcessIndex1InNode - 1];
                    if(allocateRChild){
                        if(LChild){
                            RightNode->children[ActualIndex + 1]  = LChild;
                            LChild->parent = RightNode;
                        }
                        allocateRChild = false;
                    }else{
                        if(node->children[ProcessIndex1InNode]){
                            RightNode->children[ActualIndex + 1] = node->children[ProcessIndex1InNode];
                            node->children[ProcessIndex1InNode]->parent = RightNode;
                        }
                    }
                    --ProcessIndex1InNode;
                }
            }
            // OK, now we process RightNode[0], because Child number = Key number + 1
            if(allocateRChild){
                if(LChild){
                    RightNode->children[0]  = LChild;
                    LChild->parent = RightNode;
                }
                allocateRChild = false;
            }else{
                if(node->children[ProcessIndex1InNode]){
                    RightNode->children[0] = node->children[ProcessIndex1InNode];
                    node->children[ProcessIndex1InNode]->parent = RightNode;
                }
            }
            // Get the middle key.
            DataType PopData = Data;
            if(ProcessIndex1InNode == InsertedPosition){
                InsertedPosition = Size;
            }else{
                PopData = node->data[ProcessIndex1InNode - 1];
                --ProcessIndex1InNode;
            }
            // new LeftNode key size is ceil(Size/2) - 1
            for(size_t Index = Middle;Index > 0;--Index){
                size_t ActualIndex = Index - 1;
                if(ProcessIndex1InNode == InsertedPosition){
                    node->data[ActualIndex] = Data;
                    if(RChild){
                        node->children[ActualIndex + 1]  = RChild;
                        RChild->parent = node;
                    }else{
                        node->children[ActualIndex + 1] = nullptr;
                    }
                    allocateRChild = true;
                    InsertedPosition = Size; 
                }else{
                    node->data[ActualIndex] = node->data[ProcessIndex1InNode - 1];
                    // allocateRChild may be true when the Data is the middle.
                    if(allocateRChild){
                        if(LChild){
                            node->children[ActualIndex + 1]  = LChild;
                            LChild->parent = node;
                        }else{
                            node->children[ActualIndex + 1] = nullptr;
                        }
                        allocateRChild = false;
                    }else{
                        if(node->children[ProcessIndex1InNode]){
                            node->children[ActualIndex + 1]  = node->children[ProcessIndex1InNode];
                            node->children[ProcessIndex1InNode]->parent = node;
                        }else{
                            node->children[ActualIndex + 1] = nullptr;
                        }
                    }
                    --ProcessIndex1InNode;
                }
            }
            // OK, now we process LightNode[0], because Child number = Key number + 1
            if(allocateRChild){
                if(LChild){
                    node->children[0]  = LChild;
                    LChild->parent = node;
                }else{
                    node->children[0] = nullptr;
                }
                allocateRChild = false;
            }else{
                if(node->children[ProcessIndex1InNode]){
                    node->children[0] = node->children[ProcessIndex1InNode];
                    node->children[ProcessIndex1InNode]->parent = node;// actually ProcessIndex1InNode is 1 here, because we have already dealt with the whole thing.
                }else{
                    node->children[0] = nullptr;
                }
            }
            // update some fields.
            if(node->is_leaf){
                RightNode->is_leaf = true;
                RightNode->child_size = 0;
            }else{
                node->child_size =  Middle + 1;// node as new left node.
                RightNode->is_leaf = false;
                RightNode->child_size = Size - UpperCeil + 1;
            }
            RightNode->data_size = Size - UpperCeil;
            node->data_size = Middle;
            // free the extra children avoiding double free.
            for(size_t Index = node->child_size; Index < Size; ++Index){
                node->children[Index] = nullptr;
            }
            return {node, RightNode, PopData};
        }

        /**
         * @brief 
         * In B tree deletion, delete the data directly.
         * @param child the child replacing the deleted position child.
         * @param node the node which is to be processed.
         * @param ErasePosition the position of the data to be deleted in node. 
         */
        void erase_directly(node_pointer node, size_t ErasePosition, node_pointer child){
            if(!node || ErasePosition < 0 || ErasePosition >= node->data_size){
                return;
            }
            for(size_t Index = ErasePosition; Index < node->data_size - 1; ++Index){
                if(Index == ErasePosition){
                    node->children[Index] = child;
                    if(child){
                        child->parent = node;
                    }
                }else{
                    node->children[Index]= node->children[Index + 1];
                }
                node->data[Index] = node->data[Index + 1];
            }
            // process last child
            if(ErasePosition == node->data_size - 1){
                node->children[node->data_size - 1] = child;
                if(child){
                    child->parent = node;
                }
            }else{
                node->children[node->data_size - 1] = node->children[node->data_size];
            }
            node->children[node->data_size] = nullptr;
            --node->data_size;
            if(!node->is_leaf){
                --node->child_size;
            }
        }
        /**
         * @brief 
         * In B tree deletion, rotate left, this will rotate the right to left node.
         * @param node the node which is to be processed.
         * @param RotatePosition the position of the data in which it will have two children(left and right)
         */
        void rotate_left(node_pointer node, size_t RotatePosition){
            size_t UpperCeil = std::ceil(Size / 2.0);
            if(!node || RotatePosition < 0 || RotatePosition >= node->data_size || node->is_leaf){
                return;
            }
            auto LChild = node->children[RotatePosition];
            auto RChild = node->children[RotatePosition + 1];
            if(!LChild || !RChild){
                return;
            }
            if(LChild->data_size >= Size -1){
                // cannot rotate
                return;
            }
            if(RChild->data_size <= UpperCeil - 1){
                // cannot rotate
                return;
            }
            // LChild adds a new data and a new child.
            LChild->data[LChild->data_size] = node->data[RotatePosition];
            LChild->children[LChild->child_size + 1] = RChild->children[0];
            if(RChild->children[0]){
                RChild->children[0]->parent = LChild;
            }
            node->data[RotatePosition] = RChild->data[0];
            // RChild deletes the first data.
            for(size_t Index = 0; Index < RChild->data_size - 1; ++Index){
                RChild->children[Index] = RChild->children[Index + 1];
                RChild->data[Index] = RChild->data[Index + 1];
            }
            RChild->children[RChild->data_size-1] = RChild->children[RChild->data_size];
            ++LChild->data_size;
            --RChild->data_size;
            if(!LChild->is_leaf){
                ++LChild->child_size;
            }
            if(!RChild->is_leaf){
                --RChild->child_size;
            }
        }

        /**
         * @brief 
         * In B tree deletion, rotate right, this will rotate the left to right node.
         * @param node the node which is to be processed.
         * @param RotatePosition the position of the data in which it will have two children(left and right)
         */
        void rotate_right(node_pointer node, size_t RotatePosition){
            size_t UpperCeil = std::ceil(Size / 2.0);
            if(!node || RotatePosition < 0 || RotatePosition >= node->data_size || node->is_leaf){
                return;
            }
            auto LChild = node->children[RotatePosition];
            auto RChild = node->children[RotatePosition + 1];
            if(!LChild || !RChild){
                return;
            }
            if(RChild->data_size >= Size -1){
                // cannot rotate
                return;
            }
            if(LChild->data_size <= UpperCeil - 1){
                // cannot rotate
                return;
            }
            // RChild adds a new data and a new child.
            // RChild move to right.
            for(size_t Index = RChild->data_size; Index >= 1; --Index){
                RChild->children[Index] = RChild->children[Index-1];
                RChild->data[Index] = RChild->data[Index-1];
            }
            RChild->data[0] = node->data[RotatePosition];
            RChild->children[0] = LChild->children[LChild->data_size];
            if(LChild->children[LChild->data_size]){
                LChild->children[LChild->data_size]->parent = RChild;
            }
            node->data[RotatePosition] = LChild->data[LChild->data_size - 1];
            LChild->children[LChild->data_size] = nullptr;
            --LChild->data_size;
            ++RChild->data_size;
            if(!LChild->is_leaf){
                --LChild->child_size;
            }
            if(!RChild->is_leaf){
                ++RChild->child_size;
            }
        }

        /**
         * @brief 
         * In B tree deletion, if the left and the right node both have limited keys,just merge the node with either.
         * @param node the node which is to be processed.
         * @param ErasePosition the position of the data in which it will have two merged children(left and right)
         * @return the new merged node.
         */
        [[nodiscard]] node_pointer merge(node_pointer node, size_t ErasePosition){
            if(!node || ErasePosition < 0 || ErasePosition >= node->data_size || node->is_leaf){
                return nullptr;
            }
            auto LChild = node->children[ErasePosition];
            auto RChild = node->children[ErasePosition + 1];
            if(!LChild || !RChild){
                return nullptr;
            }
            size_t MayOverFlowSum = LChild->data_size + RChild->data_size + 1 ;
            if(MayOverFlowSum < LChild->data_size){
                return nullptr;// upper overflow
            }
            if(MayOverFlowSum >= 2*Size){
                return nullptr;// cannot merge
            }
            LChild->data[LChild->data_size] = node->data[ErasePosition];
            for(size_t Index = 0; Index < RChild->data_size; ++Index){
                LChild->data[Index + LChild->data_size + 1] = RChild->data[Index];
                LChild->children[Index + LChild->data_size + 1] = RChild->children[Index];
                if(LChild->children[Index]){
                    LChild->children[Index]->parent = LChild;
                }
            }
            // last child
            LChild->children[RChild->data_size + LChild->data_size] = RChild->children[RChild->data_size];
            if(RChild->children[RChild->data_size]){
                RChild->children[RChild->data_size]->parent = LChild;
            }
            // delete the RChild.
            for(size_t Index = 0; Index < RChild->child_size; ++Index){
                RChild->children[Index] = nullptr;
            }
            LChild->data_size += RChild->data_size + 1;
            if(!LChild->is_leaf){
                LChild->child_size = LChild->data_size + 1;
            }
            // set the two children to nullptr;
            node->children[ErasePosition] = nullptr;
            node->children[ErasePosition + 1] = nullptr;
            erase_directly(node, ErasePosition, LChild);
            delete RChild;
            --basic_type::num_of_nodes;
            return LChild;
        }
    public:
        Bplus_tree_Kruth(const Bplus_tree_Kruth&) = delete;
        explicit Bplus_tree_Kruth(Compare comp_ = Compare{}):Bplus_tree_Kruth(nullptr, comp_){}
        Bplus_tree_Kruth(Bplus_tree_Kruth && tree) noexcept :basic_type(std::move(tree)) {}

        static Bplus_tree_Kruth create_tree(std::istream &in=std::cin) {
            Bplus_tree_Kruth tree; 
            char Indicator;
            DataType Data;
            // !!! allowed to read a char of indicator.
            while(true){
                root_input:
                    if(in.good()){
                        Indicator=in.peek();
                    }else{
                        Indicator=EMPTY_NODE_INDICATOR;
                    }
                if(Indicator=='\n'){
                    in.get();
                    goto root_input;
                }else if(Indicator==' '||Indicator=='\t'){
                    in.get();
                    continue;
                }else if(Indicator==EMPTY_NODE_INDICATOR || Indicator == EOF ){
                    break;
                }else {
                    // take back to stream.
                    in>> Data;
                    tree.insert(Data);
                }
            }
            return tree;
        }

        // return the result ,
        // if bool is true, then the first is the result node, the second is the data position.
        // if bool is false, then the first is the node.(also may be null,empty tree), the second is the data inserted position inside the node.
        std::tuple<const_node_pointer,size_t,bool> find(const DataType& data)const{
            if(basic_type::is_empty()){
                return std::make_tuple(nullptr,typename node_type::const_data_iterator(),false);
            }
            const_node_pointer start=basic_type::get_root();
            while(true){
                std::pair<size_t , bool> FResult = find_in_node(start,data);
                if(FResult.second){
                    return std::make_tuple(start,std::get<0>(FResult),true);
                }
                // data is not in find_in_node
                size_t Offset = FResult.first;
                if(Offset == start->data_size){
                    // we must search data in the last child
                    auto NextNode = start->children[Offset];
                    if(NextNode){
                        start = NextNode;
                    }else{
                        return {start, Offset, false};
                    }
                }else{
                    auto NextNode = start->children[Offset];
                    if(NextNode){
                        start = NextNode;
                    }else{
                        return {start, Offset, false};
                    }
                }
            }
        }


        /**
         * @brief insert the data if find it, ignored!
         * the third returns whether insert operation is successful.
         * 
         * @param data inserted key.
         * @return std::tuple<const_node_pointer,size_t,bool> 
         * @details
         * According to Wiki @a https://en.wikipedia.org/wiki/B-tree
         * By this definition of B tree.
         * insert into a leaf and split the node from bottom to up if needed, this may be needed two traversals(find and rebalanced), memory expensive.
         * in this case, we just need one traversal, @see B_tree_Cormen::insert(const DataType&)
         */
        std::tuple<const_node_pointer,size_t,bool> insert(const DataType& Data){
            if(basic_type::is_empty()){
                // empty tree inserted  the data as a root.
                basic_type::_root=new NodeType();
                // the node is either supported O(1) retrieval,
                // or has an overloaded operator[].
                basic_type::_root->data[0] = Data;
                basic_type::num_of_nodes = 1;
                basic_type::_root->data_size = 1;
                basic_type::_root->child_size=0;
                ++height;
                return {basic_type::_root,0,true};
            }
            // deps: records the lookup chain which stores the child index.
            std::vector<size_t> LookUpChain;
            node_pointer start=const_cast<node_pointer>(basic_type::get_root());
            bool find = false;
            node_pointer InsertedNode = nullptr;
            size_t InsertPosition = 0;
            while(true){
                std::pair<size_t , bool> FindResult = find_in_node(start,Data);
                if(FindResult.second){
                    // OK, we find the inserted Data.
                    find = true;
                    InsertedNode = start;
                    InsertPosition = FindResult.first;
                    break;
                }
                // data is not in find_in_node
                size_t Offset = FindResult.first;
                if(Offset == start->data_size){
                    // we must search data in the last child
                    auto NextNode = start->children[Offset];
                    if(NextNode){
                        start = NextNode;
                        LookUpChain.push_back(Offset);
                    }else{
                        find = false;
                        InsertedNode = start;
                        InsertPosition = Offset;
                        break;
                    }
                }else{
                    auto NextNode = start->children[Offset];
                    if(NextNode){
                        start = NextNode;
                        LookUpChain.push_back(Offset);
                    }else{
                        find = false;
                        InsertedNode = start;
                        InsertPosition = Offset;
                        break;
                    }
                }
            }
            if(find){
                return {InsertedNode, InsertPosition, false};
            }
            // backup the inserted position.
            node_pointer BakInsertedNode = InsertedNode;
            size_t BakInsertPosition = InsertPosition;
            // now the node of inserted position is a leaf.
            node_pointer LChild = nullptr;
            node_pointer RChild = nullptr;
            auto CBegin = LookUpChain.cbegin();
            auto InsertedData = Data;
            while(true){
                // Case 1: the node has less than m-1 keys, just insert the data without rebalancing the tree.
                if(InsertedNode->data_size < Size - 1){
                    auto BakParentNode = InsertedNode->parent;
                    // re-set the original index child.
                    insert_not_full(InsertedNode, InsertedData, InsertPosition, LChild, RChild);
                    break;
                }else{
                    // Case 2: the node has m-1 keys, we must split it into three nodes(including the inserted key):
                    // A,B and C : A with 1 key, B with ceil(m/2.0) - 1 key(s), and C with m - ceil(m/2.0) key(s)
                    auto BakParentNode = InsertedNode->parent;
                    bool isRoot = InsertedNode == basic_type::_root;
                    auto Tuple = insert_full(InsertedNode, InsertedData, InsertPosition, LChild, RChild);
                    InsertedData = std::get<2>(Tuple);
                    LChild = std::get<0>(Tuple);
                    RChild = std::get<1>(Tuple);
                    if(isRoot){
                        basic_type::_root = new NodeType();
                        ++height;
                        basic_type::_root->data[0] = InsertedData;
                        basic_type::_root->children[0] = LChild;
                        basic_type::_root->children[1] = RChild;
                        LChild->parent = basic_type::_root;
                        RChild->parent = basic_type::_root;
                        basic_type::_root->is_leaf = false;
                        basic_type::_root->child_size = 2;
                        basic_type::_root->data_size = 1;
                        ++basic_type::num_of_nodes;
                        break;
                    }else{
                        InsertedNode = BakParentNode;
                        InsertPosition = *CBegin;
                        ++CBegin;
                    }
                }
            }
            return {BakInsertedNode,BakInsertPosition, true};
        }

        /**
         * @brief delete the data if find it, ignored!
         * 
         * @param data deleted key.
         * @param left Whether to replace the internal deleted data with its left sub tree max data.
         * @param borrowLeft Whether to borrow the data from the left sibling(if allowed).
         * @param mergeLeft Whether to merge the node with its left sibling(if allowed).
         * @details
         * According to Wiki @a https://en.wikipedia.org/wiki/B-tree
         * By this definition of B tree 
         * after transform the deleted data with a leaf data, we got the inserted data on a leaf.
         * Delete the data on a leaf and merge two nodes from bottom to up if needed, this may be needed two traversals(find and rebalanced), memory expensive.
         */
        void erase(const DataType& Data,bool left = true, bool borrowLeft = true, bool mergeLeft = true){
            if(basic_type::is_empty()){
                return;
            }
            // deps: records the lookup chain which stores the child index.
            std::vector<size_t> LookUpChain;
            node_pointer start=const_cast<node_pointer>(basic_type::get_root());
            bool find = false;
            node_pointer ErasedNode = nullptr;
            size_t ErasedPosition = 0;
            while(true){
                std::pair<size_t , bool> FindResult = find_in_node(start,Data);
                if(FindResult.second){
                    // OK, we find the inserted Data.
                    find = true;
                    ErasedNode = start;
                    ErasedPosition = FindResult.first;
                    break;
                }
                // data is not in find_in_node
                size_t Offset = FindResult.first;
                if(Offset == start->data_size){
                    // we must search data in the last child
                    auto NextNode = start->children[Offset];
                    if(NextNode){
                        start = NextNode;
                        LookUpChain.push_back(Offset);
                    }else{
                        find = false;
                        ErasedNode = start;
                        ErasedPosition = Offset;
                        break;
                    }
                }else{
                    auto NextNode = start->children[Offset];
                    if(NextNode){
                        start = NextNode;
                        LookUpChain.push_back(Offset);
                    }else{
                        find = false;
                        ErasedNode = start;
                        ErasedPosition = Offset;
                        break;
                    }
                }
            }
            if(!find){
                return;
            }
            if(!ErasedNode->is_leaf){
                // it has two nodes right and left
                if(left){
                    // replace the data with the left sub tree max data.
                    node_pointer LeftNode=ErasedNode->children[ErasedPosition];
                    LookUpChain.push_back(ErasedPosition);
                    while(LeftNode->child_size != 0){
                        LookUpChain.push_back(LeftNode->child_size - 1);
                        LeftNode = LeftNode->children[LeftNode->child_size-1];
                    }
                    ErasedNode->data[ErasedPosition] = LeftNode->data[LeftNode->data_size - 1];
                    ErasedNode = LeftNode;
                    ErasedPosition = LeftNode->data_size - 1;
                }else{
                    // replace the data with the right sub tree min data.
                    node_pointer RightNode=ErasedNode->children[ErasedPosition + 1];
                    LookUpChain.push_back(ErasedPosition + 1);
                    while(RightNode->child_size != 0){
                        LookUpChain.push_back(0);
                        RightNode = *(RightNode->child_begin());
                    }
                    ErasedNode->data[ErasedPosition] = RightNode->data[0];
                    ErasedNode = RightNode;
                    ErasedPosition = 0;
                }
            }
            size_t UpperCeil = std::ceil(Size / 2.0);
            // Now InsertedNode is a leaf(by definition).
            assert(ErasedNode->is_leaf);
            auto CBegin = LookUpChain.cbegin();
            if(ErasedNode == basic_type::_root || ErasedNode->data_size > UpperCeil - 1){
                // just delete the data.
                erase_directly(ErasedNode, ErasedPosition, nullptr);
                if(ErasedNode == basic_type::_root && ErasedNode->data_size == 0){
                    basic_type::_root = nullptr;
                    basic_type::num_of_nodes = 0;
                    ErasedNode->parent = nullptr;
                    --height;
                    ErasedNode->children[0] = nullptr;// it may have one child as its data_size is 0.
                    // delete the root.                    
                    delete ErasedNode;
                }
                return;
            }
            // first delete directly, and try to rotate or merge 
            erase_directly(ErasedNode, ErasedPosition, nullptr);
            while(true){
                // fix_erase
                // if its can borrow from left child or borrowLeft is true.
                auto ParentNode = ErasedNode->parent;
                if(ParentNode == nullptr){
                    break;
                }
                bool isRoot = ParentNode == basic_type::_root;
                size_t ChildIndex = *CBegin;
                bool leftCanBorrow = ChildIndex > 0 && (ParentNode->children[ChildIndex - 1]->data_size > UpperCeil - 1);
                bool rightCanBorrow = ChildIndex < ParentNode->data_size && ParentNode->children[ChildIndex + 1]->data_size > UpperCeil - 1;
                if(leftCanBorrow && (borrowLeft || !rightCanBorrow)){
                    rotate_right(ParentNode,  ChildIndex - 1);
                    break;
                }else if(rightCanBorrow && (!borrowLeft || !leftCanBorrow)){
                    rotate_left(ParentNode, ChildIndex);
                    break;
                }else{
                    // if left and right is not null, thew must can be merged with ErasedNode
                    bool leftCanMerge = ChildIndex > 0;
                    bool rightCanMerge = ChildIndex < ParentNode->data_size;
                    
                    node_pointer newNode = nullptr;
                    if(leftCanMerge && (mergeLeft || !rightCanMerge)){
                        newNode = merge(ParentNode, ChildIndex - 1);
                    }else if(rightCanMerge && (!mergeLeft || !leftCanMerge)){
                        newNode = merge(ParentNode, ChildIndex);
                    }
                    assert(newNode != nullptr);
                    // if parent node is root and have no data, delete it.
                    if(isRoot && ParentNode->data_size == 0){
                        // ParentNode have one child, reset to null
                        ParentNode->children[0] = nullptr;
                        basic_type::_root = newNode;
                        --height;
                        newNode->parent = nullptr;
                        --basic_type::num_of_nodes;
                        delete ParentNode;
                        break;
                    }
                    ErasedNode = ParentNode;
                    ++CBegin;// if !isRoot then CBegin cannot be in the end.
                }
            }
            
        }

        [[nodiscard]] std::string to_string()const override {
            return "<-B Tree->";
        }

        size_t get_height() const {
            return height;
        }

        // find the min and the max data. 
        DataType min()const{
            const auto find_result=left_most(basic_type::_root);
            assert(find_result&&"Empty tree!");
            return find_result->data[0];
        }
        DataType max()const{
            const auto find_result=right_most(basic_type::_root);
            assert(find_result&&"Empty tree!");
            return find_result->data[find_result->data_size - 1];
        }
    };


    // m-order B tree definition by Thomas H. Cormen,Charles E. Leiserson, Ronald L. Rivest, Clifford Stein 
    // See book Introduction to Algorithms(Third Edition)
    // m means least child number, 2m means most child number
    // node child number except root node/leaf nodes is in [m, 2m]
    // key number is in [m - 1, 2m - 1]
    // Thus we can split a node even it has 2m - 1 keys, not 2m keys.
    // We can merge two nodes even they both have m keys. 
    // Root child number is in [2,2m]
    // All leaves are in the same level.
	template<typename DataType,size_t Size, typename Compare = std::less<DataType>, typename NodeType=node::B_node<DataType, 2 * Size>
        , typename NodePrintTrait = Bplus_node_print_trait<NodeType>>
	class Bplus_tree_Cormen final:public abstract_tree<DataType,Size,NodeType,Bplus_tree_Cormen<DataType,Size,Compare,NodeType,NodePrintTrait>, NodePrintTrait>{
    
    private:    
        // minimum size 2.
        // child [2,4]
        // key [1,3]
        static_assert(Size >= 2);

        size_t height = 0;

        using basic_type=abstract_tree<DataType,Size,NodeType,Bplus_tree_Cormen<DataType,Size,Compare,NodeType,NodePrintTrait>,NodePrintTrait>;
        // prohibit all create functions.
        using basic_type::create_tree_l;
        using basic_type::create_tree_r;
        using basic_type::shift_height;
    public:
        using node_type = NodeType;
        using node_pointer = NodeType*;
        using node_type_reference = NodeType&;
        using const_node_type = const NodeType;
        using const_node_pointer = const NodeType*;
        using const_node_type_reference = const NodeType&;

        using PrintTrait = typename basic_type::PrintTrait;

    private:

        explicit Bplus_tree_Cormen(std::nullptr_t, Compare comp_ = Compare{}):basic_type(nullptr), comp(comp_){}

        Compare comp;

        /**
         * @brief get the left most node of the tree.
         * 
         * @param node 
         * @return the left most node
         */
        static const_node_pointer left_most(const_node_pointer node){
            if(!node){
                return nullptr;
            }
            const_node_pointer tmp=node;
            // see node.h child_begin() and child_end()
            while(tmp->child_size != 0){
                tmp = *(tmp->child_begin());
            }
            return tmp;
        }

        /** 
         * @brief get the right most node of the tree.
         * 
         * @param node 
         * @return the right most node
         */
        static const_node_pointer right_most(const_node_pointer node) {
            if(!node){
                return nullptr;
            }
            const_node_pointer tmp=node;
            // cannot use child_rend() here
            while(tmp->child_size != 0){
                tmp = tmp->children[tmp->child_size-1];
            }
            return tmp;
        }

        
        // Find a data in one node.
        // Using binary search.
        //  the first offset denotes the position offset(and the original data will be moved to the right)
        // Otherwise, return the offset , the second is true.
        // Notice that : offset may be the size which means Data should be pushed back.
        std::pair<size_t,bool> find_in_node(const_node_pointer node,const DataType& Data) const {
            if(!node){
                return {0,false};
            }
            if(node->data_size == 0){
                return {0,false};
            }
            size_t Left = 0;
            size_t Right = node->data_size - 1; // Right >= Left
            size_t Middle = 0;
            while(Left<=Right){
                Middle = Left + (Right - Left) / 2; // avoid overflow
                if(comp(node->data[Middle],Data)){
                    // overflow.
                    if(Middle == std::numeric_limits<size_t>::max()){
                        break;
                    }
                    Left = Middle + 1;
                }else if(comp(Data,node->data[Middle])){
                    if(Middle == std::numeric_limits<size_t>::min()){
                        break;
                    }
                    Right = Middle - 1;
                }else{
                    return {Middle, true};
                }
            }
            if(comp(node->data[Middle],Data)){
                return {Middle + 1,false};
            }else {
                // comp(Data,data[Middle]) == true.
                return {Middle,false};
            }
        }

        /**
         * @brief 
         * if the node is not full, insert the data to the node
         */
        void insert_not_full(node_pointer node, const DataType& Data, size_t InsertedPosition,node_pointer LChild, node_pointer RChild){
            // if node is full,do nothing.
            if(!node  || InsertedPosition < 0 || InsertedPosition > node->data_size || node->data_size >= 2 * Size - 1){
                return;
            }
            // If the node with extra data would not be full, just insert the data.
            // Avoiding -1. Each index + 1
            for(size_t Index = node->data_size; Index > InsertedPosition; --Index){
                size_t ActualIndex = Index - 1;
                node->children[ActualIndex + 2]= node->children[ActualIndex + 1];
                node->data[ActualIndex + 1] = node->data[ActualIndex];
            }
            // insert the data.
            node->data[InsertedPosition] = Data;
            node->children[InsertedPosition + 1] = RChild;
            if(RChild){
                RChild->parent = node;
            }
            node->children[InsertedPosition] = LChild;
            if(LChild){
                LChild->parent = node;
            }
            ++node->data_size;
            if(!node->is_leaf){
                ++node->child_size;
            }
        }

        /**
         * @brief split the full node with 2m - 1 keys with one inserted data into two nodes: the left node has m-1 keys
         * , the right node has m -1  keys,and one middle data(insert directly into the node parent, by this definition
         * of B tree, if parent is full we already split it before so we can insert the middle data directly to the prarent).
         * @pre                                                                                                                                                                                            
         * 1:an internal node must be a full node(it has 2m children and 2m - 1 keys).
         * 2:an leaf node must be a full leaf(it has non children and 2m - 1keys).
         */
        [[nodiscard("allocate a new node")]] std::tuple<node_pointer, node_pointer, DataType> split_full(node_pointer node){
            // if node cannot be splitted, do nothing.
            if(!node  || node->data_size != 2 * Size - 1){
                return {nullptr, nullptr,DataType{}};
            }
            // Compute the middle position.
            size_t Middle =  Size - 1;
            // Middle data.
            auto PopData = node->data[Middle];
            node_pointer RightNode = new node_type();
            ++basic_type::num_of_nodes;
            // Filling the right node
            for(size_t Index = Middle + 1;Index < 2*Size -1;++Index){
                RightNode->data[Index - Middle - 1] = node->data[Index];
                RightNode->children[Index - Middle - 1] = node->children[Index];
                if(node->children[Index]){
                    node->children[Index]->parent = RightNode;
                }
                node->children[Index] = nullptr;
            }
            // last child.
            RightNode->children[Middle] = node->children[2*Size-1];
            if(node->children[2*Size-1]){
                node->children[2*Size-1]->parent = RightNode;
            }
            node->children[2*Size-1] = nullptr;
            // update some fields.
            if(node->is_leaf){
                RightNode->is_leaf = true;
                RightNode->child_size = 0;
            }else{
                node->child_size =  Middle + 1;// node as new left node.
                RightNode->is_leaf = false;
                RightNode->child_size = Middle + 1;
            }
            RightNode->data_size = Middle;
            node->data_size = Middle;
            return {node, RightNode, PopData};
        }

        /**
         * @brief 
         * In B tree deletion, delete the data directly.
         * @param child the child replacing the deleted position child.
         * @param node the node which is to be processed.
         * @param ErasePosition the position of the data to be deleted in node. 
         */
        void erase_directly(node_pointer node, size_t ErasePosition, node_pointer child){
            if(!node || ErasePosition < 0 || ErasePosition >= node->data_size){
                return;
            }
            for(size_t Index = ErasePosition; Index < node->data_size - 1; ++Index){
                if(Index == ErasePosition){
                    node->children[Index] = child;
                    if(child){
                        child->parent = node;
                    }
                }else{
                    node->children[Index]= node->children[Index + 1];
                }
                node->data[Index] = node->data[Index + 1];
            }
            // process last child
            if(ErasePosition == node->data_size - 1){
                node->children[node->data_size - 1] = child;
                if(child){
                    child->parent = node;
                }
            }else{
                node->children[node->data_size - 1] = node->children[node->data_size];
            }
            node->children[node->data_size] = nullptr;
            --node->data_size;
            if(!node->is_leaf){
                --node->child_size;
            }
        }
        /**
         * @brief 
         * In B tree deletion, rotate left, this will rotate the right to left node.
         * @param node the node which is to be processed.
         * @param RotatePosition the position of the data in which it will have two children(left and right)
         */
        void rotate_left(node_pointer node, size_t RotatePosition){
            if(!node || RotatePosition < 0 || RotatePosition >= node->data_size || node->is_leaf){
                return;
            }
            auto LChild = node->children[RotatePosition];
            auto RChild = node->children[RotatePosition + 1];
            if(!LChild || !RChild){
                return;
            }
            if(LChild->data_size >= 2*Size - 1){
                // cannot rotate
                return;
            }
            if(RChild->data_size <= Size - 1){
                // cannot rotate
                return;
            }
            // LChild adds a new data and a new child.
            LChild->data[LChild->data_size] = node->data[RotatePosition];
            LChild->children[LChild->child_size + 1] = RChild->children[0];
            if(RChild->children[0]){
                RChild->children[0]->parent = LChild;
            }
            node->data[RotatePosition] = RChild->data[0];
            // RChild deletes the first data.
            for(size_t Index = 0; Index < RChild->data_size - 1; ++Index){
                RChild->children[Index] = RChild->children[Index + 1];
                RChild->data[Index] = RChild->data[Index + 1];
            }
            RChild->children[RChild->data_size-1] = RChild->children[RChild->data_size];
            ++LChild->data_size;
            --RChild->data_size;
            if(!LChild->is_leaf){
                ++LChild->child_size;
            }
            if(!RChild->is_leaf){
                --RChild->child_size;
            }
        }

        /**
         * @brief 
         * In B tree deletion, rotate right, this will rotate the left to right node.
         * @param node the node which is to be processed.
         * @param RotatePosition the position of the data in which it will have two children(left and right)
         */
        void rotate_right(node_pointer node, size_t RotatePosition){
            if(!node || RotatePosition < 0 || RotatePosition >= node->data_size || node->is_leaf){
                return;
            }
            auto LChild = node->children[RotatePosition];
            auto RChild = node->children[RotatePosition + 1];
            if(!LChild || !RChild){
                return;
            }
            if(RChild->data_size >= 2*Size -1){
                // cannot rotate
                return;
            }
            if(LChild->data_size <= Size - 1){
                // cannot rotate
                return;
            }
            // RChild adds a new data and a new child.
            // RChild move to right.
            for(size_t Index = RChild->data_size; Index >= 1; --Index){
                RChild->children[Index] = RChild->children[Index-1];
                RChild->data[Index] = RChild->data[Index-1];
            }
            RChild->data[0] = node->data[RotatePosition];
            RChild->children[0] = LChild->children[LChild->data_size];
            if(LChild->children[LChild->data_size]){
                LChild->children[LChild->data_size]->parent = RChild;
            }
            node->data[RotatePosition] = LChild->data[LChild->data_size - 1];
            LChild->children[LChild->data_size] = nullptr;
            --LChild->data_size;
            ++RChild->data_size;
            if(!LChild->is_leaf){
                --LChild->child_size;
            }
            if(!RChild->is_leaf){
                ++RChild->child_size;
            }
        }

        /**
         * @brief 
         * In B tree deletion, if the left and the right node both have limited keys,just merge the node with either.
         * @param node the node which is to be processed.
         * @param ErasePosition the position of the data in which it will have two merged children(left and right)
         * @return the new merged node.
         */
        [[nodiscard]] node_pointer merge(node_pointer node, size_t ErasePosition){
            if(!node || ErasePosition < 0 || ErasePosition >= node->data_size || node->is_leaf){
                return nullptr;
            }
            auto LChild = node->children[ErasePosition];
            auto RChild = node->children[ErasePosition + 1];
            if(!LChild || !RChild){
                return nullptr;
            }
            size_t MayOverFlowSum = LChild->data_size + RChild->data_size + 1 ;
            if(MayOverFlowSum < LChild->data_size){
                return nullptr;// upper overflow
            }
            if(MayOverFlowSum >= 2*Size){
                return nullptr;// cannot merge
            }
            LChild->data[LChild->data_size] = node->data[ErasePosition];
            for(size_t Index = 0; Index < RChild->data_size; ++Index){
                LChild->data[Index + LChild->data_size + 1] = RChild->data[Index];
                LChild->children[Index + LChild->data_size + 1] = RChild->children[Index];
                if(RChild->children[Index]){
                    RChild->children[Index]->parent = LChild;
                }
            }
            // last child
            LChild->children[RChild->data_size + LChild->data_size + 1] = RChild->children[RChild->data_size];
            if(RChild->children[RChild->data_size]){
                RChild->children[RChild->data_size]->parent = LChild;
            }
            // delete the RChild.
            for(size_t Index = 0; Index < RChild->child_size; ++Index){
                RChild->children[Index] = nullptr;
            }
            LChild->data_size += RChild->data_size + 1;
            if(!LChild->is_leaf){
                LChild->child_size = LChild->data_size + 1;
            }
            // set the two children to nullptr;
            node->children[ErasePosition] = nullptr;
            node->children[ErasePosition + 1] = nullptr;
            erase_directly(node, ErasePosition, LChild);
            delete RChild;
            --basic_type::num_of_nodes;
            return LChild;
        }

        /**
         * @brief 
         * Called by deletion, if needed to transform the node wither rotate OR merge
         * @param parent the node parent which is to be processed.
         * @param Offset the position of the node in this parent(cannot be null ,meaning that node cannot be the root node)
         * @param FlagPosition a special sentinel position(0 to data_size) in node parent[Offset], after processing the node,
         *     erase_transform will return the new corresponding position and the node.
         * @param borrowLeft Whether to borrow the data from the left sibling(if allowed).
         * @param mergeLeft Whether to merge the node with its left sibling(if allowed).
         * @return the new node who may have the erased data and its child index
         */
        std::pair<node_pointer, size_t> erase_transform(node_pointer parent, size_t Offset, size_t FlagPosition, bool borrowLeft = true, bool mergeLeft = true){
            if(!parent || Offset < 0 || Offset >= parent->child_size){
                return {nullptr,0};
            }
            node_pointer node = parent->children[Offset];
            if(!node || node->data_size >= Size || FlagPosition < 0 || FlagPosition > node->data_size){
                // (more keys) node cannot be processed.
                return {nullptr,0};
            }
            size_t newPosition = FlagPosition;
            node_pointer newNode = node;
            // minimum data size.
            bool leftCanBorrow = Offset > 0 && (parent->children[Offset - 1]->data_size >= Size);
            bool rightCanBorrow = Offset < parent->data_size && parent->children[Offset + 1]->data_size >= Size;
            if(leftCanBorrow && (borrowLeft || !rightCanBorrow)){
                rotate_right(parent,  Offset - 1);
                newPosition = FlagPosition + 1;
                newNode = node;
            }else if(rightCanBorrow && (!borrowLeft || !leftCanBorrow)){
                rotate_left(parent, Offset);
                newPosition = FlagPosition;
                newNode = node;
            }else{
                // if left and right is not null, thew must can be merged with ErasedNode
                bool leftCanMerge = Offset > 0;
                bool rightCanMerge = Offset < parent->data_size;
                bool isRoot = parent == basic_type::_root;
                node_pointer mergedNode = nullptr;
                if(leftCanMerge && (mergeLeft || !rightCanMerge)){
                    size_t LeftDataSize = parent->children[Offset - 1]->data_size;
                    mergedNode = merge(parent, Offset - 1);
                    newPosition = FlagPosition + LeftDataSize + 1;
                    newNode = mergedNode;
                }else if(rightCanMerge && (!mergeLeft || !leftCanMerge)){
                    mergedNode = merge(parent, Offset);
                    newPosition = FlagPosition;
                    newNode = mergedNode;
                }
                assert(mergedNode != nullptr);
                // if parent node is root and have no data, delete it.
                if(isRoot && parent->data_size == 0){
                    basic_type::_root = mergedNode;
                    --height;
                    mergedNode->parent = nullptr;
                    --basic_type::num_of_nodes;
                    parent->children[0] = nullptr;// it may have one child as its data_size is 0.
                    delete parent;
                }
            }
            return {newNode, newPosition};
        }

    public:
        Bplus_tree_Cormen(const Bplus_tree_Cormen&) = delete;
        explicit Bplus_tree_Cormen(Compare comp_ = Compare{}):Bplus_tree_Cormen(nullptr, comp_){}
        Bplus_tree_Cormen(Bplus_tree_Cormen && tree) noexcept :basic_type(std::move(tree)) {}

        static Bplus_tree_Cormen create_tree(std::istream &in=std::cin) {
            Bplus_tree_Cormen tree; 
            char Indicator;
            DataType Data;
            // !!! allowed to read a char of indicator.
            while(true){
                root_input:
                    if(in.good()){
                        Indicator=in.peek();
                    }else{
                        Indicator=EMPTY_NODE_INDICATOR;
                    }
                if(Indicator=='\n'){
                    in.get();
                    goto root_input;
                }else if(Indicator==' '||Indicator=='\t'){
                    in.get();
                    continue;
                }else if(Indicator==EMPTY_NODE_INDICATOR || Indicator == EOF ){
                    break;
                }else {
                    // take back to stream.
                    in>> Data;
                    tree.insert(Data);
                }
            }
            return tree;
        }

        // return the result ,
        // if bool is true, then the first is the result node, the second is the data position.
        // if bool is false, then the first is the node.(also may be null,empty tree), the second is the data inserted position inside the node.
        std::tuple<const_node_pointer,size_t,bool> find(const DataType& data)const{
            if(basic_type::is_empty()){
                return std::make_tuple(nullptr,typename node_type::const_data_iterator(),false);
            }
            const_node_pointer start=basic_type::get_root();
            while(true){
                std::pair<size_t , bool> FResult = find_in_node(start,data);
                if(FResult.second){
                    return std::make_tuple(start,std::get<0>(FResult),true);
                }
                // data is not in find_in_node
                size_t Offset = FResult.first;
                if(Offset == start->data_size){
                    // we must search data in the last child
                    auto NextNode = start->children[Offset];
                    if(NextNode){
                        start = NextNode;
                    }else{
                        return {start, Offset, false};
                    }
                }else{
                    auto NextNode = start->children[Offset];
                    if(NextNode){
                        start = NextNode;
                    }else{
                        return {start, Offset, false};
                    }
                }
            }
        }


        /**
         * @brief insert the data if find it, ignored!
         * the third returns whether insert operation is successful.
         * 
         * @param data inserted key.
         * @return std::tuple<const_node_pointer,size_t,bool> 
         * @details
         * According to Wiki @a https://en.wikipedia.org/wiki/B-tree
         * By this definition of B tree
         * when finding the inserted position, splitting the full node encountered, and splitting will ensure its parent is not full,
         * in this case, we just need one traversal, @see B_tree_kruth::insert(const DataType&)
         */
        std::tuple<const_node_pointer,size_t,bool> insert(const DataType& Data){
            if(basic_type::is_empty()){
                ++height;
                // empty tree inserted  the data as a root.
                basic_type::_root=new NodeType();
                // the node is either supported O(1) retrieval,
                // or has an overloaded operator[].
                basic_type::_root->data[0] = Data;
                basic_type::num_of_nodes = 1;
                basic_type::_root->data_size = 1;
                basic_type::_root->child_size=0;
                return {basic_type::_root,0,true};
            }
            node_pointer start=const_cast<node_pointer>(basic_type::get_root());
            auto Bak = start;// backup in order to split if needed.
            size_t BakPosition = 0;
            while(true){
                bool split = false;
                bool find = false;
                size_t Offset = 0;
                std::tuple<node_pointer, node_pointer, size_t> SplitTuple;
                if(std::pair<size_t , bool> FindResult = find_in_node(start,Data); FindResult.second){
                    // OK, we find the inserted Data.
                    find = true;
                    Offset = FindResult.first;
                }else{
                    // data is not in find_in_node
                    Offset = FindResult.first;
                    Bak = start->children[Offset];
                    find = false;
                }
                // full node : 2m-1:[m-1,1,m-1]
                if(start->data_size == 2 * Size - 1){
                    auto Parent = start->parent;
                    bool isRoot = start == basic_type::_root;
                    SplitTuple = split_full(start);
                    split = true;
                    if(isRoot){
                        basic_type::_root = new NodeType();
                        ++height;
                        basic_type::_root->data[0] = std::get<2>(SplitTuple);
                        basic_type::_root->children[0] = std::get<0>(SplitTuple);
                        basic_type::_root->children[1] = std::get<1>(SplitTuple);
                        std::get<0>(SplitTuple)->parent = basic_type::_root;
                        std::get<1>(SplitTuple)->parent = basic_type::_root;
                        basic_type::_root->is_leaf = false;
                        basic_type::_root->child_size = 2;
                        basic_type::_root->data_size = 1;
                        ++basic_type::num_of_nodes;
                        // update Offset.
                    }else{
                        insert_not_full(Parent, std::get<2>(SplitTuple) 
                            , BakPosition, std::get<0>(SplitTuple), std::get<1>(SplitTuple));
                    }

                }
                if(find){
                    return {start, Offset, false}; 
                }
                if(!Bak){
                    // start is a leaf
                    if(split){
                        // ok , now start 
                        // Size is the index of middle position.
                        if(Offset <= Size - 1){
                            insert_not_full(std::get<0>(SplitTuple), Data
                            , Offset, nullptr,nullptr);
                            return {std::get<0>(SplitTuple),Offset,true};
                        }else{
                            insert_not_full(std::get<1>(SplitTuple), Data 
                            , Offset - Size, nullptr,nullptr);
                            return {std::get<1>(SplitTuple),Offset - Size,true};
                        }
                    }else{
                        insert_not_full(start, Data 
                        , Offset, nullptr, nullptr);
                        return {start, Offset, true};
                    }
                }else{
                    start = Bak;
                    BakPosition = Offset;
                    // continue loop.
                }
            }
        }

        /**
         * @brief delete the data if find it, ignored!
         * 
         * @param data deleted key.
         * @param left Whether to replace the internal deleted data with its left sub tree max data.
         * @param borrowLeft Whether to borrow the data from the left sibling(if allowed).
         * @param mergeLeft Whether to merge the node with its left sibling(if allowed).
         * @details
         * According to Wiki @a https://en.wikipedia.org/wiki/B-tree
         * By this definition of B tree 
         * after transform the deleted data with a leaf data, we got the inserted data on a leaf.
         * when finding the deleted position, merge or rotate the nodes with the minimum keys encountered, and merge will ensure its parent 
         * having not less than the minimum keys, in this case, if the erased data is in a leaf we just need one traversal.
         * If the erased data is in an internal node, we still need another a small traversal to
         * find its sub left max or sub right min data.
         */
        void erase(const DataType& Data,bool left = true, bool borrowLeft = true, bool mergeLeft = true){
            if(basic_type::is_empty()){
                return;
            }
            node_pointer ErasedNode = const_cast<node_pointer>(basic_type::get_root());
            size_t ErasedPosition = 0;
            bool find = false;
            node_pointer Bak = nullptr;
            size_t FlagPosition = 0;
            size_t ChildIndex = 0;
            while(true){
                std::pair<size_t , bool> FindResult = find_in_node(ErasedNode,Data);
                ErasedPosition = FindResult.first;
                find = FindResult.second;
                FlagPosition = ErasedPosition;
                // first transform if needed.
                if(ErasedNode->data_size < Size && ErasedNode != basic_type::_root){
                    // ROOT cannot be here.
                    auto Pair = erase_transform(ErasedNode->parent, ChildIndex, FlagPosition, borrowLeft, mergeLeft);
                    ErasedNode = Pair.first;
                    FlagPosition = Pair.second;
                    ChildIndex = FlagPosition;
                    Bak = ErasedNode->children[ChildIndex];
                }else{
                    ChildIndex = ErasedPosition;
                    Bak =  ErasedNode->children[ErasedPosition];
                }
                if(find || !Bak){
                    // find the erased data position OR reach the leaf node and still not find the data.
                    break;
                }
                ErasedNode = Bak;
            }
            if(!find){
                // still not found the erased data.
                return;
            }
            // ErasedNode is an internal node
            // Extra traversal
            if(!ErasedNode->is_leaf){
                // it has two nodes right and left
                if(left){
                    // replace the data with the left sub tree max data.
                    node_pointer LeftNode=ErasedNode->children[ErasedPosition];
                    while(LeftNode->child_size != 0){
                        LeftNode = LeftNode->children[LeftNode->child_size-1];
                    }
                    ErasedNode->data[ErasedPosition] = LeftNode->data[LeftNode->data_size - 1];
                    LeftNode=ErasedNode->children[ErasedPosition];
                    node_pointer Parent = ErasedNode;
                    size_t NextChildPosition = LeftNode->data_size;// right after the last child
                    while(LeftNode){
                        if(LeftNode->data_size < Size){
                            auto Pair = erase_transform(Parent, ErasedPosition, NextChildPosition, borrowLeft, mergeLeft);
                            Parent = Pair.first;
                            ErasedPosition = Pair.second; 
                            LeftNode = Parent->children[ErasedPosition];
                            NextChildPosition = ErasedPosition;
                        }else{
                            Parent = LeftNode;
                            ErasedPosition = LeftNode->data_size;
                            LeftNode = LeftNode->children[ErasedPosition];
                            NextChildPosition = ErasedPosition;
                        }
                    }
                    // simple erase it 
                    erase_directly(Parent, NextChildPosition - 1, nullptr);
                }else{
                    // replace the data with the right sub tree min data.
                    node_pointer RightNode=ErasedNode->children[ErasedPosition + 1];
                    while(RightNode->child_size != 0){
                        RightNode = RightNode->children[0];
                    }
                    ErasedNode->data[ErasedPosition] = RightNode->data[0];
                    // replace the data with the right sub tree min data.
                    RightNode=ErasedNode->children[ErasedPosition + 1];
                    node_pointer Parent = ErasedNode;
                    size_t ChildIndex = ErasedPosition + 1;
                    size_t NextChildPosition = 0;
                    // process including leaf node.
                    while(RightNode){
                        if(RightNode->data_size < Size){
                            auto Pair = erase_transform(Parent, ChildIndex, NextChildPosition, borrowLeft, mergeLeft);
                            Parent = Pair.first;
                            ChildIndex = Pair.second; 
                            RightNode = Parent->children[ChildIndex];
                            NextChildPosition = ChildIndex;
                        }else{
                            NextChildPosition = 0;
                            Parent = RightNode;
                            ChildIndex = 0;
                            RightNode = RightNode->children[0];
                        }
                    }
                    // simple erase it
                    erase_directly(Parent, NextChildPosition, nullptr);
                }
            }else{
                erase_directly(ErasedNode, FlagPosition, nullptr);
                // if the node is root, we need judge whether it is empty.
                // if parent node is root and have no data, delete it.
                if(ErasedNode == basic_type::_root && ErasedNode->data_size == 0){
                    basic_type::_root = nullptr;
                    ErasedNode->parent = nullptr;
                    basic_type::num_of_nodes = 0;
                    --height;
                    ErasedNode->children[0] = nullptr;// it may have one child as its data_size is 0.
                    delete ErasedNode;
                }
            }
        }

        [[nodiscard]] std::string to_string()const override {
            return "<-B Tree->";
        }

        size_t get_height() const {
            return height;
        }
        // find the min and the max data. 
        DataType min()const{
            const auto find_result=left_most(basic_type::_root);
            assert(find_result&&"Empty tree!");
            return find_result->data[0];
        }
        DataType max()const{
            const auto find_result=right_most(basic_type::_root);
            assert(find_result&&"Empty tree!");
            return find_result->data[find_result->data_size - 1];
        }
    };
}


#endif