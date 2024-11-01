class IGraph {
public:
    virtual ~IGraph() = default;
    virtual int size() const = 0;
    virtual int hammingDistance(const IGraph& other) const = 0;
    virtual int maximalCycleLength() const = 0;
    virtual void minimalExtension() = 0;
};
