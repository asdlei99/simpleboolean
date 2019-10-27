#ifndef SIMPLEBOOLEAN_UTIL_H
#define SIMPLEBOOLEAN_UTIL_H
#include <simpleboolean/meshdatatype.h>

namespace simpleboolean
{

float distanceSquaredOfVertices(const Vertex &first, const Vertex &second);
float distanceOfVertices(const Vertex &first, const Vertex &second);
float isNull(float number);
void projectToPlane(const Vector &planeNormal, const Vertex &planeOrigin,
    const Vector &planeX, const std::vector<Vertex> &points,
    std::vector<Vertex> &result);
bool pointInPolygon2D(const Vertex &vertex, const std::vector<Vertex> &ring);
void averageOfPoints2D(const std::vector<Vertex> &points, Vertex &result);

}

#endif
