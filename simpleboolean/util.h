#ifndef SIMPLEBOOLEAN_UTIL_H
#define SIMPLEBOOLEAN_UTIL_H
#include <simpleboolean/meshdatatype.h>

namespace simpleboolean
{

float distanceSquaredOfVertices(const Vertex &first, const Vertex &second);
float distanceOfVertices(const Vertex &first, const Vertex &second);
float isNull(float number);
void projectToPlane(Vector planeNormal, Vertex planeOrigin,
    Vector planeX, const std::vector<Vertex> &points,
    std::vector<Vertex> &result);

}

#endif
