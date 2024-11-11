#include<iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

using namespace std;
namespace fs = filesystem;

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
    string name;
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

    name = filename;

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
    vector<Graph> graphs;

    for (const auto& entry : filesystem::directory_iterator(fs::current_path())) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            string fileName = entry.path().filename().string();
            Graph graph = Graph(fileName);
            graphs.push_back(graph);
        }
    }

    const string red = "\033[1;31m";
    const string green = "\033[1;32m";
    const string blue = "\033[1;34m";
    const string yellow = "\033[1;33m";
    const string magento = "\033[38;5;129m";
    const string reset = "\033[0m";

    cout << magento << "Algorithms and Computability " << reset << endl;

    if (!graphs.empty()) {
        cout << endl;
        cout << yellow << "Graph files found in current directory:\n" << reset;
        for (size_t i = 0; i < graphs.size(); ++i) {
            cout << "\t" << blue << i << ": " << green << graphs[i].name << reset << endl;
        }
    } else {
        cout << red << "No .txt files found in the current directory." << reset << endl;
        return 1;
    }

    cout << endl;
    cout << yellow << "Program modes:" << reset << endl;
    cout << "\t" << blue << 0 << ": " << green << "size of graph" << reset << endl;
    cout << "\t" << blue << 1 << ": " << green << "hamming distance" << reset << endl;
    cout << "\t" << blue << 2 << ": " << green << "maximal cycle length" << reset << endl;
    cout << "\t" << blue << 3 << ": " << green << "minimal extension" << reset << endl;
    cout << "\t" << blue << 4 << ": " << green << "exit program" << reset << endl;

    while (true) {
        int choice = -1;
        cout << endl;
        cout << yellow << "Enter your choice (index of mode): " << reset;
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << red << "Invalid input. Please enter a valid number." << reset << endl;
            continue;
        }

        if (choice == 4) {
            cout << magento << "Exiting program... " << reset << endl;
            break;
        }

        int graphIndex1, graphIndex2;
        switch (choice) {
            case 0:
                cout << "\t" << "Enter the graph index: " << reset;
                if (!(cin >> graphIndex1)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << red << "\t" << "Invalid input! Please enter a valid graph index." << reset << endl;
                } else if (graphIndex1 < graphs.size()) {
                    Graph graph = graphs[graphIndex1];
                    cout << "\t" << "Size of graph " << magento << graph.name << reset
                        << " is " << green << graph.size() << reset << endl;
                } else {
                    cout << red << "\t" << "Wrong graph index!" << reset << endl;
                }
                break;
            case 1:
                if (graphs.size() < 2) {
                    cout << red << "\t" << "Calculating distance requires at least 2 graphs!" << reset << endl;
                    break;
                }
                cout << "\t" << "Enter the first graph index: " << reset;
                if (!(cin >> graphIndex1)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << red << "\t" << "Invalid input! Please enter a valid graph index." << reset << endl;
                    break;
                }
                cout << "\t" << "Enter the second graph index: " << reset;
                if (!(cin >> graphIndex2)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << red << "\t" << "Invalid input! Please enter a valid graph index." << reset << endl;
                    break;
                }
                if (graphIndex1 < graphs.size() && graphIndex2 < graphs.size()) {
                    Graph graph1 = graphs[graphIndex1];
                    Graph graph2 = graphs[graphIndex2];
                    cout << "\t" << "The distance between " << magento << graph1.name << reset << 
                    " and " << magento << graph2.name << reset << " is " << green << 
                    graph1.hammingDistance(graph2) << reset << endl;
                } else {
                    cout << red << "\t" << "Wrong graph index/indices!" << reset << endl;
                }
                break;
            case 2:
                cout << "\t" << "Enter the graph index: " << reset;
                if (!(cin >> graphIndex1)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << red << "\t" << "Invalid input! Please enter a valid graph index." << reset << endl;
                    break;
                }

                if (graphIndex1 < graphs.size()) {
                    Graph graph = graphs[graphIndex1];
                    cout << "\t" << "Maximal cycle length of graph " << magento << graph.name << reset 
                    << " is " << green << graph.maximalCycleLength() << reset << endl;
                } else {
                    cout << red << "\t" << "Wrong graph index!" << reset << endl;
                }
                break;
            case 3:
                cout << "\t" << "Enter the graph index: " << reset;
                if (!(cin >> graphIndex1)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << red << "\t" << "Invalid input! Please enter a valid graph index." << reset << endl;
                    break;
                }

                if (graphIndex1 < graphs.size()) {
                    Graph graph = graphs[graphIndex1];
                    cout << "\t" << "Finding minimal extension of graph " << magento << graph.name << reset << endl;
                    cout << green << "\t";
                    graph.minimalExtension();
                    cout << reset;
                } else {
                    cout << red << "\t" << "Wrong graph index!" << reset << endl;
                }
                break;
            default:
                cout << red << "Invalid choice. Please try again." << reset << endl;
                break;
        }
    }

    return 0;
}