// AffineTest.cpp
// cs250 5/19

#include <iostream>
#include <iomanip>
#include <cmath>
#include "Affine.h"
using namespace std;


ostream& operator<<(ostream& s, const glm::vec4 &v) {
  return s << '[' << v.x << ',' << v.y << ',' << v.z << ',' << v.w << ']';
}


ostream& operator<<(ostream& s, const glm::mat4& A) {
  s << '{';
  for (int i=0; i < 4; ++i) {
    s << '{';
    for (int j=0; j < 4; ++j)
      s << A[j][i] << ((j < 3) ? ',' : '}');
    s << ((i < 3) ? ',' : '}');
  }
  return s;
}


int main(void) {
  const glm::vec4 O = cs250::point(0,0,0);
  const float PI = 4.0f*atan(1.0f);
  cout << fixed << setprecision(3);
  cout << boolalpha;

  glm::vec4 v = cs250::vector(4,-5,3);
  glm::mat4 A = cs250::translate(v) * cs250::scale(3);
  cout << A << endl;

  glm::vec4 P = cs250::point(9,-2,-7),
            Q = O + 3.0f*(P-O) + v,
            R = A*P;
  cout << glm::length(R-Q) << endl;

  glm::vec4 u = cs250::vector(7,5,1);
  float degrees = 54.0f;
  A = cs250::rotate(degrees,v) * cs250::translate(u);
  cout << A << endl;

  v = glm::normalize(v);
  float radians = 0.3f*PI;
  Q = O + cos(radians)*(P+u-O) + (1-cos(radians))*glm::dot(v,P+u-O)*v
        + sin(radians)*cs250::cross(v,P+u-O);
  R = A*P;
  cout << glm::length(R-Q) << endl;

  u = cs250::vector(5,4,0);
  cout << cs250::angle(A*u,u) << endl;
  
  glm::mat4 B = cs250::affine(cs250::vector(5,2,1),
                              cs250::vector(1,4,5),
                              cs250::vector(9,8,2),
                              cs250::point(4,4,3));
  cout << B << endl;

  glm::mat4 iB = cs250::affineInverse(B);
  cout << (iB*B) << endl;

  u = 3.0f*P + 2.0f*Q - 5.0f*R;
  cout << u << endl;

  P = 3.0f*P + 2.0f*Q - 7.0f*R;
  cout << cs250::isPoint(P) << endl;

  return 0;
}

