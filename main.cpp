#include<iostream>
#include <fstream>

using namespace std;

class IGraph {
public:
    virtual ~IGraph() = default;
    virtual int size() const = 0;
    virtual int hammingDistance(const IGraph& other) const = 0;
    virtual int maximalCycleLength() const = 0;
    virtual void minimalExtension() = 0;
};

class Graph : public IGraph {
public:
    Graph(const string& filename);
    int size() const override;
    int hammingDistance(const IGraph& other) const override;
    int maximalCycleLength() const override;
    void minimalExtension() override;

private:
    vector<vector<int>> adjMatrix;

    void dfs(int v, vector<bool>& visited, vector<int>& path, int& maxLength, int start) const;
    bool isHamiltonianCycle(int pos, vector<bool>& visited, int count, int start) const;
};

Graph::Graph(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Unable to open file" << endl;
        exit(1);
    }

    int n;
    file >> n;
    adjMatrix.resize(n, vector<int>(n));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file >> adjMatrix[i][j];
        }
    }

    file.close();
}

int Graph::size() const {
    int size = 0;
    int n = adjMatrix.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (adjMatrix[i][j] != 0) {
                size++;
            }
        }
    }
    return size;
}

int Graph::hammingDistance(const IGraph& other) const {
    const Graph& otherGraph = dynamic_cast<const Graph&>(other);
    int distance = 0;
    int n = adjMatrix.size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (adjMatrix[i][j] != otherGraph.adjMatrix[i][j]) {
                distance++;
            }
        }
    }
    return distance;
}

void Graph::dfs(int v, vector<bool>& visited, vector<int>& path, int& maxLength, int start) const {
    visited[v] = true;
    path.push_back(v);

    for (int u = 0; u < adjMatrix.size(); ++u) {
        if (adjMatrix[v][u] != 0) {
            if (!visited[u]) {
                dfs(u, visited, path, maxLength, start);
            } else if (u == start && path.size() > maxLength) {
                maxLength = path.size();
            }
        }
    }

    visited[v] = false;
    path.pop_back();
}

int Graph::maximalCycleLength() const {
    int n = adjMatrix.size();
    vector<bool> visited(n, false);
    vector<int> path;
    int maxLength = 0;

    for (int i = 0; i < n; ++i) {
        dfs(i, visited, path, maxLength, i);
    }

    return maxLength;
}

bool Graph::isHamiltonianCycle(int pos, vector<bool>& visited, int count, int start) const {
    if (count == adjMatrix.size()) {
        return adjMatrix[pos][start] != 0;
    }

    for (int u = 0; u < adjMatrix.size(); ++u) {
        if (adjMatrix[pos][u] != 0 && !visited[u]) {
            visited[u] = true;
            if (isHamiltonianCycle(u, visited, count + 1, start)) {
                return true;
            }
            visited[u] = false;
        }
    }
    return false;
}

void Graph::minimalExtension() {
    int n = adjMatrix.size();
    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (adjMatrix[u][v] == 0) {
                adjMatrix[u][v] = adjMatrix[v][u] = 1;
                vector<bool> visited(n, false);
                visited[u] = true;
                if (isHamiltonianCycle(u, visited, 1, u)) {
                    cout << "Added edge: " << u << " - " << v << " to create a Hamiltonian cycle" << endl;
                    return;
                }
                adjMatrix[u][v] = adjMatrix[v][u] = 0;
            }
        }
    }
}

int main() {
    string graph1FileName = "graph.txt";
    string graph2FileName = "another_graph.txt";

    Graph graph(graph1FileName);
    Graph anotherGraph(graph2FileName);

    cout << "Size of the graph: " << graph.size() << endl;
    cout << "Hamming distance between graphs: " << graph.hammingDistance(anotherGraph) << endl;
    cout << "Maximal cycle length: " << graph.maximalCycleLength() << endl;
    graph.minimalExtension();
    return 0;
}