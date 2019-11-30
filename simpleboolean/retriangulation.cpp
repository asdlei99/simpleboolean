#include <simpleboolean/retriangulation.h>
#include <simpleboolean/util.h>
#include <simpleboolean/triangulate.h>
#include <thirdparty/poly2tri/poly2tri/poly2tri.h>
#include <set>
#include <vector>
#include <map>
#include <cmath>
#include <QDebug>

namespace simpleboolean
{

ReTriangulation::ReTriangulation(const std::vector<Vertex> &vertices,
        const std::vector<size_t> &triangle,
        const std::vector<std::vector<size_t>> &edgeLoops) :
    m_vertices(vertices),
    m_triangle(triangle),
    m_edgeLoops(edgeLoops)
{
}

const std::vector<Face> &ReTriangulation::getResult()
{
    return m_reTriangulatedTriangles;
}

void ReTriangulation::recalculateEdgeLoops()
{
    std::vector<std::pair<size_t, size_t>> newEdges;
    
    // Test all heads and tails of open edge loops with the edges of triangle,
    // order by distances per each triangle edge.
    std::set<size_t> endpoints;
    for (const auto &edgeLoop: m_edgeLoops) {
        if (edgeLoop.front() != edgeLoop.back()) {
            endpoints.insert(edgeLoop.front());
            endpoints.insert(edgeLoop.back());
        } else {
            std::vector<size_t> newEdgeLoop; // Remove the head
            for (size_t i = 1; i < edgeLoop.size(); ++i)
                newEdgeLoop.push_back(edgeLoop[i]);
            m_closedEdgeLoops.push_back(newEdgeLoop);
        }
    }
    
    std::vector<float> triangleEdgeLengths;
    for (size_t i = 0; i < 3; ++i) {
        size_t j = (i + 1) % 3;
        triangleEdgeLengths.push_back(distanceOfVertices(m_vertices[m_triangle[i]], m_vertices[m_triangle[j]]));
    }
    std::map<std::pair<size_t, size_t>, float> distancesBetweenEndpointAndCorner;
    for (const auto index: endpoints) {
        for (size_t i = 0; i < 3; ++i) {
            distancesBetweenEndpointAndCorner.insert({{index, i},
                distanceOfVertices(m_vertices[index], m_vertices[m_triangle[i]])
            });
        }
    }
    std::map<size_t, std::vector<std::pair<size_t, float>>> endpointsAttachedToEdges;
    for (const auto index: endpoints) {
        std::vector<std::tuple<size_t, float, float>> offsets;
        bool collapsed = false;
        for (size_t i = 0; i < 3; ++i) {
            if (m_triangle[i] == index) {
                collapsed = true;
                break;
            }
            size_t j = (i + 1) % 3;
            float firstHalf = distancesBetweenEndpointAndCorner[{index, i}];
            float secondHalf = distancesBetweenEndpointAndCorner[{index, j}];
            float lengthOffset = abs(firstHalf + secondHalf - triangleEdgeLengths[i]);
            offsets.push_back({i, lengthOffset, firstHalf});
        }
        if (collapsed)
            break;
        std::sort(offsets.begin(), offsets.end(), [](const std::tuple<size_t, float, float> &first,
                const std::tuple<size_t, float, float> &second) {
            return std::get<1>(first) < std::get<1>(second);
        });
        endpointsAttachedToEdges[std::get<0>(offsets.front())].push_back({index,
            std::get<2>(offsets.front())});
    }
    for (auto &it: endpointsAttachedToEdges) {
        size_t startCorner = m_triangle[it.first];
        size_t stopCorner = m_triangle[(it.first + 1) % 3];
        std::sort(it.second.begin(), it.second.end(), [](const std::pair<size_t, float> &first,
                const std::pair<size_t, float> &second) {
            return first.second < second.second;
        });
        std::vector<size_t> points;
        points.push_back(startCorner);
        for (const auto &it: it.second) {
            points.push_back(it.first);
        }
        points.push_back(stopCorner);
        
        for (size_t i = 1; i < points.size(); ++i) {
            newEdges.push_back({points[i - 1], points[i]});
        }
    }

    // Create edge loops from new edges
    std::map<size_t, std::vector<size_t>> halfEdgeLinkMap;
    for (const auto &edge: newEdges) {
        halfEdgeLinkMap[edge.first].push_back(edge.second);
    }
    for (size_t i = 0; i < 3; ++i) {
        if (endpointsAttachedToEdges.find(i) != endpointsAttachedToEdges.end())
            continue;
        halfEdgeLinkMap[m_triangle[i]].push_back(m_triangle[(i + 1) % 3]);
    }
    std::set<std::pair<size_t, size_t>> visited;
    while (!halfEdgeLinkMap.empty()) {
        std::vector<size_t> loop;
        size_t startVert = halfEdgeLinkMap.begin()->first;
        size_t loopVert = startVert;
        bool newEdgeLoopGenerated = false;
        while (true) {
            loop.push_back(loopVert);
            auto findNext = halfEdgeLinkMap.find(loopVert);
            if (findNext == halfEdgeLinkMap.end())
                break;
            bool foundNextVert = false;
            for (auto it = findNext->second.begin(); it != findNext->second.end(); ++it) {
                if (visited.find({loopVert, *it}) == visited.end()) {
                    visited.insert({loopVert, *it});
                    foundNextVert = true;
                    loopVert = *it;
                    findNext->second.erase(it);
                    break;
                }
            }
            if (!foundNextVert)
                break;
            if (loopVert == startVert) {
                for (const auto &vert: loop) {
                    auto findVert = halfEdgeLinkMap.find(vert);
                    if (findVert == halfEdgeLinkMap.end())
                        continue;
                    if (findVert->second.empty())
                        halfEdgeLinkMap.erase(findVert);
                }
                m_recalculatedEdgeLoops.push_back(loop);
                newEdgeLoopGenerated = true;
                break;
            }
        }
        if (!newEdgeLoopGenerated)
            break;
    }
}

void ReTriangulation::convertVerticesTo2D()
{
    Vector planeNormal;
    triangleNormal(m_vertices[m_triangle[0]],
        m_vertices[m_triangle[1]],
        m_vertices[m_triangle[2]],
        planeNormal);
    const Vertex &planeOrigin = m_vertices[m_triangle[0]];
    Vector planeX;
    directionBetweenTwoVertices(m_vertices[m_triangle[0]],
        m_vertices[m_triangle[1]], planeX);
    std::vector<Vertex> points;
    std::vector<Vertex> points2D;
    std::vector<size_t> indices;
    std::set<size_t> histories;
    auto addPoints = [&](const std::vector<std::vector<size_t>> &edgeLoops) {
        for (const auto &it: edgeLoops) {
            for (const auto &index: it) {
                if (histories.find(index) != histories.end())
                    continue;
                histories.insert(index);
                indices.push_back(index);
                points.push_back(m_vertices[index]);
            }
        }
    };
    addPoints(m_closedEdgeLoops);
    addPoints(m_recalculatedEdgeLoops);
    projectToPlane(planeNormal, planeOrigin,
        planeX, points, points2D);
    for (size_t i = 0; i < points2D.size(); ++i) {
        m_vertices2D.insert(std::make_pair(indices[i], points2D[i]));
    }
}

void ReTriangulation::convertEdgeLoopsToVertices2D()
{
    auto convertEdgeLoops = [&](const std::vector<std::vector<size_t>> &edgeLoops,
            std::vector<std::vector<Vertex>> &result) {
        for (size_t i = 0; i < edgeLoops.size(); ++i) {
            std::vector<Vertex> points2D;
            for (const auto &it: edgeLoops[i]) {
                points2D.push_back(m_vertices2D[it]);
            }
            result.push_back(points2D);
        }
    };
    convertEdgeLoops(m_closedEdgeLoops, m_closedEdgeLoopsVertices2D);
    convertEdgeLoops(m_recalculatedEdgeLoops, m_recalculatedEdgeLoopsVertices2D);
}

bool ReTriangulation::attachClosedEdgeLoopsToOutter()
{
    if (m_closedEdgeLoops.empty() || m_recalculatedEdgeLoops.empty())
        return true;
    for (size_t inner = 0; inner < m_closedEdgeLoops.size(); ++inner) {
        Vertex center2D;
        averageOfPoints2D(m_closedEdgeLoopsVertices2D[inner], center2D);
        bool attached = false;
        for (size_t outter = 0; outter < m_recalculatedEdgeLoops.size(); ++outter) {
            if (pointInPolygon2D(center2D, m_recalculatedEdgeLoopsVertices2D[outter])) {
                m_innerEdgeLoopsMap[outter].push_back(inner);
                attached = true;
                break;
            }
        }
        if (!attached)
            return false;
    }
    return true;
}

// https://github.com/greenm01/poly2tri/blob/master/testbed/main.cc
template <class C> void FreeClear( C & cntr )
{
    for ( typename C::iterator it = cntr.begin();
              it != cntr.end(); ++it ) {
        delete * it;
    }
    cntr.clear();
}

void ReTriangulation::reTriangulate()
{
    recalculateEdgeLoops();
    convertVerticesTo2D();
    convertEdgeLoopsToVertices2D();
    if (!attachClosedEdgeLoopsToOutter()) {
        ++m_errors;
    } else {
        for (size_t outter = 0; outter < m_recalculatedEdgeLoops.size(); ++outter) {
            p2t::CDT *cdt = nullptr;
            std::map<p2t::Point*, size_t> pointToIndexMap;
            std::vector<std::vector<p2t::Point*>> polylines;
            const auto &outterPoints2D = m_recalculatedEdgeLoopsVertices2D[outter];
            {
                std::vector<p2t::Point*> polyline;
                for (size_t index = 0; index < outterPoints2D.size(); ++index) {
                    const auto &it = outterPoints2D[index];
                    p2t::Point *point = new p2t::Point(it.xyz[0], it.xyz[1]);
                    const auto &vertexIndex = m_recalculatedEdgeLoops[outter][index];
                    pointToIndexMap.insert(std::make_pair(point, vertexIndex));
                    polyline.push_back(point);
                }
                cdt = new p2t::CDT(polyline);
                polylines.push_back(polyline);
            }
            const auto &findInners = m_innerEdgeLoopsMap.find(outter);
            if (findInners != m_innerEdgeLoopsMap.end()) {
                for (const auto &inner: findInners->second) {
                    const auto &innerPoints2D = m_closedEdgeLoopsVertices2D[inner];
                    {
                        std::vector<p2t::Point*> polyline;
                        for (size_t index = 0; index < innerPoints2D.size(); ++index) {
                            const auto &it = innerPoints2D[index];
                            p2t::Point *point = new p2t::Point(it.xyz[0], it.xyz[1]);
                            const auto &vertexIndex = m_closedEdgeLoops[inner][index];
                            pointToIndexMap.insert(std::make_pair(point, vertexIndex));
                            polyline.push_back(point);
                        }
                        cdt->AddHole(polyline);
                        polylines.push_back(polyline);
                    }
                }
            }
            cdt->Triangulate();
            const auto &triangles = cdt->GetTriangles();
            std::vector<Face> newFaces;
            bool foundBadTriangle = false;
            for (const auto &it: triangles) {
                Face face;
                int faceVertexIndex = 0;
                for (int i = 0; i < 3; ++i) {
                    p2t::Point *point = it->GetPoint(i);
                    auto findIndex = pointToIndexMap.find(point);
                    if (findIndex == pointToIndexMap.end()) {
                        continue;
                    }
                    face.indices[faceVertexIndex++] = findIndex->second;
                }
                if (faceVertexIndex != 3) {
                    qDebug() << "Ignore triangle";
                    foundBadTriangle = true;
                    ++m_errors;
                    continue;
                }
                newFaces.push_back(face);
            }
            
            if (foundBadTriangle) {
                if (findInners != m_innerEdgeLoopsMap.end()) {
                    qDebug() << "Triangulate failed";
                } else {
                    std::vector<Face> oldWayTriangulatedFaces;
                    const auto &ring = m_recalculatedEdgeLoops[outter];
                    triangulate(m_vertices, oldWayTriangulatedFaces, ring);
                    if (oldWayTriangulatedFaces.empty()) {
                        qDebug() << "Fallback triangulate failed";
                    } else {
                        for (const auto &it: oldWayTriangulatedFaces)
                            m_reTriangulatedTriangles.push_back(it);
                    }
                }
            } else {
                for (const auto &it: newFaces)
                    m_reTriangulatedTriangles.push_back(it);
            }
            
            // TODO: triangulate holes
            // ... ...
            
            delete cdt;
            for (size_t i = 0; i < polylines.size(); i++) {
                std::vector<p2t::Point*> poly = polylines[i];
                FreeClear(poly);
            }
        }
    }
}

}

