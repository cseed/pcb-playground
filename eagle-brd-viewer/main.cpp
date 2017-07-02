
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <map>

#include <QtWidgets>

#include "pugixml.hpp"

#include "netlist.hpp"
#include "main.hpp"

#define fmt(x) (static_cast<const std::ostringstream&>(std::ostringstream{} << x).str())

PCBScene::PCBScene() {}

void
PCBScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    qreal grid_size = 0.5;
    
    QLineF l = painter->transform().map(QLineF(0, 0, 0, 1));
    qreal scale = l.length();
    
    if (scale < 10)
	return;
    
    // std::cout << "in drawBackgrond " << scale << "\n";
    
    // FIXME why -1?
    qreal top = rect.top() - 1,
	bottom = rect.bottom();
    
    painter->save();
    painter->setPen(QPen(Qt::lightGray, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (int i = -10; i <= 10; i ++)
	{
	    painter->drawLine(grid_size*(qreal)i, top,
			      grid_size*(qreal)i, bottom);
	}
    painter->restore();
}

Via::Via(qreal x, qreal y, qreal diameter_, qreal drill_)
    : diameter(diameter_),
      drill(drill_)
{
    this->setX(x);
    this->setY(y);
}

QRectF
Via::boundingRect() const
{
    qreal h = diameter/2;
    return QRectF(-h, -h, diameter, diameter);
}

void
Via::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	   QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->save();
    painter->setBrush(QBrush(Qt::green, Qt::SolidPattern));
    painter->setPen(QPen(Qt::NoPen));
    QPainterPath path;
    path.addEllipse(QPointF(0, 0), diameter/2, diameter/2);
    path.addEllipse(QPointF(0, 0), drill/2, drill/2);
    painter->drawPath(path);
    painter->restore();
}

GroupItem::GroupItem(qreal x, qreal y)
{
    this->setX(x);
    this->setY(y);
}

QRectF
GroupItem::boundingRect() const
{
    return QRectF();
}

void
GroupItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		 QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

Text::Text(qreal x, qreal y, double size_, std::string text_)
    : size(size_),
      text(text_)
{
    this->setX(x);
    this->setY(y);
}

QRectF
Text::boundingRect() const
{
    // FIXME
    return QRectF(-10, 10, 10, 10);
}

void
Text::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		 QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->drawText(0, 0, 10, size, 0, text.c_str(), nullptr);
}

MainWindow::MainWindow()
{
  setWindowTitle("Simple Application");
  
#if 0
  QLabel *label = new QLabel("Press to quit:");
  QPushButton *button = new QPushButton("Exit");
  
  QHBoxLayout *layout = new QHBoxLayout();
  layout->addWidget(label);
  layout->addWidget(button);
  
  central->setLayout(layout);
#endif

  scene = new PCBScene;
  
  View *central = new View(scene);
  // central->setMouseTracking(true);
  
  central->setRenderHint(QPainter::Antialiasing);
  // central->setDragMode(QGraphicsView::ScrollHandDrag);
  central->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  
  
  setCentralWidget(central);
  
  QAction *quitAct = new QAction(tr("&Quit"), this);
  quitAct->setShortcuts(QKeySequence::Quit);
  connect(quitAct, SIGNAL(triggered()), this, SLOT(quit()));
  
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(quitAct);
  
  QAction *zoomInAct = new QAction(tr("Zoom &In"), this);
  zoomInAct->setShortcut(QKeySequence::ZoomIn);
  connect(zoomInAct, SIGNAL(triggered()), central, SLOT(zoomIn()));
  
  QAction *zoomOutAct = new QAction(tr("Zoom &Out"), this);
  zoomOutAct->setShortcut(QKeySequence::ZoomOut);
  connect(zoomOutAct, SIGNAL(triggered()), central, SLOT(zoomOut()));
  
  QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(zoomInAct);
  viewMenu->addAction(zoomOutAct);
}

void
MainWindow::quit()
{
  exit(EXIT_SUCCESS);
}

View::View(QGraphicsScene *scene)
  : QGraphicsView(scene)
{
}

void View::wheelEvent(QWheelEvent *event)
{
  scaleView(pow((double)2, -event->delta() / 240.0));
}

void
View::scaleView(qreal scaleFactor)
{
  qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
  if (factor < 0.07 || factor > 100)
    return;
  
  scale(scaleFactor, scaleFactor);
}

void View::zoomIn()
{
  scaleView(qreal(1.2));
}

void View::zoomOut()
{
  scaleView(1 / qreal(1.2));
}

QColor
layer_color(int layer)
{
    QColor color = Qt::green;
    if (layer == 1)
	color = Qt::red;
    else if (layer == 2)
	color = Qt::darkRed;
    else if (layer == 15)
	color = Qt::darkBlue;
    else if (layer == 16)
	color = Qt::blue;
    else if (layer == 21 // tPlace
	     || layer == 25 // tNames
	     || layer == 27 // tValues
	     || layer == 51) // tDocu
	color = Qt::gray;
    else if (layer == 20)
	color = Qt::black; // Dimension
    return color;
}

QGraphicsItem *
add_wire(pugi::xml_node node)
{
    int layer = node.attribute("layer").as_int();
    QColor color = layer_color(layer);
    QGraphicsLineItem *line
	= new QGraphicsLineItem((qreal)node.attribute("x1").as_double(),
				-(qreal)node.attribute("y1").as_double(),
				(qreal)node.attribute("x2").as_double(),
				-(qreal)node.attribute("y2").as_double());
    line->setFlag(QGraphicsItem::ItemIsMovable, true);
    line->setPen(QPen(color,
		      (qreal)node.attribute("width").as_double(),
		      Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    line->setZValue(qreal(-layer));
    return line;
}

int
main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  
  // create a window
  MainWindow *window = new MainWindow;
  
  pugi::xml_document brd;
  pugi::xml_parse_result result
      = brd.load_file("example.brd");
  
  std::cout << "Load result: " << result.description() << "\n";
  
  pugi::xml_node plain = brd.child("eagle").child("drawing").child("board").child("signals");
  std::cout << plain.name() << std::endl;
  
  for (auto xwire : brd.select_nodes("/eagle/drawing/board/signals//wire|/eagle/drawing/board/plain//wire"))
      {
	  window->scene->addItem(add_wire(xwire.node()));
      }
  
  for (auto xnode : brd.select_nodes("/eagle/drawing/board/signals//via"))
      {
	  pugi::xml_node node = xnode.node();
	  
	  qreal drill = (qreal)node.attribute("drill").as_double();
	  Via *via = new Via((qreal)node.attribute("x").as_double(),
			     -(qreal)node.attribute("y").as_double(),
			     drill + 0.2032,  // 8mil
			     drill);
	      via->setZValue(1);
	  window->scene->addItem(via);
      }  

  for (auto xtext : brd.select_nodes("/eagle/drawing/board/plain//text"))
      {
	  qreal x = (qreal)xtext.node().attribute("x").as_double(),
	      y = (qreal)xtext.node().attribute("y").as_double(),
	      size = (qreal)xtext.node().attribute("size").as_double();
	  
	  std::string text = xtext.node().text().get();
	  
	  std::cout << text << "\n";
	  
	  QFont f("Times", size);
	  QGraphicsItem *text_item =
	      window->scene->addSimpleText(text.c_str(), f);
	  text_item->setX(x);
	  text_item->setY(-y);
	  
	  // FIXME orientation depends on angle
	  // FIXME alignment
	  std::string rot = xtext.node().attribute("rot").as_string("R0");
	  if (rot == "R0")
	      ;
	  else if (rot == "R90")
	      text_item->setRotation(-90);
	  else if (rot == "R180")
	      text_item->setRotation(-180);
	  else if (rot == "R270")
	      text_item->setRotation(-270);
      }  
  
  for (auto xelement : brd.select_nodes("/eagle/drawing/board/elements//element"))
      {
	  std::string package_name = xelement.node().attribute("package").value();
	  std::string package_query
	      = fmt("/eagle/drawing/board/libraries//package[@name='"
		    << package_name
		    << "']");
	  
	  auto xpackage = brd.select_single_node(package_query.c_str());
	  
	  qreal x = (qreal)xelement.node().attribute("x").as_double(),
	      y = (qreal)xelement.node().attribute("y").as_double();

	  // FIXME package class
	  QGraphicsItem *grp = new GroupItem(x, -y);
	  
	  std::string rot = xelement.node().attribute("rot").as_string("R0");
	  if (rot == "R0")
	      ;
	  else if (rot == "R90")
	      grp->setRotation(-90);
	  else if (rot == "R180")
	      grp->setRotation(-180);
	  else if (rot == "R270")
	      grp->setRotation(-270);
	  
	  for (auto xwire : xpackage.node().select_nodes(".//wire"))
	      {
		  QGraphicsItem *wire = add_wire(xwire.node());
		  wire->setParentItem(grp);
	      }
	  for (auto xsmd : xpackage.node().select_nodes(".//smd"))
	      {
		  qreal x = (qreal)xsmd.node().attribute("x").as_double(),
		      y = (qreal)xsmd.node().attribute("y").as_double(),
		      dx = (qreal)xsmd.node().attribute("dx").as_double(),
		      dy = (qreal)xsmd.node().attribute("dy").as_double();
		  
		  x -= dx/2;
		  y += dy/2;

		  int layer = xsmd.node().attribute("layer").as_int();
		  QColor color = layer_color(layer);
		  
		  int roundness = xsmd.node().attribute("roundness").as_int();
		  if (roundness == 100)
		      {
			  QGraphicsEllipseItem *rect = new QGraphicsEllipseItem(x, -y, dx, dy, grp);
			  rect->setPen(QPen(Qt::NoPen));
			  rect->setBrush(QBrush(color, Qt::SolidPattern));
			  
		      }
		  else
		      {		      
			  QGraphicsRectItem *rect = new QGraphicsRectItem(x, -y, dx, dy, grp);
			  rect->setPen(QPen(Qt::NoPen));
			  rect->setBrush(QBrush(color, Qt::SolidPattern));
		      }
	      }
	  for (auto xpad : xpackage.node().select_nodes(".//pad"))
	      {
		  qreal x = (qreal)xpad.node().attribute("x").as_double(),
		      y = (qreal)xpad.node().attribute("y").as_double(),
		      drill = (qreal)xpad.node().attribute("drill").as_double(),
		      diameter = (qreal)xpad.node().attribute("diameter").as_double();
		  
		  Via *via = new Via(x, -y, diameter, drill);
		  via->setZValue(1);
		  via->setParentItem(grp);
	      }	  
	  window->scene->addItem(grp);
      }
  
  auto top = std::make_shared<Module>("top");
  std::map<std::string, std::shared_ptr<Module>> modules;
  
  for (auto xpackage : brd.select_nodes("/eagle/drawing/board/libraries//package"))
      {
	  std::string name = xpackage.node().attribute("name").value();
	  
	  auto m = std::make_shared<Module>(name);
	  for (auto xport : xpackage.node().select_nodes(".//smd|.//pad"))
	      {
		  std::string port_name = xport.node().attribute("name").value();
		  m->append_port(port_name);
	      }
	  modules.insert(make_pair(name, m));
      }
  
  for (auto xelement : brd.select_nodes("/eagle/drawing/board/elements//element"))
      {
	  // FIXME use library
	  std::string package_name = xelement.node().attribute("package").value();
	  
	  std::string name = xelement.node().attribute("name").value();
	  
	  std::cout << "element name " << name << "\n";
	  
	  std::shared_ptr<Module> m = modules.find(package_name)->second;
	  top->make_instance(m, name);
      }
  
  for (auto xsignal : brd.select_nodes("/eagle/drawing/board/signals//signal"))
      {
	  std::string name = xsignal.node().attribute("name").value();
	  
	  Net *n = top->make_net(name);
	  for (auto xcontactref : xsignal.node().select_nodes(".//contactref"))
	      {
		  std::string element = xcontactref.node().attribute("element").value();
		  Instance *inst = top->find_instance(element);
		  
		  std::string port = xcontactref.node().attribute("pad").value();
		  Port *p = inst->find_port(port);
		  
		  n->connect(p);
	      }
      }
  
  top->as_verilog(std::cout);
  
  // std::cout << "Document:\n";
  // doc.save(std::cout);
  
  window->show();
  
  return app.exec();
}
