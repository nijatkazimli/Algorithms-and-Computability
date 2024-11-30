#include<iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
#include <numeric>
#include <set>

using namespace std;
namespace fs = filesystem;

#ifdef _WIN32
const string red = "";
const string green = "";
const string blue = "";
const string yellow = "";
const string magenta = "";
const string reset = "";
#else
const string red = "\033[1;31m";
const string green = "\033[1;32m";
const string blue = "\033[1;34m";
const string yellow = "\033[1;33m";
const string magenta = "\033[38;5;129m";
const string reset = "\033[0m";
#endif

class IGraph {
public:
    virtual ~IGraph() = default;
    virtual tuple<int, int> size() const = 0;
    virtual vector<int> hammingDistances(const IGraph& other) const = 0;
    virtual vector<int> hammingDistanceExact(const IGraph& other) const = 0;
    virtual void maximalCycleLength() const = 0;
    virtual void minimalExtension() = 0;
    virtual void printAdjMatrix() const = 0;
    virtual bool checkIfDirected() const = 0;
};

class Graph : public IGraph {
public:
    Graph(const string& filename);
    Graph(int n, const vector<vector<int>> adjMatrix);
    string name;
    bool isDirected;
    tuple<int, int> size() const override;
    vector<int> hammingDistances(const IGraph& other) const override;
    vector<int> hammingDistanceExact(const IGraph& other) const override;
    void maximalCycleLength() const override;
    void minimalExtension() override;
    void printAdjMatrix() const override;

private:
    vector<vector<int>> adjMatrix;
    int vertices;

    void dfs(int v, vector<bool>& visited, vector<int>& path, int& maxLength, vector<vector<int>>& maxCycles, int start) const;
    bool isHamiltonianCycle(int pos, vector<bool>& visited, int count, int start) const;
    bool checkIfDirected() const override;
};

Graph::Graph(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Unable to open file " << filename << endl;
        exit(EXIT_FAILURE);
    }

    name = filename;

    if (!(file >> vertices) || vertices <= 0) {
        cerr << "Error: Invalid number of vertices in file." << endl;
        file.close();
        exit(EXIT_FAILURE);
    }

    adjMatrix.resize(vertices, vector<int>(vertices));
    for (int i = 0; i < vertices; ++i) {
        for (int j = 0; j < vertices; ++j) {
            if (!(file >> adjMatrix[i][j])) {
                cerr << "Error: Insufficient or invalid data in adjacency matrix." << endl;
                file.close();
                exit(EXIT_FAILURE);
            }
        }
    }

    isDirected = checkIfDirected();

    file.close();
}

Graph::Graph(int n, const vector<vector<int>> manualAdjMatrix) {
  name = "manual";
  vertices = n;
  adjMatrix.resize(n, vector<int>(n));
  for (int i = 0; i < vertices; ++i) {
    for (int j = 0; j < vertices; ++j) {
        adjMatrix[i][j] = manualAdjMatrix[i][j];
    }
  }
  isDirected = checkIfDirected();
}

bool Graph::checkIfDirected() const {
    for (int i = 0; i < vertices; ++i) {
        for (int j = 0; j < vertices; ++j) {
            if (adjMatrix[i][j] != adjMatrix[j][i]) {
                return true;
            }
        }
    }
    return false;
}

void Graph::printAdjMatrix() const {
    cout << "Adjacency Matrix:" << endl;
    for (const auto& row : adjMatrix) {
        for (const auto& value : row) {
            cout << value << " ";
        }
        cout << endl;
    }
}

void printAdjMat(const vector<vector<int>>& mat) {
    for (const auto& row : mat) {
        cout << "\t";
        for (const auto& value : row) {
            cout << value << " ";
        }
        cout << endl;
    }
    cout << endl;
}

// edge + vertices
tuple<int, int> Graph::size() const {
    int edges = 0;
    int n = adjMatrix.size();

    if (isDirected) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (adjMatrix[i][j] != 0) {
                    edges++;
                }
            }
        }
    } else {
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (adjMatrix[i][j] != 0) {
                    edges++;
                }
            }
        }
    }

    return make_tuple(edges, vertices);
}

vector<int> Graph::hammingDistances(const IGraph& other) const {
    const Graph& otherGraph = dynamic_cast<const Graph&>(other);
    int m = otherGraph.adjMatrix.size();
    int n = adjMatrix.size();

    // Determine which graph is larger
    const Graph* largerGraph = (n >= m) ? this : &otherGraph;
    const Graph* smallerGraph = (n >= m) ? &otherGraph : this;

    int largerSize = largerGraph->adjMatrix.size();
    int smallerSize = smallerGraph->adjMatrix.size();

    vector<int> distances; // Vector to store Hamming distances
    vector<vector<vector<int>>> subsets; // Vector to store subsets
    set<vector<vector<int>>> uniqueMatrices;

    // Generate all subsets of size 'smallerSize' from [0, largerSize-1]
    vector<int> indices(largerSize);
    iota(indices.begin(), indices.end(), 0); // [0, 1, ..., largerSize-1]
    vector<bool> selectSubset(smallerSize, true); // First 'smallerSize' elements set to true
    selectSubset.resize(largerSize, false);           // Rest set to false

    do {
        // Extract subset indices
        vector<int> subsetIndices;
        for (int i = 0; i < largerSize; ++i) {
            if (selectSubset[i]) subsetIndices.push_back(i);
        }

        // Create induced subgraph for the selected subset
        vector<vector<int>> inducedMatrix(smallerSize, vector<int>(smallerSize, 0));
        for (int i = 0; i < smallerSize; ++i) {
            for (int j = 0; j < smallerSize; ++j) {
                inducedMatrix[i][j] = largerGraph->adjMatrix[subsetIndices[i]][subsetIndices[j]];
            }
        }

        if (uniqueMatrices.count(inducedMatrix) > 0) {
            continue;
        }
        uniqueMatrices.insert(inducedMatrix);

        // Calculate Hamming distance with the smaller graph
        int distance = 0;
        for (int i = 0; i < smallerSize; ++i) {
            for (int j = 0; j < smallerSize; ++j) {
                if (inducedMatrix[i][j] != smallerGraph->adjMatrix[i][j]) {
                    distance++;
                }
            }
        }

        distances.push_back(distance);
        subsets.push_back(inducedMatrix);

    } while (prev_permutation(selectSubset.begin(), selectSubset.end()));

    if (m == n) {
        cout << "\t" << "The hamming distance between the graphs is " << green << distances.at(0) << reset << endl;  
    } else {
        auto minIt = min_element(distances.begin(), distances.end());
        cout << "\t" << "Sizes of the graphs differ. So, comparing the smaller graph with the subsets of the bigger one." << endl;
        cout << "\t" << "There exists " << green << subsets.size() << " subsets" << reset << ", hence that many hamming distances" << 
        " with the smallest one being " << green << *minIt << reset << "." << endl << endl;
        for (int i = 0; i < distances.size(); i++) {
            cout << "\t" << i + 1 << ")" << endl;
            cout << "\t" << "Distance: " << green << distances.at(i) << reset << endl;
            cout << "\t" << "Subset: " << endl;
            printAdjMat(subsets.at(i));
        }
    }

    return distances;
}

// placeholder
// how about isomorphic graphs
// for exact consider all permutations
vector<int> Graph::hammingDistanceExact(const IGraph& other) const {
    const Graph& otherGraph = dynamic_cast<const Graph&>(other);
    int m = otherGraph.adjMatrix.size();
    int n = adjMatrix.size();

    const Graph* largerGraph = (n >= m) ? this : &otherGraph;
    const Graph* smallerGraph = (n >= m) ? &otherGraph : this;

    int largerSize = largerGraph->adjMatrix.size();
    int smallerSize = smallerGraph->adjMatrix.size();

    vector<int> distances; // Store Hamming distances
    set<vector<vector<int>>> uniqueMatrices;
    vector<vector<vector<int>>> bestPermutations; // Store best permutations

    // Generate all subsets of size 'smallerSize' from [0, largerSize-1]
    vector<int> indices(largerSize);
    iota(indices.begin(), indices.end(), 0); // [0, 1, ..., largerSize-1]
    vector<bool> selectSubset(smallerSize, true); // First 'smallerSize' elements set to true
    selectSubset.resize(largerSize, false); // Rest set to false

    do {
        // Extract subset indices
        vector<int> subsetIndices;
        for (int i = 0; i < largerSize; ++i) {
            if (selectSubset[i]) subsetIndices.push_back(i);
        }

        // Create induced subgraph for the selected subset
        vector<vector<int>> inducedMatrix(smallerSize, vector<int>(smallerSize, 0));
        for (int i = 0; i < smallerSize; ++i) {
            for (int j = 0; j < smallerSize; ++j) {
                inducedMatrix[i][j] = largerGraph->adjMatrix[subsetIndices[i]][subsetIndices[j]];
            }
        }

        if (uniqueMatrices.count(inducedMatrix) > 0) continue;
        uniqueMatrices.insert(inducedMatrix);

        vector<int> vertexIndices(smallerSize);
        iota(vertexIndices.begin(), vertexIndices.end(), 0); // [0, 1, ..., smallerSize-1]
        int minDistance = INT_MAX;
        vector<vector<int>> bestPermutation;

        do {
            vector<vector<int>> permutedMatrix(smallerSize, vector<int>(smallerSize, 0));
            for (int i = 0; i < smallerSize; ++i) {
                for (int j = 0; j < smallerSize; ++j) {
                    permutedMatrix[i][j] = inducedMatrix[vertexIndices[i]][vertexIndices[j]];
                }
            }

            int distance = 0;
            for (int i = 0; i < smallerSize; ++i) {
                for (int j = 0; j < smallerSize; ++j) {
                    if (permutedMatrix[i][j] != smallerGraph->adjMatrix[i][j]) {
                        distance++;
                    }
                }
            }

            if (distance < minDistance) {
                minDistance = distance;
                bestPermutation = permutedMatrix;
            }

        } while (next_permutation(vertexIndices.begin(), vertexIndices.end()));

        distances.push_back(minDistance);
        bestPermutations.push_back(bestPermutation);

    } while (prev_permutation(selectSubset.begin(), selectSubset.end()));

    if (m == n) {
        cout << "\t" << "The hamming distance between the graphs is " << green << distances.at(0) << reset << endl;  
    } else {
        auto minIt = min_element(distances.begin(), distances.end());
        cout << "\t" << "Sizes of the graphs differ. So, comparing the smaller graph with the subsets of the bigger one." << endl;
        cout << "\t" << "There exists " << green << bestPermutations.size() << " (best) subsets" << reset << ", hence that many hamming distances" << 
        " with the smallest one being " << green << *minIt << reset << "." << endl << endl;
        for (int i = 0; i < distances.size(); i++) {
            cout << "\t" << i + 1 << ")" << endl;
            cout << "\t" << "Distance: " << green << distances.at(i) << reset << endl;
            cout << "\t" << "Subset: " << endl;
            printAdjMat(bestPermutations.at(i));
        }
    }

    return distances;
}

void Graph::dfs(int v, vector<bool>& visited, vector<int>& path, int& maxLength, vector<vector<int>>& maxCycles, int start) const {
    visited[v] = true;
    path.push_back(v);

    for (int u = 0; u < adjMatrix.size(); ++u) {
        if (adjMatrix[v][u] != 0) {  // There is an edge between v and u
            if (!visited[u]) {
                dfs(u, visited, path, maxLength, maxCycles, start);
            } else if (u == start && path.size() > 2) {
                // A cycle is detected (back to the start)
                if (path.size() > maxLength) {
                    maxLength = path.size();
                    maxCycles.clear();
                }
                if (path.size() == maxLength) {
                    maxCycles.push_back(path);
                }
            }
        }
    }

    visited[v] = false;
    path.pop_back();
}

void Graph::maximalCycleLength() const {
    int n = adjMatrix.size();
    vector<bool> visited(n, false);
    vector<int> path;
    int maxLength = 0;
    vector<vector<int>> maxCycles;
    vector<vector<int>> uniqueCycles; // To store unique maximal cycles

    for (int i = 0; i < n; ++i) {
        dfs(i, visited, path, maxLength, maxCycles, i);
    }

    if (maxLength == 0) {
        cout << red << "NO CYCLES FOUND!" << reset;
        return;
    }

    cout << maxLength << "." << endl << endl;
    // cout << "There exist(s) " << maxCycles.size() << " cycle(s) of maximal length." << endl << endl;    // maxCycles.size() doesn't output the correct number 
    cout << "\t" << blue << "Cycle path(s):" << green << endl;

    vector<vector<int>> savedCycles;
    int undirectedCounter = 0;

    for (const auto& cycle : maxCycles) {
        // Normalize the cycle to always start from the smallest vertex
        vector<int> normalizedCycle = cycle;
        int minVertex = *min_element(cycle.begin(), cycle.end());

        // Rotate the cycle so that it starts from the minimum vertex
        while (normalizedCycle.front() != minVertex) {
            rotate(normalizedCycle.begin(), normalizedCycle.begin() + 1, normalizedCycle.end());
        }

        if (isDirected) {
            // For directed graphs, check both clockwise and counterclockwise versions
            vector<int> clockwiseCycle = normalizedCycle;
            vector<int> counterClockwiseCycle = normalizedCycle;
            reverse(counterClockwiseCycle.begin(), counterClockwiseCycle.end());

            // Choose the lexicographically smaller cycle (clockwise vs counterclockwise)
            vector<int> smallestCycle = (clockwiseCycle < counterClockwiseCycle) ? clockwiseCycle : counterClockwiseCycle;

            // Ensure uniqueness of cycles
            bool isDuplicate = false;
            for (const auto& existingCycle : uniqueCycles) {
                if (existingCycle == smallestCycle) {
                    isDuplicate = true;
                    break;
                }
            }

            if (!isDuplicate) {
                uniqueCycles.push_back(smallestCycle);
                cout << "\t";
                for (int v : smallestCycle) {
                    cout << v << " ";
                }
                cout << smallestCycle[0] << endl;
            }
        } else {
            // For undirected graphs, we only store the normalized version of the cycle if its reversed version doesn't exist
            vector<int> counterClockwiseCycle = normalizedCycle;
            reverse(counterClockwiseCycle.begin(), counterClockwiseCycle.end());

            // Check if either the normalized cycle or its reversed version is already stored
            bool isDuplicate = false;
            for (const auto& existingCycle : uniqueCycles) {
                if (existingCycle == normalizedCycle || existingCycle == counterClockwiseCycle) {
                    isDuplicate = true;
                    break;
                }
            }

            if (!isDuplicate) {
                uniqueCycles.push_back(normalizedCycle);
                savedCycles.push_back(normalizedCycle); // Save to the container
            }
        }
    }

    int undirectedSize = uniqueCycles.size() / 2;

    if (!isDirected && !savedCycles.empty()) {
        for (const auto& cycle : savedCycles) {
            cout << "\t";
            for (int v : cycle) {
                cout << v << " ";  
            }
            cout << cycle[0] << endl;  

            undirectedCounter++;

            if (undirectedCounter == undirectedSize) {
                break; 
            }
        } 
    }

    if (uniqueCycles.empty()) {
        cout << red << "No unique cycles found!" << reset << endl;
    } else {
        cout << "\n\t" << "Number of cycles of maximal length is ";

        if(isDirected) {
            cout << uniqueCycles.size();
        } else {
            cout << uniqueCycles.size() / 2;
        } 
        cout << "." << endl;
    }
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

// Hamiltonian!!
// assume connectivity
// I should print the edges
// DOES NOT WORK WITH DIRECTED
void Graph::minimalExtension() {
    int n = adjMatrix.size();

    // Check if the graph already has a Hamiltonian cycle
    vector<bool> visited(n, false);
    for (int start = 0; start < n; ++start) {
        visited[start] = true;
        if (isHamiltonianCycle(start, visited, 1, start)) {
            cout << "The graph already has a Hamiltonian cycle." << endl;
            return;
        }
        visited[start] = false;
    }

    // Handle cases where more than one edge might be needed
    // Use BFS to search for the minimal set of edges to add
    queue<vector<pair<int, int>>> edgeQueue; // Queue to store edges being added
    edgeQueue.push({}); // Start with no edges added

    while (!edgeQueue.empty()) {
        auto currentEdges = edgeQueue.front();
        edgeQueue.pop();

        // Temporarily add edges from the current set
        for (const auto& edge : currentEdges) {
            adjMatrix[edge.first][edge.second] = 1;
            adjMatrix[edge.second][edge.first] = 1;
        }

        // Check if a Hamiltonian cycle exists
        for (int start = 0; start < n; ++start) {
            fill(visited.begin(), visited.end(), false);
            visited[start] = true;
            if (isHamiltonianCycle(start, visited, 1, start)) {
                // Report the edges added
                cout << "Added edges to create a Hamiltonian cycle: ";
                for (const auto& edge : currentEdges) {
                    cout << magenta << "[ " << green << edge.first << " - " << edge.second << magenta << " ]" << reset << " ";
                }
                cout << reset << endl;

                // Restore the graph to its original state before returning
                for (const auto& edge : currentEdges) {
                    adjMatrix[edge.first][edge.second] = 0;
                    adjMatrix[edge.second][edge.first] = 0;
                }
                return;
            }
        }

        // Generate new edge combinations by adding one more edge
        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                if (adjMatrix[u][v] == 0) {
                    auto newEdges = currentEdges;
                    newEdges.push_back({u, v});
                    edgeQueue.push(newEdges);
                }
            }
        }

        // Restore the graph to its original state
        for (const auto& edge : currentEdges) {
            adjMatrix[edge.first][edge.second] = 0;
            adjMatrix[edge.second][edge.first] = 0;
        }
    }

    // If no Hamiltonian cycle could be created
    cout << "Could not create a Hamiltonian cycle even with added edges." << endl;
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

    cout << magenta << "Algorithms and Computability " << reset << endl;

    if (!graphs.empty()) {
        cout << endl;
        cout << yellow << "Graph files found in current directory:\n" << reset;
        for (size_t i = 0; i < graphs.size(); ++i) {
            cout << "\t" << blue << i << ": " << green << graphs[i].name << reset << " - " 
            << yellow << (graphs[i].isDirected ? "directed" : "undirected") << reset << endl;
        }
    } else {
        cout << red << "No .txt files found in the current directory." << reset << endl;
        return 1;
    }

    cout << endl;
    cout << yellow << "Program modes:" << reset << endl;
    cout << "\t" << blue << 0 << ": " << green << "size of graph " << magenta << "(# of edges + vertices)" << reset << endl;
    cout << "\t" << blue << 1 << ": " << green << "hamming distance" << reset << endl;
    cout << "\t" << blue << 2 << ": " << green << "hamming distance exact" << reset << endl;
    cout << "\t" << blue << 3 << ": " << green << "maximal cycle length" << reset << endl;
    cout << "\t" << blue << 4 << ": " << green << "minimal extension" << reset << endl;
    cout << "\t" << blue << 5 << ": " << green << "exit program" << reset << endl;

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

        if (choice == 5) {
            cout << magenta << "Exiting program... " << reset << endl;
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
                    auto [edges, vertices] = graph.size();
                    cout << "\t" << "Size of graph " << magenta << graph.name << reset
                        << " is " << green << edges << " + " << vertices << " = " << edges + vertices << reset << endl;
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
                    graph1.hammingDistances(graph2);
                } else {
                    cout << red << "\t" << "Wrong graph index/indices!" << reset << endl;
                }
                break;
            case 2:
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
                    graph1.hammingDistanceExact(graph2);
                } else {
                    cout << red << "\t" << "Wrong graph index/indices!" << reset << endl;
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
                    cout << "\t" << "Maximal cycle length of graph " << magenta << graph.name << reset 
                    << " is " << green;
                    graph.maximalCycleLength();
                    cout << reset << endl;
                } else {
                    cout << red << "\t" << "Wrong graph index!" << reset << endl;
                }
                break;
            case 4:
                cout << "\t" << "Enter the graph index: " << reset;
                if (!(cin >> graphIndex1)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << red << "\t" << "Invalid input! Please enter a valid graph index." << reset << endl;
                    break;
                }

                if (graphIndex1 < graphs.size()) {
                    Graph graph = graphs[graphIndex1];
                    cout << "\t" << "Finding minimal extension of graph " << magenta << graph.name << reset << endl;
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
