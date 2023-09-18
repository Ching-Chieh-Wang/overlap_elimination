#ifndef _GRAPH_H
#define _GRAPH_H
#include "../pch.h"
template <class E>
class Graph {
protected:
	std::unordered_map<int, std::unordered_map<int,E&>> adj;
	std::list<E> _edges;
public:
	std::unordered_map<int, E&>& neighbors(int v) {
		return adj.at(v);
	}
	inline std::list<E>& edges() {
		return _edges;
	}
	inline size_t vCount() {
		return adj.size();
	}
	inline size_t eCount() {
		return _edges.size();
	}
	inline void addVertex(const int id) {
		adj[id];
	}
	inline void addEdge(const int v1, const int v2,const E& e) {
		auto it = adj[v1].find(v2);
		if (it != adj[v1].end()) {
			throw ("Edge existed between " + std::to_string(v1)+" and " + std::to_string(v2));
		}
		else {
			_edges.emplace_back(e);
			adj[v1].emplace_hint(it, v2, _edges.back());
			adj[v2].emplace(v1,_edges.back());
		}
	}
	inline E& edge(const int v1, const int v2) {
		return adj.at(v1).at(v2);
	}
	std::vector<int> vertices() const{
		std::vector<int> results;
		results.reserve(adj.size());
		for (const auto& [id, edge]:adj) {
			results.emplace_back(id);
		}
		return results;
	}
	void initialize(const size_t bucketCount, const std::hash<int>& hashFunc, const std::equal_to<int>& keyEq) {
		adj = std::unordered_map<int, std::unordered_map<int, E&>>(bucketCount, hashFunc, keyEq);
	}

};

template <class E>
class WeightedGraph:public Graph<E> {
public:
	using W = decltype(std::declval<E>().w());
	class VCompare {
	public:
		inline bool operator()(const std::pair<int, W> v1, const std::pair<int, W> v2) {
			return v1.second > v2.second;
		}
	};
	void bFSRecurr(const int visitingV, std::unordered_set<int>& visitedVs, std::vector<std::pair<int, int>>& bFSResults);
	std::vector<std::pair<int, int>> bFS(const int startV);
	std::vector<std::pair<int, int>> mst(const int startV);
	void mstRecurr(std::priority_queue< std::pair<int, W>, std::vector < std::pair<int, W>>, VCompare>& candidates, std::unordered_map<int, W>& distances, std::unordered_map<int, int>& predecessors);
	double shortestPath(int v1, int v2, std::vector<int>& paths);
	void shortestPathRecur(std::priority_queue< std::pair<int, W>, std::vector < std::pair<int, W>>, VCompare>& candidates, std::unordered_map<int, W>& distances, std::unordered_set<int>& visitedVs, std::unordered_map<int, int>& predecessors);
};

template<typename E>
void WeightedGraph<E>::bFSRecurr(const int visitingV, std::unordered_set<int>& visitedVs, std::vector<std::pair<int, int>>& bFSResults) {
	if (visitedVs.find(visitingV) != visitedVs.end()) return;
	const auto neighborVs = this->neighbors(visitingV);
	for (const auto& [neighborV, e] : neighborVs) {
		if (visitedVs.find(neighborV) != visitedVs.end()) continue;
		bFSResults.emplace_back(visitingV, neighborV);
	}
	visitedVs.emplace(visitingV);
	for (const auto& [neighborV, e] : neighborVs) {
		bFSRecurr(neighborV, visitedVs, bFSResults);
	}
}
template<typename E>
std::vector<std::pair<int, int>> WeightedGraph<E>::bFS(const int startV) {
	std::unordered_set<int> visitedVs;
	std::vector<std::pair<int, int>> bFSResults;
	bFSResults.reserve(this->vCount());
	bFSRecurr(startV, visitedVs, bFSResults);
	return bFSResults;
}
template<typename E>
std::vector<std::pair<int, int>> WeightedGraph<E>::mst(const int startV) {
	std::unordered_map<int, W> distances;
	std::unordered_map<int, int> predecessors;
	std::priority_queue<std::pair<int, W>, std::vector<std::pair<int, W>>, VCompare> candidates;
	distances.reserve(this->vCount());
	predecessors.reserve(this->vCount());
	for (const auto& [v, e] : this->adj) {
		distances[v] = std::numeric_limits<W>::max();
	}
	distances[startV] = 0;
	candidates.emplace(startV, 0);
	mstRecurr(candidates, distances, predecessors);
	WeightedGraph<E> tree;
	for (const auto& [v1, v2] : predecessors) {
		tree.addEdge(v1, v2, this->edge(v1, v2));
	}
	return tree.bFS(startV);
}
template<typename E>
void WeightedGraph<E>::mstRecurr(std::priority_queue< std::pair<int, W>, std::vector < std::pair<int, W>>, VCompare>& candidates, std::unordered_map<int, W>& distances, std::unordered_map<int, int>& predecessors) {
	int visitingV = candidates.top().first;
	distances.erase(visitingV);
	if (distances.empty()) return;
	candidates.pop();
	for (const auto& [neighborV, e] : this->neighbors(visitingV)) {
		auto it = distances.find(neighborV);
		if (it == distances.end())continue;
		if (e.w() < it->second) {
			predecessors[neighborV] = visitingV;
			it->second = e.w();
			candidates.emplace(neighborV, e.w());
		}
	}
	while (distances.find(candidates.top().first) == distances.end()) {
		candidates.pop();
	}
	mstRecurr(candidates, distances, predecessors);
}
template<typename E>
double WeightedGraph<E>::shortestPath(int v1, int v2, std::vector<int>& paths) {
	std::unordered_map<int, W> distances;
	std::priority_queue<std::pair<int, W>, std::vector<std::pair<int, W>>, VCompare> candidates;
	std::unordered_set<int> visitedVs;
	std::unordered_map<int, int> predecessors;
	for (const auto& [v, e] : this->adj) {
		distances[v] = std::numeric_limits<W>::max();
	}
	distances[v2] = 0;
	candidates.emplace(v2, 0);
	shortestPathRecur(candidates, distances, visitedVs, predecessors);
	int visitingV = v1;
	paths.emplace_back(v1);
	while (visitingV != v2) {
		visitingV = predecessors[visitingV];
		paths.emplace_back(visitingV);
	}
	return distances.at(v1);
}
template<typename E>
void WeightedGraph<E>::shortestPathRecur(std::priority_queue< std::pair<int, W>, std::vector < std::pair<int, W>>, VCompare>& candidates, std::unordered_map<int, W>& distances, std::unordered_set<int>& visitedVs, std::unordered_map<int, int>& predecessors) {
	int visitingV = candidates.top().first;
	W visitingW = candidates.top().second;
	visitedVs.emplace(candidates.top().first);
	candidates.pop();
	for (auto& [neighborV, e] : this->neighbors(visitingV)) {
		if (visitedVs.find(neighborV) != visitedVs.end()) continue;
		W& distance = distances.at(neighborV);
		if (visitingW + e.w() < distance) {
			distance = visitingW + e.w();
			candidates.emplace(neighborV, distance);
			predecessors[neighborV] = visitingV;
		}
	}
	if (candidates.empty()) return;
	while (visitedVs.find(candidates.top().first) != visitedVs.end()) {
		candidates.pop();
		if (candidates.empty()) return;
	}
	shortestPathRecur(candidates, distances, visitedVs, predecessors);
}


#endif