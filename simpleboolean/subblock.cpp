#include <simpleboolean/subblock.h>
#include <QDebug>

namespace simpleboolean
{

void SubBlock::createSubBlocks(const std::vector<SubSurface> &firstSubSurfaces,
        const std::vector<SubSurface> &secondSubSurfaces,
        std::vector<SubBlock> &subBlocks)
{
    struct SubSurfaceLink
    {
        const SubSurface *subSurface = nullptr;
        int sourceMesh = -1;
        bool untested = true;
    };
    std::map<QString, std::vector<SubSurfaceLink>> subSurfaceMap;
    for (const auto &subSurface: firstSubSurfaces) {
        SubSurfaceLink link;
        link.subSurface = &subSurface;
        link.sourceMesh = 0;
        subSurfaceMap[subSurface.edgeLoopName].push_back(link);
    }
    for (const auto &subSurface: secondSubSurfaces) {
        SubSurfaceLink link;
        link.subSurface = &subSurface;
        link.sourceMesh = 1;
        subSurfaceMap[subSurface.edgeLoopName].push_back(link);
    }
    
    for (auto &itWithSameName: subSurfaceMap) {
        for (auto &it: itWithSameName.second) {
            if (!it.untested)
                continue;
            SubBlock subBlock;
            it.untested = false;
            for (const auto &face: it.subSurface->faces)
                subBlock.faces.insert({std::array<size_t, 3> {{face.indices[0], face.indices[1], face.indices[2]}}, it.sourceMesh});
            for (const auto &edgeLoopName: it.subSurface->ownerNames) {
                for (auto &condidate: subSurfaceMap[edgeLoopName]) {
                    if (!condidate.untested)
                        continue;
                    if (condidate.subSurface->isSharedByOthers)
                        continue;
                    if (condidate.sourceMesh == it.sourceMesh)
                        continue;
                    if (condidate.subSurface->isFrontSide != it.subSurface->isFrontSide)
                        continue;
                    condidate.untested = false;
                    for (const auto &face: condidate.subSurface->faces)
                        subBlock.faces.insert({std::array<size_t, 3> {{face.indices[0], face.indices[1], face.indices[2]}}, condidate.sourceMesh});
                }
            }
            subBlocks.push_back(subBlock);
        }
    }
    
    qDebug() << "subBlocks:" << subBlocks.size();
}

}

