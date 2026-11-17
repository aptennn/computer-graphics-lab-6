#pragma once

#include <QMainWindow>
#include <QPointF>
#include <vector>

#include "geometry.h"

class QComboBox;
class QDoubleSpinBox;
class QLabel;

// главное окно — управление моделью и её аффинными преобразованиями.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void rebuildModel();     
    void applyTranslate();
    void applyRotate();
    void applyScale();
    void applyScaleAroundCenter();
    void applyReflect();
    void resetModel();

private:
    std::vector<QPointF> projectToScreen() const;

    // Модель
    geometry::Polyhedron model_;
    double edgeLength_ = 2.0;

    // Виджеты
    QComboBox* solidBox_ = nullptr;
    QDoubleSpinBox* edgeBox_ = nullptr;

    QDoubleSpinBox* tx_ = nullptr; QDoubleSpinBox* ty_ = nullptr; QDoubleSpinBox* tz_ = nullptr;

    QDoubleSpinBox* rotAngle_ = nullptr;
    QComboBox* rotAxis_ = nullptr;

    QDoubleSpinBox* sx_ = nullptr; QDoubleSpinBox* sy_ = nullptr; QDoubleSpinBox* sz_ = nullptr;

    QComboBox* reflectPlane_ = nullptr;
};