#ifndef SIMPLEBOOLEAN_EDGE_LOOP_H
#define SIMPLEBOOLEAN_EDGE_LOOP_H
#include <vector>

namespace simpleboolean
{

class EdgeLoop
{
public:
    static void merge(std::vector<std::vector<size_t>> &edgeLoops);
    static void buildEdgeLoopsFromDirectedEdges(const std::vector<std::pair<size_t, size_t>> &edges,
        std::vector<std::vector<size_t>> *edgeLoops,
        bool allowOpenEdgeLoop=false,
        bool allowOppositeEdgeLoop=true);
};

}

#endif
