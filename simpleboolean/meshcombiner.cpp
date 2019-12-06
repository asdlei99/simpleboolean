#include <queue>
#include <map>
#include <set>
#include <simpleboolean/meshcombiner.h>
#include <simpleboolean/retriangulation.h>
#include <simpleboolean/edgeloop.h>
#include <simpleboolean/subsurface.h>
#include <simpleboolean/subblock.h>
#include <thirdparty/moller97/tritri_isectline.h>
#include <QDebug>
#include <QElapsedTimer>

namespace simpleboolean
{

size_t MeshCombiner::m_maxOctreeDepth = 3;
size_t MeshCombiner::m_minIntersectsInOctant = 5;
int MeshCombiner::m_vertexToKeyMultiplyFactor = 1000;

void MeshCombiner::setMeshes(const Mesh &first, const Mesh &second)
{
    m_firstMesh = first;
    m_secondMesh = second;
    m_firstMeshFaceAABBs.resize(m_firstMesh.faces.size());
    m_secondMeshFaceAABBs.resize(m_secondMesh.faces.size());
    for (size_t i = 0; i < m_firstMesh.faces.size(); ++i) {
        addFaceToAxisAlignedBoundingBox(m_firstMesh, m_firstMesh.faces[i], m_firstMeshFaceAABBs[i]);
    }
    for (size_t i = 0; i < m_secondMesh.faces.size(); ++i) {
        addFaceToAxisAlignedBoundingBox(m_secondMesh, m_secondMesh.faces[i], m_secondMeshFaceAABBs[i]);
    }
}

void MeshCombiner::addMeshToAxisAlignedBoundingBox(const Mesh &mesh, AxisAlignedBoudingBox &box)
{
    for (const auto &vertex: mesh.vertices) {
        box.update(vertex);
    }
}

void MeshCombiner::addFaceToAxisAlignedBoundingBox(const Mesh &mesh, const Face &face, AxisAlignedBoudingBox &box)
{
    for (size_t i = 0; i < 3; ++i) {
        box.update(mesh.vertices[face.indices[i]]);
    }
}

void MeshCombiner::searchPotentialIntersectedPairs(std::vector<std::pair<size_t, size_t>> &pairs)
{
    AxisAlignedBoudingBox firstBox;
    AxisAlignedBoudingBox secondBox;
    AxisAlignedBoudingBox intersectedBox;
    addMeshToAxisAlignedBoundingBox(m_firstMesh, firstBox);
    addMeshToAxisAlignedBoundingBox(m_secondMesh, secondBox);
    firstBox.intersectWith(secondBox, &intersectedBox);
    std::vector<size_t> firstGroupOfFacesIn;
    std::vector<size_t> secondGroupOfFacesIn;
    for (size_t i = 0; i < m_firstMeshFaceAABBs.size(); ++i) {
        if (intersectedBox.intersectWith(m_firstMeshFaceAABBs[i])) {
            firstGroupOfFacesIn.push_back(i);
        }
    }
    for (size_t i = 0; i < m_secondMeshFaceAABBs.size(); ++i) {
        if (intersectedBox.intersectWith(m_secondMeshFaceAABBs[i])) {
            secondGroupOfFacesIn.push_back(i);
        }
    }
    for (const auto &i: firstGroupOfFacesIn) {
        addFaceToAxisAlignedBoundingBox(m_firstMesh, m_firstMesh.faces[i], intersectedBox);
    }
    for (const auto &i: secondGroupOfFacesIn) {
        addFaceToAxisAlignedBoundingBox(m_secondMesh, m_secondMesh.faces[i], intersectedBox);
    }
    std::queue<std::tuple<size_t, AxisAlignedBoudingBox, std::vector<size_t>, std::vector<size_t>>> octants;
    octants.push({1, intersectedBox, firstGroupOfFacesIn, secondGroupOfFacesIn});
    std::set<std::pair<size_t, size_t>> histories;
    while (!octants.empty()) {
        auto item = octants.front();
        octants.pop();
        std::vector<size_t> firstGroupCandidates;
        std::vector<size_t> secondGroupCandidates;
        for (const auto &i: std::get<2>(item)) {
            if (std::get<1>(item).intersectWith(m_firstMeshFaceAABBs[i]))
                firstGroupCandidates.push_back(i);
        }
        for (const auto &i: std::get<3>(item)) {
            if (std::get<1>(item).intersectWith(m_secondMeshFaceAABBs[i]))
                secondGroupCandidates.push_back(i);
        }
        if (0 == firstGroupCandidates.size() || 0 == secondGroupCandidates.size())
            continue;
        if ((firstGroupCandidates.size() < MeshCombiner::m_minIntersectsInOctant &&
                    secondGroupCandidates.size() < MeshCombiner::m_minIntersectsInOctant) ||
                std::get<0>(item) >= MeshCombiner::m_maxOctreeDepth) {
            for (const auto &i: firstGroupCandidates) {
                for (const auto &j: secondGroupCandidates) {
                    if (m_firstMeshFaceAABBs[i].intersectWith(m_secondMeshFaceAABBs[j])) {
                        std::pair<size_t, size_t> candidate = {i, j};
                        auto insertResult = histories.insert(candidate);
                        if (insertResult.second)
                            pairs.push_back(candidate);
                    }
                }
            }
            continue;
        }
        std::vector<AxisAlignedBoudingBox> children(8);
        if (!std::get<1>(item).makeOctree(children))
            continue;
        for (const auto &child: children) {
            octants.push({std::get<0>(item) + 1, child, firstGroupCandidates, secondGroupCandidates});
        }
    }
}

bool MeshCombiner::intersectTwoFaces(size_t firstIndex, size_t secondIndex, std::pair<Vertex, Vertex> &newEdge)
{
    const auto &firstFace = m_firstMesh.faces[firstIndex];
    const auto &secondFace = m_secondMesh.faces[secondIndex];
    int coplanar = 0;
    if (!tri_tri_intersect_with_isectline(m_firstMesh.vertices[firstFace.indices[0]].xyz,
            m_firstMesh.vertices[firstFace.indices[1]].xyz,
            m_firstMesh.vertices[firstFace.indices[2]].xyz,
            m_secondMesh.vertices[secondFace.indices[0]].xyz,
            m_secondMesh.vertices[secondFace.indices[1]].xyz,
            m_secondMesh.vertices[secondFace.indices[2]].xyz,
            &coplanar,
            newEdge.first.xyz,
            newEdge.second.xyz)) {
        return false;
    }
    if (coplanar)
        return false;
    return true;
}

std::tuple<int, int, int> MeshCombiner::vertexToKey(const Vertex &vertex)
{
    return {vertex.xyz[0] * MeshCombiner::m_vertexToKeyMultiplyFactor,
        vertex.xyz[1] * MeshCombiner::m_vertexToKeyMultiplyFactor,
        vertex.xyz[2] * MeshCombiner::m_vertexToKeyMultiplyFactor
    };
}

size_t MeshCombiner::newVertexToIndex(const Vertex &vertex)
{
    const auto &key = vertexToKey(vertex);
    const auto &findResult = m_newVertexToIndexMap.find(key);
    if (findResult != m_newVertexToIndexMap.end()) {
        return findResult->second;
    }
    size_t newIndex = m_newVertices.size();
    m_newVertexToIndexMap.insert({key, newIndex});
    m_newVertices.push_back(vertex);
    return newIndex;
}

void MeshCombiner::groupEdgesToLoops(const std::vector<std::pair<size_t, size_t>> &edges,
        std::vector<std::vector<size_t>> &edgeLoops)
{
    EdgeLoop::buildEdgeLoopsFromDirectedEdges(edges, &edgeLoops, true, false);
}

bool MeshCombiner::combine()
{
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    std::vector<std::pair<size_t, size_t>> potentailPairs;
    
    auto searchPotentialIntersectedPairsStartTime = elapsedTimer.elapsed();
    searchPotentialIntersectedPairs(potentailPairs);
    qDebug() << "Search potential intersected pairs took" << (elapsedTimer.elapsed() - searchPotentialIntersectedPairsStartTime) << "milliseconds";
    
    auto checkPotentialIntersectedPairsStartTime = elapsedTimer.elapsed();
    std::map<size_t, std::vector<std::pair<size_t, size_t>>> newEdgesPerTriangleInFirstMesh;
    std::map<size_t, std::vector<std::pair<size_t, size_t>>> newEdgesPerTriangleInSecondMesh;
    std::set<size_t> reTriangulatedFacesInFirstMesh;
    std::set<size_t> reTriangulatedFacesInSecondMesh;
    for (const auto &pair: potentailPairs) {
        std::pair<Vertex, Vertex> newEdge;
        if (intersectTwoFaces(pair.first, pair.second, newEdge)) {
            if (vertexToKey(newEdge.first) == vertexToKey(newEdge.second))
                continue;
            std::pair<size_t, size_t> newVertexPair = {
                newVertexToIndex(newEdge.first),
                newVertexToIndex(newEdge.second)
            };
            std::pair<size_t, size_t> newVertexOppositePair = {
                newVertexPair.second,
                newVertexPair.first
            };
            
            newEdgesPerTriangleInFirstMesh[pair.first].push_back(newVertexPair);
            newEdgesPerTriangleInSecondMesh[pair.second].push_back(newVertexPair);

            newEdgesPerTriangleInFirstMesh[pair.first].push_back(newVertexOppositePair);
            newEdgesPerTriangleInSecondMesh[pair.second].push_back(newVertexOppositePair);

            reTriangulatedFacesInFirstMesh.insert(pair.first);
            reTriangulatedFacesInSecondMesh.insert(pair.second);
        }
    }
    qDebug() << "Check potential intersected pairs took" << (elapsedTimer.elapsed() - checkPotentialIntersectedPairsStartTime) << "milliseconds";
    auto doReTriangulation = [&](const Mesh *mesh, const std::map<size_t, std::vector<std::pair<size_t, size_t>>> &newEdgesPerTriangle, std::vector<Face> &toTriangles, std::vector<std::vector<size_t>> &edgeLoops) {
        for (const auto &it: newEdgesPerTriangle) {
            const auto &face = mesh->faces[it.first];
            const std::vector<std::pair<size_t, size_t>> &newEdges = it.second;
            std::vector<std::vector<size_t>> edgeLoopsPerFace;
            groupEdgesToLoops(newEdges, edgeLoopsPerFace);
            std::vector<size_t> triangleVertices = {
                newVertexToIndex(mesh->vertices[face.indices[0]]),
                newVertexToIndex(mesh->vertices[face.indices[1]]),
                newVertexToIndex(mesh->vertices[face.indices[2]]),
            };
            ReTriangulation re(m_newVertices, triangleVertices, edgeLoopsPerFace);
            re.reTriangulate();
            const auto &reTriangulatedTriangles = re.getResult();
            for (const auto &loop: edgeLoopsPerFace) {
                edgeLoops.push_back(loop);
            }
            for (const auto &triangle: reTriangulatedTriangles) {
                toTriangles.push_back(triangle);
            }
        }
    };
    auto addUnIntersectedFaces = [&](const Mesh *mesh, const std::set<size_t> &reTriangulatedFaces,
            std::vector<Face> &toTriangles) {
        for (size_t i = 0; i < mesh->faces.size(); ++i) {
            if (reTriangulatedFaces.find(i) != reTriangulatedFaces.end())
                continue;
            const auto &face = mesh->faces[i];
            Face triangle = {{
                newVertexToIndex(mesh->vertices[face.indices[0]]),
                newVertexToIndex(mesh->vertices[face.indices[1]]),
                newVertexToIndex(mesh->vertices[face.indices[2]]),
            }};
            toTriangles.push_back(triangle);
        }
    };
    auto createSubSurfacesStartTime = elapsedTimer.elapsed();
    std::vector<SubSurface> firstSubSurfaces;
    std::vector<SubSurface> secondSubSurfaces;
    std::vector<std::vector<size_t>> firstMergedEdgeLoops;
    std::vector<std::vector<size_t>> secondMergedEdgeLoops;
    {
        std::vector<std::vector<size_t>> edgeLoops;
        std::vector<Face> triangles;
        doReTriangulation(&m_firstMesh, newEdgesPerTriangleInFirstMesh, triangles, edgeLoops);
        addUnIntersectedFaces(&m_firstMesh, reTriangulatedFacesInFirstMesh, triangles);
        EdgeLoop::merge(edgeLoops, &firstMergedEdgeLoops);
        SubSurface::createSubSurfaces(firstMergedEdgeLoops, triangles, firstSubSurfaces);
    }
    {
        std::vector<std::vector<size_t>> edgeLoops;
        std::vector<Face> triangles;
        doReTriangulation(&m_secondMesh, newEdgesPerTriangleInSecondMesh, triangles, edgeLoops);
        addUnIntersectedFaces(&m_secondMesh, reTriangulatedFacesInSecondMesh, triangles);
        EdgeLoop::merge(edgeLoops, &secondMergedEdgeLoops);
        EdgeLoop::unifyDirection(firstMergedEdgeLoops, &secondMergedEdgeLoops);
        SubSurface::createSubSurfaces(secondMergedEdgeLoops, triangles, secondSubSurfaces);
    }
    qDebug() << "Create subsurfaces took" << (elapsedTimer.elapsed() - createSubSurfacesStartTime) << "milliseconds";
    
    auto createSubBlocksStartTime = elapsedTimer.elapsed();
    SubBlock::createSubBlocks(firstSubSurfaces, secondSubSurfaces, m_subBlocks);
    qDebug() << "Create subblocks took" << (elapsedTimer.elapsed() - createSubBlocksStartTime) << "milliseconds";
    
    auto distinguishStartTime = elapsedTimer.elapsed();
    if (!Distinguish::distinguish(m_subBlocks, m_newVertices, &m_indicesToSubBlocks))
        return false;
    qDebug() << "Distinguish took" << (elapsedTimer.elapsed() - distinguishStartTime) << "milliseconds";
    
    return true;
}

void MeshCombiner::getResult(Type booleanType, Mesh *result)
{
    size_t index = (size_t)booleanType;
    if (index >= m_indicesToSubBlocks.size())
        return;
    
    int subBlockIndex = m_indicesToSubBlocks[index];
    if (-1 == subBlockIndex)
        return;
    
    const auto &subBlock = m_subBlocks[subBlockIndex];
    result->vertices = m_newVertices;
    result->faces.reserve(subBlock.faces.size());
    for (const auto &it: subBlock.faces) {
        if (-1 == it.second)
            result->faces.push_back(Face {{it.first[2], it.first[1], it.first[0]}});
        else
            result->faces.push_back(Face {{it.first[0], it.first[1], it.first[2]}});
    }
}

}
