#include "vertex.h"
#include <sstream>
std::ostream& operator<< (std::ostream& os, const Vertex& v) {
  os << std::fixed;
  os << "(" << v.m_xyz[0] << ", " << v.m_xyz[1] << ", " << v.m_xyz[2] << ") ";
  os << "(" << v.m_normal[0] << ", " << v.m_normal[1] << ", " << v.m_normal[2] << ") ";
  os << "(" << v.m_st[0] << ", " << v.m_st[1] << ") ";
  return os;
}
std::istream& operator>> (std::istream& is, Vertex& v) {
// Trenger fire temporære variabler som kun skal lese inn parenteser og komma
  char dum, dum2, dum3, dum4;
  is >> dum >> v.m_xyz[0] >> dum2 >> v.m_xyz[1] >> dum3 >> v.m_xyz[2] >> dum4;
  is >> dum >> v.m_normal[0] >> dum2 >> v.m_normal[1] >> dum3 >> v.m_normal[2] >> dum4;
  is >> dum >> v.m_st[0] >> dum2 >> v.m_st[1] >> dum3;
  return is;
}

std::string& operator>> (std::string& is, Vertex& v) {
    // Trenger fire temporære variabler som kun skal lese inn parenteser og komma
    std::stringstream ss(is);

    char dum1;
    std::string xstr, ystr, zstr;
    ss >> dum1 >> xstr >> ystr >> zstr;
    v.m_xyz[0] = std::stof(xstr);
    v.m_xyz[1] = std::stof(ystr);
    v.m_xyz[2] = std::stof(zstr);
    v.m_normal[0] = std::stof(xstr);
    v.m_normal[1] = std::stof(ystr);
    v.m_normal[2] = std::stof(zstr);
    return is;
}

QVector3D Vertex::GetXYZ()
{
    return QVector3D{m_xyz[0], m_xyz[1], m_xyz[2]};
}

Vertex::Vertex()
{

}

Vertex::Vertex(float x, float y, float z, float r, float g, float b)
    : m_xyz{x,y,z}, m_normal{r,g,b}, m_st{0.f, 0.f}
{

}

Vertex::Vertex(float x, float y, float z, float r, float g, float b, float s, float t)
    : m_xyz{x,y,z}, m_normal{r,g,b}, m_st{s, t}
{

}

Vertex::Vertex(QVector3D xyz, float r, float g, float b)
    :m_xyz{xyz.x(), xyz.y(), xyz.z()}, m_normal{r,g,b}, m_st{0.f, 0.f}
{

}

Vertex::Vertex(QVector3D xyz, QVector3D normal)
    :m_xyz{xyz.x(), xyz.y(), xyz.z()}, m_normal{normal.x(), normal.y(), normal.z()}, m_st{0.f, 0.f}
{

}

Vertex::Vertex(QVector3D xyz, QVector3D normal, float s, float t)
    :m_xyz{xyz.x(), xyz.y(), xyz.z()}, m_normal{normal.x(), normal.y(), normal.z()}, m_st{s, t}
{

}
