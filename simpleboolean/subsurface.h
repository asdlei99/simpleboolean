#ifndef SIMPLEBOOLEAN_SUB_SURFACE_H
#define SIMPLEBOOLEAN_SUB_SURFACE_H
#include <vector>
#include <simpleboolean/meshdatatype.h>
#include <QString>
#include <map>
#include <set>

namespace simpleboolean
{

class SubSurface
{
public:
    QString edgeLoopName;
    std::vector<Face> faces;
    bool isFrontSide = true;
    bool isSharedByOthers = false;
    std::set<std::pair<QString, bool>> ownerNames;
    
    static QString createEdgeLoopName(const std::vector<size_t> &edgeLoop);
    static void createSubSurfaces(const std::vector<std::vector<size_t>> &edgeLoops,
        const std::vector<Face> &triangles,
        std::vector<SubSurface> &subSurfaces);
};

}

#endif
