#include <simpleboolean/subblock.h>
#include <map>

namespace simpleboolean
{

void SubBlock::createSubBlocks(const std::vector<SubSurface> &firstSubSurfaces,
        const std::vector<SubSurface> &secondSubSurfaces,
        std::vector<SubBlock> &subBlocks)
{
    std::pair<std::map<QString, std::pair<const SubSurface *, const SubSurface *>>,
        std::map<QString, std::pair<const SubSurface *, const SubSurface *>>> subSurfaceMap;
    for (size_t i = 0; i < firstSubSurfaces.size(); ++i) {
        const auto &subSurface = firstSubSurfaces[i];
        if (subSurface.isFrontSide)
            subSurfaceMap.first[subSurface.edgeLoopName].first = &firstSubSurfaces[i];
        else
            subSurfaceMap.first[subSurface.edgeLoopName].second = &firstSubSurfaces[i];
    }
    for (size_t i = 0; i < secondSubSurfaces.size(); ++i) {
        const auto &subSurface = secondSubSurfaces[i];
        if (subSurface.isFrontSide)
            subSurfaceMap.second[subSurface.edgeLoopName].first = &secondSubSurfaces[i];
        else
            subSurfaceMap.second[subSurface.edgeLoopName].second = &secondSubSurfaces[i];
    }
    
    SubBlock subBlock;
    for (const auto &first: firstSubSurfaces) {
        if (first.isSharedByOthers) {
            for (const auto &face: first.faces)
                subBlock.faces.push_back(face);
            const SubSurface *subSurface = nullptr;
            if (first.isFrontSide)
                subSurface = subSurfaceMap.second[first.edgeLoopName].second;
            else
                subSurface = subSurfaceMap.second[first.edgeLoopName].first;
            if (nullptr != subSurface) {
                for (const auto &face: subSurface->faces)
                    subBlock.faces.push_back(face);
            }
        }
    }
    
    subBlocks.push_back(subBlock);
}

}

