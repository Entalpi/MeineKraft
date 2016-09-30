#ifndef MEINEKRAFT_VECTOR_H
#define MEINEKRAFT_VECTOR_H

#include <SDL2/SDL_opengl.h>
#include <math.h>
#include <ostream>
#include <cmath>

template<typename T>
struct Vec4 {
    T values[4];

    Vec4(T x, T y, T z, T w) { values[0] = x; values[1] = y; values[2] = z; values[3] = w; };
    Vec4() { values[0] = 0.0; values[1] = 0.0; values[2] = 0.0; values[3] = 0.0; };

    // Operators
    T& operator[] (const int index) {
        return values[index];
    }

    bool operator==(const Vec4 &rhs) {
        return (values[0] == rhs.values[0]) && (values[1] == rhs.values[1]) && (values[2] == rhs.values[2]) && (values[3] == rhs.values[3]);
    }

    friend std::ostream &operator<<(std::ostream &os, const Vec4 &vec) {
        return os << "(x:" << vec.values[0] << " y:" << vec.values[1] << " z:" << vec.values[2] << " w:" << vec.values[3];
    }
};

/// Defaults to Vec3<GLfloat>
template<typename T = GLfloat>
struct Vec3 {
    T x, y, z;
    Vec3(T x, T y, T z): x(x), y(y), z(z) {};
    Vec3(): x(0), y(0), z(0) {};
    inline static Vec3 ZERO() { return Vec3(0.0, 0.0, 0.0); }
    inline double length() const { return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2)); }

    inline Vec3<T> normalize() const {
        double length = this->length();
        Vec3 result;
        result.x = x / length;
        result.y = y / length;
        result.z = z / length;
        return result;
    }

    /// Result = v x u
    inline Vec3<T> cross(Vec3<T> u) const {
        Vec3<T> result;
        result.x = y * u.z - z * u.y;
        result.y = z * u.x - x * u.z;
        result.z = x * u.y - y * u.x;
        return result;
    }

    inline T dot(Vec3<T> u) const { return x * u.x + y * u.y + z * u.z; }

    /// Operators
    inline Vec3<T> operator+(const Vec3 &rhs) const {
        return Vec3<T>{x + rhs.x, y + rhs.y, z + rhs.z};
    }

    inline Vec3<T> operator*(const T s) const {
        return Vec3<T>{x * s, y * s, z * s};
    }

    inline bool operator==(const Vec3 &rhs) const {
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }

    friend std::ostream &operator<<(std::ostream& os, const Vec3 &vec) {
        return os << "(x:" << vec.x << " y:" << vec.y << " z:" << vec.z << ")";
    }

    inline Vec3 operator-(const Vec3 &rhs) const {
        return Vec3{x - rhs.x, y - rhs.y, z - rhs.z};
    }
};

template<typename T>
struct Vec2 {
    T x, y = 0.0f;
    Vec2(T x, T y): x(x), y(y) {};
    Vec2(): x(0), y(0) {};

    inline Vec2<T> normalize() {
        T length = sqrt(pow(x, 2) + pow(y, 2));
        Vec2 result;
        result.x = x / length;
        result.y = y / length;
        return result;
    }

    /// Dot product
    inline T dot(Vec2<T> u) const { return x * u.x + y * u.y; }
};

template<typename T>
struct Mat4 {
private:
    Vec4<T> rows[4];
    static constexpr float RADIAN = M_PI / 180;

public:
    inline T *data() {
        return &rows[0][0];
    }

    /// Identity matrix by default
    Mat4<T>() {
        rows[0] = Vec4<T>{1.0f, 0.0f, 0.0f, 0.0f};
        rows[1] = Vec4<T>{0.0f, 1.0f, 0.0f, 0.0f};
        rows[2] = Vec4<T>{0.0f, 0.0f, 1.0f, 0.0f};
        rows[3] = Vec4<T>{0.0f, 0.0f, 0.0f, 1.0f};
    }

    /// Translation - moves the matrix projection in space ...
    inline Mat4<T> translate(Vec3<T> vec) const {
        Mat4<T> matrix;
        matrix[0] = {1.0f, 0.0f, 0.0f, vec.x};
        matrix[1] = {0.0f, 1.0f, 0.0f, vec.y};
        matrix[2] = {0.0f, 0.0f, 1.0f, vec.z};
        matrix[3] = {0.0f, 0.0f, 0.0f, 1.0f};
        return *this * matrix;
    }

    /// Scales the matrix the same over all axis except w
    inline Mat4<T> scale(T scale) const {
        Mat4<T> matrix;
        matrix[0] = {scale, 0.0f, 0.0f, 0.0f};
        matrix[1] = {0.0f, scale, 0.0f, 0.0f};
        matrix[2] = {0.0f, 0.0f, scale, 0.0f};
        matrix[3] = {0.0f, 0.0f, 0.0f, 1.0f};
        return *this * matrix;
    }

    /// Transposes the current matrix and returns that matrix
    inline Mat4<T> transpose() {
        Mat4<T> mat;
        mat[0][0] = rows[0][0];
        mat[1][1] = rows[1][1];
        mat[2][2] = rows[2][2];
        mat[3][3] = rows[3][3];

        mat[1][0] = rows[0][1];
        mat[2][0] = rows[0][2];
        mat[3][0] = rows[0][3];
        mat[0][1] = rows[1][0];
        mat[0][2] = rows[2][0];
        mat[0][3] = rows[3][0];

        mat[2][1] = rows[1][2];
        mat[3][1] = rows[1][3];
        mat[1][2] = rows[2][1];
        mat[1][3] = rows[3][1];

        mat[2][3] = rows[3][2];
        mat[3][2] = rows[2][3];
        return mat;
    }

    /// Rotation matrix around x-axis (counterclockwise)
    inline Mat4<T> rotate_x(float theta) const {
        Mat4<T> matrix;
        matrix[0] = {1.0f, 0.0f, 0.0f, 0.0f};
        matrix[1] = {0.0f, cosf(theta * RADIAN), -sinf(theta * RADIAN), 0.0f};
        matrix[2] = {0.0f, sinf(theta * RADIAN), cosf(theta * RADIAN), 0.0f};
        matrix[3] = {0.0f, 0.0f, 0.0f, 1.0f};
        return *this * matrix;
    }

    /// Rotation matrix around y-axis (counterclockwise)
    inline Mat4<T> rotate_y(float theta) const {
        Mat4<T> matrix;
        matrix[0] = {cosf(theta * RADIAN), 0.0f, sinf(theta * RADIAN), 0.0f};
        matrix[1] = {0.0f, 1.0f, 0.0f, 0.0f};
        matrix[2] = {-sinf(theta * RADIAN), 0.0f, cosf(theta * RADIAN), 0.0f};
        matrix[3] = {0.0f, 0.0f, 0.0f, 1.0f};
        return *this * matrix;
    }

    /// Rotation matrix around z-axis (counterclockwise)
    inline Mat4<T> rotate_z(float theta) const {
        Mat4<T> matrix;
        matrix[0] = {cosf(theta * RADIAN), -sinf(theta * RADIAN), 0.0f, 0.0f};
        matrix[1] = {sinf(theta * RADIAN), cosf(theta * RADIAN), 0.0f, 0.0f};
        matrix[2] = {0.0f, 0.0f, 1.0f, 0.0f};
        matrix[3] = {0.0f, 0.0f, 0.0f, 1.0f};
        return *this * matrix;
    }

    // Operators
    /// Standard matrix multiplication row-column wise; *this * mat
    inline Mat4<T> operator*(Mat4<T> mat) const {
        Mat4<T> matrix;
        for (int i = 0; i < 4; ++i) {
            auto row = rows[i];
            for (int j = 0; j < 4; ++j) {
                Vec4<T> column = Vec4<T>{mat[0][j], mat[1][j], mat[2][j], mat[3][j]};
                matrix.rows[i][j] = row[0]*column[0] + row[1]*column[1] + row[2]*column[2] + row[3]*column[3];
            }
        }
        return matrix;
    }

    /// TODO: Refactor into Mat3<> ...
    /// A * v, where v = (rhs, 1.0), v is a Vec4 with w set to 1.0
    inline Vec3<T> operator*(Vec3<T> rhs) const {
        auto vec = Vec4<T>{rhs.x, rhs.y, rhs.z, 1.0};
        auto result = *this * vec;
        return Vec3<>{result.values[0], result.values[1], result.values[2]};
    }

    /// A * v = b
    inline Vec4<T> operator*(Vec4<T> rhs) const {
        Vec4<T> result;
        for (int i = 0; i < 4; i++) {
            result.values[i] = rows[i].values[0] * rhs.values[0] + rows[i].values[1] * rhs.values[1] + rows[i].values[2] * rhs.values[2] + rows[i].values[3] * rhs.values[3];
        }
        return result;
    }

    /// matrix[row_i][colum_j]
    Vec4<T> &operator[](const int index) {
        return rows[index];
    }

    friend std::ostream &operator<<(std::ostream& os, const Mat4 &mat) {
        return os << "\n { \n" << mat.rows[0] << "), \n" << mat.rows[1] << "), \n" << mat.rows[2] << "), \n" << mat.rows[3] << ")\n }";
    }
};

#endif //MEINEKRAFT_VECTOR_H
