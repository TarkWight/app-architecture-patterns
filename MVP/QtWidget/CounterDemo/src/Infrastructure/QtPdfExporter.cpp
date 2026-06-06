#include "QtPdfExporter.hpp"

#include "../Ui/Render/PlotRenderer.hpp"

#include <QFont>
#include <QFontMetrics>
#include <QMarginsF>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPen>
#include <QString>

#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace {

constexpr int pageMargin = 180;
constexpr int paragraphSpacing = 38;
constexpr int sectionSpacing = 64;

QString toQString(const std::string &value) {
    return QString::fromStdString(value);
}

QString withFallback(const std::string &value, const QString &fallback) {
    return value.empty() ? fallback : toQString(value);
}

struct PdfPage {
    int width{0};
    int height{0};
    int left{0};
    int top{0};
    int contentWidth{0};
};

struct VerticalCursor {
    int y{0};
};

struct DrawingContext {
    QPainter &painter;
    PdfPage page;
};

PdfPage pageSpec(const QPdfWriter &writer) {
    return PdfPage{.width = writer.width(),
                   .height = writer.height(),
                   .left = pageMargin,
                   .top = pageMargin,
                   .contentWidth = writer.width() - (2 * pageMargin)};
}

QFont reportFont(int pointSize, QFont::Weight weight = QFont::Normal) {
    return QFont("Times New Roman", pointSize, weight);
}

void drawCenteredLine(DrawingContext &context, VerticalCursor &cursor, const QString &text, int height = 55) {
    context.painter.drawText(QRect(context.page.left, cursor.y, context.page.contentWidth, height),
                             Qt::AlignCenter | Qt::TextWordWrap, text);
    cursor.y += height;
}

void drawParagraph(DrawingContext &context, VerticalCursor &cursor, const QString &text,
                   int spacing = paragraphSpacing) {
    const QRect available(context.page.left, cursor.y, context.page.contentWidth,
                          context.page.height - cursor.y - pageMargin);
    context.painter.drawText(available, Qt::AlignLeft | Qt::TextWordWrap, text);

    const QFontMetrics metrics(context.painter.font());
    const QRect usedRect =
        metrics.boundingRect(QRect(0, 0, context.page.contentWidth, context.page.height), Qt::TextWordWrap, text);
    cursor.y += usedRect.height() + spacing;
}

void drawField(DrawingContext &context, VerticalCursor &cursor, const QString &label, const QString &value) {
    drawParagraph(context, cursor, label + value, 28);
}

void drawTitlePage(DrawingContext &context, const application::dto::PdfDocumentModel &document) {
    VerticalCursor cursor{.y = context.page.top};

    context.painter.setFont(reportFont(16));
    drawCenteredLine(context, cursor, QStringLiteral("ООО «С.И.Т»"));

    cursor.y += 260;
    context.painter.setFont(reportFont(16));
    drawCenteredLine(context, cursor, withFallback(document.organization, QStringLiteral("Организация")));

    cursor.y += 110;
    context.painter.setFont(reportFont(10));
    drawCenteredLine(context, cursor,
                     QStringLiteral("№ Лицензии: ") +
                         withFallback(document.licenseNumber, QStringLiteral("не указан")));

    cursor.y += 70;
    context.painter.setFont(reportFont(16));
    drawCenteredLine(context, cursor, QStringLiteral("Протокол испытаний"));
    drawCenteredLine(context, cursor, QStringLiteral("беспилотной авиационной системы"));
    drawCenteredLine(context, cursor, QStringLiteral("(БАС)"));

    cursor.y += 360;
    drawCenteredLine(context, cursor,
                     QStringLiteral("Адрес: ") + withFallback(document.address, QStringLiteral("не указан")));

    cursor.y = context.page.height - pageMargin - 190;
    context.painter.setFont(reportFont(14));
    drawCenteredLine(context, cursor,
                     QStringLiteral("Тип: ") + withFallback(document.testMode, QStringLiteral("не указан")));
    drawCenteredLine(context, cursor,
                     QStringLiteral("Испытание: ") +
                         withFallback(document.testProgramTitle, QStringLiteral("не указано")));
    drawCenteredLine(context, cursor, QStringLiteral("Дата проведения: ") + toQString(document.reportDate));
}

void drawCell(QPainter &painter, const QRect &rect, const QString &text, bool header) {
    painter.drawRect(rect);

    const QFont oldFont = painter.font();
    if (header) {
        painter.setFont(reportFont(9, QFont::Bold));
    }

    painter.drawText(rect.adjusted(12, 12, -12, -12), Qt::AlignCenter | Qt::TextWordWrap, text);
    painter.setFont(oldFont);
}

void drawReportTable(DrawingContext &context, VerticalCursor &cursor,
                     const application::dto::PdfDocumentModel &document) {
    const std::array<QString, 5> headers{QStringLiteral("Тип"), QStringLiteral("Сила ветра от t"),
                                         QStringLiteral("Расчетное время (мин.)"), QStringLiteral("Время теста (мин.)"),
                                         QStringLiteral("Годен/Негоден")};
    const std::array<int, 5> widths{context.page.contentWidth * 18 / 100, context.page.contentWidth * 25 / 100,
                                    context.page.contentWidth * 19 / 100, context.page.contentWidth * 19 / 100,
                                    context.page.contentWidth * 19 / 100};

    const QFontMetrics tableMetrics(context.painter.font());
    const int rowHeight = std::max(104, tableMetrics.height() * 3);
    int x = context.page.left;

    for (std::size_t i = 0; i < headers.size(); ++i) {
        drawCell(context.painter, QRect(x, cursor.y, widths[i], rowHeight), headers[i], true);
        x += widths[i];
    }
    cursor.y += rowHeight;

    for (const auto &row : document.tableRows) {
        const std::array<QString, 5> values{toQString(row.testType), toQString(row.windFormula),
                                            toQString(row.estimatedDurationMinutes), toQString(row.testDurationMinutes),
                                            toQString(row.result)};
        x = context.page.left;
        for (std::size_t i = 0; i < values.size(); ++i) {
            drawCell(context.painter, QRect(x, cursor.y, widths[i], rowHeight), values[i], false);
            x += widths[i];
        }
        cursor.y += rowHeight;
    }

    cursor.y += sectionSpacing;
}

void drawPlotPage(DrawingContext &context, const domain::PlotModel &plot, const QString &caption) {
    VerticalCursor cursor{.y = context.page.top};
    const QRect plotRect(context.page.left, cursor.y, context.page.contentWidth,
                         context.page.height - (2 * pageMargin) - 120);

    ui::render::PlotRenderer::drawPlot(context.painter, plotRect, plot);

    cursor.y = plotRect.bottom() + 40;
    context.painter.setFont(reportFont(11));
    drawCenteredLine(context, cursor, caption, 80);
}

void drawDroneParameters(DrawingContext &context, VerticalCursor &cursor,
                         const std::vector<domain::TestProtocolParameter> &parameters) {
    if (parameters.empty()) {
        drawParagraph(context, cursor, QStringLiteral("Конфигурация не указана"), sectionSpacing);
        return;
    }

    const QFontMetrics parameterMetrics(context.painter.font());
    const int rowHeight = std::max(68, parameterMetrics.height() + 28);
    const int labelWidth = context.page.contentWidth * 42 / 100;
    const int valueWidth = context.page.contentWidth - labelWidth;

    for (const auto &parameter : parameters) {
        const QRect labelRect(context.page.left, cursor.y, labelWidth, rowHeight);
        const QRect valueRect(context.page.left + labelWidth, cursor.y, valueWidth, rowHeight);
        drawCell(context.painter, labelRect, toQString(parameter.label), false);
        drawCell(context.painter, valueRect, toQString(parameter.value), false);
        cursor.y += rowHeight;
    }

    cursor.y += sectionSpacing;
}

void drawSummaryPage(DrawingContext &context, const application::dto::PdfDocumentModel &document) {
    VerticalCursor cursor{.y = context.page.top};

    context.painter.setFont(reportFont(14, QFont::Bold));
    drawParagraph(context, cursor, QStringLiteral("Конфигурация БПЛА и параметры теста:"), 24);

    context.painter.setFont(reportFont(10));
    drawDroneParameters(context, cursor, document.droneParameters);

    if (!document.comment.empty()) {
        context.painter.setFont(reportFont(14, QFont::Bold));
        drawParagraph(context, cursor, QStringLiteral("Комментарий оператора:"), 24);

        context.painter.setFont(reportFont(10));
        drawParagraph(context, cursor, toQString(document.comment), sectionSpacing);
    }

    context.painter.setFont(reportFont(14, QFont::Bold));
    drawParagraph(context, cursor, QStringLiteral("Заключение:"), 24);

    context.painter.setFont(reportFont(10));
    drawParagraph(context, cursor, withFallback(document.conclusion, QStringLiteral("Заключение не указано")), 32);

    const QString blankLine(110, QChar('_'));
    drawParagraph(context, cursor, blankLine, 24);
    drawParagraph(context, cursor, blankLine, 24);
    drawParagraph(context, cursor, blankLine, sectionSpacing);

    drawField(context, cursor, QStringLiteral("Оператор ФИО: "),
              withFallback(document.operatorName, QStringLiteral("не указан")));
}

void setupPainter(QPainter &painter) {
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::black, 2));
}

} // namespace

namespace infrastructure {

void QtPdfExporter::exportDocument(const application::dto::PdfDocumentModel &document, const std::string &filePath) {
    QPdfWriter writer(toQString(filePath));
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15.0, 15.0, 15.0, 15.0));
    writer.setResolution(300);

    QPainter painter(&writer);
    if (!painter.isActive()) {
        return;
    }

    setupPainter(painter);
    DrawingContext context{.painter = painter, .page = pageSpec(writer)};

    drawTitlePage(context, document);

    writer.newPage();
    VerticalCursor cursor{.y = context.page.top};
    painter.setFont(reportFont(14, QFont::Bold));
    drawParagraph(context, cursor, QStringLiteral("Параметры теста к протоколу испытаний №_____________"),
                  sectionSpacing);
    painter.setFont(reportFont(10));
    drawReportTable(context, cursor, document);

    writer.newPage();
    drawPlotPage(context, document.controlPlot, toQString(document.controlPlotCaption));

    writer.newPage();
    drawPlotPage(context, document.telemetryPlot, toQString(document.telemetryPlotCaption));

    writer.newPage();
    drawSummaryPage(context, document);

    painter.end();
}

} // namespace infrastructure
