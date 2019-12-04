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
        int usedCount = 0;
    };
    
    std::vector<SubSurfaceLink *> subSurfaceLinks;
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
    for (auto &it: subSurfaceMap) {
        for (auto &link: it.second)
            subSurfaceLinks.push_back(&link);
    }
    
    std::map<std::pair<std::pair<QString, bool>, int>, SubSurfaceLink *> subSurfaceLinkMap;
    for (auto &it: subSurfaceMap) {
        for (auto &subIt: it.second) {
            subSurfaceLinkMap[std::make_pair(std::make_pair(it.first, subIt.subSurface->isFrontSide),
                subIt.sourceMesh)] = &subIt;
        }
    }
    
    auto isLinkUsuable = [&](const SubSurfaceLink &link) {
        if (link.usedCount > 1)
            return false;
        return true;
    };
    
    std::vector<SubSurfaceLink *> subSurfacesContributeToMesh;
    for (auto &it: subSurfaceLinks) {
        if (!isLinkUsuable(*it))
            continue;
        std::queue<SubSurfaceLink *> links;
        links.push(it);
        SubBlock subBlock;
        std::set<std::pair<QString, int>> cycles;
        std::set<SubSurfaceLink *> visited;
        std::map<QString, int> cycleUsedNumMap;
        std::set<SubSurfaceLink *> usedCountChangedLinks;
        while (!links.empty()) {
            SubSurfaceLink *link = links.front();
            links.pop();
            if (visited.find(link) != visited.end())
                continue;
            visited.insert(link);
            auto &fromSubSurface = *link->subSurface;
            bool areAllSubSurfaceUsable = true;
            for (const auto &edgeLoopName: fromSubSurface.ownerNames) {
                if (!isLinkUsuable(*subSurfaceLinkMap[std::make_pair(edgeLoopName, link->sourceMesh)])) {
                    areAllSubSurfaceUsable = false;
                    break;
                }
            }
            bool cycleExisted = false;
            if (areAllSubSurfaceUsable) {
                for (const auto &edgeLoopName: fromSubSurface.ownerNames) {
                    if (cycles.find(std::make_pair(edgeLoopName.first,
                            link->sourceMesh)) != cycles.end()) {
                        cycleExisted = true;
                        break;
                    }
                }
            }
            if (areAllSubSurfaceUsable && !cycleExisted) {
                for (const auto &edgeLoopName: fromSubSurface.ownerNames) {
                    auto &neighbor = subSurfaceLinkMap[std::make_pair(edgeLoopName, link->sourceMesh)];
                    neighbor->usedCount++;
                    usedCountChangedLinks.insert(neighbor);
                    cycles.insert(std::make_pair(edgeLoopName.first, link->sourceMesh));
                    cycleUsedNumMap[edgeLoopName.first]++;
                }
                subSurfacesContributeToMesh.push_back(link);
            }
            for (const auto &edgeLoopName: fromSubSurface.ownerNames) {
                for (auto &condidate: subSurfaceMap[edgeLoopName.first]) {
                    if (visited.find(&condidate) != visited.end())
                        continue;
                    links.push(&condidate);
                }
            }
        }
        if (!subSurfacesContributeToMesh.empty()) {
            bool pass = true;
            for (auto &itCheckCycle: subSurfaceMap) {
                auto findUsedNum = cycleUsedNumMap.find(itCheckCycle.first);
                if (findUsedNum == cycleUsedNumMap.end()) {
                    pass = false;
                    break;
                }
                if (findUsedNum->second != 2) {
                    pass = false;
                    break;
                }
            }
            if (pass) {
                break;
            } else {
                subSurfacesContributeToMesh.clear();
                for (const auto &link: usedCountChangedLinks) {
                    --link->usedCount;
                }
            }
        }
    }
    
    if (subSurfacesContributeToMesh.empty())
        return;
    
    auto subSurfacesToSubBlock = [&](const std::vector<SubSurfaceLink *> &links) {
        SubBlock subBlock;
        for (const auto &link: links) {
            auto &fromSubSurface = *link->subSurface;
            for (const auto &face: fromSubSurface.faces) {
                subBlock.faces.insert({std::array<size_t, 3> {{face.indices[0], face.indices[1], face.indices[2]}}, link->sourceMesh});
            }
        }
        return subBlock;
    };

    auto createSubBlockFromTemplate = [&](const std::vector<SubSurfaceLink *> &links,
            bool flipFirst, bool flipSecond) {
        if (!flipFirst && !flipSecond)
            return subSurfacesToSubBlock(links);
        std::vector<SubSurfaceLink *> modifiedLinks;
        for (auto &link: links) {
            if ((0 == link->sourceMesh && !flipFirst) ||
                    (1 == link->sourceMesh && !flipSecond)) {
                modifiedLinks.push_back(link);
                continue;
            }
            for (const auto &edgeLoopName: link->subSurface->ownerNames) {
                auto oppositeLink = subSurfaceLinkMap[std::make_pair(std::make_pair(edgeLoopName.first,
                    !edgeLoopName.second), link->sourceMesh)];
                modifiedLinks.push_back(oppositeLink);
            }
        }
        return subSurfacesToSubBlock(modifiedLinks);
    };
    subBlocks.push_back(createSubBlockFromTemplate(subSurfacesContributeToMesh, false, false));
    subBlocks.push_back(createSubBlockFromTemplate(subSurfacesContributeToMesh, true, false));
    subBlocks.push_back(createSubBlockFromTemplate(subSurfacesContributeToMesh, false, true));
    subBlocks.push_back(createSubBlockFromTemplate(subSurfacesContributeToMesh, true, true));
    
    qDebug() << "subBlocks:" << subBlocks.size();
}

}

