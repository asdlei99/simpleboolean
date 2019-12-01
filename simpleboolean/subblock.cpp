#include <simpleboolean/subblock.h>
#include <map>

namespace simpleboolean
{

void SubBlock::createSubBlocks(const std::vector<SubSurface> &firstSubSurfaces,
        const std::vector<SubSurface> &secondSubSurfaces,
        std::vector<SubBlock> &subBlocks)
{
    SubBlock subBlock;
    for (const auto &a: firstSubSurfaces) {
        if (a.isSharedByOthers) {
            for (const auto &it: a.faces)
                subBlock.faces.push_back(it);
        }
    }
    for (const auto &b: secondSubSurfaces) {
        if (!b.isSharedByOthers) {
            for (const auto &it: b.faces)
                subBlock.faces.push_back(it);
        }
    }
    
    subBlocks.push_back(subBlock);
}

}

