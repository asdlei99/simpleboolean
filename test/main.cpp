#include <QApplication>
#include <simpleboolean/meshcombiner.h>
#include <string.h>
#include <stdio.h>

static void help()
{
    printf("test <a.obj> <b.obj> <output.obj>\r\n");
}

int main(int argc, char *argv[])
{
    if (argc - 1 < 2) {
        help();
        return 1;
    }
    
    for (int i = 1; i < argc; ++i) {
        if (0 == strstr(argv[i], ".obj")) {
            help();
            return 1;
        }
    }
    
    char *firstObj = argv[1];
    char *secondObj = argv[2];
    char *outputObj = nullptr;
    
    if (argc >= 4)
        outputObj = argv[3];
    
    QApplication a(argc, argv);
    
    simpleboolean::Mesh mesh1;
    simpleboolean::Mesh mesh2;
    simpleboolean::loadTriangulatedObj(mesh1, firstObj);
    simpleboolean::loadTriangulatedObj(mesh2, secondObj);
    
    simpleboolean::MeshCombiner combiner;
    combiner.setMeshes(mesh1, mesh2);
    if (!combiner.combine())
        return 1;
    
    if (nullptr != outputObj) {
        simpleboolean::Mesh unionMesh;
        combiner.getResult(simpleboolean::Type::Union, &unionMesh);
        simpleboolean::exportTriangulatedObj(unionMesh, QString(outputObj));
    }
    
    return 0;
}
