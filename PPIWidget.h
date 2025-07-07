#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <vector>
#include "DataStructures.h"

class PPIWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PPIWidget(QWidget *parent = nullptr);
    
    void updateTargets(const TargetTrackData& trackData);
    void setMaxRange(float range);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawBackground(QPainter& painter);
    void drawRangeRings(QPainter& painter);
    void drawAzimuthLines(QPainter& painter);
    void drawTargets(QPainter& painter);
    void drawLabels(QPainter& painter);
    
    QColor getTargetColor(float radialSpeed) const;
    QPointF polarToCartesian(float range, float azimuth) const;
    
    TargetTrackData m_currentTargets;
    float m_maxRange;
    QRect m_plotRect;
    QPointF m_center;
    float m_plotRadius;
    
    // Visual settings
    static constexpr int NUM_RANGE_RINGS = 5;
    static constexpr int NUM_AZIMUTH_LINES = 9; // -90, -60, -30, 0, 30, 60, 90
    static constexpr float MIN_AZIMUTH = -90.0f;
    static constexpr float MAX_AZIMUTH = 90.0f;
};