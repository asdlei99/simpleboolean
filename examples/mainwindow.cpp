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
    simpleboolean::loadTriangulatedObj(mesh1, "/Users/jeremy/Repositories/simpleboolean/examples/a.obj");
    simpleboolean::loadTriangulatedObj(mesh2, "/Users/jeremy/Repositories/simpleboolean/examples/b.obj");
    
    simpleboolean::MeshCombiner combiner;
    combiner.setMeshes(mesh1, mesh2);
    combiner.combine(simpleboolean::Operation::Union);
    
    for (size_t i = 0; i < combiner.m_debugSubBlocks.size(); ++i) {
        const auto &subBlock = combiner.m_debugSubBlocks[i];
        exportTriangulatedObj(subBlock, QString("/Users/jeremy/Desktop/debug-subblock-%1.obj").arg(i + 1));
    }
    
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
