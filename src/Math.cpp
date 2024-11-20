/*
 *  Math.cpp
 *
 *  Created on: 2024/11/12
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "Math.hpp"
#include <cmath>
#include <algorithm>

namespace small3d {
  Vec3::Vec3()
  {
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
  }
  Vec3::Vec3(float x, float y, float z)
  {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Vec3::Vec3(float v) 
  {
    this->x = v;
    this->y = v;
    this->z = v;
  }

  Vec3::Vec3(const Vec4& vec) {
    this->x = vec.x;
    this->y = vec.y;
    this->z = vec.z;
  }

  Vec3& Vec3::operator+=(const Vec3& other)
  {
    this->x += other.x;
    this->y += other.y;
    this->z += other.z;
    return *this;
  }

  Vec3 Vec3::operator+(const Vec3& other) const
  {
    Vec3 result = *this;
    result += other;
    return result;
  }

  Vec3& Vec3::operator-=(const Vec3& other)
  {
    this->x -= other.x;
    this->y -= other.y;
    this->z -= other.z;
    return *this;
  }

  Vec3 Vec3::operator-(const Vec3& other) const
  {
    Vec3 result = *this;
    result -= other;
    return result;
  }

  Vec3 Vec3::operator*(const Vec3& other) const
  {
    Vec3 result = *this;
    result.x *= other.x;
    result.y *= other.y;
    result.z *= other.z;
    return result;
  }

  Vec3 Vec3::operator*(const float v) const
  {
    Vec3 result = *this;
    result.x *= v;
    result.y *= v;
    result.z *= v;
    return result;
  }

  Vec3 Vec3::operator/(const float v) const
  {
    Vec3 result = *this;
    result.x /= v;
    result.y /= v;
    result.z /= v;
    return result;
  }

  Vec3& Vec3::operator/=(const float v)
  {
    this->x /= v;
    this->y /= v;
    this->z /= v;
    return *this;
  }

  Vec3& Vec3::operator=(const Vec3& other)
  {
    this->x = other.x;
    this->y = other.y;
    this->z = other.z;

    return *this;
  }

  Vec3& Vec3::operator+=(const float v) {
    this->x += v;
    this->y += v;
    this->z += v;
    return *this;

  }

  bool Vec3::operator==(const Vec3& other) const
  {
    return this->x == other.x && this->y == other.y && this->z == other.z;
  }

  bool Vec3::operator!=(const Vec3& other) const
  {
    return !(*this == other);
  }

  Vec4::Vec4()
  {
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
    this->w = 0.0f;
  }

  Vec4::Vec4(float x, float y, float z, float w)
  {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }

  Vec4::Vec4(const Vec3& vec3, float w)
  {
    this->x = vec3.x;
    this->y = vec3.y;
    this->z = vec3.z;
    this->w = w;
  }

  Vec4& Vec4::operator+=(const Vec4& other)
  {
    this->x += other.x;
    this->y += other.y;
    this->z += other.z;
    this->w += other.w;
    return *this;
  }

  Vec4 Vec4::operator+(const Vec4& other)
  {
    Vec4 result = *this;
    result += other;
    return result;
  }

  Vec4 Vec4::operator-(const Vec4& other)
  {
    Vec4 result = *this;
    result.x -= other.x;
    result.y -= other.y;
    result.z -= other.z;
    result.w -= other.w;
    return result;
  }

  Vec4& Vec4::operator*=(const float v)
  {
    this->x *= v;
    this->y *= v;
    this->z *= v;
    this->w *= v;
    return *this;
  }

  Vec4 Vec4::operator*(const float v) const
  {
    Vec4 result = *this;
    result *= v;
    return result;
  }

  Vec4 Vec4::operator*(const Vec4& other) const
  {
    Vec4 result = *this;
    result.x *= other.x;
    result.y *= other.y;
    result.z *= other.z;
    result.w *= other.w;
    return result;
  }

  Vec4& Vec4::operator/=(const float div)
  {
    this->x /= div;
    this->y /= div;
    this->z /= div;
    this->w /= div;
    return *this;
  }

  float& Vec4::operator[](int idx)
  {
    switch (idx) {
    case 0:
      return x;
      break;
    case 1:
      return y;
      break;
    case 2:
      return z;
      break;
    case 3:
      return w;
      break;
    default:
      return w;
      break;
    }
  }

  Vec4& Vec4::operator=(const Vec4& other)
  {
    this->x = other.x;
    this->y = other.y;
    this->z = other.z;
    this->w = other.w;

    return *this;
  }


  Mat4::Mat4()
  {
    data[0] = Vec4();
    data[1] = Vec4();
    data[2] = Vec4();
    data[3] = Vec4();
  }

  Mat4::Mat4(float v) : Mat4()
  {
    data[0].x = v;
    data[1].y = v;
    data[2].z = v;
    data[3].w = v;
  }

  Mat4::Mat4(Vec4 r0, Vec4 r1, Vec4 r2, Vec4 r3)
  {
    data[0] = r0;
    data[1] = r1;
    data[2] = r2;
    data[3] = r3;
  }

  Mat4::Mat4(float r0x, float r0y, float r0z, float r0w, float r1x, float r1y, float r1z, float r1w, float r2x, float r2y, float r2z, float r2w, float r3x, float r3y, float r3z, float r3w)
  {
    data[0].x = r0x;
    data[0].y = r0y;
    data[0].z = r0z;
    data[0].w = r0w;

    data[1].x = r1x;
    data[1].y = r1y;
    data[1].z = r1z;
    data[1].w = r1w;

    data[2].x = r2x;
    data[2].y = r2y;
    data[2].z = r2z;
    data[2].w = r2w;

    data[3].x = r3x;
    data[3].y = r3y;
    data[3].z = r3z;
    data[3].w = r3w;
    
  }

  Vec4& Mat4::operator[](int idx)
  {
    return this->data[idx];
  }

  Mat4 Mat4::operator-(const Mat4& other) const
  {
    auto result = *this;

    result.data[0].x = this->data[0].x - other.data[0].x;
    result.data[0].y = this->data[0].y - other.data[0].y;
    result.data[0].z = this->data[0].z - other.data[0].z;
    result.data[0].w = this->data[0].w - other.data[0].w;

    result.data[1].x = this->data[1].x - other.data[1].x;
    result.data[1].y = this->data[1].y - other.data[1].y;
    result.data[1].z = this->data[1].z - other.data[1].z;
    result.data[1].w = this->data[1].w - other.data[1].w;

    result.data[2].x = this->data[2].x - other.data[2].x;
    result.data[2].y = this->data[2].y - other.data[2].y;
    result.data[2].z = this->data[2].z - other.data[2].z;
    result.data[2].w = this->data[2].w - other.data[2].w;

    result.data[3].x = this->data[3].x - other.data[3].x;
    result.data[3].y = this->data[3].y - other.data[3].y;
    result.data[3].z = this->data[3].z - other.data[3].z;
    result.data[3].w = this->data[3].w - other.data[3].w;

    return result;
  }

  Mat4 Mat4::operator*(const Mat4& other) const
  {
    Vec4 const srcA0 = this->data[0];
    Vec4 const srcA1 = this->data[1];
    Vec4 const srcA2 = this->data[2];
    Vec4 const srcA3 = this->data[3];

    Vec4 const srcB0 = other.data[0];
    Vec4 const srcB1 = other.data[1];
    Vec4 const srcB2 = other.data[2];
    Vec4 const srcB3 = other.data[3];

    Mat4 result;
    result[0] = srcA0 * srcB0.x + srcA1 * srcB0.y + srcA2 * srcB0.z + srcA3 * srcB0.w;
    result[1] = srcA0 * srcB1.x + srcA1 * srcB1.y + srcA2 * srcB1.z + srcA3 * srcB1.w;
    result[2] = srcA0 * srcB2.x + srcA1 * srcB2.y + srcA2 * srcB2.z + srcA3 * srcB2.w;
    result[3] = srcA0 * srcB3.x + srcA1 * srcB3.y + srcA2 * srcB3.z + srcA3 * srcB3.w;
    return result;

  }

  Vec4 Mat4::operator*(const Vec4& vec) const
  {

    Vec4 result;

    result.x = this->data[0].x * vec.x +
      this->data[1].x * vec.y +
      this->data[2].x * vec.z +
      this->data[3].x * vec.w;

    result.y = this->data[0].y * vec.x +
      this->data[1].y * vec.y +
      this->data[2].y * vec.z +
      this->data[3].y * vec.w;

    result.z = this->data[0].z * vec.x +
      this->data[1].z * vec.y +
      this->data[2].z * vec.z +
      this->data[3].z * vec.w;

    result.w = this->data[0].w * vec.x +
      this->data[1].w * vec.y +
      this->data[2].w * vec.z +
      this->data[3].w * vec.w;

    return result;
  }

  Mat4& Mat4::operator*=(const Mat4& other)
  {
    *this = const_cast<Mat4&>(other) * (*this);
    return *this;
  }

  Mat4& Mat4::operator*=(const float v) {
    this->data[0] *= v;
    this->data[1] *= v;
    this->data[2] *= v;
    this->data[3] *= v;
    return *this;
  }

  Mat4 Mat4::operator*(const float v) const {
    Mat4 result = *this;
    result *= v;
    return result;
  }

  Mat4& Mat4::operator/=(const float div)
  {

    this->data[0] /= div;
    this->data[1] /= div;
    this->data[2] /= div;
    this->data[3] /= div;

    return *this;

  }

  Mat4& Mat4::operator=(const Mat4& other)
  {
    this->data[0] = const_cast<Mat4&>(other)[0];
    this->data[1] = const_cast<Mat4&>(other)[1];
    this->data[2] = const_cast<Mat4&>(other)[2];
    this->data[3] = const_cast<Mat4&>(other)[3];
    return *this;
  }

  Mat4 Quat::toMatrix() const
  {
    Mat4 matrix(1.0f - 2 * y * y - 2 * z * z, 2 * x * y + 2 * w * z, 2 * x * z - 2 * w * y, 0.0f,
      2 * x * y - 2 * w * z, 1.0f - 2 * x * x - 2 * z * z, 2 * y * z + 2 * w * x, 0.0f,
      2 * x * z + 2 * w * y, 2 * y * z - 2 * w * x, 1.0f - 2 * x * x - 2 * y * y, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
    return matrix;
  }

  Quat Quat::operator*(const Quat& other)
  {
    Quat result;
    result.w = w * other.w;
    result.x = x * other.x;
    result.y = y * other.y;
    result.z = z * other.z;
    return result;

  }

  Vec3 operator*(const float v, const Vec3& vec)
  {
    Vec3 result = vec;
    result.x *= v;
    result.y *= v;
    result.z *= v;
    return result;

  }

  Mat4 translate(const Mat4& mat, const Vec3& vec)
  {
    Mat4 result = mat;
    result.data[3] = mat.data[0] * vec.x + mat.data[1] * vec.y + mat.data[2] * vec.z + mat.data[3];
    return result;
  }

  Mat4 scale(const Mat4& mat, const Vec3& vec)
  {
    Mat4 result;
    result[0] = mat.data[0] * vec.x;
    result[1] = mat.data[1] * vec.y;
    result[2] = mat.data[2] * vec.z;
    result[3] = mat.data[3];
    return result;
  }

  Mat4 rotate(const Mat4& mat, const float angle, const Vec3& vec)
  {
    float const a = angle;
    float const c = cos(a);
    float const s = sin(a);

    // Normalise
    Vec3 vecmult = vec * vec;
    float vecl = std::sqrt(vecmult.x + vecmult.y + vecmult.z);

    Vec3 axis(vec / vecl);
    Vec3 temp = axis * (1.0f - c);

    Mat4 rotate;
    rotate[0].x = c + temp.x * axis.x;
    rotate[0].y = temp.x * axis.y + s * axis.z;
    rotate[0].z = temp.x * axis.z - s * axis.y;

    rotate[1].x = temp.y * axis.x - s * axis.z;
    rotate[1].y = c + temp.y * axis.y;
    rotate[1].z = temp.y * axis.z + s * axis.x;

    rotate[2].x = temp.z * axis.x + s * axis.y;
    rotate[2].y = temp.z * axis.y - s * axis.x;
    rotate[2].z = c + temp.z * axis.z;

    Mat4 result;
    result[0] = mat.data[0] * rotate[0].x + mat.data[1] * rotate[0].y + mat.data[2] * rotate[0].z;
    result[1] = mat.data[0] * rotate[1].x + mat.data[1] * rotate[1].y + mat.data[2] * rotate[1].z;
    result[2] = mat.data[0] * rotate[2].x + mat.data[1] * rotate[2].y + mat.data[2] * rotate[2].z;
    result[3] = mat.data[3];
    return result;
  }

  Mat4 ortho(float left, float right, float bottom, float top, float zNear, float zFar)
  {
    Mat4 result(1.0f);

    result[0].x = 2.0f / (right - left);
    result[1].y = 2.0f / (top - bottom);
    result[2].z = - 2.0f / (zFar - zNear);
    result[3].x = - (right + left) / (right - left);
    result[3].y = - (top + bottom) / (top - bottom);
    result[3].z = - (zFar + zNear) / (zFar - zNear);
    return result;

  }

  Mat4 perspective(float fovy, float aspect, float zNear, float zFar)
  {
    Mat4 result(1.0f);

    float const tanHalfFovy = tan(fovy / 2.0f);

    result[0].x = 1.0f / (aspect * tanHalfFovy);
    result[1].y = 1.0f / (tanHalfFovy);
    result[2].z = -(zFar + zNear) / (zFar - zNear);
    result[2].w = -1.0f;
    result[3].z = -(2.0f * zFar * zNear) / (zFar - zNear);
    return result;

  }

  Mat4 inverse(const Mat4& mat)
  {

    float coef00 = mat.data[2].z * mat.data[3].w - mat.data[3].z * mat.data[2].w;
    float coef02 = mat.data[1].z * mat.data[3].w - mat.data[3].z * mat.data[1].w;
    float coef03 = mat.data[1].z * mat.data[2].w - mat.data[2].z * mat.data[1].w;

    float coef04 = mat.data[2].y * mat.data[3].w - mat.data[3].y * mat.data[2].w;
    float coef06 = mat.data[1].y * mat.data[3].w - mat.data[3].y * mat.data[1].w;
    float coef07 = mat.data[1].y * mat.data[2].w - mat.data[2].y * mat.data[1].w;

    float coef08 = mat.data[2].y * mat.data[3].z - mat.data[3].y * mat.data[2].z;
    float coef10 = mat.data[1].y * mat.data[3].z - mat.data[3].y * mat.data[1].z;
    float coef11 = mat.data[1].y * mat.data[2].z - mat.data[2].y * mat.data[1].z;

    float coef12 = mat.data[2].x * mat.data[3].w - mat.data[3].x * mat.data[2].w;
    float coef14 = mat.data[1].x * mat.data[3].w - mat.data[3].x * mat.data[1].w;
    float coef15 = mat.data[1].x * mat.data[2].w - mat.data[2].x * mat.data[1].w;

    float coef16 = mat.data[2].x * mat.data[3].z - mat.data[3].x * mat.data[2].z;
    float coef18 = mat.data[1].x * mat.data[3].z - mat.data[3].x * mat.data[1].z;
    float coef19 = mat.data[1].x * mat.data[2].z - mat.data[2].x * mat.data[1].z;

    float coef20 = mat.data[2].x * mat.data[3].y - mat.data[3].x * mat.data[2].y;
    float coef22 = mat.data[1].x * mat.data[3].y - mat.data[3].x * mat.data[1].y;
    float coef23 = mat.data[1].x * mat.data[2].y - mat.data[2].x * mat.data[1].y;

    Vec4 fac0(coef00, coef00, coef02, coef03);
    Vec4 fac1(coef04, coef04, coef06, coef07);
    Vec4 fac2(coef08, coef08, coef10, coef11);
    Vec4 fac3(coef12, coef12, coef14, coef15);
    Vec4 fac4(coef16, coef16, coef18, coef19);
    Vec4 fac5(coef20, coef20, coef22, coef23);

    Vec4 vec0(mat.data[1].x, mat.data[0].x, mat.data[0].x, mat.data[0].x);
    Vec4 vec1(mat.data[1].y, mat.data[0].y, mat.data[0].y, mat.data[0].y);
    Vec4 vec2(mat.data[1].z, mat.data[0].z, mat.data[0].z, mat.data[0].z);
    Vec4 vec3(mat.data[1].w, mat.data[0].w, mat.data[0].w, mat.data[0].w);

    Vec4 inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
    Vec4 inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
    Vec4 inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
    Vec4 inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

    Vec4 signA(+1, -1, +1, -1);
    Vec4 signB(-1, +1, -1, +1);
    Mat4 inverse(inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB);

    Vec4 row0(inverse[0].x, inverse[1].x, inverse[2].x, inverse[3].x);

    Vec4 dot0(mat.data[0] * row0);
    float dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

    float oneDivDeterminant = 1.0f / dot1;

    return inverse * oneDivDeterminant;

  }

  Vec3 clamp(const Vec3& vec, const Vec3& minv, const Vec3& maxv)
  {
    return Vec3(std::max(std::min(vec.x, minv.x), maxv.x),
      std::max(std::min(vec.y, minv.y), maxv.y),
      std::max(std::min(vec.z, minv.z), maxv.z));
  }

  float dot(const Vec3& vec1, const Vec3& vec2)
  {
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
  }

  Vec3 cross(const Vec3& vec1, const Vec3& vec2)
  {
    return Vec3(
      vec1.y * vec2.z - vec2.y * vec1.z,
      vec1.z * vec2.x - vec2.z * vec1.x,
      vec1.x * vec2.y - vec2.x * vec1.y);
  }

  float length(const Vec3& vec)
  {
      return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
  }

  float* Value_ptr(Mat4& mat)
  {
    return reinterpret_cast<float*>(&mat.data);
  }

  float* Value_ptr(Vec3& vec)
  {
    return reinterpret_cast<float*>(&vec);
  }

  float* Value_ptr(Vec4& vec)
  {
    return reinterpret_cast<float*>(&vec);
  }

  Vec3i::Vec3i()
  {

  }

  Vec3i::Vec3i(int x, int y, int z)
  {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Vec4i::Vec4i()
  {
  }

  Vec4i::Vec4i(int x, int y, int z, int w)
  {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }

  Vec4i::Vec4i(const Vec3i& vec3i, float w)
  {
    this->x = vec3i.x;
    this->y = vec3i.y;
    this->z = vec3i.z;
    this->w = w;
  }

}