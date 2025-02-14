#include <QByteArray>
#include <QRegularExpression>

#include "markdownparser.h"
#include "md4c-html.h"
#include "common.h"

/* Global options. */
#if MD_UNDERLINE
static unsigned parser_flags = MD_FLAG_UNDERLINE;
#else
static unsigned parser_flags = 0;
#endif

static QByteArray templateArray = QByteArrayLiteral("<!DOCTYPE html>\n"
                                                    "<html>\n"
                                                    "<head>\n"
                                                    "<title>HTML generated by md4c</title>\n"
                                                    "</head>\n"
                                                    "<body class=\"preview\">\n");


void captureHtmlFragment (const MD_CHAR* data, MD_SIZE data_size, void* userData) {
    QByteArray *array = static_cast<QByteArray*>(userData);

    array->append(data, data_size);
}

QString Parser::toHtml(const QString &in, const int &dia)
{
    if (dia == GitHub)
        parser_flags |= MD_DIALECT_GITHUB;
    else
        parser_flags |= MD_DIALECT_COMMONMARK;

    const QByteArray array = in.toUtf8();
    QByteArray out = templateArray;

    md_html(array.data(), array.size(), &captureHtmlFragment, &out,
            parser_flags, MD_HTML_FLAG_DEBUG | MD_HTML_FLAG_SKIP_UTF8_BOM);

    out.append("</body>\n"
               "</html>\n");

    return out;
}

QString Parser::toMarkdown(QString in)
{
    // replace Windows line breaks
    in.replace(QChar(QChar::LineSeparator), QLatin1String("\n"));

    // remove some blocks
    in.remove(
        QRegularExpression(QStringLiteral("<head.*?>(.+?)<\\/head>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption));

    in.remove(
        QRegularExpression(QStringLiteral("<script.*?>(.+?)<\\/script>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption));

    in.remove(
        QRegularExpression(QStringLiteral("<style.*?>(.+?)<\\/style>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption));

    // replace some html tags with markdown
    in.replace(
        QRegularExpression(QStringLiteral("<strong.*?>(.+?)<\\/strong>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("**\\1**"));
    in.replace(
        QRegularExpression(QStringLiteral("<b.*?>(.+?)<\\/b>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("**\\1**"));
    in.replace(
        QRegularExpression(QStringLiteral("<em.*?>(.+?)<\\/em>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("*\\1*"));
    in.replace(
        QRegularExpression(QStringLiteral("<i.*?>(.+?)<\\/i>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("*\\1*"));
    in.replace(
        QRegularExpression(QStringLiteral("<pre.*?>(.+?)<\\/pre>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("\n```\n\\1\n```\n"));
    in.replace(
        QRegularExpression(QStringLiteral("<code.*?>(.+?)<\\/code>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("\n```\n\\1\n```\n"));
    in.replace(
        QRegularExpression(QStringLiteral("<h1.*?>(.+?)<\\/h1>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("\n# \\1\n"));
    in.replace(
        QRegularExpression(QStringLiteral("<h2.*?>(.+?)<\\/h2>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("\n## \\1\n"));
    in.replace(
        QRegularExpression(QStringLiteral("<h3.*?>(.+?)<\\/h3>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("\n### \\1\n"));
    in.replace(
        QRegularExpression(QStringLiteral("<h4.*?>(.+?)<\\/h4>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("\n#### \\1\n"));
    in.replace(
        QRegularExpression(QStringLiteral("<h5.*?>(.+?)<\\/h5>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("\n##### \\1\n"));
    in.replace(
        QRegularExpression(QStringLiteral("<h6.*?>(.+?)<\\/h6>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("\n###### \\1\n"));
    in.replace(
        QRegularExpression(QStringLiteral("<li.*?>(.+?)<\\/li>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("- \\1\n"));
    in.replace(QRegularExpression(QStringLiteral("<br.*?>"),
                                    QRegularExpression::CaseInsensitiveOption),
                 QStringLiteral("\n"));
    in.replace(QRegularExpression(
                     QStringLiteral("<a[^>]+href=\"(.+?)\".*?>(.+?)<\\/a>"),
                     QRegularExpression::CaseInsensitiveOption |
                         QRegularExpression::DotMatchesEverythingOption),
                 QStringLiteral("[\\2](\\1)"));
    in.replace(
        QRegularExpression(QStringLiteral("<p.*?>(.+?)</p>"),
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption),
        QStringLiteral("\n\n\\1\n\n"));

    // replace multiple line breaks
    in.replace(QRegularExpression(QStringLiteral("\n\n+")),
                 QStringLiteral("\n\n"));

    return in;
}
