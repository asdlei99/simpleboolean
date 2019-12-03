#include <simpleboolean/subblock.h>
#include <QDebug>
#include <queue>

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
            if (it.subSurface->isSharedByOthers)
                continue;
            std::queue<SubSurfaceLink *> links;
            links.push(&it);
            SubBlock subBlock;
            std::set<std::pair<QString, int>> cycles;
            std::set<SubSurfaceLink *> visited;
            while (!links.empty()) {
                SubSurfaceLink *link = links.front();
                links.pop();
                if (visited.find(link) != visited.end())
                    continue;
                visited.insert(link);
                auto &fromSubSurface = *link->subSurface;
                bool cycleExisted = false;
                for (const auto &condidateEdgeLoopName: fromSubSurface.ownerNames) {
                    if (cycles.find(std::make_pair(condidateEdgeLoopName,
                            link->sourceMesh)) != cycles.end()) {
                        cycleExisted = true;
                        break;
                    }
                }
                if (!cycleExisted) {
                    for (const auto &edgeLoopName: fromSubSurface.ownerNames) {
                        cycles.insert(std::make_pair(edgeLoopName, link->sourceMesh));
                    }
                    for (const auto &face: fromSubSurface.faces) {
                        subBlock.faces.insert({std::array<size_t, 3> {{face.indices[0], face.indices[1], face.indices[2]}}, it.sourceMesh});
                    }
                }
                for (const auto &edgeLoopName: fromSubSurface.ownerNames) {
                    for (auto &condidate: subSurfaceMap[edgeLoopName]) {
                        if (visited.find(&condidate) != visited.end())
                            continue;
                        if (condidate.sourceMesh == link->sourceMesh)
                            continue;
                        //if (condidate.subSurface->isFrontSide != fromSubSurface.isFrontSide)
                        //    continue;
                        links.push(&condidate);
                    }
                }
            }
            if (!subBlock.faces.empty())
                subBlocks.push_back(subBlock);
        }
    }
    
    qDebug() << "subBlocks:" << subBlocks.size();
}

}

