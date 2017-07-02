
#include <QtWidgets>

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  QGraphicsScene *scene;
    
public slots:
  void quit();
  
public:
  MainWindow();
};

class PCBScene : public QGraphicsScene
{
    Q_OBJECT
    
public:
    PCBScene();
    
    void drawBackground(QPainter *painter, const QRectF &rect);
};

class Via : public QGraphicsItem
{
    qreal diameter;
    qreal drill;
    
public:
    Via(qreal x, qreal y, qreal diameter_, qreal drill_);
    
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	       QWidget *widget = 0);
};

class GroupItem : public QGraphicsItem
{
public:
    GroupItem(qreal x, qreal y);
    
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	       QWidget *widget = 0);
};

class Text : public QAbstractGraphicsShapeItem
{
    qreal size;
    std::string text;
    
public:
    Text(qreal x, qreal y, qreal size_, std::string text_);
    
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	       QWidget *widget = 0);
};

class View : public QGraphicsView
{
  Q_OBJECT
    
public slots:
  void zoomIn();
  void zoomOut();
  
public:
  View(QGraphicsScene *scene);
  
protected:
  void scaleView(qreal scaleFactor);
  
  // void mouseMoveEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);
};
