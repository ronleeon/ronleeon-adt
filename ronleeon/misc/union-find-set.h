// union-find set.


#ifndef RONLEEON_ADT_UNION_SET
#define RONLEEON_ADT_UNION_SET

#include <initializer_list>
#include <iostream>
#include <vector>
// TODO: tests
namespace ronleeon {
    // By default, enable path compress, union by rank(by rank or by number)
    template <typename T>
    class union_find_set{
    private:
        // describe the node using in the union-find set
        struct _node{
            T data;
            size_t parent;
            // for union by rank, a node stores its rank, which is an upper bound for its height(approximately, not exactly)
            // because maintain the correct height is difficult
            size_t rank;
            size_t num;// number of nodes(children and itself)
        };

        std::vector<_node> data;

        [[nodiscard]] size_t rootKey(size_t Index) const {
            return data[Index].parent == Index ? Index : rootKey(data[Index].parent);
        }

    public:

        union_find_set() = default;

        union_find_set(std::initializer_list<T> initialData){
            size_t size = initialData.size();
            if(size == 0){
                return; // initialize an empty UFS
            }
            for(const auto& Key : initialData){
                makeKey(Key);
            }
        }

        explicit union_find_set(std::vector<T>& initialData){
            size_t size = initialData.size();
            if(size == 0){
                return; // initialize an empty UFS
            }
            for(const auto& Key : initialData){
                makeKey(Key);
            }
        }

        size_t size() const{
            return data.size();
        }
        size_t makeKey(const T& Key){
            size_t Index = data.size();
            _node NewNode;
            NewNode.parent = Index;
            NewNode.data = Key;
            NewNode.num = 1;
            NewNode.rank = 1;
            data.push_back(NewNode);
            return Index;
        }

        // union operation
        // 0: union_by_rank
        // 1: union_by_num
        // 2: no,just attach LHS parent to RHS
        union_find_set& unionKey(size_t LHS, size_t RHS, int type = 0){
            if(LHS >= size() || RHS >= size() || LHS == RHS){
                return *this;
            }
            size_t LRootIndex = rootKey(LHS);
            size_t RRootIndex = rootKey(RHS);
            if(LRootIndex == RRootIndex){
                return *this;
            }
            bool LChild = true;
            if(type == 0){
                if(data[LRootIndex].rank >= data[RRootIndex].rank){
                    data[RRootIndex].parent = LRootIndex;
                    LChild = false;
                }else{
                    data[LRootIndex].parent = RRootIndex;
                    LChild = true;
                }
            }else if(type == 1){
                // union by number
                if(data[LRootIndex].num >= data[RRootIndex].num){
                    data[RRootIndex].parent = LRootIndex;
                    LChild = false;

                }else{
                    data[LRootIndex].parent = RRootIndex;
                    LChild = true;
                }
            }else{
                data[LRootIndex].parent = RRootIndex;
                LChild = true;
            }
            if(LChild){
                if(data[RRootIndex].rank == data[LRootIndex].rank){
                    data[RRootIndex].rank += 1;
                }
                data[RRootIndex].num += data[LRootIndex].num;
            }else{
                if(data[RRootIndex].rank == data[LRootIndex].rank){
                    data[LRootIndex].rank += 1;
                }
                data[LRootIndex].num += data[RRootIndex].num;
            }
            return *this;
        }

        // find operation, do not update the rank.
        [[nodiscard]] int doFindRootKey(size_t LHS, bool compress = true) const {
            if(LHS >= size()){
                return -1;
            }
            size_t pIndex = LHS;
            if(pIndex != data[pIndex].parent){
                size_t root = doFindRootKey(data[pIndex].parent, compress);
                if(compress){
                    data[data[pIndex].parent].num -= 1;
                    data[pIndex].parent = root;
                }
                return root;
            }else{
                return pIndex;
            }
        }

        // This requires T is not a reference.
        T getData(size_t Index){
            if(Index >= size()){
                return T();
            }
            return data[Index].data;

        }

        void setData(size_t Index, T& Data){
            if(Index >= size()){
                return;
            }
            this->data[Index].data = Data;
        }

        // For debugging.
        void print() const {
            for(int Index = 0; Index < size(); ++Index){
                std::cout<<"Index:"<<Index<<":["<<data[Index].data<<","<<"parent:"<<data[Index].parent
                    <<",rank:"<<data[Index].rank<<",num:"<<data[Index].num<<"]"<<'\n';
            }
        }
    };
}

#endif