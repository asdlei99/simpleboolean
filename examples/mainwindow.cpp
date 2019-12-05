#include <QtGui>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QMatrix4x4>
#include <simpleboolean/meshcombiner.h>
#include "mainwindow.h"
#include "testmesh.h"

MainWindow::MainWindow(void) 
{
    simpleboolean::Mesh mesh1;
    simpleboolean::Mesh mesh2;
    simpleboolean::loadTriangulatedObj(mesh1, "/Users/jeremy/Repositories/simpleboolean/examples/addax.obj");
    simpleboolean::loadTriangulatedObj(mesh2, "/Users/jeremy/Repositories/simpleboolean/examples/meerkat.obj");
    
    simpleboolean::MeshCombiner combiner;
    combiner.setMeshes(mesh1, mesh2);
    combiner.combine();
    
    {
        simpleboolean::Mesh mesh;
        combiner.getResult(simpleboolean::Type::Union, &mesh);
        exportTriangulatedObj(mesh, "/Users/jeremy/Desktop/debug-union.obj");
    }
    
    {
        simpleboolean::Mesh mesh;
        combiner.getResult(simpleboolean::Type::Intersection, &mesh);
        exportTriangulatedObj(mesh, "/Users/jeremy/Desktop/debug-intersection.obj");
    }
    
    {
        simpleboolean::Mesh mesh;
        combiner.getResult(simpleboolean::Type::Subtraction, &mesh);
        exportTriangulatedObj(mesh, "/Users/jeremy/Desktop/debug-subtraction.obj");
    }
    
    {
        simpleboolean::Mesh mesh;
        combiner.getResult(simpleboolean::Type::InversedSubtraction, &mesh);
        exportTriangulatedObj(mesh, "/Users/jeremy/Desktop/debug-inversedSubtraction.obj");
    }
    
    simpleboolean::Mesh subBlocksMesh;
    for (size_t i = 0; i < combiner.m_debugSubBlocks.size(); ++i) {
        const auto &subBlock = combiner.m_debugSubBlocks[i];
        size_t startVertexIndex = subBlocksMesh.vertices.size();
        for (const auto &vertex: subBlock.vertices) {
            auto newVertex = vertex;
            newVertex.xyz[0] += i * 1.0 - (combiner.m_debugSubBlocks.size() / 2);
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
    exportTriangulatedObj(subBlocksMesh, QString("/Users/jeremy/Desktop/debug-subblock-list.obj"));
    
    /*
    {
        exportTriangulatedObj(combiner.m_debugFirstMeshReTriangulated, "/Users/jeremy/Desktop/debug1.obj");
    }
    
    {
        exportTriangulatedObj(combiner.m_debugSecondMeshReTriangulated, "/Users/jeremy/Desktop/debug2.obj");
    }
    */
    
    /*
    m_renderWidget = new GLWidget(model);
    int size = QDesktopWidget().availableGeometry(this).size().height() * 0.7;
    resize(QSize(size, size));
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_renderWidget);
    setLayout(mainLayout);
    setWindowTitle(tr("simpleboolean"));
    */
    
    exit(0);
}

void MainWindow::keyPressEvent(QKeyEvent *e) 
{
    if (e->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(e);
    }
}
