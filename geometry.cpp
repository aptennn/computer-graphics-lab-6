#include "geometry.h"

#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace geometry {

// Егор: проверка данных модели при создании многогранника.
Polyhedron::Polyhedron(std::vector<Point> vertices, std::vector<Polygon> faces)
    : vertices_(std::move(vertices)), faces_(std::move(faces)) {
    if (vertices_.empty()) {
        throw std::invalid_argument("A polyhedron needs vertices");
    }
    for (const Point& vertex : vertices_) {
        if (!std::isfinite(vertex.x) || !std::isfinite(vertex.y) || !std::isfinite(vertex.z)) {
            throw std::invalid_argument("Vertex coordinates must be finite");
        }
    }
    for (const Polygon& face : faces_) {
        if (face.vertexIndices.size() < 3) {
            throw std::invalid_argument("A face needs at least three vertices");
        }
        std::unordered_set<std::size_t> uniqueIndices;
        for (std::size_t index : face.vertexIndices) {
            if (index >= vertices_.size() || !uniqueIndices.insert(index).second) {
                throw std::invalid_argument("A face has an invalid or repeated vertex index");
            }
        }
    }
}

// Егор: вычисление центра по координатам всех вершин.
Point Polyhedron::center() const noexcept {
    Point result;
    for (const Point& vertex : vertices_) {
        result.x += vertex.x;
        result.y += vertex.y;
        result.z += vertex.z;
    }
    const double count = static_cast<double>(vertices_.size());
    result.x /= count;
    result.y /= count;
    result.z /= count;
    return result;
}

// Егор: координаты и грани тетраэдра, куба и октаэдра.
Polyhedron makeRegularSolid(RegularSolid solid, double edgeLength) {
    if (!std::isfinite(edgeLength) || edgeLength <= 0.0) {
        throw std::invalid_argument("Edge length must be finite and positive");
    }

    switch (solid) {
    case RegularSolid::Tetrahedron: {
        const double a = edgeLength / (2.0 * std::sqrt(2.0));
        return {
            {{ a,  a,  a}, {-a, -a,  a}, {-a,  a, -a}, { a, -a, -a}},
            {{{0, 2, 1}}, {{0, 1, 3}}, {{0, 3, 2}}, {{1, 2, 3}}}
        };
    }
    case RegularSolid::Hexahedron: {
        const double a = edgeLength / 2.0;
        return {
            {{-a, -a, -a}, { a, -a, -a}, { a,  a, -a}, {-a,  a, -a},
             {-a, -a,  a}, { a, -a,  a}, { a,  a,  a}, {-a,  a,  a}},
            {{{0, 3, 2, 1}}, {{4, 5, 6, 7}}, {{0, 1, 5, 4}},
             {{3, 7, 6, 2}}, {{0, 4, 7, 3}}, {{1, 2, 6, 5}}}
        };
    }
    case RegularSolid::Octahedron: {
        const double a = edgeLength / std::sqrt(2.0);
        return {
            {{ a, 0, 0}, {-a, 0, 0}, {0,  a, 0}, {0, -a, 0},
             {0, 0,  a}, {0, 0, -a}},
            {{{4, 0, 2}}, {{4, 2, 1}}, {{4, 1, 3}}, {{4, 3, 0}},
             {{5, 2, 0}}, {{5, 1, 2}}, {{5, 3, 1}}, {{5, 0, 3}}}
        };
    }
    }
    throw std::invalid_argument("Unknown regular solid");
}

} // namespace geometry
