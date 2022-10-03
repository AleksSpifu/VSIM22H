#ifndef VERTEX_H
#define VERTEX_H
#include <QVector3D>
#include <iostream>
//#include "interactiveobject.h"

class Vertex {
   //! Overloaded ostream operator which writes all vertex data on an open textfile stream
   friend std::ostream& operator<< (std::ostream&, const Vertex&);

   //! Overloaded ostream operator which reads all vertex data from an open textfile stream
   friend std::istream& operator>> (std::istream&, Vertex&);
   friend std::string& operator>> (std::string& is, Vertex& v);

private:
   float m_xyz[3];       	// position i 3D
   float m_normal[3];       // normal in 3D or rgb colors
   float m_st[2];        	// texture coordinates if used
   friend class InteractiveObject;

public:
   // Constructors
   QVector3D GetXYZ();
   Vertex();
   Vertex(float x, float y, float z, float r, float g, float b);
   Vertex(QVector3D xyz, float r, float g, float b);
   Vertex(QVector3D xyz, QVector3D normal);
   Vertex(float x, float y, float z, float r, float g, float b, float s, float t);
   Vertex(QVector3D xyz, QVector3D normal, float s, float t);
};
#endif // VERTEX_H
