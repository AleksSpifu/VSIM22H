#include "lastextreader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "vertex.h"




void Las::LasTextReader::ReadFile(std::string fileName, PointCloud& out)
{
    std::ifstream file;
    file.open("../3Dprog22/txt_files/"+fileName);

    if (file.is_open())
    {
        std::cout << "file is open, reading" << std::endl;
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
             if (z < out.minMax.zMin) out.minMax.zMin = z;
             else if (z > out.minMax.zMax) out.minMax.zMax = z;
             out.minMax.points++;
        }

        file.seekg(0, std::ios_base::beg);

        std::cout << "finished reading file, putting points in out struct" << std::endl;
        out.points.reserve(out.minMax.points);

        while (!file.eof())
        {
            file >> x >> y >> z;
            x = x - out.minMax.xMin;
            y = y - out.minMax.yMin;
            z = z - out.minMax.zMin;

        }

        file.close();

    } else {
        std::cout << "could not open file" << std::endl;
    }

}

Las::PointCloudMesh Las::LasTextReader::GenerateVerticesFromFile(std::string fileName, int resolution, float size)
{
    PointCloudMesh out;
    PointCloud pointCloud;

    std::ifstream file;
    file.open("../3Dprog22/txt_files/"+fileName);

    if (file.is_open())
    {
        std::cout << "file is open, reading" << std::endl;
        double x,y,z;
        file >> x >> y >> z;
        pointCloud.minMax.xMax = x;
        pointCloud.minMax.yMax = y;
        pointCloud.minMax.zMax = z;
        pointCloud.minMax.xMin = x;
        pointCloud.minMax.yMin = y;
        pointCloud.minMax.zMin = z;

        pointCloud.minMax.points++;

        while (!file.eof())
        {
             file >> x >> y >> z;
             if (x > pointCloud.minMax.xMax) pointCloud.minMax.xMax = x;
             else if (x < pointCloud.minMax.xMin) pointCloud.minMax.xMin = x;
             if (y > pointCloud.minMax.yMax) pointCloud.minMax.yMax = y;
             else if (y < pointCloud.minMax.yMin) pointCloud.minMax.yMin = y;
             if (z < pointCloud.minMax.zMin) pointCloud.minMax.zMin = z;
             else if (z > pointCloud.minMax.zMax) pointCloud.minMax.zMax = z;
             pointCloud.minMax.points++;
        }

        file.close();


        std::cout << "finished reading file, putting points in out struct" << std::endl;
        pointCloud.points.reserve(pointCloud.minMax.points);

        float step = size / (float)resolution;

        float span = std::max(pointCloud.minMax.xMax - pointCloud.minMax.xMin, pointCloud.minMax.yMax - pointCloud.minMax.yMin);

        span += 0.001f; // Denne er for å unngå avrundingsfeil. Ikke spør hvor lang tid det tok før jeg fant ut det.

        std::vector<std::vector<std::pair<int, float>>> averageHeights(resolution, std::vector<std::pair<int, float>>(resolution));

        file.open("../3Dprog22/txt_files/"+fileName);
        while (!file.eof())
        {
            file >> x >> y >> z;

            float normalisedx = x - pointCloud.minMax.xMin;
            float normalisedy = y - pointCloud.minMax.yMin;
            float normalisedz = z - pointCloud.minMax.zMin;

            int xPos, yPos;
            xPos = (normalisedx / span) * resolution;
            yPos = (normalisedy / span) * resolution;


            averageHeights[xPos][yPos].first++;
            averageHeights[xPos][yPos].second += normalisedz;


        }

        float scaleDifference = size / span;
        float heightSpan = pointCloud.minMax.zMax - pointCloud.minMax.zMin;
        std::cout << "scaledifference " << scaleDifference << std::endl;

        std::cout << "making mesh" << std::endl;

        for (int x = 0; x * step < size; x++) {
            for (int y = 0; y * step < size; y++) {
                float height;
                if (averageHeights[x][y].first == 0) {
                    height = 0;
                } else {
                    height = averageHeights[x][y].second / (float)averageHeights[x][y].first;
                }

                out.vertices.push_back(Vertex(x * step, y * step, height * scaleDifference, 1,1,1));
            }
        }

        file.close();

        std::cout << "returning" << std::endl;

    } else {
        std::cout << "could not open file" << std::endl;
    }



    return out;

}
























































