#ifndef WAFERMAPWIDGET_H
#define WAFERMAPWIDGET_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QMap>

#include "../datalog/datalog.h"



class WaferMapWidget : public QWidget
{
    Q_OBJECT

    struct WaferMapData
    {
        int X;
        int Y;
        unsigned short HBin;
        unsigned short SBin;
    };

public:
    explicit WaferMapWidget(QWidget *parent = 0);

public slots:
    bool SetDataLog(DataLog* datalog);
    void UpdateSize();
    unsigned int GetBinCount() const;
    QColor GetBinColor(unsigned int index) const;
    unsigned int GetHBinNumber(unsigned int index) const;
    QString GetSBinName(unsigned int hbin_number) const;
    bool show_bin_count();
    unsigned GetHbinDeviceCount(unsigned int hbin_number) const;
	
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
	
private:
    unsigned int m_hardbin_count;
    unsigned int m_hbin_site_index;
	int  m_max_xcoord;
	int  m_min_xcoord;
	int  m_max_ycoord;
	int  m_min_ycoord;
    int  m_pixcount;
    int  m_span;
    bool m_show_bin_count;

    QVector<WaferMapData> DeviceDataList;
    QVector<unsigned short> HBinNumberList;
    QMap<unsigned short, QString> SBinNumberList;//<SBin, Name>
    QMap<unsigned int, QColor> ColorTableList;//<HBin, Color>
    QMap<unsigned short, unsigned short> HBin_SBinList; // <SBin, HBin>
    QMap<unsigned short, unsigned int> HBinCountList;//<HBin, Count>
	
private:
    void DrawWaferMap(QPainter *painter);

};

#endif // WAFERMAPWIDGET_H
