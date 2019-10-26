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
    simpleboolean::loadTriangulatedObj(mesh1, "/Users/jeremy/Repositories/simpleboolean/examples/meerkat.obj");
    simpleboolean::loadTriangulatedObj(mesh2, "/Users/jeremy/Repositories/simpleboolean/examples/addax.obj");
    
    simpleboolean::MeshCombiner combiner;
    combiner.setMeshes(mesh1, mesh2);
    combiner.combine(simpleboolean::Operation::Union);
    
    Model *model = new Model(combiner.m_debugFirstMeshReTriangulated);
    exportTriangulatedObj(combiner.m_debugFirstMeshReTriangulated, "/Users/jeremy/Desktop/debug.obj");

    m_renderWidget = new GLWidget(model);
    int size = QDesktopWidget().availableGeometry(this).size().height() * 0.7;
    resize(QSize(size, size));
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_renderWidget);
    setLayout(mainLayout);
    setWindowTitle(tr("simpleboolean"));
}

void MainWindow::keyPressEvent(QKeyEvent *e) 
{
    if (e->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(e);
    }
}
