#include "lastextreader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "vertex.h"

template <typename T>
bool Las::LasTextReader::ReadVector(std::string fileName, std::vector<T>& vec)
{
    std::ifstream file;
    file.open("../VSIM22H/txt_files/"+fileName);
    if (file.is_open())
    {
        int x{0};
        T input;
        file >> x;
        vec.reserve(x);
        for (int i=0; i<x; i++)
        {
             file >> input;
             vec.push_back(input);
        }
        file.close();
    } else {
        return false;
    }
    return true;
}

bool Las::LasTextReader::ReadFromFile(std::string fileName, PointCloudMesh& ptcloudmesh)
{
    if (!ReadVector(fileName.substr(0, fileName.find(".txt")) + "-verts.txt", ptcloudmesh.vertices))
    {
        return false;
    }
    if (!ReadVector<Triangle>(fileName.substr(0, fileName.find(".txt")) + "-tris.txt", ptcloudmesh.indicesAndNeighbours))
    {
        return false;
    }
    return true;
}

double Las::LasTextReader::GetHeight(std::pair<int, float> averageHeights)
{
    if (averageHeights.first == 0) {
        return 0;
    } else {
        return averageHeights.second / (float)averageHeights.first;
    }
}


template <typename T> void Las::LasTextReader::WriteToFile(std::string fileName, const std::vector<T>& input)
{
    std::ofstream file;
    file.open("../VSIM22H/txt_files/"+fileName);
    if(file.is_open())
    {
        file << input.size() << std::endl;
        for(unsigned int i = 0; i < input.size(); i++)
        {
            file << input[i] << std::endl;
        }
        file.close();
    }
}

Las::PointCloudMesh Las::LasTextReader::GenerateVerticesFromFile(std::string fileName, int resolution, float size)
{
    PointCloudMesh out;
    PointCloud pointCloud;

    std::ifstream file;
    file.open("../VSIM22H/txt_files/"+fileName);

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

        file.open("../VSIM22H/txt_files/"+fileName);
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

        out.indicesAndNeighbours.reserve(resolution * resolution * 2);

        for (int y = 0; y * step < size; y++) {
            for (int x = 0; x * step < size; x++) {
                float height = GetHeight(averageHeights[x][y]);

                if (x == 0 || y == 0) {
                    out.vertices.push_back(Vertex(x * step, y * step, height * scaleDifference, 0,0,1, ((float)x / (float)resolution), ((float)y / (float)resolution)));
                } else {

                    QVector3D adjacentVert1{(x-1)*step, y*step, (float)GetHeight(averageHeights[x-1][y]) * scaleDifference};
                    QVector3D adjacentVert2{x*step, (y-1)*step, (float)GetHeight(averageHeights[x][y-1]) * scaleDifference};
                    QVector3D mypos{x * step, y * step, height * scaleDifference};

                    QVector3D tri2normal = QVector3D::crossProduct(adjacentVert1-mypos, adjacentVert2-mypos);

                    out.vertices.push_back(Vertex(x * step, y * step, height * scaleDifference, tri2normal.x(),tri2normal.y(),tri2normal.z(),  ((float)x / (float)resolution), ((float)y / (float)resolution)));
                }

                if (x == resolution-1 || y == resolution-1) {
                    continue;
                }
                int currentTriangleIndex = (resolution - 1) * y * 2 + (2 * x);
                Triangle tri;
                tri.indicies[0] = y*resolution+x;
                tri.indicies[1] = y*resolution+x+1;
                tri.indicies[2] = (y+1)*resolution+x;

                tri.neighbours[0] = currentTriangleIndex + 1;
                tri.neighbours[1] = currentTriangleIndex - 1;
                tri.neighbours[2] = currentTriangleIndex - (2 * (resolution-1)) + 1;

                currentTriangleIndex +=1;

                Triangle tri2;
                tri2.indicies[0] = (y+1)*resolution+x;
                tri2.indicies[1] = y*resolution+x+1;
                tri2.indicies[2] = (y+1)*resolution+x+1;

                tri2.neighbours[0] = currentTriangleIndex + 1;
                tri2.neighbours[1] = currentTriangleIndex + ((resolution-1) * 2) - 1;
                tri2.neighbours[2] = currentTriangleIndex - 1;

                if (y == 0)
                {
                    tri.neighbours[2] = -1;
                }
                if (x == 0)
                {
                    tri.neighbours[1] = -1;
                }
                if (y == resolution-2)
                {
                    tri2.neighbours[1] = -1;
                }
                if (x == resolution-2)
                {
                    tri2.neighbours[0] = -1;
                }


                out.indicesAndNeighbours.push_back(tri);
                out.indicesAndNeighbours.push_back(tri2);
            }
        }

        file.close();

        WriteToFile<Vertex>(fileName.substr(0, fileName.find(".txt")) + "-verts.txt", out.vertices);
        WriteToFile<Triangle>(fileName.substr(0, fileName.find(".txt")) + "-tris.txt", out.indicesAndNeighbours);

        std::cout << "returning" << std::endl;

    } else {
        std::cout << "could not open file" << std::endl;
    }



    return out;

}
























































