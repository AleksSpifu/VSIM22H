#ifndef TRIANGLESURFACE_H
#define TRIANGLESURFACE_H

#include "visualobject.h"
#include <iostream>
#include <fstream>
#include <string>
//#include "vector3d.h"

class TriangleSurface : public VisualObject
{
public:
   TriangleSurface();
   TriangleSurface(std::string fileName);
   ~TriangleSurface() override;

//   void readFile(std::string filnavn);
   void init(GLint matrixUniform) override;
   void draw() override;
    static float myEquation(float x, float y);
//~~
   void WriteToFile(std::string fileName);
   void ReadFromFile(std::string fileName);
   void CreateGraphVertices(float (f)(float, float));
   float MultivariableFunction(float x, float y);
//~~//

};
#endif // TRIANGLESURFACE_H
