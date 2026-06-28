#pragma once
#include <array>
#include <vector>

struct Point3D { double x, y, z; };
struct Tet { std::array<int, 4> vi; };

struct Mesh3D {
    std::vector<Point3D> nodes;
    std::vector<Tet>     tets;
    int n_inverted = 0;
};

inline double tet_volume(const Point3D& a, const Point3D& b,
                          const Point3D& c, const Point3D& d) {
    double bx=b.x-a.x, by=b.y-a.y, bz=b.z-a.z;
    double cx=c.x-a.x, cy=c.y-a.y, cz=c.z-a.z;
    double dx=d.x-a.x, dy=d.y-a.y, dz=d.z-a.z;
    return (bx*(cy*dz - cz*dy) - by*(cx*dz - cz*dx) + bz*(cx*dy - cy*dx)) / 6.0;
}

inline bool circumsphere_contains(const Point3D& a, const Point3D& b,
                                   const Point3D& c, const Point3D& d,
                                   const Point3D& p) {
    auto sq = [](double v){ return v*v; };
    double ax=a.x-p.x, ay=a.y-p.y, az=a.z-p.z, ar=sq(ax)+sq(ay)+sq(az);
    double bx=b.x-p.x, by=b.y-p.y, bz=b.z-p.z, br=sq(bx)+sq(by)+sq(bz);
    double cx=c.x-p.x, cy=c.y-p.y, cz=c.z-p.z, cr=sq(cx)+sq(cy)+sq(cz);
    double dx=d.x-p.x, dy=d.y-p.y, dz=d.z-p.z, dr=sq(dx)+sq(dy)+sq(dz);
    auto d3 = [](double a0,double a1,double a2,
                 double b0,double b1,double b2,
                 double c0,double c1,double c2) {
        return a0*(b1*c2-b2*c1) - a1*(b0*c2-b2*c0) + a2*(b0*c1-b1*c0);
    };
    double det = ax*d3(by,bz,br, cy,cz,cr, dy,dz,dr)
               - ay*d3(bx,bz,br, cx,cz,cr, dx,dz,dr)
               + az*d3(bx,by,br, cx,cy,cr, dx,dy,dr)
               - ar*d3(bx,by,bz, cx,cy,cz, dx,dy,dz);
    return det < 0.0;
}
