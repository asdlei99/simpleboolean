#ifndef SIMPLEBOOLEAN_SUB_SURFACE_H
#define SIMPLEBOOLEAN_SUB_SURFACE_H
#include <vector>
#include <simpleboolean/meshdatatype.h>
#include <QString>
#include <map>

namespace simpleboolean
{

class SubSurface
{
public:
    QString edgeLoopName;
    std::vector<Face> faces;
    bool isFrontSide = true;
    bool isSharedByOthers = false;
    
    static QString createEdgeLoopName(const std::vector<size_t> &edgeLoop, bool *nameReversed);
    static void createSubSurfaces(const std::vector<std::vector<size_t>> &edgeLoops,
        const std::vector<Face> &triangles,
        std::vector<SubSurface> &subSurfaces);
};

}

#endif
