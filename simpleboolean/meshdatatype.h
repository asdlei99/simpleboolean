#ifndef SIMPLEBOOLEAN_MESH_DATA_TYPE_H
#define SIMPLEBOOLEAN_MESH_DATA_TYPE_H
#include <vector>
#include <cstdlib>
#include <QString>

namespace simpleboolean
{

struct Vector
{
    float xyz[3];
};

typedef Vector Vertex;

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
void exportTriangulatedObj(const Mesh &mesh, const QString &filename);
  
}

#endif

