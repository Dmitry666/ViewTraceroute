/**************************************************************************
** GameX license
** Author: Oznabikhin Dmitry
** Email: gamexgroup@gmail.com
** Filename: mainwindow.h
** Copyright (c) 2014 GameX. All rights reserved.
**************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "geocode_data_manager.h"
#include <maxminddb.h>

#include <QVector>
#include <QVector2D>

#include <QProcess>
#include <QListWidget>

namespace Ui {
class MainWindow;
}

struct PointInfo
{
    qint32 ttl;
    QString name;
    QString address;
    QString full;
    QString country;
    QString city;
    double latitude;
    double longitude;
    double time;

    PointInfo()
        : latitude(0.0)
        , longitude(0.0)
        , time(0.0)
    {}

    PointInfo(const PointInfo& pi)
        : name(pi.name)
        , address(pi.address)
        , full(pi.full)
        , country(pi.country)
        , city(pi.city)

        , latitude(pi.latitude)
        , longitude(pi.longitude)
        , time(pi.time)
    {}
};

struct Market
{
    QString caption;
    QStringList nodes;

    Market()
    {}

    Market(const QString& caption, const QStringList& nodes)
        : caption(caption)
        , nodes(nodes)
    {}

    Market(const Market& market)
        : caption(market.caption)
        , nodes(market.nodes)
    {}

    ~Market()
    {}
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onItemClicked(QListWidgetItem* item);

    void on_pushButtonTracert_clicked();
    void onReadyReadStandardOutput();
    void onProccessFinished(int exitCode, QProcess::ExitStatus exitStatus);


    void onHttpFinished(QNetworkReply* reply);

    void showCoordinates(double east, double north, bool saveMarker = true);

    void cleanAll();

    void pushPoint(const PointInfo& pi);
    void addMarker(double east, double north, const QString& caption, const QStringList& nodes);
    void updateNode(quint32 index);
    void updateMarket(quint32 index);

    void addLine(const QVector<QVector2D>& points, const QString& color);
    void setCenter(double east, double north);

private:
    void readCoordinate(PointInfo& pi);

private:
    Ui::MainWindow *ui;

    QProcess *m_process;

    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;

    QList<PointInfo> m_points;
    QList<Market> m_markers;

    MMDB_s mmdb;
    GeocodeDataManager m_geocodeDataManager;
};

#endif // MAINWINDOW_H
