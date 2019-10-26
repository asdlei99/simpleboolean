#ifndef SIMPLEBOOLEAN_MESH_DATA_TYPE_H
#define SIMPLEBOOLEAN_MESH_DATA_TYPE_H
#include <vector>
#include <cstdlib>
#include <QString>

namespace simpleboolean
{
  
struct Vertex
{
    float xyz[3];
};

struct Vector
{
    float xyz[3];
};

struct Face
{
    size_t indices[3];
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
};

bool loadTriangulatedObj(Mesh &mesh, const QString &filename);
  
}

#endif

