#pragma once

#include <QWidget>
#include <QPainter>
#include <vector>
#include <complex>
#include "DataStructures.h"

class FFTWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FFTWidget(QWidget *parent = nullptr);
    
    void updateData(const RawADCFrame& adcFrame);
    void setFrequencyRange(float minFreq, float maxFreq);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void performFFT(const std::vector<float>& input);
    void drawBackground(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawSpectrum(QPainter& painter);
    void drawLabels(QPainter& painter);
    
    // Simple FFT implementation
    void fft(std::vector<std::complex<float>>& data);
    void bit_reverse(std::vector<std::complex<float>>& data);
    
    std::vector<float> m_magnitudeSpectrum;
    std::vector<float> m_frequencyAxis;
    
    RawADCFrame m_currentFrame;
    float m_minFrequency;
    float m_maxFrequency;
    float m_maxMagnitude;
    
    QRect m_plotRect;
    int m_margin;
    
    // Visual settings
    static constexpr int GRID_LINES_X = 10;
    static constexpr int GRID_LINES_Y = 8;
    static constexpr float MIN_MAG_DB = -80.0f;
    static constexpr float MAX_MAG_DB = 20.0f;
};