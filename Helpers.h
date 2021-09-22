#ifndef HELPERS_H
#define HELPERS_H

#include<QJsonObject>
#include<QTableWidget>
#include<sstream>
#include<iostream>
#include<ctime>
#include<cmath>


class ctrader;

namespace Helper {

    int ROW_COUNT = 10;
    int COLUMN_COUNT = 6;


    void print(Ui::ctrader* trader, QString msg) {
        trader->textBrowser->append(msg);
    }


    void shiftRows(QTableWidget* table) {
        table->removeRow(table->rowCount() - 1);
        table->insertRow(0);

    }

    void readOnly(QTableWidgetItem* item) {
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setFlags(item->flags() ^ Qt::ItemIsSelectable);
    }

    std::string** processData(std::string& str) {

        str = str.substr(1,str.size()-2);
        std::string** array2 = new std::string*[ROW_COUNT];

        std::istringstream str2(str);
        std::string s;
        int i = 0;
        while(std::getline(str2, s, ']')) {

            array2[i] = new std::string[12];
            std::istringstream ss(i == 0 ? s.substr(1,s.size()) : s.substr(2,s.size())); //remove the [
            std::string s2;
            int j = 0;
            while(std::getline(ss, s2, ',')) {
                array2[i][j] = j != 0 ? s2.substr(1,s2.size()-2) : s2;      //remove the " "
                j++;
            }
            i++;
        }
        for(int i = 0; i < ROW_COUNT; ++i) {

            long timeStamp = stol(array2[i][0])/1000;
            std::time_t time = timeStamp;
            array2[i][0] = ctime(&time);
            std::istringstream ss(array2[i][0]);
            std::string str;
            int counter = 0;
            while(std::getline(ss, str,' ')) {
                if(counter == 3) {                  //get the third element when the string is splitted
                    array2[i][0] = str;
                }
                counter++;
            }
        }
        return array2;
    }


}





#endif // HELPERS_H
