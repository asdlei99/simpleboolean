#include <QApplication>
#include <simpleboolean/meshcombiner.h>
#include <string.h>
#include <stdio.h>
#include <QElapsedTimer>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const char *firstObj = "/Users/jeremy/Repositories/simpleboolean/models/box.obj";
    const char *secondObj = "/Users/jeremy/Repositories/simpleboolean/models/subdived-box.obj";

    simpleboolean::Mesh mesh1;
    simpleboolean::Mesh mesh2;
    simpleboolean::loadTriangulatedObj(mesh1, firstObj);
    simpleboolean::loadTriangulatedObj(mesh2, secondObj);

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    simpleboolean::MeshCombiner combiner;
    combiner.setMeshes(mesh1, mesh2);
    if (!combiner.combine())
        return 1;

    qDebug() << "Simpleboolean took" << elapsedTimer.elapsed() << "milliseconds";

    return 0;
}
