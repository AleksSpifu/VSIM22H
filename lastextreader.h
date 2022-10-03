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

    friend std::ostream& operator<< (std::ostream& os, const Las::Triangle& t) {
      os << std::fixed;
      os << t.indicies[0] << " " << t.indicies[1] << " " << t.indicies[2] << " ";
      os << t.neighbours[0] << " " << t.neighbours[1] << " " << t.neighbours[2];
      return os;
    }
    friend std::istream& operator>> (std::istream& is, Las::Triangle& t) {
        is >> t.indicies[0] >> t.indicies[1] >> t.indicies[2];
        is >> t.neighbours[0] >> t.neighbours[1] >> t.neighbours[2];
      return is;
    }
};

struct PointCloudMesh {
    std::vector<Vertex> vertices;
    std::vector<Triangle> indicesAndNeighbours;
    float scale;
};


class LasTextReader
{
public:

    static PointCloudMesh GenerateVerticesFromFile(std::string fileName, int resolution, float size = 10.f);

    static bool ReadFromFile(std::string fileName, PointCloudMesh &ptcloudmesh);
private:

    static double GetHeight(std::pair<int, float> averageHeights);
    template<typename T>
    static void WriteToFile(std::string fileName, const std::vector<T> &mVertices);
    template<typename T>
    static bool ReadVector(std::string fileName, std::vector<T> &vec);
};

}

#endif // LASTEXTREADER_H
