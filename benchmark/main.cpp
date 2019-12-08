#include <QApplication>
#include <simpleboolean/meshcombiner.h>
#include <string.h>
#include <stdio.h>
#include <QElapsedTimer>
#include <QDebug>
#include "cgalmesh.h"
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/Polygon_mesh_processing/repair.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const char *firstObj = "/Users/jeremy/Repositories/simpleboolean/models/addax.obj";
    const char *secondObj = "/Users/jeremy/Repositories/simpleboolean/models/meerkat-with-extra.obj";

    simpleboolean::Mesh mesh1;
    simpleboolean::Mesh mesh2;
    simpleboolean::loadTriangulatedObj(mesh1, firstObj);
    simpleboolean::loadTriangulatedObj(mesh2, secondObj);

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    
    auto simpleBooleanStartTime = elapsedTimer.elapsed();
    simpleboolean::MeshCombiner combiner;
    combiner.setMeshes(mesh1, mesh2);
    bool combineSucceed = combiner.combine();
    
    if (combineSucceed)
        qDebug() << "Simpleboolean took" << (elapsedTimer.elapsed() - simpleBooleanStartTime) << "milliseconds";
    
    auto collectMeshesToList = [&](const std::vector<simpleboolean::Mesh> &meshes) {
        simpleboolean::Mesh subBlocksMesh;
        for (size_t i = 0; i < meshes.size(); ++i) {
            const auto &subBlock = meshes[i];
            size_t startVertexIndex = subBlocksMesh.vertices.size();
            for (const auto &vertex: subBlock.vertices) {
                auto newVertex = vertex;
                newVertex.xyz[0] += i * 1.0 - (meshes.size() / 2);
                subBlocksMesh.vertices.push_back(newVertex);
            }
            for (const auto &face: subBlock.faces) {
                auto newFace = face;
                newFace.indices[0] += startVertexIndex;
                newFace.indices[1] += startVertexIndex;
                newFace.indices[2] += startVertexIndex;
                subBlocksMesh.faces.push_back(newFace);
            }
        }
        return subBlocksMesh;
    };
    auto subBlocksMesh = collectMeshesToList(combiner.m_debugSubBlocks);
    exportTriangulatedObj(subBlocksMesh, QString("/Users/jeremy/Desktop/debug-subblock-list.obj"));
    
    if (combineSucceed) {
        simpleboolean::Mesh unionMesh;
        simpleboolean::Mesh intersectionMesh;
        simpleboolean::Mesh subtractionMesh;
        simpleboolean::Mesh inversedSubtractionMesh;
        combiner.getResult(simpleboolean::Type::Union, &unionMesh);
        combiner.getResult(simpleboolean::Type::Intersection, &intersectionMesh);
        combiner.getResult(simpleboolean::Type::Subtraction, &subtractionMesh);
        combiner.getResult(simpleboolean::Type::InversedSubtraction, &inversedSubtractionMesh);
        std::vector<simpleboolean::Mesh> meshes = {
            unionMesh, intersectionMesh, subtractionMesh, inversedSubtractionMesh
        };
        auto booleanListMesh = collectMeshesToList(meshes);
        exportTriangulatedObj(booleanListMesh, QString("/Users/jeremy/Desktop/debug-boolean-list.obj"));
    }
    
    if (!combineSucceed)
        return 1;
    
    auto simplebooleanMeshToCgalMesh = [&](const simpleboolean::Mesh &mesh) {
        std::vector<QVector3D> vertices(mesh.vertices.size());
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            const auto &source = mesh.vertices[i];
            vertices[i] = QVector3D(source.xyz[0], source.xyz[1], source.xyz[2]);
        }
        std::vector<std::vector<size_t>> faces(mesh.faces.size());
        for (size_t i = 0; i < mesh.faces.size(); ++i) {
            const auto &source = mesh.faces[i];
            faces[i] = {source.indices[0], source.indices[1], source.indices[2]};
        }
        auto cgalMesh = (CgalMesh *)buildCgalMesh<CgalKernel>(vertices, faces);
        return cgalMesh;
    };
    
    auto cgalMesh1 = simplebooleanMeshToCgalMesh(mesh1);
    auto cgalMesh2 = simplebooleanMeshToCgalMesh(mesh2);
    CgalMesh *resultCgalMesh = nullptr;
    resultCgalMesh = new CgalMesh;
    auto cgalStartTime = elapsedTimer.elapsed();
    CGAL::Polygon_mesh_processing::corefine_and_compute_union(*cgalMesh1, *cgalMesh2, *resultCgalMesh);
    qDebug() << "CGAL took" << (elapsedTimer.elapsed() - cgalStartTime) << "milliseconds";
    /*
    {
        std::vector<QVector3D> vertices;
        std::vector<std::vector<size_t>> faces;
        fetchFromCgalMesh<CgalKernel>(resultCgalMesh, vertices, faces);
        nodemesh::exportMeshAsObj(vertices, faces, "/Users/jeremy/Desktop/cgal.obj");
    }
    */
    
    return 0;
}
