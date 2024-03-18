#ifndef RONLEEON_ADT_SKIPLIST_H
#define RONLEEON_ADT_SKIPLIST_H


namespace ronleeon{

    template<typename DataType>
    struct skip_list_node{
        skip_list_node * next;
        skip_list_node * pre;
        skip_list_node * above;
        skip_list_node * below;
        DataType value;
    };
    template<typename DataType>
    class skip_list{
     };


}

#endif //RONLEEON_ADT_SKIPLIST_H
