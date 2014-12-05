/**************************************************************************
** GameX license
** Author: Oznabikhin Dmitry
** Email: gamexgroup@gmail.com
** Filename: mainwindow.cpp
** Copyright (c) 2014 GameX. All rights reserved.
**************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

#include <QDebug>

#include <QWebFrame>
#include <QWebElement>

#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_process(nullptr)
{
    ui->setupUi(this);

    connect(ui->listWidgetTrace, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));

    //connect(ui->pushButtonTracert, SIGNAL(clicked()), this, SLOT(on_pushButtonTracert_clicked()));


    connect(&m_geocodeDataManager, SIGNAL(coordinatesReady(double,double)), this, SLOT(showCoordinates(double,double)));
    connect(&m_geocodeDataManager, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));

    //connect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(onHttpFinished(QNetworkReply*)));

    int status = MMDB_open("GeoLite2-City.mmdb", MMDB_MODE_MMAP, &mmdb);
    if (MMDB_SUCCESS != status)
    {
        fprintf(stderr, "\n  Can't open %s - %s\n",
                "GeoLiteCity.dat", MMDB_strerror(status));

        if (MMDB_IO_ERROR == status) {
            fprintf(stderr, "    IO error: %s\n", strerror(errno));
        }
        exit(1);
    }
    //

    ui->webView->setUrl(QUrl("qrc:/html/google_maps.html"));
    ui->webView->update();
}

MainWindow::~MainWindow()
{
    //MMDB_free_entry_data_list(entry_data_list);
    MMDB_close(&mmdb);

    delete ui;
}

void MainWindow::onItemClicked(QListWidgetItem* item)
{
    int index = item->data(Qt::UserRole).toInt();
    if(index != -1)
    {
        const PointInfo& first = m_points[index];
        double longitude = first.longitude, latitude = first.latitude;

        setCenter(longitude,latitude);
    }
}

void MainWindow::on_pushButtonTracert_clicked()
{
    const QString address = ui->lineEditAddress->text();
    if(address.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("Address is empty.");
        msgBox.exec();
        return;
    }

    m_points.clear();
    ui->listWidgetTrace->clear();

    cleanAll();

    const QString key = "c73fb6010253f25b1bcecba69496105e44ab435a";

#ifdef WIN32
    QString program = "tracert";
#else
    QString program = "traceroute";
#endif

    QStringList arguments;
    arguments << address;

    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onProccessFinished(int,QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));

    ui->pushButtonTracert->setEnabled(false);

    m_process->start(program, arguments);
}

void MainWindow::onReadyReadStandardOutput()
{
    qDebug() << "onReadyReadStandardOutput";

    if(m_process == nullptr)
        return;

    const QByteArray bytes = m_process->readAllStandardOutput();
    const QString text = QString(bytes);
    m_text += text;

    int index = m_text.lastIndexOf("\n");
    if(index != -1)
    {
        const QString textLines = m_text.left(index + 1);
        m_text.remove(0, index + 1);
        const QStringList lines = textLines.split("\n");

        for(const QString& line : lines)
        {
           onLineOutput(line);
        }
    }
}

void MainWindow::onProccessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "onProccessFinished";

    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    if(m_process == nullptr)
        return;

    ui->pushButtonTracert->setEnabled(true);

    const QByteArray bytes = m_process->readAllStandardOutput();
    const QString text = QString(bytes);
    m_text += text;

    //delete m_process;
    m_process = nullptr;

    const QStringList lines = m_text.split("\n");
    for(const QString& line : lines)
    {
       onLineOutput(line);
    }
}

void MainWindow::onLineOutput(const QString& line)
{
    qDebug() << line;

#ifdef WIN32
    static const quint32 ttlIndex = 0;
    static const quint32 nameIndex = 7;
    static const quint32 ipIndex = 1;
    static const quint32 timeIndex = 8;
#else
    static const quint32 ttlIndex = 0;
    static const quint32 nameIndex = 1;
    static const quint32 ipIndex = 2;
    static const quint32 timeIndex = 7;
#endif

    QStringList words = line.split(" ");
    words.removeAll("");

    if(words.count() == 9 ) //line.indexOf("traceroute to") == -1)
    {
        quint32 ttl = words[ttlIndex].toInt();

        const QString name = words[nameIndex];
        QString ip = words[ipIndex];
        ip.remove("(");
        ip.remove(")");

        double time = words[timeIndex].toDouble();

        //http://api.db-ip.com/addrinfo?addr=173.194.67.1&api_key=123456789
        if(ip.indexOf("192.168") == -1)
        {
            PointInfo pi;
            pi.ttl = ttl;
            pi.name = name;
            pi.address = ip;

            pi.full = line;

            readCoordinate(pi);

            pi.time = time;

            pushPoint(pi);
        }
    }
}

/*
void MainWindow::onHttpFinished(QNetworkReply* reply)
{
    auto findPoint = [&](const QString& address) -> PointInfo* {
        for(PointInfo& pi : m_points)
        {
            if(pi.address == address)
                return &pi;
        }
        return nullptr;
    };

    const QByteArray bytes = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if( !doc.isEmpty() )
    {
        QJsonObject json = doc.object();

        const QString address = json["address"].toString();

        const QString city = json["city"].toString();
        const QString country = json["country"].toString();

        PointInfo* pi = findPoint(address);
        if(pi != nullptr)
        {
            pi->city = city;
            pi->country = country;
            qDebug() << address << city << country;

            m_geocodeDataManager.getCoordinates(country + "+" + city );//address.replace(" ", "+"));
        }
    }

    delete reply;
}


void MainWindow::showCoordinates(double east, double north, bool saveMarker)
{
    Q_UNUSED(saveMarker)

    qDebug() << "Form, showCoordinates" << east << north;

    //QString str =
    //        QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
    //        QString("map.setCenter(newLoc);") +
    //        QString("map.setZoom(%1);").arg(ui->zoomSpinBox->value());

    // qDebug() << str;

    //ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    //if (saveMarker)
    //    setMarker(east, north, "test");
}
*/

void MainWindow::cleanAll()
{
    m_markers.clear();
    const QString str = QString("cleanAll()");
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}

void MainWindow::pushPoint(const PointInfo& pi)
{
    if(!m_points.isEmpty())
    {
        const PointInfo& lastPi = m_points.back();
        if( (pi.latitude == 0.0 && pi.longitude == 0.0) || (lastPi.latitude == pi.latitude && lastPi.longitude == pi.longitude))
        {
            Market& market = m_markers.back();

            const QString text = QString("%1 (%2:%3) - %4")
                .arg(pi.address)
                .arg(pi.country)
                .arg(pi.city)
                .arg(pi.full);

            market.nodes += text;

            quint32 index = m_markers.count() - 1;
            updateMarket(index);

            QListWidgetItem* item = new QListWidgetItem(pi.name + " - " + pi.address, ui->listWidgetTrace);
            item->setData(Qt::UserRole, m_points.count());
            m_points.push_back(pi);

            return;
        }
    }

    QStringList nodes;
    const QString text = QString("%1 (%2:%3) - %4")
        .arg(pi.address)
        .arg(pi.country)
        .arg(pi.city)
        .arg(pi.full);

    nodes << text;

    addMarker(pi.longitude, pi.latitude, "Node", nodes);

    setCenter(pi.longitude, pi.latitude);

    // line.
    if(!m_points.isEmpty())
    {
        const PointInfo& lastPi = m_points.back();

        QVector<QVector2D> points;
        points.push_back(QVector2D(lastPi.latitude, lastPi.longitude));
        points.push_back(QVector2D(pi.latitude, pi.longitude));

        QString color = "#00FF00";
        if(pi.time - lastPi.time > 50.0)
            color = "#FF5300";
        if(pi.time - lastPi.time > 100.0)
            color = "#FF0000";
        addLine(points, color);
    }

    QListWidgetItem* item = new QListWidgetItem(pi.name + " - " + pi.address, ui->listWidgetTrace);
    item->setData(Qt::UserRole, m_points.count());
    m_points.push_back(pi);
}

void MainWindow::addMarker(double east, double north, const QString& caption, const QStringList& nodes)
{
    QString contentString =
          QString("'<div id=\"siteNotice\"></div>"
          "<h1 id=\"firstHeading\" class=\"firstHeading\">%1</h1>"
          "<div id=\"bodyContent\">").arg(caption);

    for(const QString& node : nodes)
    {
        contentString += QString("<p>%1</p>").arg(node);
    }
    contentString += QString("</div></div>'");

    const QString str = QString("addMarker(%1, %2, %3, '%4')")
            .arg(contentString)
            .arg(north)
            .arg(east)
            .arg(caption);

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    m_markers.push_back(Market(caption, nodes));
}

void MainWindow::updateNode(quint32 index)
{
    Q_UNUSED(index)
    //Market& marker = m_markers[index];
    //marker.nodes.push_back(node);
}

void MainWindow::updateMarket(quint32 index)
{
    const Market& marker = m_markers[index];
    const QStringList& nodes = marker.nodes;

    QString contentString =
          QString("'<div id=\"siteNotice\"></div>"
          "<h1 id=\"firstHeading\" class=\"firstHeading\">%1</h1>"
          "<div id=\"bodyContent\">")
            .arg(marker.caption);

    for(const QString& node : nodes)
    {
        contentString += QString("<p>%1</p>").arg(node);
    }
    contentString += QString("</div></div>'");

    const QString str = QString("updateMarker(%1, %2)")
            .arg(index)
            .arg(contentString);

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}

void MainWindow::addLine(const QVector<QVector2D>& points, const QString& color)
{
    QString args = "[";
    for(auto it=points.begin(); it != points.end(); ++it)
    {
        const QVector2D& p = *it;
        args += QString("{latitude: %1, longitude: %2}").arg(p.x()).arg(p.y());

        if(it + 1 != points.end())
            args += ",";
    }
    args += "]";

    const QString str = QString("addLine(%1, '%2')")
            .arg(args)
            .arg(color);

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}

void MainWindow::setCenter(double east, double north)
{
    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);");

    //qDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}

void MainWindow::readCoordinate(PointInfo& pi)
{
    const char* ipstr = pi.address.toLocal8Bit().data();

    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(&mmdb, ipstr, &gai_error, &mmdb_error);

    if (0 != gai_error)
    {
        fprintf(stderr,"\n  Error from getaddrinfo for %s - %s\n\n",
                ipstr, gai_strerror(gai_error));
        return;
    }

    if (MMDB_SUCCESS != mmdb_error)
    {
        fprintf(stderr, "\n  Got an error from libmaxminddb: %s\n\n",
                MMDB_strerror(mmdb_error));
        return;
    }


    if (result.found_entry)
    {
        int status;
        //MMDB_entry_data_list_s *entry_data_list = NULL;
        //int status = MMDB_get_entry_data_list(&result.entry,
        //                                      &entry_data_list);

        MMDB_entry_data_s
                entry_data_latitude,
                entry_data_longitude,
                entry_data_country,
                entry_data_city;

        status = MMDB_get_value(&result.entry, &entry_data_latitude,
                           "location", "latitude", NULL);

        if (MMDB_SUCCESS != status)
        {
            fprintf(stderr,"Got an error looking up the entry data - %s\n", MMDB_strerror(status));
            return;
        }



        status = MMDB_get_value(&result.entry, &entry_data_longitude,
                           "location", "longitude", NULL);
        if (MMDB_SUCCESS != status)
        {
            fprintf(stderr,"Got an error looking up the entry data - %s\n", MMDB_strerror(status));
            return;
        }


        pi.latitude = entry_data_latitude.double_value;
        pi.longitude = entry_data_longitude.double_value;

        status = MMDB_get_value(&result.entry, &entry_data_country,
                           "country", "en", NULL);

        if (MMDB_SUCCESS == status)
        {
            pi.country = entry_data_country.utf8_string;
        }

        status = MMDB_get_value(&result.entry, &entry_data_city,
                           "city", "en", NULL);

        if (MMDB_SUCCESS == status)
        {
            pi.city = entry_data_country.utf8_string;
        }

        /*
        if (MMDB_SUCCESS != status)
        {
            fprintf(stderr,"Got an error looking up the entry data - %s\n",
                MMDB_strerror(status));
            return;
        }

        if (NULL != entry_data_list)
        {
            MMDB_get_value()
            //entry_data_list->entry_data.utf8_string
            MMDB_dump_entry_data_list(stdout, entry_data_list, 2);
            MMDB_free_entry_data_list(entry_data_list);
        }*/
    }
    else
    {
        fprintf(stderr,"\n  No entry for this IP address (%s) was found\n\n", ipstr);
    }


}
