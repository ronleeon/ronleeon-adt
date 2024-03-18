#include "testBTree.h"
#include "testSet.h"
#include "testBbTree.h"
#include "ronleeon/tree/m_tree.h"
#include "ronleeon/tree/B_tree.h"
#include "ronleeon/misc/skip_list.h"
#include "ronleeon/misc/union-find-set.h"
#include "ronleeon/graph/graph.h"
#include <vector>
#include <set>


int main() {
    //ronleeon::tree::B_tree<int,4> bbb;
    //testBTree();
    //testBbTree();
	testSet();
	// ronleeon::graph::DefaultGraph<int> g;
	// auto Node1 = g.addNode(1);
	// auto Node2 =g.addNode(2);
	// auto Node3 =g.addNode(3);

	// g.addEdge(Node1, Node2,12).addEdge(Node2,Node3,22).addEdge(Node1,Node4,33);
	// g.bfs();
	return 0;
}