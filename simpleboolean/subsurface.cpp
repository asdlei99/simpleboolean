#include <simpleboolean/subsurface.h>
#include <QStringList>
#include <set>
#include <queue>
#include <array>
#include <QDebug>
#include <QElapsedTimer>

namespace simpleboolean
{

QString SubSurface::createEdgeLoopName(const std::vector<size_t> &edgeLoop)
{
    QStringList stringList;
    stringList.reserve(edgeLoop.size());
    auto sortedEdgeLoop = edgeLoop;
    std::sort(sortedEdgeLoop.begin(), sortedEdgeLoop.end());
    for (const auto &it: sortedEdgeLoop)
        stringList.append(QString::number(it));
    return stringList.join(",");
}

void SubSurface::createSubSurfaces(std::vector<std::vector<size_t>> &edgeLoops,
        const std::vector<Face> &triangles,
        std::vector<SubSurface> &subSurfaces,
        bool reviseEdgeLoopsDirection)
{
    if (edgeLoops.empty())
        return;
    
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    
    auto createHalfEdgesStartTime = elapsedTimer.elapsed();
    std::map<std::pair<size_t, size_t>, size_t> halfEdges;
    for (size_t m = 0; m < triangles.size(); ++m) {
        const auto &triangle = triangles[m];
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;
            auto edge = std::make_pair(triangle.indices[i], triangle.indices[j]);
            halfEdges.insert({edge, m});
        }
    }
    //qDebug() << "Create halfedges took" << (elapsedTimer.elapsed() - createHalfEdgesStartTime) << "milliseconds";
    
    auto createEdgeLoopMapStartTime = elapsedTimer.elapsed();
    std::vector<bool> edgeLoopFlippedMap(edgeLoops.size(), false);
    std::map<std::pair<size_t, size_t>, std::pair<size_t, bool>> edgeToLoopMap;
    for (size_t m = 0; m < edgeLoops.size(); ++m) {
        auto &edgeLoop = edgeLoops[m];
        for (size_t i = 0; i < edgeLoop.size(); ++i) {
            size_t j = (i + 1) % edgeLoop.size();
            edgeToLoopMap.insert({std::make_pair(edgeLoop[i], edgeLoop[j]), std::make_pair(m, true)});
            edgeToLoopMap.insert({std::make_pair(edgeLoop[j], edgeLoop[i]), std::make_pair(m, false)});
        }
    }
    //qDebug() << "Create edgeloopmap took" << (elapsedTimer.elapsed() - createEdgeLoopMapStartTime) << "milliseconds";
    
    auto createEdgeLoopNamesStartTime = elapsedTimer.elapsed();
    std::vector<QString> edgeLoopNames(edgeLoops.size());
    for (size_t edgeLoopIndex = 0; edgeLoopIndex < edgeLoops.size(); ++edgeLoopIndex) {
        const auto &edgeLoop = edgeLoops[edgeLoopIndex];
        edgeLoopNames[edgeLoopIndex] = createEdgeLoopName(edgeLoop);
    }
    //qDebug() << "Create edgeloopnames took" << (elapsedTimer.elapsed() - createEdgeLoopNamesStartTime) << "milliseconds";
    
    std::vector<bool> visitedTriangles(triangles.size(), false);
    std::set<size_t> visitedEdgeLoops;
    std::queue<size_t> edgeLoopQueue;
    
    auto buildSubSurfacesFromEdgeLoop = [&](size_t edgeLoopIndex) {
        const auto &edgeLoop = edgeLoops[edgeLoopIndex];

        const auto &edgeLoopName = edgeLoopNames[edgeLoopIndex];
        
        SubSurface frontSurface;
        frontSurface.edgeLoopName = edgeLoopName;
        frontSurface.isFrontSide = true;
        frontSurface.ownerNames.insert(std::make_pair(frontSurface.edgeLoopName, frontSurface.isFrontSide));
        
        SubSurface backSurface;
        backSurface.edgeLoopName = edgeLoopName;
        backSurface.isFrontSide = false;
        backSurface.ownerNames.insert(std::make_pair(backSurface.edgeLoopName, backSurface.isFrontSide));
        
        std::queue<size_t> frontTriangleIndices;
        std::queue<size_t> backTriangleIndices;
        
        auto addFrontTriangle = [&](size_t triangleIndex) {
            if (visitedTriangles[triangleIndex])
                return;
            const auto &face = triangles[triangleIndex];
            visitedTriangles[triangleIndex] = true;
            frontSurface.faces.push_back(face);
            for (size_t m = 0; m < 3; ++m) {
                size_t n = (m + 1) % 3;
                auto edge = std::make_pair(face.indices[n], face.indices[m]);
                auto findEdgeLoop = edgeToLoopMap.find(edge);
                if (findEdgeLoop != edgeToLoopMap.end()) {
                    if (findEdgeLoop->second.first != edgeLoopIndex) {
                        auto &edgeLoopName = edgeLoopNames[findEdgeLoop->second.first];
                        if (reviseEdgeLoopsDirection) {
                            if (findEdgeLoop->second.second != false) {
                                edgeLoopFlippedMap[findEdgeLoop->second.first] = true;
                                edgeLoopQueue.push(findEdgeLoop->second.first);
                            }
                        }
                        frontSurface.ownerNames.insert(std::make_pair(edgeLoopName, false));
                        frontSurface.isSharedByOthers = true;
                    }
                    continue;
                }
                auto findNeighbor = halfEdges.find(edge);
                if (findNeighbor != halfEdges.end() && !visitedTriangles[findNeighbor->second])
                    frontTriangleIndices.push(findNeighbor->second);
            }
        };
        
        auto addBackTriangle = [&](size_t triangleIndex) {
            if (visitedTriangles[triangleIndex])
                return;
            const auto &face = triangles[triangleIndex];
            visitedTriangles[triangleIndex] = true;
            backSurface.faces.push_back(face);
            for (size_t m = 0; m < 3; ++m) {
                size_t n = (m + 1) % 3;
                auto edge = std::make_pair(face.indices[n], face.indices[m]);
                auto findEdgeLoop = edgeToLoopMap.find(edge);
                if (findEdgeLoop != edgeToLoopMap.end()) {
                    if (findEdgeLoop->second.first != edgeLoopIndex) {
                        auto &edgeLoopName = edgeLoopNames[findEdgeLoop->second.first];
                        if (reviseEdgeLoopsDirection) {
                            if (findEdgeLoop->second.second != true) {
                                edgeLoopFlippedMap[findEdgeLoop->second.first] = true;
                                edgeLoopQueue.push(findEdgeLoop->second.first);
                            }
                        }
                        backSurface.ownerNames.insert(std::make_pair(edgeLoopName, true));
                        backSurface.isSharedByOthers = true;
                    }
                    continue;
                }
                auto findNeighbor = halfEdges.find(edge);
                if (findNeighbor != halfEdges.end() && !visitedTriangles[findNeighbor->second])
                    backTriangleIndices.push(findNeighbor->second);
            }
        };
        
        for (size_t i = 0; i < edgeLoop.size(); ++i) {
            size_t j = (i + 1) % edgeLoop.size();
            auto edge = std::make_pair(edgeLoop[i], edgeLoop[j]);
            auto findTriangle = halfEdges.find(edge);
            if (findTriangle != halfEdges.end())
                frontTriangleIndices.push(findTriangle->second);
            auto oppositeEdge = std::make_pair(edge.second, edge.first);
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
    };
    
    for (size_t waitEdgeLoopIndex = 0; waitEdgeLoopIndex < edgeLoops.size(); ++waitEdgeLoopIndex) {
        edgeLoopQueue.push(waitEdgeLoopIndex);
        while (!edgeLoopQueue.empty()) {
            auto edgeLoopIndex = edgeLoopQueue.front();
            edgeLoopQueue.pop();
            if (visitedEdgeLoops.find(edgeLoopIndex) != visitedEdgeLoops.end())
                continue;
            visitedEdgeLoops.insert(edgeLoopIndex);
            if (edgeLoopFlippedMap[edgeLoopIndex]) {
                auto &edgeLoop = edgeLoops[edgeLoopIndex];
                std::reverse(edgeLoop.begin(), edgeLoop.end());
            }
            buildSubSurfacesFromEdgeLoop(edgeLoopIndex);
        }
    }
}

}
