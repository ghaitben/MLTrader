#ifndef CTRADER_H
#define CTRADER_H
#include<Python.h>
#include<QProcess>
#include<QMainWindow>
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ctrader; }
QT_END_NAMESPACE

class ctrader : public QMainWindow
{
    Q_OBJECT

public:
    ctrader(QWidget *parent = nullptr);
    ~ctrader();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

private:
    Ui::ctrader *ui;
    QWebSocket _websocket;
    QUrl _websocketUrl;
    QUrl _apiUrl;

};


#endif // CTRADER_H
