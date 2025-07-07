#include "PPIWidget.h"
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFont>
#include <QFontMetrics>
#include <cmath>

PPIWidget::PPIWidget(QWidget *parent)
    : QWidget(parent)
    , m_maxRange(10000.0f) // 10km default
    , m_plotRadius(0)
{
    setMinimumSize(400, 200);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void PPIWidget::updateTargets(const TargetTrackData& trackData)
{
    m_currentTargets = trackData;
    update();
}

void PPIWidget::setMaxRange(float range)
{
    if (range > 0) {
        m_maxRange = range;
        update();
    }
}

void PPIWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // Calculate plot area (semi-circle)
    int margin = 40;
    int availableWidth = width() - 2 * margin;
    int availableHeight = height() - 2 * margin;
    
    // For semi-circle, height should be at least half of width
    int diameter = std::min(availableWidth, availableHeight * 2);
    m_plotRadius = diameter / 2.0f;
    
    m_plotRect = QRect(
        (width() - diameter) / 2,
        height() - margin - m_plotRadius,
        diameter,
        m_plotRadius
    );
    
    m_center = QPointF(width() / 2.0f, height() - margin);
}

void PPIWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter);
    drawRangeRings(painter);
    drawAzimuthLines(painter);
    drawTargets(painter);
    drawLabels(painter);
}

void PPIWidget::drawBackground(QPainter& painter)
{
    // Fill background
    painter.fillRect(rect(), QColor(20, 20, 20));
    
    // Draw semi-circular plot area
    painter.setBrush(QColor(0, 0, 0));
    painter.setPen(QPen(QColor(100, 100, 100), 2));
    
    QRectF ellipseRect(m_center.x() - m_plotRadius, m_center.y() - m_plotRadius,
                       2 * m_plotRadius, 2 * m_plotRadius);
    painter.drawChord(ellipseRect, 0, 180 * 16); // Draw upper semi-circle
}

void PPIWidget::drawRangeRings(QPainter& painter)
{
    painter.setPen(QPen(QColor(100, 100, 100), 1));
    
    for (int i = 1; i <= NUM_RANGE_RINGS; ++i) {
        float radius = (float(i) / NUM_RANGE_RINGS) * m_plotRadius;
        QRectF ellipseRect(m_center.x() - radius, m_center.y() - radius,
                           2 * radius, 2 * radius);
        painter.drawChord(ellipseRect, 0, 180 * 16);
    }
}

void PPIWidget::drawAzimuthLines(QPainter& painter)
{
    painter.setPen(QPen(QColor(100, 100, 100), 1));
    
    for (int i = 0; i < NUM_AZIMUTH_LINES; ++i) {
        float azimuth = MIN_AZIMUTH + (float(i) / (NUM_AZIMUTH_LINES - 1)) * (MAX_AZIMUTH - MIN_AZIMUTH);
        float radians = qDegreesToRadians(90.0f - azimuth); // Convert to math coordinates
        
        QPointF endPoint(
            m_center.x() + m_plotRadius * std::cos(radians),
            m_center.y() - m_plotRadius * std::sin(radians)
        );
        
        painter.drawLine(m_center, endPoint);
    }
}

void PPIWidget::drawTargets(QPainter& painter)
{
    for (const auto& target : m_currentTargets.targets) {
        // Skip targets outside our azimuth range
        if (target.azimuth < MIN_AZIMUTH || target.azimuth > MAX_AZIMUTH) {
            continue;
        }
        
        // Skip targets outside our range
        if (target.radius > m_maxRange) {
            continue;
        }
        
        QPointF targetPos = polarToCartesian(target.radius, target.azimuth);
        QColor targetColor = getTargetColor(target.radial_speed);
        
        // Draw target as a circle
        painter.setBrush(targetColor);
        painter.setPen(QPen(targetColor.lighter(), 2));
        
        float targetSize = 6 + target.level * 0.1f; // Size based on level
        painter.drawEllipse(targetPos, targetSize, targetSize);
        
        // Draw target ID
        painter.setPen(QPen(Qt::white, 1));
        painter.setFont(QFont("Arial", 8));
        QPointF textPos = targetPos + QPointF(8, -8);
        painter.drawText(textPos, QString::number(target.target_id));
    }
}

void PPIWidget::drawLabels(QPainter& painter)
{
    painter.setPen(QPen(Qt::white, 1));
    painter.setFont(QFont("Arial", 10));
    
    // Range labels
    for (int i = 1; i <= NUM_RANGE_RINGS; ++i) {
        float range = (float(i) / NUM_RANGE_RINGS) * m_maxRange;
        float radius = (float(i) / NUM_RANGE_RINGS) * m_plotRadius;
        
        QPointF labelPos(m_center.x() + radius * 0.707f, m_center.y() - radius * 0.707f);
        QString rangeText = QString("%1").arg(range / 1000.0f, 0, 'f', 1) + "km";
        painter.drawText(labelPos, rangeText);
    }
    
    // Azimuth labels
    QFont azFont("Arial", 12, QFont::Bold);
    painter.setFont(azFont);
    
    for (int i = 0; i < NUM_AZIMUTH_LINES; i += 2) { // Label every other line
        float azimuth = MIN_AZIMUTH + (float(i) / (NUM_AZIMUTH_LINES - 1)) * (MAX_AZIMUTH - MIN_AZIMUTH);
        float radians = qDegreesToRadians(90.0f - azimuth);
        
        QPointF labelPos(
            m_center.x() + (m_plotRadius + 20) * std::cos(radians),
            m_center.y() - (m_plotRadius + 20) * std::sin(radians)
        );
        
        QString azText = QString("%1°").arg(azimuth, 0, 'f', 0);
        QFontMetrics fm(azFont);
        QRect textRect = fm.boundingRect(azText);
        labelPos -= QPointF(textRect.width() / 2, -textRect.height() / 2);
        
        painter.drawText(labelPos, azText);
    }
    
    // Title
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    painter.drawText(QPointF(10, 25), "PPI Display - Target Tracks");
}

QColor PPIWidget::getTargetColor(float radialSpeed) const
{
    // Color coding based on radial speed
    // Red for approaching (positive speed)
    // Blue for receding (negative speed)
    // Green for stationary
    
    if (std::abs(radialSpeed) < 1.0f) {
        return QColor(0, 255, 0); // Green for stationary
    } else if (radialSpeed > 0) {
        // Approaching - red (intensity based on speed)
        int intensity = std::min(255, static_cast<int>(50 + std::abs(radialSpeed) * 10));
        return QColor(intensity, 0, 0);
    } else {
        // Receding - blue (intensity based on speed)
        int intensity = std::min(255, static_cast<int>(50 + std::abs(radialSpeed) * 10));
        return QColor(0, 0, intensity);
    }
}

QPointF PPIWidget::polarToCartesian(float range, float azimuth) const
{
    // Normalize range to plot radius
    float normalizedRange = (range / m_maxRange) * m_plotRadius;
    
    // Convert azimuth to radians (0° is up/north, positive is clockwise)
    float radians = qDegreesToRadians(90.0f - azimuth);
    
    return QPointF(
        m_center.x() + normalizedRange * std::cos(radians),
        m_center.y() - normalizedRange * std::sin(radians)
    );
}