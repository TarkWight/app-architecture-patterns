#include "QtPdfExporter.hpp"

#include "../Ui/Render/PlotRenderer.hpp"

#include <QFont>
#include <QFontMetrics>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QString>

namespace {

QString toQString(const std::string &value) {
    return QString::fromStdString(value);
}

struct VerticalCursor {
    int left{0};
    int top{0};
    int width{0};
};

void drawParagraph(QPainter &painter, VerticalCursor &cursor, const QString &text, int lineSpacing = 28) {
    const QRect rect(cursor.left, cursor.top, cursor.width, 1000);
    painter.drawText(rect, Qt::TextWordWrap, text);

    const QFontMetrics metrics(painter.font());
    const QRect usedRect = metrics.boundingRect(QRect(0, 0, cursor.width, 1000), Qt::TextWordWrap, text);

    cursor.top += usedRect.height() + lineSpacing;
}

void drawPlot(QPainter &painter, VerticalCursor &cursor, int height, const domain::PlotModel &plot) {
    const QRect rect(cursor.left, cursor.top, cursor.width, height);
    ui::render::PlotRenderer::drawPlot(painter, rect, plot);
    cursor.top += height + 20;
}

} // namespace

namespace infrastructure {

void QtPdfExporter::exportDocument(const application::dto::PdfDocumentModel &document, const std::string &filePath) {
    QPdfWriter writer(toQString(filePath));
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(20.0, 20.0, 20.0, 20.0));

    QPainter painter(&writer);
    if (!painter.isActive()) {
        return;
    }

    VerticalCursor cursor{.left = 100, .top = 120, .width = 2200};

    QFont titleFont("Times New Roman", 18, QFont::Bold);
    painter.setFont(titleFont);
    drawParagraph(painter, cursor, toQString(document.title), 40);

    QFont textFont("Times New Roman", 12);
    painter.setFont(textFont);

    QString firstStanza;
    for (const auto &line : document.firstStanza) {
        firstStanza += toQString(line);
        firstStanza += "\n";
    }
    drawParagraph(painter, cursor, firstStanza, 30);

    drawPlot(painter, cursor, 500, document.plot1);
    drawParagraph(painter, cursor, toQString(document.plot1Caption), 40);

    drawPlot(painter, cursor, 500, document.plot2);
    drawParagraph(painter, cursor, toQString(document.plot2Caption), 40);

    QString secondStanza;
    for (const auto &line : document.secondStanza) {
        secondStanza += toQString(line);
        secondStanza += "\n";
    }
    drawParagraph(painter, cursor, secondStanza, 30);

    painter.end();
}

} // namespace infrastructure
