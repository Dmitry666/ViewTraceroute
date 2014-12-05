#include "geocode_data_manager.h"


#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <QDebug>


const QString apiKey = "AIzaSyCkfyXYy56o1La5I3d2rGzFY6RvyGLkYK";

GeocodeDataManager::GeocodeDataManager(QObject *parent) :
    QObject(parent)
{
    m_pNetworkAccessManager = new QNetworkAccessManager(this);
    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));


}

void GeocodeDataManager::getCoordinates(const QString& address)
{
    QString url = QString("http://maps.google.com/maps/geo?q=%1&key=%2&output=json&oe=utf8&sensor=false").arg(address).arg(apiKey);
    m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url)));
}


void GeocodeDataManager::replyFinished(QNetworkReply* reply)
{
    QByteArray json = reply->readAll();
    qDebug() << "Reply = " << json;
    qDebug() << "URL = " << reply->url();
    QString strUrl = reply->url().toString();

    // json is a QString containing the data to convert
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if( doc.isEmpty() )
    {
        emit errorOccured(QString("Cannot convert to QJson object: %1").arg(QString(json)));
        return;
    }

    QJsonObject root = doc.object();

    int code = root["Status"].toObject()["code"].toInt();
    if(code != 200)
    {
        emit errorOccured(QString("Code of request is: %1").arg(code));
        return;
    }

    QJsonArray placeMarks = root["Placemark"].toArray();
    if(placeMarks.count() == 0)
    {
        emit errorOccured(QString("Cannot find any locations"));
        return;
    }

    double east  = placeMarks[0].toObject()["Point"].toObject()["coordinates"].toArray()[0].toDouble();
    double north = placeMarks[0].toObject()["Point"].toObject()["coordinates"].toArray()[1].toDouble();

    emit coordinatesReady(east, north);

}
