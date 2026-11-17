#include "wireframe_renderer.h"

#include <QPainter>

#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
#include <utility>

namespace geometry {

// Егор: обход граней, удаление повторяющихся ребер и вывод линий на экран.
void WireframeRenderer::draw(QPainter& painter, const Polyhedron& polyhedron,
                             const std::vector<QPointF>& screenVertices) {
    if (screenVertices.size() != polyhedron.vertices().size()) {
        throw std::invalid_argument("Every polyhedron vertex needs a screen position");
    }

    std::set<std::pair<std::size_t, std::size_t>> drawnEdges;
    for (const Polygon& face : polyhedron.faces()) {
        const auto& indices = face.vertexIndices;
        for (std::size_t i = 0; i < indices.size(); ++i) {
            const auto edge = std::minmax(indices[i], indices[(i + 1) % indices.size()]);
            if (!drawnEdges.insert(edge).second) {
                continue;
            }
            const QPointF& first = screenVertices[edge.first];
            const QPointF& second = screenVertices[edge.second];
            // Проекция может пометить точку вне области видимости бесконечной координатой.
            if (std::isfinite(first.x()) && std::isfinite(first.y()) &&
                std::isfinite(second.x()) && std::isfinite(second.y())) {
                painter.drawLine(first, second);
            }
        }
    }
}

} // namespace geometry
