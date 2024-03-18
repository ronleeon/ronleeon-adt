
#ifndef RONLEEON_ADT_GRAPH_H
#define RONLEEON_ADT_GRAPH_H

#include <string>
#include <vector>
#include <map>
#include <queue>

namespace ronleeon::graph{

	using NodeIdType = unsigned int;
	using EdgeIdType = unsigned int;
	using GraphIdType = unsigned int;

	// ----------------------------------Undirected Graph-----------------------------//

	template <typename NodeTy,typename EdgeTy, typename DataType>
	class Edge;

	template <typename NodeTy,typename EdgeTy, typename DataType>
	class Graph;

	template <typename NodeTy,typename EdgeTy, typename DataType>
	class Node{
		friend class Edge<NodeTy,EdgeTy, DataType>;
		friend class Graph<NodeTy,EdgeTy,DataType>;
	protected:
		// both this and To will add the Edge.
		// Equal to calling ``To.addEdge(this,Edge)``.
		void addEdge(NodeTy* To,EdgeTy& Edge){
			if(!To){
				return;
			}
			if((Edge.getSource()==this&&Edge.getDest()==To)
					|| (Edge.getSource()==To&&Edge.getDest()==this)){
				Edges.push_back(Edge);
				To->Edges.push_back(Edge);
			}
		}

		std::vector<EdgeTy> Edges; // in-coming edges

		size_t Id;


		explicit Node(NodeIdType ID, const DataType& Data):Id(ID),Data(Data){}

	public:

		Node(const Node&)=delete;

		Node(Node&&)=delete;

		DataType Data;

		using edge_iterator =typename std::vector<EdgeTy>::iterator;

		using const_edge_iterator=typename std::vector<EdgeTy>::const_iterator;


		[[nodiscard]] NodeIdType getId() const{
			return Id;
		}

		edge_iterator begin() {
			return Edges.begin();
		}
		edge_iterator end(){
			return Edges.end();
		}
		const_edge_iterator cbegin() const {
			return Edges.begin();
		}
		const_edge_iterator cend() const {
			return Edges.end();
		}

		[[nodiscard]] size_t getEdgeSize() const{
			return Edges.size();
		}

	};

	template <typename NodeTy,typename EdgeTy, typename DataType>
	class Edge{
		friend class Node<NodeTy,EdgeTy, DataType>;
		friend class Graph<NodeTy,EdgeTy,DataType>;
	protected:
		NodeTy* Source;
		NodeTy* Dest;
		size_t Id;

		Edge(NodeTy *Src,NodeTy *Dst,EdgeIdType ID, const DataType& Data):Source(Src)
			,Dest(Dst),Id(ID),Data(Data){}


	public:
		Edge(const Edge& Value) {
			Source = Value.Source;
			Dest = Value.Dest;
			Id = Value.Id;
			Data = Value.Data;
		};

		Edge(Edge&&)=delete;

		DataType Data;


		[[nodiscard]] EdgeIdType getId() const{
			return Id;
		}

		NodeTy* getSource() const {
			return Source;
		}

		NodeTy* getDest() const {
			return Dest;
		}


		NodeTy* getPairedNode(NodeTy* Value) const {
			if(!Value) {
				return nullptr;
			}
			if(Source == Value) {
				return Dest;
			}else if(Dest == Value) {
				return Source;
			}
			return nullptr;
		}

	};

	// Nodes lifetime are handled by Graph.
	template <typename NodeTy,typename EdgeTy, typename DataType>
	class Graph{
	protected:
		template <bool Echo=true>
		void dfs_internal_no_r(std::stack<NodeTy*>& s
			,std::map<NodeTy*,bool>& v,std::ostream &out=std::cout){
			while(!s.empty()){
				bool finish=true;
				auto cur=s.top();
				if(!v[cur]){
					if(Echo){
						out<<"Visiting Node:"<<cur->getId()<<":"<<cur->Data<<'\n';
					}
					v[cur]=true;
				}
				for(auto e_begin=cur->begin(),e_end=cur->end();e_begin!=e_end;++e_begin){
					const auto d= (*e_begin)->getPairedNode(cur);
					if(!v[d]){
						s.push(d);
						finish=false;
						break;
					}
				}
				if(finish){
					s.pop();
				}
			}
		}
		template <bool Echo=true>
		void dfs_internal_r(NodeTy* n,std::map<NodeTy*,bool>& v
			,std::ostream& out=std::cout){
			if(!v[n]){
				v[n]=true;
				if(Echo){
					out<<"Visiting Node:"<<n->getId()<<":"<<n->Data<<'\n';
				}
			}
			for(auto e_begin=n->begin(),e_end=n->end()
				;e_begin!=e_end;++e_begin){
				const auto d=(*e_begin).getPairedNode(n);
				if(!v[d]){
					dfs_internal_r<Echo>(d,v,out);
				}
			}
		}


		// node strategy
		size_t NodeId;

		// edge strategy
		size_t EdgeId;
	public:
		// The graph only maps id to each node,
		// do not hold any edge information.
		std::map<NodeIdType,NodeTy*> Nodes;

		explicit Graph(){
			NumOfNodes=0;
			NumOfEdges=0;
			NodeId = 0;
			EdgeId = 0;
		}

		Graph(const Graph&)=delete;
		Graph(Graph&&)=delete;

	public:
		using node_iterator = typename std::map<NodeIdType,NodeTy*>::iterator;
		using const_node_iterator = typename std::map<NodeIdType,NodeTy*>::const_iterator;

		node_iterator begin()
		{
			return Nodes.begin();
		}
		node_iterator end()
		{
			return Nodes.end();
		}

		const_node_iterator begin() const
		{
			return Nodes.cbegin();
		}

		const_node_iterator end() const
		{
			return Nodes.end();
		}

		const_node_iterator cbegin() const
		{
			return Nodes.cbegin();
		}

		const_node_iterator cend() const
		{
			return Nodes.cend();
		}


		NodeTy* addNode(const DataType& Data){
			auto Node = new NodeTy(NodeId++,Data);
			Nodes.insert({
				Node->getId(),Node
			});
			++NumOfNodes;
			return Node;
		}


		Graph& addEdge(NodeTy* Former, NodeTy* Latter,const DataType& Data){
			if(!Former || !Latter){
				return *this;
			}
			if(Nodes.find(Former->getId()) == Nodes.end() || Nodes.find(Latter->getId()) == Nodes.end()){
				// invalid nodes.
				return *this;
			}
			++NumOfEdges;
			EdgeTy Edge(Former, Latter, EdgeId++, Data);
			Former->addEdge(Latter, Edge);
			return *this;
		}

		~Graph(){
			destroy();
		}

		void destroy(){
			// free all nodes.
			for(std::pair<size_t,NodeTy*> N:Nodes){
				delete N.second;
			}
			NumOfEdges=0;
			NumOfNodes=0;
		}
	public:
		// statistics.
		size_t NumOfNodes;
		size_t NumOfEdges;
	public:
		size_t getNumOfNodes() const{
			return NumOfNodes;
		}

		size_t getNumOfEdges() const{
			return NumOfEdges;
		}

		bool hasNode(NodeTy *Node){
			return Nodes.find(Node->getId())!=Nodes.end();
		}

		// DFS
		template <bool Echo=true>
		void dfs_no_r(std::ostream& out=std::cout){
			size_t numOfNodes=getNumOfNodes();
			std::stack<NodeTy*> s;
			std::map<NodeTy*,bool> v;
			for(auto& p:Nodes){
				v.insert({p.second,false});
			}
			for(auto& p:Nodes){
				const auto& n=p.second;
				if(!v[n]){
					s.push(n);
					dfs_internal_no_r<Echo>(s,v,out);
				}
			}
		}

		template <bool Echo=true>
		void dfs_r(std::ostream& out=std::cout){
			std::map<NodeTy*,bool> v;
			for(auto& p:Nodes){
				v.insert({p.second,false});
			}
			for(auto& p:Nodes){
				const auto& n=p.second;
				if(!v[n]){
					dfs_internal_r<Echo>(n,v,out);
				}
			}
		}

		template <bool Echo=true>
		void bfs(std::ostream& out=std::cout){
			std::map<NodeTy*,bool> v;
			std::queue<NodeTy *> q;
			for(auto& p:Nodes){
				v.insert({p.second,false});
			}
			for(auto& p:Nodes){
				const auto& n=p.second;
				if(!v[n]){
					q.push(n);
					while(!q.empty()) {
						auto cur = q.front();
						q.pop();
						if(Echo){
							out<<"Visiting Node:"<<cur->getId()<<":"<<cur->Data<<'\n';
						}
						v[cur] = true;
						for(auto e_begin=cur->begin(),e_end=cur->end()
						;e_begin!=e_end;++e_begin){
							const auto d=(*e_begin).getPairedNode(cur);
							if(!v[d]){
								q.push(d);
							}
						}
					}

				}
			}
		}

	};

	// Provided default Edge/Node/Graph for use.
	template<typename DataType>
	class DefaultEdge;

	template<typename DataType>
	class DefaultGraph;

	template<typename DataType>
	class DefaultNode:public Node<DefaultNode<DataType>,DefaultEdge<DataType>, DataType>{
		friend class DefaultGraph<DataType>;
		friend class Graph<DefaultNode<DataType>,DefaultEdge<DataType>, DataType>;
	private:
		DefaultNode(NodeIdType ID, const DataType& Data):Node<DefaultNode<DataType>,DefaultEdge<DataType>, DataType>(ID,Data){}
	};
	template<typename DataType>
	class DefaultEdge: public Edge<DefaultNode<DataType>,DefaultEdge<DataType>,DataType>{
		friend class DefaultGraph<DataType>;
		friend class Graph<DefaultNode<DataType>,DefaultEdge<DataType>, DataType>;
	private:
		DefaultEdge(DefaultNode<DataType> *Src,DefaultNode<DataType>  *Dst,EdgeIdType ID, const DataType& Data)
			:Edge<DefaultNode<DataType>,DefaultEdge<DataType>,DataType>(Src,Dst,ID,Data){}
	};
	template<typename DataType>
	class DefaultGraph:public Graph<DefaultNode<DataType>,DefaultEdge<DataType>, DataType>{
	};


}

#endif // RONLEEON_ADT_GRAPH_H

