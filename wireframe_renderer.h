#pragma once

#include "geometry.h"

#include <QPointF>

#include <vector>

class QPainter;

namespace geometry {

// Егор: модуль каркасной отрисовки готового многогранника.
class WireframeRenderer {
public:
    // Каждое ребро рисуется один раз текущим пером QPainter.
    // screenVertices[i] — экранная проекция polyhedron.vertices()[i].
    static void draw(QPainter& painter, const Polyhedron& polyhedron,
                     const std::vector<QPointF>& screenVertices);
};

} // namespace geometry
