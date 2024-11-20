/**
 *  @file Math.hpp
 *  @brief Vectors, matrices and other math things
 *
 *  Created on: 2024/11/12
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

namespace small3d {

  struct Vec4;

  /**
   * @class	Vec3
   *
   * @brief	3 component float vector
   */
  struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    Vec3();
    Vec3(float x, float y, float z);
    explicit Vec3(float v);
    explicit Vec3(const Vec4& vec);
    Vec3& operator+=(const Vec3& other);
    Vec3 operator+(const Vec3& other) const;
    Vec3& operator-=(const Vec3& other);
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(const Vec3& other) const;
    Vec3 operator*(const float v) const;
    Vec3 operator/(const float v) const;
    Vec3& operator/=(const float v);
    Vec3& operator=(const Vec3& other);
    Vec3& operator+=(const float v);
    bool operator==(const Vec3& other) const;
    bool operator!=(const Vec3& other) const;
    template <class Archive>
    void serialize(Archive& archive) {
      archive(x, y, z);
    }
  };

  Vec3 operator*(const float v, const Vec3& vec);

  /**
   * @class	Vec3i
   *
   * @brief	3 component integer vector
   */
  struct Vec3i {
    int x = 0;
    int y = 0;
    int z = 0;
    Vec3i();
    Vec3i(int x, int y, int z);
  };

  /**
   * @class	Vec4
   *
   * @brief	4 component vector
   */
  struct Vec4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
    Vec4();
    Vec4(float x, float y, float z, float w);
    Vec4(const Vec3& vec3, float w);
    
    Vec4& operator+=(const Vec4& other);
    Vec4 operator+(const Vec4& other);
    Vec4 operator-(const Vec4& other);
    Vec4& operator*=(const float v);
    Vec4 operator*(const float v) const;
    Vec4 operator*(const Vec4& other) const;
    Vec4& operator/=(const float div);
    float& operator[](int idx);
    Vec4& operator=(const Vec4& other);
    template <class Archive>
    void serialize(Archive& archive) {
      archive(x, y, z, w);
    }
  };

  /**
   * @class	Vec4i
   *
   * @brief	4 component integer vector
   */
  struct Vec4i {
    int x = 0;
    int y = 0;
    int z = 0;
    int w = 0;
    Vec4i();
    Vec4i(int x, int y, int z, int w);
    Vec4i(const Vec3i& vec3i, float w);
  };

  /**
   * @class	Mat4
   *
   * @brief 4x4 float matrix
   */
  struct Mat4 {
    Vec4 data[4];
    Mat4();
    explicit Mat4(float v);
    Mat4(Vec4 r0, Vec4 r1, Vec4 r2, Vec4 r3);
    Mat4(float r0x, float r0y, float r0z, float r0w,
      float r1x, float r1y, float r1z, float r1w, 
      float r2x, float r2y, float r2z, float r2w, 
      float r3x, float r3y, float r3z, float r3w );
    Vec4& operator[](int idx);
    Mat4 operator-(const Mat4& other) const;
    Mat4 operator*(const Mat4& other) const;
    Vec4 operator*(const Vec4& vec) const;
    Mat4& operator*=(const Mat4& other);
    Mat4& operator*=(const float v);
    Mat4 operator*(const float v) const;
    Mat4& operator/=(const float div);
    Mat4& operator=(const Mat4& other);
    template <class Archive>
    void serialize(Archive& archive) {
      archive(data);
    }
  };

  /**
   * @class	Vec3
   *
   * @brief	Quaternion
   */
  struct Quat {

    
    float x;
    float y;
    float z;
    float w;

    Mat4 toMatrix() const;
    Quat operator*(const Quat& other);
    template <class Archive>
    void serialize(Archive& archive) {
      archive(w, x, y, z);
    }
  };

  Mat4 translate(const Mat4& mat, const Vec3& vec);

  Mat4 scale(const Mat4& mat, const Vec3& vec);

  Mat4 rotate(const Mat4& mat, const float angle, const Vec3& vec);

  Mat4 ortho(float left, float right, float bottom, float top, float zNear, float zFar);

  Mat4 perspective(float fovy, float aspect, float zNear, float zFar);

  Mat4 inverse(const Mat4& mat);

  Vec3 clamp(const Vec3& vec, const Vec3& minv, const Vec3& maxv);

  float dot(const Vec3& vec1, const Vec3& vec2);

  Vec3 cross(const Vec3& vec1, const Vec3& vec2);

  float length(const Vec3& vec);

  float* Value_ptr(Mat4& mat);

  float* Value_ptr(Vec3& vec);

  float* Value_ptr(Vec4& vec);

}
