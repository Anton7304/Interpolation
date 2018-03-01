#include "widget.h"
#include "ui_widget.h"
#include <qdebug.h>
#include "math.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    _type = 0;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::getPtsFromFile1()//получение списка точек из файла1
{
    listOfArgs.clear();//очистка списка аргументов
    listOfPoints.clear();//очистка списка точек
    QJsonObject ptObj,ptObj2;
    QJsonValue ptVal,ptVal2;
    QString str;
    QStringList stl1;
    QFile file("file1.txt");
    if( !file.open(QIODevice::ReadOnly) )
    {
        qDebug()<<"Error! Input file1.txt is not open";
    }
    else
    {
        qDebug()<<"Input file1.txt is open. Read file...";
    }
    QByteArray saveData = file.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonArray ptsArray = loadDoc.array();
    for(int i = 1; i<ptsArray.count(); i++)//2
    {
        ptVal=ptsArray.at(i);
        ptObj=ptVal.toObject();
        read(ptObj);
        str=str+","+_m_ptR;
    }
    stl1=str.split(",", QString::SkipEmptyParts);
    ptVal=ptsArray.at(1);
    ptObj=ptVal.toObject();

    ptVal2=ptsArray.at(0);
    ptObj2=ptVal2.toObject();
    read(ptObj2);
    //qDebug()<<_cnt;
    _count = _cnt;
//    qDebug()<<stl1;
    for(int i = 0; i<stl1.count();i+=2)
    {
        QPointF pt;
        pt.setX(stl1[i].toFloat());
        pt.setY(stl1[i+1].toFloat());
        listOfPoints.push_back(pt);
//        qDebug()<<"Point"<<i<<"="<<pt;
    }
    //qDebug()<<listOfPoints;
    getArgsFromFile2();
}

void Widget::getArgsFromFile2()//получение аргументов из файла1
{
    QJsonObject ptObj,ptObj2;
    QJsonValue ptVal,ptVal2;
    QFile file("file2.txt");
    if( !file.open(QIODevice::ReadOnly) )
    {
        qDebug()<<"Error! Input file2.txt is not open";
    }
    else
    {
        qDebug()<<"Input file2.txt is open. Read file...";
    }
    QByteArray saveData = file.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonArray ptsArray = loadDoc.array();
    for(int i = 1; i<ptsArray.count(); i++)//2
    {
        ptVal=ptsArray.at(i);
        ptObj=ptVal.toObject();
        read(ptObj);
        listOfArgs.push_back(_arg);
    }
    ptVal=ptsArray.at(1);
    ptObj=ptVal.toObject();

    ptVal2=ptsArray.at(0);
    ptObj2=ptVal2.toObject();
    read(ptObj2);
//    qDebug()<<_size;
    //qDebug()<<listOfArgs;
    calculate();
}

void Widget::calculate()//расчет интерполяции
{
    for(int k = 0; k<listOfArgs.count();k++)
    {
        QList<float> listOfArguments;
        float x = listOfArgs[k];
        float y = 0;
        for(int i = 0; i<listOfPoints.count();i++)
        {
            listOfArguments.push_back( listOfPoints[i].x() );
        }
        listOfArguments.push_back( x );
        std::sort(listOfArguments.begin(),listOfArguments.end());
        //qDebug()<<listOfArguments;
        for(int i = 0; i<listOfArguments.count();i++)
        {
            if(listOfArguments[i] == x && i>0 && i<listOfArguments.count()-1)
            {
                if(_type == 1)//ступенчатая
                {
                    qDebug()<<"Calculation of stepwise interpolation for the argument"<<x;
                    y = stepwise(i,x);
                    listOfPoints.insert( i, QPointF( x, y ) );
                }
                else if(_type == 2)//линейная
                {
                    qDebug()<<"Calculation of linear interpolation for the argument"<<x;
                    y = linear(i,x);
                    listOfPoints.insert( i, QPointF( x, y ) );
                }
                else if(_type == 3 && i>1)//квадратичная
                {
                    qDebug()<<"Calculation of quadratic interpolation for the argument"<<x;
                    y = quadratic(i,x);
                    listOfPoints.insert( i, QPointF( x, y ) );
                }
                else if(_type == 3 && i==1)
                {
                    qDebug()<<"Warning! Argument"<<x
                           << "is out of range for quadratic interpolation! This argument was not interpolated.";
                    _size--;
                }

            }
            //защита от аргументов, которые не входят в диапазон точек, для которых выполняется интерполяция:
            else if( ( listOfArguments[i] == x && i==0 ) ||
                     ( listOfArguments[i] == x && i==listOfArguments.count()-1 ) )
            {
                qDebug()<<"Warning! Argument"<<x<< "outside the quadratic interpolation range! This argument was not interpolated.";
                _size--;
            }
        }
    }
    writeToFile3();
}

void Widget::writeToFile3()//запись результата расчетов в файл3
{
//    qDebug()<<"cnt="<<_cnt;
    QJsonArray ptsArray;
    QFile file("file3.txt");
    if( !file.open( QIODevice::WriteOnly ) )
    {
        qDebug()<<"Error saving file3.txt";
    }
    else
    {
        qDebug()<<"Saving output file3.txt...";
    }
    QJsonObject ptObj,ptObj2;
    ptObj2["Count"]=_count+_size;//method 1
    ptsArray.append(ptObj2);
    for(int i=0; i<listOfPoints.count();i++)
    {
        _m_ptW=QString::number(listOfPoints[i].x())+","+QString::number(listOfPoints[i].y());
        write(ptObj);//method 2
        ptsArray.append(ptObj);
    }
    QJsonDocument saveDoc(ptsArray);
    file.write(saveDoc.toJson());
    qDebug()<<"Done! Output file3.txt saved.";
}

void Widget::read(const QJsonObject &json)//чтение из файла
{
    _m_ptR=json["Point"].toString();
    _cnt=json["Count"].toInt();
    _size=json["Size"].toInt();
    _arg=json["Argument"].toDouble();
}

void Widget::write(QJsonObject &json) const//запись в файл
{
    json[ "Point" ] = _m_ptW;
}

float Widget::stepwise(int i,float x)//функция расчта ступенчатой интерполяции
{
    float y = 0;
    if( ( x-listOfPoints[i-1].x() ) < ( listOfPoints[i].x()-x ) )
    {
        y = listOfPoints[i-1].y();
    }
    else
    {
        y = listOfPoints[i].y();
    }
    return y;
}

float Widget::linear(int i, float x)//функция расчта линейной интерполяции
{
    //f(X1)-( f(X1) - f(X2) )*(X - X1)/(X2 - X1)
    float y = 0;
    y = listOfPoints[i-1].y()-( listOfPoints[i-1].y() - listOfPoints[i].y() )
            * ( x - listOfPoints[i-1].x() )/( listOfPoints[i].x() - listOfPoints[i-1].x() );
    return y;
}

float Widget::quadratic(int i, float x)//функция расчта квадратичной интерполяции
{
    //a = ((y3 - y1)(x2 - x1) - (y2 - y1)(x3 - x1)) / ((x3^2 - x1^2)(x2 - x1) - (x2^2 - x1^2)(x3 - x1))
    //b = (y2 - y1 - a(x2^2 - x1^2)) / (x2 - x1)
    //c = y1 - (ax1^2 + bx1)
    //1     listOfPoints[i-2].x()   listOfPoints[i-2].y()
    //2     listOfPoints[i-1].x()   listOfPoints[i-1].y()
    //3     listOfPoints[i].x()     listOfPoints[i].y()
    float y = 0;
    float a = 0;
    float b = 0;
    float c = 0;
    a = ( (listOfPoints[i].y() - listOfPoints[i-2].y())*(listOfPoints[i-1].x() - listOfPoints[i-2].x() )
            - (listOfPoints[i-1].y() - listOfPoints[i-2].y())*(listOfPoints[i].x() - listOfPoints[i-2].x() ) )
            / ( (pow(listOfPoints[i].x(), 2) - pow(listOfPoints[i-2].x(), 2) )*(listOfPoints[i-1].x() - listOfPoints[i-2].x())
            - ( pow(listOfPoints[i-1].x(), 2) - pow(listOfPoints[i-2].x(), 2) )*(listOfPoints[i].x() - listOfPoints[i-2].x()) );
    b = (listOfPoints[i-1].y() - listOfPoints[i-2].y() - a*( pow(listOfPoints[i-1].x(), 2) - pow(listOfPoints[i-2].x(), 2) ) )
            / (listOfPoints[i-1].x() - listOfPoints[i-2].x() );
    c = listOfPoints[i-2].y() - (a*pow(listOfPoints[i-2].x(), 2) + b*listOfPoints[i-2].x());
    y = a*(x*x)+b*x+c;
    return y;
}

void Widget::on_pushButton_clicked()//выбор(переключение) типа интерполяции
{
    _type = ui->comboBox->currentIndex();
    if(_type>0)
    {
        getPtsFromFile1();
    }
}
