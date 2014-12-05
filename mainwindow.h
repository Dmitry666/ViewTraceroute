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

/**
 * @brief Point information from every address
 */
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

/**
 * @brief Market ont map.
 */
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

/**
 * @brief Main window.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    /**
     * @brief On item clicked ib trace view.
     * @param item lis widget item.
     */
    void onItemClicked(QListWidgetItem* item);

    /**
     * @brief On clicked trace button.
     */
    void on_pushButtonTracert_clicked();

    /**
     * @brief On read standart output from traceroute program.
     */
    void onReadyReadStandardOutput();

    /**
     * @brief On traceroute program finished.
     * @param exitCode
     * @param exitStatus
     */
    void onProccessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief On line output.
     * @param line output line.
     */
    void onLineOutput(const QString& line);
    /*
    void onHttpFinished(QNetworkReply* reply);
    void showCoordinates(double east, double north, bool saveMarker = true);
    */

private:
    /**
     * @brief Clean all markers.
     */
    void cleanAll();

    /**
     * @brief push new point.
     * @param pi point information.
     */
    void pushPoint(const PointInfo& pi);

    /**
     * @brief Add new marker on the google map.
     * @param east
     * @param north
     * @param caption
     * @param nodes inforamtion list in the marker.
     */
    void addMarker(double east, double north, const QString& caption, const QStringList& nodes);

    /**
     * @brief Regenerate poine inforamation.
     * @param index index point.
     */
    void updateNode(quint32 index);


    /**
     * @brief update information into google marker.
     * @param index marker index.
     */
    void updateMarket(quint32 index);

    /**
     * @brief Add line to the google map.
     * @param points line points.
     * @param color line color.
     */
    void addLine(const QVector<QVector2D>& points, const QString& color);

    /**
     * @brief Set view center on map.
     * @param east
     * @param north
     */
    void setCenter(double east, double north);

    /**
     * @brief Read database coordinate from point info.
     * @param pi point info.
     */
    void readCoordinate(PointInfo& pi);

private:
    Ui::MainWindow *ui;

    QProcess *m_process;

    //QUrl url;
    //QNetworkAccessManager qnam;
    //QNetworkReply *reply;

    QString m_text;

    QList<PointInfo> m_points;
    QList<Market> m_markers;

    MMDB_s mmdb;
    GeocodeDataManager m_geocodeDataManager;
};

#endif // MAINWINDOW_H
