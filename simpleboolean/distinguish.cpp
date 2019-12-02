#include <simpleboolean/distinguish.h>
#include <QVector3D>
#include <cmath>
#include <QDebug>
#include <set>

namespace simpleboolean
{

static void resolveBoundingBox(const SubBlock &subBlock, const std::vector<Vertex> &vertices,
    QVector3D *low, QVector3D *high)
{
    float left = 0;
    bool leftFirstTime = true;
    float right = 0;
    bool rightFirstTime = true;
    float top = 0;
    bool topFirstTime = true;
    float bottom = 0;
    bool bottomFirstTime = true;
    float zLeft = 0;
    bool zLeftFirstTime = true;
    float zRight = 0;
    bool zRightFirstTime = true;
    for (const auto &face: subBlock.faces) {
        for (size_t i = 0; i < 3; ++i) {
            const auto &v = vertices[face.first[i]];
            const auto &x = v.xyz[0];
            const auto &y = v.xyz[1];
            const auto &z = v.xyz[2];
            if (leftFirstTime || x < left) {
                left = x;
                leftFirstTime = false;
            }
            if (topFirstTime || y < top) {
                top = y;
                topFirstTime = false;
            }
            if (rightFirstTime || x > right) {
                right = x;
                rightFirstTime = false;
            }
            if (bottomFirstTime || y > bottom) {
                bottom = y;
                bottomFirstTime = false;
            }
            if (zLeftFirstTime || z < zLeft) {
                zLeft = z;
                zLeftFirstTime = false;
            }
            if (zRightFirstTime || z > zRight) {
                zRight = z;
                zRightFirstTime = false;
            }
        }
    }
    low->setX(left);
    high->setX(right);
    low->setY(top);
    high->setY(bottom);
    low->setZ(zLeft);
    high->setZ(zRight);
}

void Distinguish::distinguish(std::vector<SubBlock> &subBlocks,
        const std::vector<Vertex> &vertices,
        std::vector<int> *indicesToSubBlocks)
{
    indicesToSubBlocks->resize(4, -1);
    
    if (subBlocks.size() != 4)
        return;
    
    auto calculateCenter = [&](const SubBlock &subBlock) {
        QVector3D sumOfPositions;
        size_t num = 0;
        for (const auto &face: subBlock.faces) {
            for (size_t i = 0; i < 3; ++i) {
                const auto &v = vertices[face.first[i]];
                sumOfPositions += QVector3D(v.xyz[0], v.xyz[1], v.xyz[2]);
                ++num;
            }
        }
        if (0 == num)
            return sumOfPositions;
        return sumOfPositions / num;
    };
    
    auto calculateMaxRadius2 = [&](const SubBlock &subBlock) {
        QVector3D center = calculateCenter(subBlock);
        std::set<size_t> indices;
        for (const auto &face: subBlock.faces) {
            for (size_t i = 0; i < 3; ++i) {
                indices.insert(face.first[i]);
            }
        }
        float maxRadius2 = 0;
        for (const auto &index: indices) {
            const auto &v = vertices[index];
            float radius2 = (QVector3D(v.xyz[0], v.xyz[1], v.xyz[2]) - center).lengthSquared();
            if (radius2 > maxRadius2)
                maxRadius2 = radius2;
        }
        return maxRadius2;
    };
    
    std::vector<std::pair<QVector3D, QVector3D>> boundingBoxs(2);
    std::vector<float> volumes(2);
    for (size_t i = 0; i < 2; ++i) {
        auto &boundingBox = boundingBoxs[i];
        resolveBoundingBox(subBlocks[i], vertices, &boundingBox.first, &boundingBox.second);
        volumes[i] = std::sqrt(std::pow(boundingBox.first.x() - boundingBox.second.x(), 2) +
            std::pow(boundingBox.first.y() - boundingBox.second.y(), 2) +
            std::pow(boundingBox.first.z() - boundingBox.second.z(), 2));
    }
    
    int unionIndex = -1;
    int intersectionIndex = -1;
    int subtractionIndex = -1;
    int inversedSubtractionIndex = -1;
    
    if (qFuzzyIsNull(volumes[0] - volumes[1])) {
        float firstMaxRadius2 = calculateMaxRadius2(subBlocks[0]);
        float secondMaxRadius2 = calculateMaxRadius2(subBlocks[1]);
        if (firstMaxRadius2 > secondMaxRadius2) {
            unionIndex = 0;
            intersectionIndex = 1;
        } else {
            unionIndex = 1;
            intersectionIndex = 0;
        }
    } else if (volumes[0] > volumes[1]) {
        unionIndex = 0;
        intersectionIndex = 1;
    } else {
        unionIndex = 1;
        intersectionIndex = 0;
    }
    
    const auto &outterSubBlock = subBlocks[unionIndex];
    const auto &innerSubBlock = subBlocks[intersectionIndex];
    for (const auto &it: subBlocks[2].faces) {
        auto findOutter = outterSubBlock.faces.find(it.first);
        if (findOutter != outterSubBlock.faces.end()) {
            if (0 == findOutter->second) {
                subtractionIndex = 2;
                inversedSubtractionIndex = 3;
            } else {
                subtractionIndex = 3;
                inversedSubtractionIndex = 2;
            }
            break;
        }
        auto findInner = innerSubBlock.faces.find(it.first);
        if (findInner != innerSubBlock.faces.end()) {
            if (0 == findInner->second) {
                subtractionIndex = 3;
                inversedSubtractionIndex = 2;
            } else {
                subtractionIndex = 2;
                inversedSubtractionIndex = 3;
            }
            break;
        }
    }
    
    for (size_t i = 2; i < 4; ++i) {
        auto &subBlock = subBlocks[i];
        for (auto &face: subBlock.faces) {
            if (innerSubBlock.faces.find(face.first) == innerSubBlock.faces.end())
                continue;
            face.second = -1; //-1 means this face should reverse
        }
    }
    
    (*indicesToSubBlocks)[(int)Type::Union] = unionIndex;
    (*indicesToSubBlocks)[(int)Type::Intersection] = intersectionIndex;
    (*indicesToSubBlocks)[(int)Type::Subtraction] = subtractionIndex;
    (*indicesToSubBlocks)[(int)Type::InversedSubtraction] = inversedSubtractionIndex;
}

}


