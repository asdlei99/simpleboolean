#include <simpleboolean/subsurface.h>
#include <QStringList>
#include <set>
#include <queue>

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

    for (const auto &edgeLoop: edgeLoops) {
        if (edgeLoop.size() < 2)
            continue;
        
        std::set<size_t> visitedFaces;
        
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
                auto findNeighbor = halfEdges.find(std::make_pair(face.indices[m], face.indices[n]));
                if (findNeighbor != halfEdges.end())
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
                auto findNeighbor = halfEdges.find(std::make_pair(face.indices[m], face.indices[n]));
                if (findNeighbor != halfEdges.end())
                    backTriangleIndices.push(findNeighbor->second);
            }
        };
        
        for (size_t i = 0; i < edgeLoop.size(); ++i) {
            size_t j = (i + 1) % edgeLoop.size();
            auto edge = std::make_pair(edgeLoop[i], edgeLoop[j]);
            auto findTriangle = halfEdges.find(edge);
            if (findTriangle != halfEdges.end())
                addFrontTriangle(findTriangle->second);
            auto oppositeEdge = std::make_pair(edge.second, edge.first);
            auto findOppositeTriangle = halfEdges.find(oppositeEdge);
            if (findOppositeTriangle != halfEdges.end())
                addBackTriangle(findOppositeTriangle->second);
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
}

}
