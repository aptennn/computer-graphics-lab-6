#include "geometry.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace geometry {

namespace {

Point subtract(Point a, Point b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Point cross(Point a, Point b) {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

double dot(Point a, Point b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Point scaled(Point p, double factor) {
    return {p.x * factor, p.y * factor, p.z * factor};
}

// Егор: вершины икосаэдра задаются через золотое сечение; треугольные
// грани выбираются по равным ребрам и ориентируются наружу.
Polyhedron makeIcosahedron(double edgeLength) {
    const double phi = (1.0 + std::sqrt(5.0)) / 2.0;
    std::vector<Point> vertices = {
        {0, -1, -phi}, {0, -1, phi}, {0, 1, -phi}, {0, 1, phi},
        {-1, -phi, 0}, {-1, phi, 0}, {1, -phi, 0}, {1, phi, 0},
        {-phi, 0, -1}, {-phi, 0, 1}, {phi, 0, -1}, {phi, 0, 1}
    };

    std::vector<Polygon> faces;
    const auto isEdge = [&](std::size_t i, std::size_t j) {
        const Point delta = subtract(vertices[i], vertices[j]);
        return std::abs(dot(delta, delta) - 4.0) < 1e-10;
    };
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        for (std::size_t j = i + 1; j < vertices.size(); ++j) {
            for (std::size_t k = j + 1; k < vertices.size(); ++k) {
                if (!isEdge(i, j) || !isEdge(j, k) || !isEdge(k, i)) {
                    continue;
                }
                if (dot(cross(subtract(vertices[j], vertices[i]),
                              subtract(vertices[k], vertices[i])), vertices[i]) < 0.0) {
                    faces.push_back({{i, k, j}});
                } else {
                    faces.push_back({{i, j, k}});
                }
            }
        }
    }

    for (Point& vertex : vertices) {
        vertex = scaled(vertex, edgeLength / 2.0);
    }
    return {std::move(vertices), std::move(faces)};
}

// Егор: додекаэдр строится как двойственный икосаэдру: центры его
// треугольных граней становятся вершинами пятиугольников.
Polyhedron makeDodecahedron(double edgeLength) {
    const Polyhedron icosahedron = makeIcosahedron(2.0);
    std::vector<Point> vertices;
    vertices.reserve(icosahedron.faces().size());
    for (const Polygon& face : icosahedron.faces()) {
        Point center;
        for (std::size_t index : face.vertexIndices) {
            const Point& p = icosahedron.vertices()[index];
            center.x += p.x;
            center.y += p.y;
            center.z += p.z;
        }
        vertices.push_back(scaled(center, 1.0 / 3.0));
    }

    std::vector<Polygon> faces;
    faces.reserve(icosahedron.vertices().size());
    for (std::size_t i = 0; i < icosahedron.vertices().size(); ++i) {
        std::vector<std::size_t> adjacent;
        for (std::size_t j = 0; j < icosahedron.faces().size(); ++j) {
            const auto& indices = icosahedron.faces()[j].vertexIndices;
            if (std::find(indices.begin(), indices.end(), i) != indices.end()) {
                adjacent.push_back(j);
            }
        }

        const Point axis = icosahedron.vertices()[i];
        const Point reference = vertices[adjacent.front()];
        const Point tangent = subtract(reference, scaled(axis, dot(reference, axis) / dot(axis, axis)));
        const Point bitangent = cross(axis, tangent);
        std::sort(adjacent.begin(), adjacent.end(), [&](std::size_t a, std::size_t b) {
            return std::atan2(dot(vertices[a], bitangent), dot(vertices[a], tangent)) <
                   std::atan2(dot(vertices[b], bitangent), dot(vertices[b], tangent));
        });
        faces.push_back({std::move(adjacent)});
    }

    const auto& firstFace = faces.front().vertexIndices;
    const Point edge = subtract(vertices[firstFace[0]], vertices[firstFace[1]]);
    const double scale = edgeLength / std::sqrt(dot(edge, edge));
    for (Point& vertex : vertices) {
        vertex = scaled(vertex, scale);
    }
    return {std::move(vertices), std::move(faces)};
}

} // namespace

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

// Егор: координаты и грани трех обязательных и двух бонусных тел.
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
    case RegularSolid::Icosahedron:
        return makeIcosahedron(edgeLength);
    case RegularSolid::Dodecahedron:
        return makeDodecahedron(edgeLength);
    }
    throw std::invalid_argument("Unknown regular solid");
}

} // namespace geometry
