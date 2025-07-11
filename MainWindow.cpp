#include "MainWindow.h"
#include <QApplication>
#include <QNetworkDatagram>
#include <QHeaderView>
#include <QMessageBox>
#include <QGridLayout>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ppiWidget(nullptr)
    , m_fftWidget(nullptr)
    , m_trackTable(nullptr)
    , m_udpSocket(nullptr)
    , m_updateTimer(nullptr)
    , m_simulationEnabled(true)
    , m_randomEngine(std::random_device{}())
    , m_rangeDist(100.0f, 500.0f)  // 1-15 km
    , m_azimuthDist(-90.0f, 90.0f)    // -90 to +90 degrees
    , m_speedDist(-50.0f, 50.0f)      // -50 to +50 m/s
    , m_levelDist(10.0f, 100.0f)      // 10-100 dB
    , m_numTargetsDist(3, 8)          // 3-8 targets
    , m_frameCount(0)
    , m_targetCount(0)
{
    setupUI();
    setupNetworking();
    setupTimer();
    
    // Initialize with simulated data
    //generateSimulatedTargetData();
   // generateSimulatedADCData();
}

MainWindow::~MainWindow()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
}

void MainWindow::setupUI()
{
    setWindowTitle("Radar Data Visualization - Qt C++");
    setMinimumSize(1200, 800);
    
    // Create central widget with splitter layout
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Main horizontal splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    
    // Left side: PPI Display
    QGroupBox* ppiGroup = new QGroupBox("PPI Display (Target Tracks)");
    QVBoxLayout* ppiLayout = new QVBoxLayout(ppiGroup);
    
    m_ppiWidget = new PPIWidget();
    ppiLayout->addWidget(m_ppiWidget);
    
    // PPI controls
    QHBoxLayout* ppiControlsLayout = new QHBoxLayout();
    ppiControlsLayout->addWidget(new QLabel("Max Range (m):"));
    m_rangeSpinBox = new QSpinBox();
    m_rangeSpinBox->setRange(1, 50);
    m_rangeSpinBox->setValue(10);
    m_rangeSpinBox->setSuffix(" m");
    connect(m_rangeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onRangeChanged);
    ppiControlsLayout->addWidget(m_rangeSpinBox);
    
    ppiControlsLayout->addStretch();
    ppiLayout->addLayout(ppiControlsLayout);
    
    m_mainSplitter->addWidget(ppiGroup);
    
    // Right side splitter (vertical)
    m_rightSplitter = new QSplitter(Qt::Vertical);
    
    // FFT Display
    QGroupBox* fftGroup = new QGroupBox("FFT Spectrum (Raw ADC Data)");
    QVBoxLayout* fftLayout = new QVBoxLayout(fftGroup);
    
    m_fftWidget = new FFTWidget();
    fftLayout->addWidget(m_fftWidget);
    
    m_rightSplitter->addWidget(fftGroup);
    
    // Track Table
    QGroupBox* tableGroup = new QGroupBox("Target Track Table");
    QVBoxLayout* tableLayout = new QVBoxLayout(tableGroup);
    
    m_trackTable = new QTableWidget();
    m_trackTable->setColumnCount(4);
    QStringList headers;
    headers << "ID" << "Range (m)" << "Azimuth (°)"
            << "Radial Speed (m/s)";
    m_trackTable->setHorizontalHeaderLabels(headers);
    m_trackTable->horizontalHeader()->setStretchLastSection(true);
    m_trackTable->setAlternatingRowColors(true);
    m_trackTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    tableLayout->addWidget(m_trackTable);
    m_rightSplitter->addWidget(tableGroup);
    
    m_mainSplitter->addWidget(m_rightSplitter);
    
    // Set splitter proportions
    m_mainSplitter->setSizes({600, 600});
    m_rightSplitter->setSizes({400, 400});
    
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Control panel
    QHBoxLayout* controlLayout = new QHBoxLayout();
    
    m_simulateButton = new QPushButton("Enable Simulation");
    m_simulateButton->setCheckable(true);
    m_simulateButton->setChecked(true);
    connect(m_simulateButton, &QPushButton::toggled,
            this, &MainWindow::onSimulateDataToggled);
    controlLayout->addWidget(m_simulateButton);
    
    controlLayout->addStretch();
    
    m_frameCountLabel = new QLabel("Frames: 0");
    controlLayout->addWidget(m_frameCountLabel);
    
    m_statusLabel = new QLabel("Status: Simulation Active");
    controlLayout->addWidget(m_statusLabel);
    
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(m_mainSplitter);
    
    // Status bar
    statusBar()->showMessage("Radar Visualization Ready - Listening on UDP port 5000");
}

void MainWindow::setupNetworking()
{
    m_udpSocket = new QUdpSocket(this);
    
    if (!m_udpSocket->bind(QHostAddress::Any, UDP_PORT)) {
        QMessageBox::warning(this, "Network Error",
                           QString("Failed to bind to UDP port %1. "
                                  "Real data reception disabled.").arg(UDP_PORT));
        m_statusLabel->setText("Status: Network Error - Simulation Only");
    } else {
        connect(m_udpSocket, &QUdpSocket::readyRead,
                this, &MainWindow::readPendingDatagrams);
        m_statusLabel->setText("Status: UDP Listening");
    }
}

void MainWindow::setupTimer()
{
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout,
            this, &MainWindow::updateDisplay);
    m_updateTimer->start(UPDATE_INTERVAL_MS);
}

void MainWindow::updateDisplay()
{
    if (m_simulationEnabled) {
        //generateSimulatedTargetData();
        //generateSimulatedADCData();
    }
    
    // Update widgets
    m_ppiWidget->updateTargets(m_currentTargets);
    m_fftWidget->updateData(m_currentADCFrame);
    updateTrackTable();
    
    // Update statistics
    m_frameCount++;
    m_frameCountLabel->setText(QString("Frames: %1").arg(m_frameCount));
    
    if (m_simulationEnabled) {
        m_statusLabel->setText(QString("Status: Simulation Active - %1 targets")
                              .arg(m_currentTargets.numTracks));
    }
}

void MainWindow::readPendingDatagrams() {
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size());
        qDebug()<<"r";
        QString msg = QString::fromUtf8(datagram);
        if (msg.contains("NumTargets:")) {
            parseTrackMessage(msg);  // Track data if present
        }else
        {
            qDebug()<<"No track";
        }
        if (msg.contains("ADC:")) {
            parseADCMessage(msg);  // Raw FFT data
        }
    }
}

void MainWindow::parseTrackMessage(const QString& message) {
                qDebug()<<"track";
                QStringList tokens = message.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
                m_currentTargets.targets.clear();
                m_currentTargets.numTracks = 0;

                TargetTrack target;
                int numTargets = 0;
                int parsedTargets = 0;

                for (int i = 0; i < tokens.size(); ++i) {
                    const QString& token = tokens[i];

                    if (token == "NumTargets:" && i + 1 < tokens.size()) {
                        numTargets = tokens[++i].toInt();
                    } else if (token == "TgtId:" && i + 1 < tokens.size()) {
                        if (parsedTargets > 0) {
                            m_currentTargets.targets.push_back(target);
                            target = TargetTrack(); // Reset
                        }
                        target.target_id = tokens[++i].toInt();
                        ++parsedTargets;
                    } else if (token == "Level:" && i + 1 < tokens.size()) {
                        target.level = tokens[++i].toFloat();
                    } else if (token == "Range:" && i + 1 < tokens.size()) {
                        target.radius = tokens[++i].toFloat();
                    } else if (token == "Azimuth:" && i + 1 < tokens.size()) {
                        target.azimuth = tokens[++i].toFloat();
                    } else if (token == "Elevation:" && i + 1 < tokens.size()) {
                        target.elevation = tokens[++i].toFloat();
                    } else if (token == "RadialSpeed:" && i + 1 < tokens.size()) {
                        target.radial_speed = tokens[++i].toFloat();
                    } else if (token == "AzimuthSpeed:" && i + 1 < tokens.size()) {
                        target.azimuth_speed = tokens[++i].toFloat();
                    } else if (token == "ElevationSpeed:" && i + 1 < tokens.size()) {
                        target.elevation_speed = tokens[++i].toFloat();
                    }
                }

                // Append final target if one exists
                if (parsedTargets > 0) {
                    m_currentTargets.targets.push_back(target);
                }

                m_currentTargets.numTracks = m_currentTargets.targets.size();

}

void MainWindow::parseADCMessage(const QString& message) {
    QStringList tokens = message.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
    qDebug()<<"parseADCMessage";
    RawADCFrameTest frame;
    frame.sample_data.clear();

    for (int i = 0; i < tokens.size(); ++i) {
        const QString& token = tokens[i];

        if (token == "MsgId:" && i + 1 < tokens.size()) {
            frame.msgId = tokens[++i].toUInt();
        } else if (token == "NumSamples:" && i + 1 < tokens.size()) {
            frame.num_samples_per_chirp = tokens[++i].toUInt();
        } else if (token == "ADC:" && i + 1 < tokens.size()) {
            frame.sample_data.push_back(tokens[++i].toFloat());
        }
    }

    // Now assign to your current frame and update plot
    m_currentADCFrame = frame;
    //updateADCPlot();  // call your FFT or frequency domain plot update method
}


//void MainWindow::readPendingDatagrams1()
//{
//    while (m_udpSocket->hasPendingDatagrams()) {
//        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
//        QByteArray data = datagram.data();

//        QStringList tokens = QString::fromUtf8(data).split(" ", QString::SkipEmptyParts);

//        if(0)
//        {
//            m_currentTargets.targets.clear();
//            m_currentTargets.numTracks = 0;

//            TargetTrack target;
//            int numTargets = 0;
//            int parsedTargets = 0;

//            for (int i = 0; i < tokens.size(); ++i) {
//                const QString& token = tokens[i];

//                if (token == "NumTargets:" && i + 1 < tokens.size()) {
//                    numTargets = tokens[++i].toInt();
//                } else if (token == "TgtId:" && i + 1 < tokens.size()) {
//                    if (parsedTargets > 0) {
//                        m_currentTargets.targets.push_back(target);
//                        target = TargetTrack(); // Reset
//                    }
//                    target.target_id = tokens[++i].toInt();
//                    ++parsedTargets;
//                } else if (token == "Level:" && i + 1 < tokens.size()) {
//                    target.level = tokens[++i].toFloat();
//                } else if (token == "Range:" && i + 1 < tokens.size()) {
//                    target.radius = tokens[++i].toFloat();
//                } else if (token == "Azimuth:" && i + 1 < tokens.size()) {
//                    target.azimuth = tokens[++i].toFloat();
//                } else if (token == "Elevation:" && i + 1 < tokens.size()) {
//                    target.elevation = tokens[++i].toFloat();
//                } else if (token == "RadialSpeed:" && i + 1 < tokens.size()) {
//                    target.radial_speed = tokens[++i].toFloat();
//                } else if (token == "AzimuthSpeed:" && i + 1 < tokens.size()) {
//                    target.azimuth_speed = tokens[++i].toFloat();
//                } else if (token == "ElevationSpeed:" && i + 1 < tokens.size()) {
//                    target.elevation_speed = tokens[++i].toFloat();
//                }
//            }

//            // Append final target if one exists
//            if (parsedTargets > 0) {
//                m_currentTargets.targets.push_back(target);
//            }

//            m_currentTargets.numTracks = m_currentTargets.targets.size();

//            qDebug() << "Parsed" << m_currentTargets.numTracks << "targets (expected:" << numTargets << ")";
//            qDebug() << "Data received from UDP.";
//        }
//        else
//        {
//            for (int i = 0; i < tokens.size(); ++i) {
//                const QString& token = tokens[i];

//                if (token == "MsgId:" && i + 1 < tokens.size()) {
//                    MsgId = tokens[++i].toInt();
//                } else if (token == "NumSamples:" && i + 1 < tokens.size()) {
//                    if (parsedTargets > 0) {
//                        m_currentTargets.targets.push_back(target);
//                        target = TargetTrack(); // Reset
//                    }
//                    target.target_id = tokens[++i].toInt();
//                    ++parsedTargets;
//                } else if (token == "ADC:" && i + 1 < tokens.size()) {
//                    target.level = tokens[++i].toFloat();
//        }

//    }
//}

void MainWindow::onSimulateDataToggled()
{
    m_simulationEnabled = m_simulateButton->isChecked();
    
    if (m_simulationEnabled) {
        m_simulateButton->setText("Disable Simulation");
        m_statusLabel->setText("Status: Simulation Active");
    } else {
        m_simulateButton->setText("Enable Simulation");
        m_statusLabel->setText("Status: Simulation Disabled");
    }
}

void MainWindow::onRangeChanged(int range)
{
    float rangeMeters = range;
    m_ppiWidget->setMaxRange(rangeMeters);
}

void MainWindow::updateTrackTable()
{
    m_trackTable->setRowCount(m_currentTargets.numTracks);
    //qDebug()<<"In Table "<<m_currentTargets.numTracks<<"\n";
    for (uint32_t i = 0; i < m_currentTargets.numTracks; ++i) {
        const TargetTrack& target = m_currentTargets.targets[i];
        qDebug()<<target.target_id<<target.radius<<target.azimuth<<target.radial_speed<<"\n";
        m_trackTable->setItem(i, 0, new QTableWidgetItem(QString::number(target.target_id)));
        m_trackTable->setItem(i, 1, new QTableWidgetItem(QString::number(target.radius, 'f', 0)));
        m_trackTable->setItem(i, 2, new QTableWidgetItem(QString::number(target.azimuth, 'f', 1)));
        m_trackTable->setItem(i, 3, new QTableWidgetItem(QString::number(target.radial_speed, 'f', 1)));
    }
    
    m_trackTable->resizeColumnsToContents();
}

void MainWindow::generateSimulatedTargetData()
{
    // Generate random number of targets
    uint32_t numTargets = m_numTargetsDist(m_randomEngine);
    m_currentTargets.resize(numTargets);
    
    for (uint32_t i = 0; i < numTargets; ++i) {
        TargetTrack& target = m_currentTargets.targets[i];
        
        target.target_id = i + 1;
        target.level = m_levelDist(m_randomEngine);
        target.radius = m_rangeDist(m_randomEngine);
        target.azimuth = m_azimuthDist(m_randomEngine);
        target.elevation = std::uniform_real_distribution<float>(-30.0f, 30.0f)(m_randomEngine);
        target.radial_speed = m_speedDist(m_randomEngine);
        target.azimuth_speed = std::uniform_real_distribution<float>(-5.0f, 5.0f)(m_randomEngine);
        target.elevation_speed = std::uniform_real_distribution<float>(-2.0f, 2.0f)(m_randomEngine);
    }
    
    m_targetCount += numTargets;
}

void MainWindow::generateSimulatedADCData()
{
    // Generate simulated ADC data with some frequency components
    const uint32_t numSamples = 512;
    m_currentADCFrame.sample_data.resize(numSamples);
   // m_currentADCFrame.frame_number++;
    m_currentADCFrame.num_samples_per_chirp = numSamples;
    
    // Generate signal with multiple frequency components + noise
    float sampleRate = 100000.0f; // 100 kHz
    float t_step = 1.0f / sampleRate;
    
    // Add some dominant frequency components
    float freq1 = 5000.0f;  // 5 kHz
    float freq2 = 15000.0f; // 15 kHz
    float freq3 = 25000.0f; // 25 kHz
    
    std::uniform_real_distribution<float> noiseDist(-0.1f, 0.1f);
    
    for (uint32_t i = 0; i < numSamples; ++i) {
        float t = i * t_step;
        
        // Sum of sinusoids with different amplitudes
        float signal = 0.8f * std::sin(2.0f * M_PI * freq1 * t) +
                      0.5f * std::sin(2.0f * M_PI * freq2 * t) +
                      0.3f * std::sin(2.0f * M_PI * freq3 * t);
        
        // Add noise
        signal += noiseDist(m_randomEngine);
        
        m_currentADCFrame.sample_data[i] = signal;
    }
}
