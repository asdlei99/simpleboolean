#include <simpleboolean/retriangulation.h>
#include <simpleboolean/util.h>
#include <simpleboolean/triangulate.h>
#include <simpleboolean/edgeloop.h>
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

void ReTriangulation::buildEdgeLoopsFromDirectedEdges(const std::vector<std::pair<size_t, size_t>> &edges,
        std::vector<std::vector<size_t>> *edgeLoops)
{
    EdgeLoop::buildEdgeLoopsFromDirectedEdges(edges, edgeLoops, false, true);
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
            
            for (size_t i = 1; i < edgeLoop.size(); ++i) {
                newEdges.push_back({edgeLoop[i - 1], edgeLoop[i]});
                newEdges.push_back({edgeLoop[i], edgeLoop[i - 1]});
            }
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
            continue;
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
        
        //qDebug() << "Attach" << points << "to" << startCorner << "-" << stopCorner;
        
        for (size_t i = 1; i < points.size(); ++i) {
            newEdges.push_back({points[i - 1], points[i]});
        }
    }
    for (size_t i = 0; i < 3; ++i) {
        if (endpointsAttachedToEdges.find(i) != endpointsAttachedToEdges.end()) {
            //qDebug() << "Full triangle edge:" << m_triangle[i] << m_triangle[(i + 1) % 3];
            continue;
        }
        //qDebug() << "Empty triangle edge:" << m_triangle[i] << m_triangle[(i + 1) % 3];
        newEdges.push_back({m_triangle[i], m_triangle[(i + 1) % 3]});
    }
    
    buildEdgeLoopsFromDirectedEdges(newEdges, &m_recalculatedEdgeLoops);
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
        //qDebug() << "attached:" << attached;
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

void ReTriangulation::unifyFaceDirections(const std::vector<Face> &existedFaces,
        std::vector<Face> *newFaces)
{
    if (existedFaces.empty())
        return;
    std::set<std::pair<size_t, size_t>> halfEdges;
    for (const auto &face: existedFaces) {
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;
            halfEdges.insert(std::make_pair(face.indices[i], face.indices[j]));
        }
    }
    for (const auto &face: *newFaces) {
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;
            if (halfEdges.find(std::make_pair(face.indices[i], face.indices[j])) != halfEdges.end()) {
                for (auto &newFace: *newFaces)
                    std::reverse(newFace.indices, newFace.indices + 3);
                return;
            }
        }
    }
}

void ReTriangulation::reTriangulate()
{
    recalculateEdgeLoops();
    convertVerticesTo2D();
    convertEdgeLoopsToVertices2D();
    if (!attachClosedEdgeLoopsToOutter()) {
        ++m_errors;
    } else {
        auto fetchTriangulatedResult = [&](const std::vector<p2t::Triangle*> &triangles, const std::map<p2t::Point*, size_t> &pointToIndexMap) {
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
            if (foundBadTriangle)
                return std::vector<Face>();
            return newFaces;
        };
    
        for (size_t outter = 0; outter < m_recalculatedEdgeLoops.size(); ++outter) {
            p2t::CDT *cdt = nullptr;
            std::map<p2t::Point*, size_t> pointToIndexMap;
            std::vector<std::vector<p2t::Point*>> polylines;
            std::vector<std::vector<p2t::Point*>> holePolylines;
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
                    //qDebug() << "Inner edge loop:" << m_closedEdgeLoops[inner];
                    const auto &innerPoints2D = m_closedEdgeLoopsVertices2D[inner];
                    {
                        std::vector<p2t::Point*> polyline;
                        std::vector<p2t::Point*> holePolyline;
                        for (size_t index = 0; index < innerPoints2D.size(); ++index) {
                            const auto &it = innerPoints2D[index];
                            {
                                p2t::Point *point = new p2t::Point(it.xyz[0], it.xyz[1]);
                                const auto &vertexIndex = m_closedEdgeLoops[inner][index];
                                pointToIndexMap.insert(std::make_pair(point, vertexIndex));
                                polyline.push_back(point);
                            }
                            {
                                p2t::Point *point = new p2t::Point(it.xyz[0], it.xyz[1]);
                                const auto &vertexIndex = m_closedEdgeLoops[inner][index];
                                pointToIndexMap.insert(std::make_pair(point, vertexIndex));
                                holePolyline.push_back(point);
                            }
                        }

                        cdt->AddHole(polyline);
                        polylines.push_back(polyline);
                        polylines.push_back(holePolyline);
                        holePolylines.push_back(holePolyline);
                    }
                }
            }
            cdt->Triangulate();
            std::vector<Face> outterFaces = fetchTriangulatedResult(cdt->GetTriangles(), pointToIndexMap);
            for (const auto &it: outterFaces)
                m_reTriangulatedTriangles.push_back(it);
            
            delete cdt;
            
            std::vector<Face> holeTriangles;
            for (size_t i = 0; i < holePolylines.size(); i++) {
                std::vector<p2t::Point*> polyline = holePolylines[i];
                p2t::CDT *holeCdt = new p2t::CDT(polyline);
                holeCdt->Triangulate();
                std::vector<Face> holeFaces = fetchTriangulatedResult(holeCdt->GetTriangles(), pointToIndexMap);
                unifyFaceDirections(outterFaces, &holeFaces);
                for (const auto &it: holeFaces)
                    m_reTriangulatedTriangles.push_back(it);
                holeTriangles.insert(holeTriangles.end(), holeFaces.begin(), holeFaces.end());
                delete holeCdt;
            }

            for (size_t i = 0; i < polylines.size(); i++) {
                std::vector<p2t::Point*> poly = polylines[i];
                FreeClear(poly);
            }
        }
    }
}

}

