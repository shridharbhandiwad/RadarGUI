#include "FFTWidget.h"
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFont>
#include <QFontMetrics>
#include <cmath>
#include <algorithm>

FFTWidget::FFTWidget(QWidget *parent)
    : QWidget(parent)
    , m_maxMagnitude(0.0f)
    , m_margin(50)
{
    setMinimumSize(400, 300);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void FFTWidget::updateData(const RawADCFrameTest& adcFrame)
{
    m_currentFrame = adcFrame;
    if (!adcFrame.sample_data.empty()) {
        performFFT(adcFrame.sample_data);
    }
    update();
}

void FFTWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    m_plotRect = QRect(
        m_margin,
        m_margin,
        width() - 2 * m_margin,
        height() - 2 * m_margin
    );
}

void FFTWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawBackground(painter);
    drawGrid(painter);
    drawSpectrum(painter);
    drawLabels(painter);
}

void FFTWidget::performFFT(const std::vector<float>& input)
{
    if (input.empty()) return;

    size_t n = 1;
    while (n < input.size()) {
        n *= 2;
    }

    std::vector<std::complex<float>> complexData(n);
    for (size_t i = 0; i < input.size() && i < n; ++i) {
        complexData[i] = std::complex<float>(input[i], 0.0f);
    }

    fft(complexData);

    m_magnitudeSpectrum.resize(n / 2);
    m_frequencyAxis.resize(n / 2);

    m_maxMagnitude = 0.0f;
    for (size_t i = 0; i < n / 2; ++i) {
        float magnitude = std::abs(complexData[i]);
        m_magnitudeSpectrum[i] = 20.0f * std::log10(magnitude + 1e-10f);
        m_frequencyAxis[i] = static_cast<float>(i);

        if (m_magnitudeSpectrum[i] > m_maxMagnitude) {
            m_maxMagnitude = m_magnitudeSpectrum[i];
        }
    }
}

void FFTWidget::fft(std::vector<std::complex<float>>& data)
{
    size_t n = data.size();
    if (n <= 1) return;

    bit_reverse(data);

    for (size_t len = 2; len <= n; len <<= 1) {
        float angle = -2.0f * M_PI / len;
        std::complex<float> wlen(std::cos(angle), std::sin(angle));

        for (size_t i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (size_t j = 0; j < len / 2; ++j) {
                std::complex<float> u = data[i + j];
                std::complex<float> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

void FFTWidget::bit_reverse(std::vector<std::complex<float>>& data)
{
    size_t n = data.size();
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;

        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
}

void FFTWidget::drawBackground(QPainter& painter)
{
    painter.fillRect(rect(), QColor(20, 20, 20));
    painter.fillRect(m_plotRect, QColor(0, 0, 0));
    painter.setPen(QPen(QColor(100, 100, 100), 2));
    painter.drawRect(m_plotRect);
}

void FFTWidget::drawGrid(QPainter& painter)
{
    painter.setPen(QPen(QColor(60, 60, 60), 1));

    for (int i = 0; i <= GRID_LINES_X; ++i) {
        int x = m_plotRect.left() + (i * m_plotRect.width()) / GRID_LINES_X;
        painter.drawLine(x, m_plotRect.top(), x, m_plotRect.bottom());
    }

    for (int i = 0; i <= GRID_LINES_Y; ++i) {
        int y = m_plotRect.top() + (i * m_plotRect.height()) / GRID_LINES_Y;
        painter.drawLine(m_plotRect.left(), y, m_plotRect.right(), y);
    }
}

void FFTWidget::drawSpectrum(QPainter& painter)
{
    if (m_magnitudeSpectrum.empty()) return;

    painter.setPen(QPen(QColor(0, 255, 255), 2));
    painter.setBrush(Qt::NoBrush);

    QPolygonF spectrum;

    for (size_t i = 0; i < m_magnitudeSpectrum.size(); ++i) {
        float index = m_frequencyAxis[i];
        float x = m_plotRect.left() + (index / m_magnitudeSpectrum.size()) * m_plotRect.width();

        float magDb = m_magnitudeSpectrum[i];
        float y = m_plotRect.bottom() - ((magDb - MIN_MAG_DB) / (MAX_MAG_DB - MIN_MAG_DB)) * m_plotRect.height();
        y = std::max(float(m_plotRect.top()), std::min(float(m_plotRect.bottom()), y));

        spectrum << QPointF(x, y);
    }

    if (spectrum.size() > 1) {
        painter.drawPolyline(spectrum);
    }
}

void FFTWidget::drawLabels(QPainter& painter)
{
    painter.setPen(QPen(Qt::white, 1));
    painter.setFont(QFont("Arial", 10));

    if (!m_magnitudeSpectrum.empty()) {
        int numBins = static_cast<int>(m_magnitudeSpectrum.size());

        for (int i = 0; i <= GRID_LINES_X; ++i) {
            int bin = (i * numBins) / GRID_LINES_X;
            int x = m_plotRect.left() + (i * m_plotRect.width()) / GRID_LINES_X;

            QString label = QString::number(bin);
            QFontMetrics fm(painter.font());
            QRect textRect = fm.boundingRect(label);
            painter.drawText(x - textRect.width() / 2, m_plotRect.bottom() + 15, label);
        }
    }

    for (int i = 0; i <= GRID_LINES_Y; ++i) {
        float mag = MIN_MAG_DB + (float(i) / GRID_LINES_Y) * (MAX_MAG_DB - MIN_MAG_DB);
        int y = m_plotRect.bottom() - (i * m_plotRect.height()) / GRID_LINES_Y;

        QString magText = QString("%1dB").arg(mag, 0, 'f', 0);
        painter.drawText(m_plotRect.left() - 35, y + 5, magText);
    }

    painter.setFont(QFont("Arial", 12, QFont::Bold));

    QFontMetrics fm(painter.font());
    QString xLabel = "Sample Index";
    QRect xLabelRect = fm.boundingRect(xLabel);
    painter.drawText(
        m_plotRect.center().x() - xLabelRect.width() / 2,
        height() - 10,
        xLabel
    );

    painter.save();
    painter.translate(15, m_plotRect.center().y());
    painter.rotate(-90);
    QString yLabel = "Magnitude (dB)";
    QRect yLabelRect = fm.boundingRect(yLabel);
    painter.drawText(-yLabelRect.width() / 2, 0, yLabel);
    painter.restore();

    painter.setFont(QFont("Arial", 14, QFont::Bold));
    painter.drawText(QPointF(10, 25), "FFT Spectrum - Raw ADC Data");

    painter.setFont(QFont("Arial", 10));
    QString frameInfo = QString("Frame: %1, Samples: %2")
                       .arg(1)
                       .arg(m_currentFrame.sample_data.size());
    painter.drawText(QPointF(10, height() - 10), frameInfo);
}
