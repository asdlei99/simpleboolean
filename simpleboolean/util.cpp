#include <simpleboolean/util.h>
#include <QVector3D>
#include <QtGlobal>

namespace simpleboolean
{

static QVector3D vertexToQVector3D(const Vertex &first)
{
    return QVector3D(first.xyz[0], first.xyz[1], first.xyz[2]);
}

float distanceSquaredOfVertices(const Vertex &first, const Vertex &second)
{
    return (vertexToQVector3D(first) - vertexToQVector3D(second)).lengthSquared();
}

float distanceOfVertices(const Vertex &first, const Vertex &second)
{
    return (vertexToQVector3D(first) - vertexToQVector3D(second)).length();
}

float isNull(float number)
{
    return qFuzzyIsNull(number);
}

void projectToPlane(Vector planeNormal, Vertex planeOrigin,
    Vector planeX, const std::vector<Vertex> &points,
    std::vector<Vertex> &result)
{
    QVector3D normal = vertexToQVector3D(planeNormal);
    QVector3D origin = vertexToQVector3D(planeOrigin);
    QVector3D xAxis = vertexToQVector3D(planeX);
    QVector3D yAxis = QVector3D::crossProduct(normal, xAxis);
    for (const auto &it: points) {
        Vertex point2D;
        QVector3D direction = vertexToQVector3D(it) - origin;
        point2D.xyz[0] = QVector3D::dotProduct(direction, xAxis);
        point2D.xyz[1] = QVector3D::dotProduct(direction, yAxis);
        result.push_back(point2D);
    }
}

}
