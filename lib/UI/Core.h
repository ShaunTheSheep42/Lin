#pragma once

#include "Lin/Config.h"
#include "Lin/RenderData.h"
#include "UI/Theme.h"
#include <QFont>
#include <QInputEvent>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QString>
#include <QVariant>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <memory>

namespace Lin::UI {

class Core : public QOpenGLWidget {
  Q_OBJECT
public:
  QWidget *w;
  explicit Core(Config &cfg, ThemeKind themeKind, QWindow *parent = nullptr);

  ~Core() = default;

  Q_SLOT void update(RenderData rd);
  Q_SIGNAL void sendEvent(QKeyEvent *eK, QInputMethodEvent *eI);

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  void keyPressEvent(QKeyEvent *e) override;
  bool event(QEvent *e) override;
  QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

private:
  void changeTheme(ThemeKind themeKind);
  void loadFontAsset();
  QFont chooseFontFamily(const Char &ch);

  QPoint cursorPos() const;
  void drawBlock(QPainter &p, const Block *b);
  void drawVisualSelection(QPainter &p);
  void drawCursor(QPainter &p);
  void drawBackground(QPainter &p);

  Config &cfg;

  std::unique_ptr<Theme> theme;

  RenderData rd;

  std::unique_ptr<QOpenGLPaintDevice> paintDevice;
};

} // namespace Lin::UI
