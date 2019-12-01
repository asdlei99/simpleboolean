#include <simpleboolean/subsurface.h>
#include <QStringList>
#include <set>
#include <queue>
#include <array>
#include <QDebug>

namespace simpleboolean
{

QString SubSurface::createEdgeLoopName(const std::vector<size_t> &edgeLoop, bool *nameReversed)
{
    QStringList stringList;
    stringList.reserve(edgeLoop.size());
    std::set<std::pair<size_t, size_t>> originalHalfEdges;
    for (size_t i = 0; i < edgeLoop.size(); ++i) {
        size_t j = (i + 1) % edgeLoop.size();
        originalHalfEdges.insert({edgeLoop[i], edgeLoop[j]});
    }
    auto sortedEdgeLoop = edgeLoop;
    std::sort(sortedEdgeLoop.begin(), sortedEdgeLoop.end());
    *nameReversed = originalHalfEdges.find(
        std::make_pair(sortedEdgeLoop[0], sortedEdgeLoop[1])) == originalHalfEdges.end();
    for (const auto &it: sortedEdgeLoop)
        stringList.append(QString::number(it));
    return stringList.join(",");
}

void SubSurface::createSubSurfaces(const std::vector<std::vector<size_t>> &edgeLoops,
        const std::vector<Face> &triangles,
        std::vector<SubSurface> &subSurfaces)
{
    std::map<std::pair<size_t, size_t>, size_t> halfEdges;
    for (size_t m = 0; m < triangles.size(); ++m) {
        const auto &triangle = triangles[m];
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;
            auto edge = std::make_pair(triangle.indices[i], triangle.indices[j]);
            halfEdges.insert({edge, m});
        }
    }
    
    std::map<std::pair<size_t, size_t>, size_t> edgeToLoopMap;
    for (size_t m = 0; m < edgeLoops.size(); ++m) {
        const auto &edgeLoop = edgeLoops[m];
        for (size_t i = 0; i < edgeLoop.size(); ++i) {
            size_t j = (i + 1) % edgeLoop.size();
            edgeToLoopMap.insert({std::make_pair(edgeLoop[i], edgeLoop[j]), m});
            edgeToLoopMap.insert({std::make_pair(edgeLoop[j], edgeLoop[i]), m});
        }
    }

    for (size_t edgeLoopIndex = 0; edgeLoopIndex < edgeLoops.size(); ++edgeLoopIndex) {
        const auto &edgeLoop = edgeLoops[edgeLoopIndex];
        if (edgeLoop.size() < 2)
            continue;
        
        std::set<size_t> visitedFaces;
        std::set<std::pair<size_t, size_t>> borderEdges;
        
        bool nameReversed = false;
        QString edgeLoopName = createEdgeLoopName(edgeLoop, &nameReversed);
        
        SubSurface frontSurface;
        frontSurface.edgeLoopName = edgeLoopName;
        frontSurface.isFrontSide = !nameReversed;
        
        SubSurface backSurface;
        backSurface.edgeLoopName = edgeLoopName;
        backSurface.isFrontSide = nameReversed;
        
        std::queue<size_t> frontTriangleIndices;
        std::queue<size_t> backTriangleIndices;
        
        auto addFrontTriangle = [&](size_t triangleIndex) {
            if (visitedFaces.find(triangleIndex) != visitedFaces.end())
                return;
            const auto &face = triangles[triangleIndex];
            visitedFaces.insert(triangleIndex);
            frontSurface.faces.push_back(face);
            for (size_t m = 0; m < 3; ++m) {
                size_t n = (m + 1) % 3;
                auto edge = std::make_pair(face.indices[n], face.indices[m]);
                auto findEdgeLoop = edgeToLoopMap.find(edge);
                if (findEdgeLoop != edgeToLoopMap.end() && findEdgeLoop->second != edgeLoopIndex)
                    frontSurface.isSharedByOthers = true;
                auto findNeighbor = halfEdges.find(edge);
                if (findNeighbor != halfEdges.end() && borderEdges.find(edge) == borderEdges.end())
                    frontTriangleIndices.push(findNeighbor->second);
            }
        };
        
        auto addBackTriangle = [&](size_t triangleIndex) {
            if (visitedFaces.find(triangleIndex) != visitedFaces.end())
                return;
            const auto &face = triangles[triangleIndex];
            visitedFaces.insert(triangleIndex);
            backSurface.faces.push_back(face);
            for (size_t m = 0; m < 3; ++m) {
                size_t n = (m + 1) % 3;
                auto edge = std::make_pair(face.indices[n], face.indices[m]);
                auto findEdgeLoop = edgeToLoopMap.find(edge);
                if (findEdgeLoop != edgeToLoopMap.end() && findEdgeLoop->second != edgeLoopIndex)
                    backSurface.isSharedByOthers = true;
                auto findNeighbor = halfEdges.find(edge);
                if (findNeighbor != halfEdges.end() && borderEdges.find(edge) == borderEdges.end())
                    backTriangleIndices.push(findNeighbor->second);
            }
        };
        
        for (size_t i = 0; i < edgeLoop.size(); ++i) {
            size_t j = (i + 1) % edgeLoop.size();
            auto edge = std::make_pair(edgeLoop[i], edgeLoop[j]);
            borderEdges.insert(edge);
            auto findTriangle = halfEdges.find(edge);
            if (findTriangle != halfEdges.end())
                frontTriangleIndices.push(findTriangle->second);
            auto oppositeEdge = std::make_pair(edge.second, edge.first);
            borderEdges.insert(oppositeEdge);
            auto findOppositeTriangle = halfEdges.find(oppositeEdge);
            if (findOppositeTriangle != halfEdges.end())
                backTriangleIndices.push(findOppositeTriangle->second);
        }
        
        while (!frontTriangleIndices.empty()) {
            auto triangleIndex = frontTriangleIndices.front();
            frontTriangleIndices.pop();
            addFrontTriangle(triangleIndex);
        }
        
        while (!backTriangleIndices.empty()) {
            auto triangleIndex = backTriangleIndices.front();
            backTriangleIndices.pop();
            addBackTriangle(triangleIndex);
        }
        
        subSurfaces.push_back(frontSurface);
        subSurfaces.push_back(backSurface);
    }
    
    std::set<std::array<size_t, 3>> privateFaces;
    for (const auto &it: subSurfaces) {
        if (!it.isSharedByOthers) {
            for (const auto &face: it.faces) {
                privateFaces.insert(std::array<size_t, 3> {{face.indices[0], face.indices[1], face.indices[2]}});
            }
        }
    }
    
    for (auto &it: subSurfaces) {
        if (it.isSharedByOthers) {
            std::vector<Face> newFaces;
            newFaces.reserve(it.faces.size());
            for (const auto &face: it.faces) {
                if (privateFaces.find(std::array<size_t, 3> {{face.indices[0], face.indices[1], face.indices[2]}}) != privateFaces.end())
                    continue;
                newFaces.push_back(face);
            }
            it.faces = newFaces;
        }
    }
}

}
