#-------------------------------------------------
#
# Project created by QtCreator 2016-03-29T17:21:30
#
#-------------------------------------------------

QT       += core gui printsupport charts gui-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TestDataAnalysis.Win10.4.0
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS   += -static


SOURCES += main.cpp\
        mainwindow.cpp \
    correlation/correlation.cpp \
    correlation/corrlationdialog.cpp \
    datalog/csv_api.cpp \
    datalog/csv_internal.cpp \
    datalog/datalog.cpp \
    datalog/debug_api.cpp \
    datalog/stdf_v4_api.cpp \
    datalog/stdf_v4_internal.cpp \
    datalog/datalog_internal.cpp \
    grr/grr.cpp \
    grr/showgrrdialog.cpp \
    statistic/statistics.cpp \
    statistic/statisticdialog.cpp \
    stdfreader/stdf_v4_file.cpp \
    stdfreader/stdfdialog.cpp \
    widgets/calsettingdialog.cpp \
    widgets/chartsettingdialog.cpp \
    widgets/convertfiledialog.cpp \
    wafermap/wafermapdialog.cpp \
    wafermap/wafermapwidget.cpp \
    scatterplot/scatterplotdialog.cpp \
    xlsx/xlsxabstractooxmlfile.cpp \
    xlsx/xlsxabstractsheet.cpp \
    xlsx/xlsxcell.cpp \
    xlsx/xlsxcellformula.cpp \
    xlsx/xlsxcellrange.cpp \
    xlsx/xlsxcellreference.cpp \
    xlsx/xlsxchart.cpp \
    xlsx/xlsxchartsheet.cpp \
    xlsx/xlsxcolor.cpp \
    xlsx/xlsxconditionalformatting.cpp \
    xlsx/xlsxcontenttypes.cpp \
    xlsx/xlsxdatavalidation.cpp \
    xlsx/xlsxdocpropsapp.cpp \
    xlsx/xlsxdocpropscore.cpp \
    xlsx/xlsxdocument.cpp \
    xlsx/xlsxdrawing.cpp \
    xlsx/xlsxdrawinganchor.cpp \
    xlsx/xlsxformat.cpp \
    xlsx/xlsxmediafile.cpp \
    xlsx/xlsxnumformatparser.cpp \
    xlsx/xlsxrelationships.cpp \
    xlsx/xlsxrichstring.cpp \
    xlsx/xlsxsharedstrings.cpp \
    xlsx/xlsxsimpleooxmlfile.cpp \
    xlsx/xlsxstyles.cpp \
    xlsx/xlsxtheme.cpp \
    xlsx/xlsxutility.cpp \
    xlsx/xlsxworkbook.cpp \
    xlsx/xlsxworksheet.cpp \
    xlsx/xlsxzipreader.cpp \
    xlsx/xlsxzipwriter.cpp

HEADERS  += mainwindow.h \
    correlation/correlation.h \
    correlation/corrlationdialog.h \
    datalog/csv_api.h \
    datalog/csv_internal.h \
    datalog/datalog.h \
    datalog/datalog_internal.h \
    datalog/debug_api.h \
    datalog/stdf_v4_internal.h \
    datalog/stdf_v4_api.h \
    grr/grr.h \
    grr/showgrrdialog.h \
    statistic/statisticdialog.h \
    statistic/statistics.h \
    stdfreader/stdf_v4_file.h \
    stdfreader/stdfdialog.h \
    widgets/calsettingdialog.h \
    widgets/chartsettingdialog.h \
    widgets/convertfiledialog.h \
    wafermap/wafermapdialog.h \
    wafermap/wafermapwidget.h \
    scatterplot/scatterplotdialog.h \
    xlsx/xlsxabstractooxmlfile.h \
    xlsx/xlsxabstractooxmlfile_p.h \
    xlsx/xlsxabstractsheet.h \
    xlsx/xlsxabstractsheet_p.h \
    xlsx/xlsxcell.h \
    xlsx/xlsxcell_p.h \
    xlsx/xlsxcellformula.h \
    xlsx/xlsxcellformula_p.h \
    xlsx/xlsxcellrange.h \
    xlsx/xlsxcellreference.h \
    xlsx/xlsxchart.h \
    xlsx/xlsxchart_p.h \
    xlsx/xlsxchartsheet.h \
    xlsx/xlsxchartsheet_p.h \
    xlsx/xlsxcolor_p.h \
    xlsx/xlsxconditionalformatting.h \
    xlsx/xlsxconditionalformatting_p.h \
    xlsx/xlsxcontenttypes_p.h \
    xlsx/xlsxdatavalidation.h \
    xlsx/xlsxdatavalidation_p.h \
    xlsx/xlsxdocpropsapp_p.h \
    xlsx/xlsxdocpropscore_p.h \
    xlsx/xlsxdocument.h \
    xlsx/xlsxdocument_p.h \
    xlsx/xlsxdrawing_p.h \
    xlsx/xlsxdrawinganchor_p.h \
    xlsx/xlsxformat.h \
    xlsx/xlsxformat_p.h \
    xlsx/xlsxglobal.h \
    xlsx/xlsxmediafile_p.h \
    xlsx/xlsxnumformatparser_p.h \
    xlsx/xlsxrelationships_p.h \
    xlsx/xlsxrichstring.h \
    xlsx/xlsxrichstring_p.h \
    xlsx/xlsxsharedstrings_p.h \
    xlsx/xlsxsimpleooxmlfile_p.h \
    xlsx/xlsxstyles_p.h \
    xlsx/xlsxtheme_p.h \
    xlsx/xlsxutility_p.h \
    xlsx/xlsxworkbook.h \
    xlsx/xlsxworkbook_p.h \
    xlsx/xlsxworksheet.h \
    xlsx/xlsxworksheet_p.h \
    xlsx/xlsxzipreader_p.h \
    xlsx/xlsxzipwriter_p.h
    

FORMS    += mainwindow.ui \
    correlation/corrlationdialog.ui \
    grr/showgrrdialog.ui \
    statistic/statisticdialog.ui \
    stdfreader/stdfdialog.ui \
    widgets/calsettingdialog.ui \
    widgets/chartsettingdialog.ui \
    widgets/convertfiledialog.ui \
    wafermap/wafermapdialog.ui \
    scatterplot/scatterplotdialog.ui

TRANSLATIONS += TestDataAnslysis.ts
RESOURCES    += TestDataAnslysis.qrc
RC_FILE      += TestDataAnslysis.rc
INCLUDEPATH  += xlsx
DEFINES += XLSX_NO_LIB



