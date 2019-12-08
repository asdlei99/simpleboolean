#ifndef SIMPLEBOOLEAN_MESH_COMBINER_H
#define SIMPLEBOOLEAN_MESH_COMBINER_H
#include <vector>
#include <map>
#include <simpleboolean/meshdatatype.h>
#include <simpleboolean/axisalignedboundingbox.h>
#include <simpleboolean/distinguish.h>

namespace simpleboolean 
{

class MeshCombiner
{
public:
    void setMeshes(const Mesh &first, const Mesh &second);
    bool combine();
    void getResult(Type booleanType, Mesh *result);
    ~MeshCombiner();
    std::vector<Mesh> m_debugSubBlocks;
private:
    void searchPotentialIntersectedPairs();
    bool intersectTwoFaces(size_t firstIndex, size_t secondIndex, std::pair<Vertex, Vertex> &newEdge);
    void addMeshToAxisAlignedBoundingBox(const Mesh &mesh, AxisAlignedBoudingBox &box);
    void addFaceToAxisAlignedBoundingBox(const Mesh &mesh, const Face &face, AxisAlignedBoudingBox &box);
    std::tuple<int, int, int> vertexToKey(const Vertex &vertex);
    size_t newVertexToIndex(const Vertex &vertex);
    void groupEdgesToLoops(const std::vector<std::pair<size_t, size_t>> &edges,
        std::vector<std::vector<size_t>> &edgeLoops);
    Mesh m_firstMesh;
    Mesh m_secondMesh;
    std::vector<Face> m_firstTriangles;
    std::vector<bool> m_firstVisitedTriangles;
    std::vector<Face> m_secondTriangles;
    std::vector<bool> m_secondVisitedTriangles;
    static int m_vertexToKeyMultiplyFactor;
    std::vector<AxisAlignedBoudingBox> m_firstMeshFaceAABBs;
    std::vector<AxisAlignedBoudingBox> m_secondMeshFaceAABBs;
    std::vector<Vertex> m_newVertices;
    std::map<std::tuple<int, int, int>, size_t> m_newVertexToIndexMap;
    std::vector<SubBlock> m_subBlocks;
    std::vector<int> m_indicesToSubBlocks;
    std::vector<std::pair<size_t, size_t>> *m_potentialIntersectedPairs = nullptr;
};

}

#endif
