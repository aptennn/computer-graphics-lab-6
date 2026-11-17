#include "mainwindow.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

#include <cmath>

#include "mat4.h"
#include "wireframe_renderer.h"

namespace {
constexpr double kPi = 3.14159265358979323846;

geometry::RegularSolid solidFromIndex(int index) {
    switch (index) {
    case 0: return geometry::RegularSolid::Tetrahedron;
    case 1: return geometry::RegularSolid::Hexahedron;
    case 2: return geometry::RegularSolid::Octahedron;
    case 3: return geometry::RegularSolid::Icosahedron;
    default: return geometry::RegularSolid::Dodecahedron;
    }
}
} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);

    auto* panel = new QWidget(central);
    auto* panelLayout = new QVBoxLayout(panel);
    panel->setFixedWidth(320);

    // Выбор многогранника
    auto* solidGroup = new QGroupBox("Многогранник", panel);
    auto* solidForm = new QFormLayout(solidGroup);
    solidBox_ = new QComboBox(solidGroup);
    solidBox_->addItems({"Тетраэдр", "Гексаэдр (куб)", "Октаэдр",
                         "Икосаэдр", "Додекаэдр"});
    solidForm->addRow("Тип:", solidBox_);
    edgeBox_ = new QDoubleSpinBox(solidGroup);
    edgeBox_->setRange(0.1, 10.0);
    edgeBox_->setValue(edgeLength_);
    edgeBox_->setDecimals(2);
    solidForm->addRow("Длина ребра:", edgeBox_);
    auto* rebuildBtn = new QPushButton("Построить", solidGroup);
    solidForm->addRow(rebuildBtn);
    panelLayout->addWidget(solidGroup);

    // Смещение
    auto* transGroup = new QGroupBox("Смещение", panel);
    auto* transForm = new QFormLayout(transGroup);
    tx_ = new QDoubleSpinBox(transGroup); tx_->setRange(-100, 100); tx_->setDecimals(2);
    ty_ = new QDoubleSpinBox(transGroup); ty_->setRange(-100, 100); ty_->setDecimals(2);
    tz_ = new QDoubleSpinBox(transGroup); tz_->setRange(-100, 100); tz_->setDecimals(2);
    transForm->addRow("dx:", tx_);
    transForm->addRow("dy:", ty_);
    transForm->addRow("dz:", tz_);
    auto* transBtn = new QPushButton("Применить", transGroup);
    transForm->addRow(transBtn);
    panelLayout->addWidget(transGroup);

    // Поворот
    auto* rotGroup = new QGroupBox("Поворот", panel);
    auto* rotForm = new QFormLayout(rotGroup);
    rotAxis_ = new QComboBox(rotGroup);
    rotAxis_->addItems({"X", "Y", "Z"});
    rotForm->addRow("Ось:", rotAxis_);
    rotAngle_ = new QDoubleSpinBox(rotGroup);
    rotAngle_->setRange(-360, 360);
    rotAngle_->setValue(45);
    rotAngle_->setSuffix("°");
    rotForm->addRow("Угол:", rotAngle_);
    auto* rotBtn = new QPushButton("Применить", rotGroup);
    rotForm->addRow(rotBtn);
    panelLayout->addWidget(rotGroup);

    // Масштаб
    auto* scaleGroup = new QGroupBox("Масштаб", panel);
    auto* scaleForm = new QFormLayout(scaleGroup);
    sx_ = new QDoubleSpinBox(scaleGroup); sx_->setRange(-10, 10); sx_->setValue(1.0); sx_->setDecimals(2);
    sy_ = new QDoubleSpinBox(scaleGroup); sy_->setRange(-10, 10); sy_->setValue(1.0); sy_->setDecimals(2);
    sz_ = new QDoubleSpinBox(scaleGroup); sz_->setRange(-10, 10); sz_->setValue(1.0); sz_->setDecimals(2);
    scaleForm->addRow("sx:", sx_);
    scaleForm->addRow("sy:", sy_);
    scaleForm->addRow("sz:", sz_);
    auto* scaleBtn = new QPushButton("Относительно начала координат", scaleGroup);
    auto* scaleCenterBtn = new QPushButton("Относительно центра", scaleGroup);
    scaleForm->addRow(scaleBtn);
    scaleForm->addRow(scaleCenterBtn);
    panelLayout->addWidget(scaleGroup);

    // Отражение
    auto* reflGroup = new QGroupBox("Отражение", panel);
    auto* reflForm = new QFormLayout(reflGroup);
    reflectPlane_ = new QComboBox(reflGroup);
    reflectPlane_->addItems({"Плоскость XY (z→-z)", "Плоскость XZ (y→-y)",
                             "Плоскость YZ (x→-x)"});
    reflForm->addRow("Плоскость:", reflectPlane_);
    auto* reflBtn = new QPushButton("Отразить", reflGroup);
    reflForm->addRow(reflBtn);
    panelLayout->addWidget(reflGroup);

    auto* resetBtn = new QPushButton("Сбросить модель", panel);
    panelLayout->addWidget(resetBtn);
    panelLayout->addStretch();

    // Область отрисовки
    auto* canvas = new QLabel(central);
    canvas->setMinimumSize(640, 480);
    canvas->setAlignment(Qt::AlignCenter);
    canvas->setStyleSheet("background: white;");

    layout->addWidget(panel);
    layout->addWidget(canvas, 1);
    setCentralWidget(central);
    resize(1000, 640);

    // Подключения
    connect(rebuildBtn, &QPushButton::clicked, this, &MainWindow::rebuildModel);
    connect(transBtn,   &QPushButton::clicked, this, &MainWindow::applyTranslate);
    connect(rotBtn,     &QPushButton::clicked, this, &MainWindow::applyRotate);
    connect(scaleBtn,   &QPushButton::clicked, this, &MainWindow::applyScale);
    connect(scaleCenterBtn, &QPushButton::clicked, this, &MainWindow::applyScaleAroundCenter);
    connect(reflBtn,    &QPushButton::clicked, this, &MainWindow::applyReflect);
    connect(resetBtn,   &QPushButton::clicked, this, &MainWindow::resetModel);

    rebuildModel();
}

void MainWindow::rebuildModel() {
    edgeLength_ = edgeBox_->value();
    model_ = geometry::makeRegularSolid(solidFromIndex(solidBox_->currentIndex()), edgeLength_);
    update();
}

void MainWindow::applyTranslate() {
    model_.translate(tx_->value(), ty_->value(), tz_->value());
    update();
}

void MainWindow::applyRotate() {
    const double rad = rotAngle_->value() * kPi / 180.0;
    const char axis = (rotAxis_->currentIndex() == 0) ? 'x'
                    : (rotAxis_->currentIndex() == 1) ? 'y' : 'z';
    model_.rotate(rad, axis);
    update();
}

void MainWindow::applyScale() {
    model_.scale(sx_->value(), sy_->value(), sz_->value());
    update();
}

void MainWindow::applyScaleAroundCenter() {
    model_.scaleAroundCenter(sx_->value(), sy_->value(), sz_->value());
    update();
}

void MainWindow::applyReflect() {
    switch (reflectPlane_->currentIndex()) {
    case 0: model_.reflect(geometry::Mat4::reflectionXY()); break;
    case 1: model_.reflect(geometry::Mat4::reflectionXZ()); break;
    default: model_.reflect(geometry::Mat4::reflectionYZ()); break;
    }
    update();
}

void MainWindow::resetModel() { rebuildModel(); }

// Простая аксонометрическая (изометрическая) проекция.
std::vector<QPointF> MainWindow::projectToScreen() const {
    const double w = width() * 0.62;   
    const double h = height();
    const double cx = w * 0.5;
    const double cy = h * 0.5;

    const double scale = 80.0 / edgeLength_;
    const double a = kPi / 6.0;        
    const double ca = std::cos(a), sa = std::sin(a);

    std::vector<QPointF> result;
    result.reserve(model_.vertices().size());
    for (const geometry::Point& p : model_.vertices()) {
        const double sx = (p.x - p.z) * ca;
        const double sy = (p.x + p.z) * sa - p.y;
        result.emplace_back(cx + sx * scale, cy + sy * scale);
    }
    return result;
}

void MainWindow::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    const auto screen = projectToScreen();
    painter.setPen(QPen(Qt::black, 1.5));
    geometry::WireframeRenderer::draw(painter, model_, screen);

    painter.setPen(Qt::darkGray);
    painter.drawText(10, height() - 10, "Каркасный режим");
}