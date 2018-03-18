#ifndef BMW_H
#define BMW_H
//内部圆半径比例因子
#define RADIUS_FACTOR 0.8

//外部圆开始和停止颜色
#define OUTER_CIRCLE_START_COLOR QColor(65,65,65)
#define OUTER_CIRCLE_END_COLOR QColor(89,89,89)

//内部圆的蓝色
#define BLUE_CIRCLE_START_COLOR QColor(0,133,203)
#define BLUE_CIRCLE_END_COLOR QColor(0,118,177)
//内部圆的白色
#define WHITE_CIRCLE_START_COLOR QColor(255,255,255)
#define WHITE_CIRCLE_END_COLOR QColor(233,233,233)

#include <QWidget>
#include <QtGui>

class myBMW : public QWidget
{
    Q_OBJECT
public:
    explicit myBMW(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);

    //重写sizeHint()
    QSize sizeHint() const
    {
      return QSize(300,300);
    }

private:
    void drawUnderCircle(QPainter* painter);//画外圆

    void drawBMW(QPainter* painter);//画宝马

private:
    QTimer* m_updateTimer;//定时器时间

    qreal   m_angle;    //旋转角度
    qreal   m_outerRadius;//外半径

private slots:
    void UpdateAngle();//自定义槽，更新角度旋转

};


#endif // BMW_H
