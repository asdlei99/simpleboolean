#ifndef SIMPLEBOOLEAN_SUB_BLOCK_H
#define SIMPLEBOOLEAN_SUB_BLOCK_H
#include <simpleboolean/subsurface.h>
#include <array>
#include <set>

namespace simpleboolean
{

class SubBlock
{
public:
    std::set<std::array<size_t, 3>> faces;

    static void createSubBlocks(const std::vector<SubSurface> &firstSubSurfaces,
        const std::vector<SubSurface> &secondSubSurfaces,
        std::vector<SubBlock> &subBlocks);
};

}

#endif
