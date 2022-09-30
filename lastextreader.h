#ifndef LASTEXTREADER_H
#define LASTEXTREADER_H

#include <vector>
#include <string>
#include "vertex.h"

namespace Las {

struct MinAndMax {
    double xMax{0}, xMin{0}, yMax{0}, yMin{0}, zMin{0}, zMax{0};
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

    static PointCloudMesh GenerateVerticesFromFile(std::string fileName, int resolution, float size = 10.f);
    static void ReadFile(std::string fileName, PointCloud &out);
    static double GetHeight(std::pair<int, float> averageHeights);
};

}

#endif // LASTEXTREADER_H
