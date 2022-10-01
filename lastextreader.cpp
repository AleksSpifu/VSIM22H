#include "lastextreader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "vertex.h"




void Las::LasTextReader::ReadFile(std::string fileName, PointCloud& out)
{
    std::ifstream file;
    file.open("../VSIM22H/txt_files/"+fileName);

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

double Las::LasTextReader::GetHeight(std::pair<int, float> averageHeights)
{
    if (averageHeights.first == 0) {
        return 0;
    } else {
        return averageHeights.second / (float)averageHeights.first;
    }
}

void Las::LasTextReader::WriteToFile(std::string fileName, const std::vector<Vertex>& mVertices)
{
    std::ofstream file;
    file.open("../VSIM22H/txt_files/"+fileName);
    if(file.is_open())
    {
        file << mVertices.size() << std::endl;
        for(auto i = 0; i < mVertices.size(); i++)
        {
            file << mVertices[i] << std::endl;
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
                Triangle tri;
                tri.indicies[0] = y*resolution+x;
                tri.indicies[1] = y*resolution+x+1;
                tri.indicies[2] = (y+1)*resolution+x;

                tri.neighbours[0] = std::max((resolution-1)*y*2+1+2*x, -1);
                tri.neighbours[1] = std::max((resolution-1)*y*2-1+2*x, -1);
                tri.neighbours[2] = std::max((resolution-1)*(y-1)*2+1+2*x, -1);

                Triangle tri2;
                tri2.indicies[0] = (y+1)*resolution+x;
                tri2.indicies[1] = y*resolution+x+1;
                tri2.indicies[2] = (y+1)*resolution+x+1;

                tri2.neighbours[0] = std::max((resolution-1)*y*2+2+2*x, -1);
                tri2.neighbours[1] = std::max((resolution-1)*y*2-1+2*x, -1);
                tri2.neighbours[2] = std::max((resolution-1)*y*2+2*x, -1);

                out.indicesAndNeighbours.push_back(tri);
                out.indicesAndNeighbours.push_back(tri2);
            }
        }

        file.close();

        WriteToFile(fileName + "verts.txt", out.vertices);

        std::cout << "returning" << std::endl;

    } else {
        std::cout << "could not open file" << std::endl;
    }



    return out;

}
























































