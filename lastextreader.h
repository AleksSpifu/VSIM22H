#ifndef LASTEXTREADER_H
#define LASTEXTREADER_H

#include <vector>
#include <string>
#include "vertex.h"

namespace Las {

struct MinAndMax {
    double xMax{0}, xMin{0}, yMax{0}, yMin{0};
    unsigned long long points{0};
};

struct XYZ {
    double x{0}, y{0}, z{0};
};

struct PointCloud {
    std::vector<XYZ> points;
    MinAndMax minMax;
};

struct Triangle {
    int indicies[3];
    int neighbours[3];
};

struct PointCloudMesh {
    std::vector<Vertex> vertices;
    std::vector<Triangle> indicesAndNeighbours;
};


class LasTextReader
{
public:

    static PointCloud ReadFile(std::string fileName);
    static PointCloudMesh GenerateVerticesFromFile(std::string fileName, int resolution, float scale = 5.f);
};

}

#endif // LASTEXTREADER_H
