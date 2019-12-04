#include <simpleboolean/subsurface.h>
#include <QStringList>
#include <set>
#include <queue>
#include <array>
#include <QDebug>

namespace simpleboolean
{

QString SubSurface::createEdgeLoopName(const std::vector<size_t> &edgeLoop)
{
    QStringList stringList;
    stringList.reserve(edgeLoop.size());
    /*
    std::set<std::pair<size_t, size_t>> originalHalfEdges;
    for (size_t i = 0; i < edgeLoop.size(); ++i) {
        size_t j = (i + 1) % edgeLoop.size();
        originalHalfEdges.insert({edgeLoop[i], edgeLoop[j]});
    }
    */
    auto sortedEdgeLoop = edgeLoop;
    std::sort(sortedEdgeLoop.begin(), sortedEdgeLoop.end());
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
    
    std::map<std::pair<size_t, size_t>, std::pair<size_t, bool>> edgeToLoopMap;
    for (size_t m = 0; m < edgeLoops.size(); ++m) {
        const auto &edgeLoop = edgeLoops[m];
        for (size_t i = 0; i < edgeLoop.size(); ++i) {
            size_t j = (i + 1) % edgeLoop.size();
            edgeToLoopMap.insert({std::make_pair(edgeLoop[i], edgeLoop[j]), std::make_pair(m, true)});
            edgeToLoopMap.insert({std::make_pair(edgeLoop[j], edgeLoop[i]), std::make_pair(m, false)});
        }
    }
    
    std::vector<QString> edgeLoopNames(edgeLoops.size());
    for (size_t edgeLoopIndex = 0; edgeLoopIndex < edgeLoops.size(); ++edgeLoopIndex) {
        const auto &edgeLoop = edgeLoops[edgeLoopIndex];
        edgeLoopNames[edgeLoopIndex] = createEdgeLoopName(edgeLoop);
    }

    for (size_t edgeLoopIndex = 0; edgeLoopIndex < edgeLoops.size(); ++edgeLoopIndex) {
        const auto &edgeLoop = edgeLoops[edgeLoopIndex];
        if (edgeLoop.size() < 2)
            continue;
        
        std::set<size_t> visitedFaces;
        
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
            if (visitedFaces.find(triangleIndex) != visitedFaces.end())
                return;
            const auto &face = triangles[triangleIndex];
            visitedFaces.insert(triangleIndex);
            frontSurface.faces.push_back(face);
            for (size_t m = 0; m < 3; ++m) {
                size_t n = (m + 1) % 3;
                auto edge = std::make_pair(face.indices[n], face.indices[m]);
                auto findEdgeLoop = edgeToLoopMap.find(edge);
                if (findEdgeLoop != edgeToLoopMap.end()) {
                    if (findEdgeLoop->second.first != edgeLoopIndex) {
                        const auto &edgeLoopName = edgeLoopNames[findEdgeLoop->second.first];
                        frontSurface.ownerNames.insert(std::make_pair(edgeLoopName, !findEdgeLoop->second.second));
                        frontSurface.isSharedByOthers = true;
                    }
                    continue;
                }
                auto findNeighbor = halfEdges.find(edge);
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
                auto edge = std::make_pair(face.indices[n], face.indices[m]);
                auto findEdgeLoop = edgeToLoopMap.find(edge);
                if (findEdgeLoop != edgeToLoopMap.end()) {
                    if (findEdgeLoop->second.first != edgeLoopIndex) {
                        const auto &edgeLoopName = edgeLoopNames[findEdgeLoop->second.first];
                        backSurface.ownerNames.insert(std::make_pair(edgeLoopName, !findEdgeLoop->second.second));
                        backSurface.isSharedByOthers = true;
                    }
                    continue;
                }
                auto findNeighbor = halfEdges.find(edge);
                if (findNeighbor != halfEdges.end())
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
    }
    
    qDebug() << "edgeLoops:" << edgeLoops.size();
    qDebug() << "subSurfaces:" << subSurfaces.size();
    //for (size_t i = 0; i < subSurfaces.size(); ++i) {
    //    const auto &subSurface = subSurfaces[i];
    //    qDebug() << "============== subSurface[" << i << "]" << (subSurface.isSharedByOthers ? "Public" : "Private") << "==================";
    //    for (const auto &it: subSurface.ownerNames)
    //        qDebug() << "owner:" << it;
    //}
}

}
