#include "UI/Core.h"
#include "Basic/Block.h"
#include "Basic/Canvas.h"
#include "Basic/Char.h"
#include "Basic/Cursor.h"
#include "Basic/Direction.h"
#include "Basic/Icon.h"
#include "Edit/Mode.h"
#include "UI/Theme.h"
#include <QFontDatabase>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <algorithm>
#include <memory>
#include <type_traits>
#include <unordered_set>

namespace Lin::UI {

Core::Core(Config &cfg, ThemeKind themeKind, QWindow *parent)
    : paintDevice(nullptr), cfg(cfg), QOpenGLWidget(nullptr) {
  theme = std::make_unique<NormalTheme>();

  loadFontAsset();
  setWindowTitle("Lin");
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint); // Window without border
  setAttribute(Qt::WA_InputMethodEnabled, true);

  QSurfaceFormat fmt;
  fmt.setRenderableType(QSurfaceFormat::OpenGL);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  fmt.setVersion(3, 3);
  fmt.setSamples(4);

  if (cfg.isTransparent) {
    qInfo() << "Enable transparent background";
    fmt.setAlphaBufferSize(8); // Configure support for Alpha channel
    setWindowOpacity(cfg.opacity);
  }

  setFormat(fmt);
}

void Core::initializeGL() {
  paintDevice = std::make_unique<QOpenGLPaintDevice>();
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Core::resizeGL(int w, int h) {
  const qreal dpr = devicePixelRatio();
  paintDevice->setSize(QSizeF(width() * dpr, height() * dpr).toSize());
}

void Core::paintGL() {
  QPainter p;
  const qreal dpr = devicePixelRatio();
  paintDevice->setSize(QSizeF(width() * dpr, height() * dpr).toSize());
  p.begin(paintDevice.get());

  p.setWindow(0, 0, width(), height());
  p.setCompositionMode(QPainter::CompositionMode_Source);

  QRect rect(0, 0, width(), height());
  if (cfg.isTransparent)
    p.fillRect(rect, QColor(30, 30, 46, 150));

  p.setCompositionMode(QPainter::CompositionMode_SourceOver);

  QFontMetrics fm(theme->normalfont.english);
  const qreal canvasWidth = Canvas::Width * fm.horizontalAdvance('M');
  const qreal canvasHeight = Canvas::Height * fm.height();
  p.translate((width() - canvasWidth) / 2.0, (height() - canvasHeight) / 2.0);

  if (rd.mode != Mode::Insert && rd.mode != Mode::Lin)
    drawBackground(p);

  if (rd.mode == Mode::Visual || rd.mode == Mode::Lin)
    drawVisualSelection(p);

  Canvas *t = rd.tmpCanvas;
  quint64 operatingBlockId = -1;

  if (rd.mode == Mode::Insert && rd.block) {
    drawBlock(p, &(*rd.block));
    operatingBlockId = (*rd.block).getId();
  }

  // Draw(down) the characters of all blocks
  for (const auto &[_, block] : rd.mainCanvas->getBlocks()) {
    if (rd.mode == Mode::Insert && operatingBlockId == block.getId())
      continue;

    drawBlock(p, &block);
  }

  if (rd.mode == Mode::Lin) {
    for (const auto &[_, block] : rd.tmpCanvas->getBlocks())
      drawBlock(p, &block);
  }

  drawCursor(p);
  p.end();
}

void Core::drawVisualSelection(QPainter &p) {
  if (!rd.visualRegion)
    return;

  Canvas *selectionCanvas =
      rd.mode == Mode::Lin ? rd.tmpCanvas : rd.mainCanvas;
  QFontMetrics fm(theme->normalfont.english);
  const int charW = fm.horizontalAdvance('M');
  const int charH = fm.height();
  const QColor selectionColor(255, 255, 255, 45);

  std::visit(
      [&](const auto &selection) {
        using T = std::decay_t<decltype(selection)>;
        if constexpr (std::is_same_v<T, quint64>) {
          for (int y = 1; y <= Canvas::Height; ++y)
            for (int x = 1; x <= Canvas::Width; ++x)
              if (selectionCanvas->getBlockId({x, y}) == selection)
                p.fillRect(
                    QRect((x - 1) * charW, (y - 1) * charH, charW, charH),
                    selectionColor);
        } else if constexpr (std::is_same_v<T, std::vector<quint64>>) {
          const std::unordered_set<quint64> selectedIds(selection.begin(),
                                                        selection.end());
          for (int y = 1; y <= Canvas::Height; ++y)
            for (int x = 1; x <= Canvas::Width; ++x)
              if (selectedIds.find(selectionCanvas->getBlockId({x, y})) !=
                  selectedIds.end())
                p.fillRect(
                    QRect((x - 1) * charW, (y - 1) * charH, charW, charH),
                    selectionColor);
        } else {
          if (!rd.visualRectangle) {
            const Block *block = selectionCanvas->getBlock(selection.first);
            if (!block)
              return;

            const auto &text = block->getText();
            const QPoint anchor = block->getAnchor();
            auto indexOf = [&](QPoint point) {
              int index = 0;
              for (int row = 0; row < point.y() - anchor.y(); ++row)
                index += static_cast<int>(text[row].size());
              return index + point.x() - anchor.x();
            };

            int first = indexOf(selection.first);
            int last = indexOf(selection.second);
            if (first > last)
              std::swap(first, last);

            int index = 0;
            for (int row = 0; row < static_cast<int>(text.size()); ++row) {
              for (int col = 0; col < static_cast<int>(text[row].size());
                   ++col, ++index) {
                if (index < first || index > last)
                  continue;
                const int x = anchor.x() + col;
                const int y = anchor.y() + row;
                p.fillRect(
                    QRect((x - 1) * charW, (y - 1) * charH, charW, charH),
                    selectionColor);
              }
            }
            return;
          }

          const int left = std::min(selection.first.x(), selection.second.x());
          const int right = std::max(selection.first.x(), selection.second.x());
          const int top = std::min(selection.first.y(), selection.second.y());
          const int bottom =
              std::max(selection.first.y(), selection.second.y());
          p.fillRect(QRect((left - 1) * charW, (top - 1) * charH,
                           (right - left + 1) * charW,
                           (bottom - top + 1) * charH),
                     selectionColor);
        }
      },
      *rd.visualRegion);
}

void Core::keyPressEvent(QKeyEvent *e) {
  emit sendEvent(e, nullptr);
  e->accept();
}

QVariant Core::inputMethodQuery(Qt::InputMethodQuery query) const {
  switch (query) {
  case Qt::ImEnabled:
    return rd.mode == Mode::Insert;
  case Qt::ImCursorRectangle: {
    QFontMetrics fm(theme->normalfont.english);
    QPoint cursor = cursorPos();
    const int charW = fm.horizontalAdvance('M');
    const int charH = fm.height();
    return QRect(cursor.x(), cursor.y(), charW, charH);
  }
  default:
    break;
  }

  return QOpenGLWidget::inputMethodQuery(query);
}

bool Core::event(QEvent *e) {
  switch (e->type()) {
  case QEvent::InputMethod:
    emit sendEvent(nullptr, static_cast<QInputMethodEvent *>(e));
    e->accept();
    break;
  default:
    break;
  }

  return QOpenGLWidget::event(e);
}

void Core::update(RenderData rd) {
  this->rd = rd;
  QGuiApplication::inputMethod()->update(Qt::ImCursorRectangle);
  QOpenGLWidget::update();
}

void Core::loadFontAsset() {
  const QStringList fontPaths = {
      ":/fonts/chinese/yezigongchangboboti.ttf",
      ":/fonts/english/RecMonoCasualNerdFont-Bold.ttf",
      ":/fonts/english/RecMonoCasualNerdFont-BoldItalic.ttf",
      ":/fonts/english/RecMonoCasualNerdFont-Italic.ttf",
      ":/fonts/english/RecMonoCasualNerdFont-Regular.ttf",
  };

  for (const QString &path : fontPaths) {
    int id = QFontDatabase::addApplicationFont(path);
    if (id == -1) {
      qWarning() << "Failed to load font:" << path;
    } else {
      QStringList families = QFontDatabase::applicationFontFamilies(id);
      if (!families.isEmpty())
        qInfo() << "Loaded font:" << families.first();
    }
  }
}

void Core::changeTheme(ThemeKind themeKind) {
  switch (themeKind) {
  case ThemeKind::Normal:
    theme = std::make_unique<NormalTheme>();
  }

  update(rd);
}

QFont Core::chooseFontFamily(const Char &ch) {
  if (IsChinese(CharToString(ch))) {
    return theme->normalfont.chinese;
  } else if (IsIcon(CharToString(ch))) {
    return theme->normalfont.icon;
  }

  return theme->normalfont.english;
}

void Core::drawCursor(QPainter &p) {
  const QFontMetrics fm(theme->normalfont.icon);
  const int charW = fm.horizontalAdvance('M');
  const int charH = fm.height();

  p.setFont(theme->normalfont.icon);
  p.setPen(theme->color.cursor);

  const QPoint pos = cursorPos();

  if (rd.mode == Mode::Insert) {
    p.fillRect(QRect(pos.x(), pos.y(), 2, charH), theme->color.cursor);
    return;
  }

  const QPoint cursor = rd.mainCanvas->getCursorPos();
  const Block *block = rd.mainCanvas->getBlock(cursor);
  if (!block || !block->onBlock(cursor)) {
    p.drawText(pos.x(), pos.y() + fm.ascent(), CharToString(Icons["Dino"]));
    return;
  }

  const Char ch = block->getRawChar(cursor);
  const bool isPlaceholder = IsPlaceHolder(ch);
  const bool isDoubleWidth =
      IsDoubleWidth(ch) ||
      (isPlaceholder && cursor.x() > block->getAnchor().x() &&
       IsDoubleWidth(block->getRawChar(cursor - QPoint(1, 0))));
  const int cursorWidth = isDoubleWidth ? charW * 2 : charW;
  const int cursorX = pos.x() - (isPlaceholder ? charW : 0);
  p.fillRect(QRect(cursorX, pos.y(), cursorWidth, charH), theme->color.cursor);
}

// Coloring the Canvas using a four-color algorithm
void Core::drawBackground(QPainter &p) {
  // Build an adjacency list
  std::unordered_map<quint64, std::unordered_set<quint64>> adjacency;
  for (int y = 1; y <= Canvas::Height; ++y) {
    for (int x = 1; x <= Canvas::Width; ++x) {
      quint64 g = rd.mainCanvas->getGroupId({x, y});
      const QPoint dirs[4] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
      for (auto d : dirs) {
        int nx = x + d.x();
        int ny = y + d.y();

        if (nx <= 0 || nx > Canvas::Width || ny <= 0 || ny > Canvas::Height)
          continue;

        quint64 ng = rd.mainCanvas->getGroupId({nx, ny});
        if (ng != g) {
          adjacency[g].insert(ng);
          adjacency[ng].insert(g);
        }
      }
    }
  }

  // Greedy coloring
  std::unordered_map<quint64, int> groupColor;
  for (auto &[gid, neighbors] : adjacency) {
    std::set<int> used;
    for (auto n : neighbors) {
      if (groupColor.count(n)) {
        used.insert(groupColor[n]);
      }
    }

    for (int c = 0; c < 4; ++c) {
      if (!used.count(c)) {
        groupColor[gid] = c;
        break;
      }
    }
  }

  QFontMetrics fm(theme->normalfont.english);
  int charW = fm.horizontalAdvance('M');
  int charH = fm.height();

  for (int y = 1; y <= Canvas::Height; ++y) {
    for (int x = 1; x <= Canvas::Width; ++x) {
      quint64 g = rd.mainCanvas->getGroupId(QPoint(x, y));
      int c = groupColor[g];
      QColor colors[4] = {
          QColor(70, 130, 180, 80),  // Steel Blue
          QColor(100, 149, 237, 80), // Cornflower Blue
          QColor(123, 104, 238, 80), // Medium Slate Blue
          QColor(106, 90, 205, 80)   // Slate Blue
      };

      p.fillRect(QRect((x - 1) * charW, (y - 1) * charH, charW, charH),
                 colors[c]);
    }
  }
}

void Core::drawBlock(QPainter &p, const Block *b) {
  const auto &text = b->getText();
  QPoint anchor = b->getAnchor();

  QFontMetrics fm(theme->normalfont.english);
  int charW = fm.horizontalAdvance('M');
  int charH = fm.height();

  for (int row = 0; row < static_cast<int>(text.size()); ++row) {
    const auto &line = text[row];
    int logicCol = 0;

    for (int col = 0; col < static_cast<int>(line.size()); ++col) {
      const Char &ch = line[col];
      if (IsPlaceHolder(ch)) {
        logicCol++;
        continue;
      }

      int x = (anchor.x() - 1 + logicCol) * charW;
      int y = (anchor.y() - 1 + row) * charH;

      p.setFont(chooseFontFamily(ch));
      p.setPen(theme->color.font);
      p.drawText(x, y + fm.ascent(), CharToString(ch));

      logicCol++;
    }
  }
}

QPoint Core::cursorPos() const {
  QFontMetrics fm(theme->normalfont.icon);
  const int charW = fm.horizontalAdvance('M');
  const int charH = fm.height();

  const QPoint cursor =
      (rd.mode == Mode::Insert || rd.mode == Mode::Lin)
          ? rd.tmpCanvas->getCursorPos()
          : rd.mainCanvas->getCursorPos();

  int cursorX = cursor.x() - 1;
  int cursorY = cursor.y() - 1;
  if (rd.mode == Mode::Insert && rd.ep == Backward) {
    if (rd.direction == Left2Right) {
      cursorX = cursor.x();
      cursorY = cursor.y() - 1;
    } else {
      cursorX = cursor.x() - 1;
      cursorY = cursor.y();
    }
  }

  return {cursorX * charW, cursorY * charH};
}

} // namespace Lin::UI
