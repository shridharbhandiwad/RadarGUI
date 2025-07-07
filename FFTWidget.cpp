#include "FFTWidget.h"
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFont>
#include <QFontMetrics>
#include <cmath>
#include <algorithm>

FFTWidget::FFTWidget(QWidget *parent)
    : QWidget(parent)
    , m_minFrequency(-50000.0f) // -50 MHz
    , m_maxFrequency(50000.0f)  // +50 MHz
    , m_maxMagnitude(0.0f)
    , m_margin(50)
{
    setMinimumSize(400, 300);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void FFTWidget::updateData(const RawADCFrame& adcFrame)
{
    m_currentFrame = adcFrame;
    if (!adcFrame.sample_data.empty()) {
        performFFT(adcFrame.sample_data);
    }
    update();
}

void FFTWidget::setFrequencyRange(float minFreq, float maxFreq)
{
    m_minFrequency = minFreq;
    m_maxFrequency = maxFreq;
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
    
    // Find the next power of 2
    size_t n = 1;
    while (n < input.size()) {
        n *= 2;
    }
    
    // Convert to complex and zero-pad if necessary
    std::vector<std::complex<float>> complexData(n);
    for (size_t i = 0; i < input.size() && i < n; ++i) {
        complexData[i] = std::complex<float>(input[i], 0.0f);
    }
    
    // Perform FFT
    fft(complexData);
    
    // Calculate magnitude spectrum
    m_magnitudeSpectrum.resize(n / 2);
    m_frequencyAxis.resize(n / 2);
    
    float sampleRate = 100000.0f; // Assume 100 kHz sample rate
    float freqResolution = sampleRate / n;
    
    m_maxMagnitude = 0.0f;
    for (size_t i = 0; i < n / 2; ++i) {
        float magnitude = std::abs(complexData[i]);
        m_magnitudeSpectrum[i] = 20.0f * std::log10(magnitude + 1e-10f); // Convert to dB
        m_frequencyAxis[i] = (float(i) - float(n/2)) * freqResolution;
        
        if (m_magnitudeSpectrum[i] > m_maxMagnitude) {
            m_maxMagnitude = m_magnitudeSpectrum[i];
        }
    }
}

void FFTWidget::fft(std::vector<std::complex<float>>& data)
{
    size_t n = data.size();
    if (n <= 1) return;
    
    // Bit-reverse
    bit_reverse(data);
    
    // FFT
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
    // Fill background
    painter.fillRect(rect(), QColor(20, 20, 20));
    
    // Draw plot area
    painter.fillRect(m_plotRect, QColor(0, 0, 0));
    painter.setPen(QPen(QColor(100, 100, 100), 2));
    painter.drawRect(m_plotRect);
}

void FFTWidget::drawGrid(QPainter& painter)
{
    painter.setPen(QPen(QColor(60, 60, 60), 1));
    
    // Vertical grid lines (frequency)
    for (int i = 0; i <= GRID_LINES_X; ++i) {
        int x = m_plotRect.left() + (i * m_plotRect.width()) / GRID_LINES_X;
        painter.drawLine(x, m_plotRect.top(), x, m_plotRect.bottom());
    }
    
    // Horizontal grid lines (magnitude)
    for (int i = 0; i <= GRID_LINES_Y; ++i) {
        int y = m_plotRect.top() + (i * m_plotRect.height()) / GRID_LINES_Y;
        painter.drawLine(m_plotRect.left(), y, m_plotRect.right(), y);
    }
}

void FFTWidget::drawSpectrum(QPainter& painter)
{
    if (m_magnitudeSpectrum.empty()) return;
    
    painter.setPen(QPen(QColor(0, 255, 255), 2)); // Cyan color for spectrum
    painter.setBrush(Qt::NoBrush);
    
    QPolygonF spectrum;
    
    for (size_t i = 0; i < m_magnitudeSpectrum.size(); ++i) {
        // Map frequency to x-coordinate
        float freq = m_frequencyAxis[i];
        if (freq < m_minFrequency || freq > m_maxFrequency) continue;
        
        float x = m_plotRect.left() + 
                 ((freq - m_minFrequency) / (m_maxFrequency - m_minFrequency)) * m_plotRect.width();
        
        // Map magnitude to y-coordinate
        float magDb = m_magnitudeSpectrum[i];
        float y = m_plotRect.bottom() - 
                 ((magDb - MIN_MAG_DB) / (MAX_MAG_DB - MIN_MAG_DB)) * m_plotRect.height();
        
        // Clamp y to plot area
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
    
    // Frequency labels (x-axis)
    for (int i = 0; i <= GRID_LINES_X; ++i) {
        float freq = m_minFrequency + (float(i) / GRID_LINES_X) * (m_maxFrequency - m_minFrequency);
        int x = m_plotRect.left() + (i * m_plotRect.width()) / GRID_LINES_X;
        
        QString freqText;
        if (std::abs(freq) >= 1000) {
            freqText = QString("%1k").arg(freq / 1000.0f, 0, 'f', 1);
        } else {
            freqText = QString("%1").arg(freq, 0, 'f', 0);
        }
        
        QFontMetrics fm(painter.font());
        QRect textRect = fm.boundingRect(freqText);
        painter.drawText(x - textRect.width() / 2, m_plotRect.bottom() + 15, freqText);
    }
    
    // Magnitude labels (y-axis)
    for (int i = 0; i <= GRID_LINES_Y; ++i) {
        float mag = MIN_MAG_DB + (float(i) / GRID_LINES_Y) * (MAX_MAG_DB - MIN_MAG_DB);
        int y = m_plotRect.bottom() - (i * m_plotRect.height()) / GRID_LINES_Y;
        
        QString magText = QString("%1dB").arg(mag, 0, 'f', 0);
        painter.drawText(m_plotRect.left() - 35, y + 5, magText);
    }
    
    // Axis labels
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    
    // X-axis label
    QFontMetrics fm(painter.font());
    QString xLabel = "Frequency (Hz)";
    QRect xLabelRect = fm.boundingRect(xLabel);
    painter.drawText(
        m_plotRect.center().x() - xLabelRect.width() / 2,
        height() - 10,
        xLabel
    );
    
    // Y-axis label
    painter.save();
    painter.translate(15, m_plotRect.center().y());
    painter.rotate(-90);
    QString yLabel = "Magnitude (dB)";
    QRect yLabelRect = fm.boundingRect(yLabel);
    painter.drawText(-yLabelRect.width() / 2, 0, yLabel);
    painter.restore();
    
    // Title
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    painter.drawText(QPointF(10, 25), "FFT Spectrum - Raw ADC Data");
    
    // Frame info
    painter.setFont(QFont("Arial", 10));
    QString frameInfo = QString("Frame: %1, Samples: %2")
                       .arg(m_currentFrame.frame_number)
                       .arg(m_currentFrame.sample_data.size());
    painter.drawText(QPointF(10, height() - 10), frameInfo);
}