#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void getPtsFromFile1();
    void getArgsFromFile2();
    void calculate();
    void writeToFile3();
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;
    float stepwise(int i, float x);
    float linear(int i, float x);
    float quadratic(int i, float x);

private slots:
    void on_pushButton_clicked();

private:
    Ui::Widget *ui;
    QList<QPointF> listOfPoints;
    QList<double> listOfArgs;
    QString _m_ptR;
    QString _m_ptW;
    int _cnt;
    int _count;
    int _size;
    double _arg;
    int _type;
};

#endif // WIDGET_H
