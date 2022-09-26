#include "lastextreader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "vertex.h"




Las::PointCloud Las::LasTextReader::ReadFile(std::string fileName)
{
    std::ifstream file;
    file.open("../3Dprog22/txt_files/"+fileName);
    PointCloud out;
    if (file.is_open())
    {
        double x,y,z;
        file >> x >> y >> z;
        out.minMax.xMax = x;
        out.minMax.yMax = y;
        out.minMax.xMin = x;
        out.minMax.yMin = y;
        out.minMax.points++;

        while (!file.eof())
        {
             file >> x >> y >> z;
             if (x > out.minMax.xMax) out.minMax.xMax = x;
             else if (x < out.minMax.xMin) out.minMax.xMin = x;
             if (y > out.minMax.yMax) out.minMax.yMax = y;
             else if (y < out.minMax.yMin) out.minMax.yMin = y;
             out.minMax.points++;
        }

        file.seekg(0, std::ios_base::beg);

        out.points.reserve(out.minMax.points);

        while (!file.eof())
        {
            file >> x >> y >> z;
            out.points.push_back(XYZ{x,y,z});
        }

        file.close();
        std::cout << out.minMax.xMax-out.minMax.xMin << "\t" << out.minMax.yMax-out.minMax.yMin << "\t" << out.minMax.points << "\t" << std::endl;
        std::cout << out.points.size() << std::endl;
    } else {
        std::cout << "could not open file" << std::endl;
    }
    return out;
}

Las::PointCloudMesh Las::LasTextReader::GenerateVerticesFromFile(std::string fileName, int resolution, float scale)
{
    PointCloudMesh out;
    PointCloud pointCloud = LasTextReader::ReadFile(fileName);


    for (int x = 0; x < resolution; x++) {
        for (int y = 0; y < resolution; y++) {
            out.vertices.push_back(Vertex{x*(scale/(float)resolution), y*(scale/(float)resolution),2.f, 0,0,0});
            out.vertices.push_back(Vertex{(x+1)*(scale/(float)resolution), y*(scale/(float)resolution),1.5f, 0.5,0.5,0});
            out.vertices.push_back(Vertex{x*(scale/(float)resolution), (y+1)*(scale/(float)resolution),1.5f, 0.5,0.5,0});

            out.vertices.push_back(Vertex{(x+1)*(scale/(float)resolution), y*(scale/(float)resolution),1.5f, 0.5,0.5,0});
            out.vertices.push_back(Vertex{(x+1)*(scale/(float)resolution), (y+1)*(scale/(float)resolution),1.f, 1,1,0});
            out.vertices.push_back(Vertex{x*(scale/(float)resolution), (y+1)*(scale/(float)resolution),1.5f, 0.5,0.5,0});

            Triangle tri1;
            tri1.indicies[0] = 0 + out.indicesAndNeighbours.size();
            tri1.indicies[1] = 1 + out.indicesAndNeighbours.size();
            tri1.indicies[2] = 2 + out.indicesAndNeighbours.size();
            out.indicesAndNeighbours.push_back(tri1);

            Triangle tri2;
            tri2.indicies[0] = 0 + out.indicesAndNeighbours.size();
            tri2.indicies[1] = 1 + out.indicesAndNeighbours.size();
            tri2.indicies[2] = 2 + out.indicesAndNeighbours.size();
            out.indicesAndNeighbours.push_back(tri2);
        }
    }
    return out;

}

