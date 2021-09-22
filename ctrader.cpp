#include"ctrader.h"
#include"ui_ctrader.h"
#include"Helpers.h"
#include<QJsonDocument>
#include<QFile>
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<fstream>


ctrader::ctrader(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::ctrader),
      _websocketUrl(QUrl("wss://stream.binance.com:9443/ws/btcusdt@kline_1m")),
      _apiUrl(QUrl("https://api.binance.com/api/v3/klines?symbol=BTCUSDT&interval=1m&limit=10"))
{

    ui->setupUi(this);

    //setting up the table
    QTableWidget* table = ui->tableWidget;
    table->setRowCount(Helper::ROW_COUNT);
    table->setColumnCount(Helper::COLUMN_COUNT);

    table->setHorizontalHeaderLabels({"time","open", "high", "low", "close", "volume"});
    table->verticalHeader()->hide();

    //make rows and columns stretch to match the table width and height
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    //http request to get the last X candle data to view it in table
    QNetworkAccessManager* manager = new QNetworkAccessManager();

    QNetworkRequest request = QNetworkRequest(_apiUrl);
    QNetworkReply* reply = manager->get(request);

    //When receiving the data
    connect(reply, &QNetworkReply::finished,
            [&,reply]() {

            QByteArray replyTxt = reply->readAll();
            std::string str(replyTxt.data());

            std::string** processed_data = Helper::processData(str);

            for(int i = 0; i < Helper::ROW_COUNT; ++i) {
                for(int j = 0; j < 6; ++j) {            //0 to 5 are the indexes of time/o/h/l/c/v in the response
                    QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(processed_data[Helper::ROW_COUNT - i - 1][j]));
                    Helper::readOnly(item);
                    ui->tableWidget->setItem(i,j,item);
                }
            }

            delete [] processed_data;
            reply->deleteLater();
        }
    );

}

ctrader::~ctrader()
{
    delete ui;
}

void ctrader::on_pushButton_clicked() {

    Helper::print(ui,QString("establishing websocket connection...."));
    _websocket.open(_websocketUrl);

    //verify when the connection is successful and print something the the user
    connect(&_websocket, &QWebSocket::connected,
            [=] () { Helper::print(ui,QString("connection established !")); });


    //listen for server messages and execute the lambda function
    connect(&_websocket, &QWebSocket::textMessageReceived,
    [=]( QString msg ){

        //need to display OHLCVT data T being timestamp in a QTable        
        QJsonDocument jdoc = QJsonDocument::fromJson(msg.toUtf8());   //turns QString to a json document

        QStringList candle;

        //shift rows before inserting new row
        if(jdoc["k"]["x"] == true) {

            Helper::shiftRows(ui->tableWidget);

            //constructing my new row
            QStringList liste = {"o", "h", "l", "c", "v"};

            //add the time as the first column
            QTime* time = new QTime();
            QString current_time = time->currentTime().toString(QString("hh:mm:ss.zzz"));
            QTableWidgetItem* item = new QTableWidgetItem(current_time);
            Helper::readOnly(item);
            ui->tableWidget->setItem(0, 0, item);


            //add the rest
            for(int i = 0; i < liste.size(); ++i) {
                item = new QTableWidgetItem(jdoc["k"][liste.at(i)].toString());

                //append ohlcv data to my candle list
                candle << jdoc["k"][liste.at(i)].toString();

                //make item read only
                Helper::readOnly(item);
                ui->tableWidget->setItem(0, i+1, item);
            }

            //write the candles data to a file

            std::ofstream inputCsv("/home/ghait/Desktop/inputCsv.csv");

            //write candle data
            for(int i = 0; i < candle.size(); ++i) {
                inputCsv << candle.at(i).toStdString() << ",";
            }
            inputCsv.close();

            //start the python process for the prediction

            QProcess process1;
            QStringList parameters1;

            process1.setWorkingDirectory(QString("/home/ghait/Desktop"));

            parameters1 << "pred.py";

            process1.start("python3", parameters1);

            process1.waitForFinished(-1);

            //read the prediction from outputCsv

            std::ifstream outputCsv("/home/ghait/Desktop/outputCsv.txt");
            std::string prediction;
            while(getline(outputCsv, prediction)) {

                std::cout << prediction << std::endl;
            }
        }

    });
}


void ctrader::on_pushButton_2_clicked() {

    //send a message when the connection is closed
    connect(&_websocket, &QWebSocket::disconnected,
            [=]() { Helper::print(ui,QString("connection Closed !")); });

    //close the connection
    _websocket.close();
}


void ctrader::on_pushButton_3_clicked()
{
    //Bridging the python and c++ scripts
    //execute the python script

    QProcess process;
    QStringList parameters;

    process.setWorkingDirectory(QString("/home/ghait/Desktop/"));

    parameters << "mltrader.py";

    process.start("python3", parameters);

    process.waitForFinished(-1);

    QPixmap pix("/home/ghait/Desktop/model_data.png");

    ui->label->setPixmap(pix);

    //python file writes the output to output.txt and then I read it with c++
    //that's a good way to transfer even data structures between python and c++

    std::ifstream output("/home/ghait/Desktop/output.txt");
    std::string text;
    while(getline(output, text)) {
        std::cout << text << std::endl;
    }
    output.close();
}



