#include "wafermapwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <math.h>

#define MAX_BIN_COUNT 12

static const QRgb colorTable[MAX_BIN_COUNT] =
{
    0x00FF00, 0x0000FF, 0xFF0000, 0xCC6666,
    0x6666CC, 0x66CC66, 0xCCCC66, 0xCC66CC,
    0x66CCCC, 0xDAAA00, 0x808080, 0xCC0066
};

WaferMapWidget::WaferMapWidget(QWidget *parent) : QWidget(parent)
{
    m_hardbin_count = 0;
    m_hbin_site_index = 0;
    m_span = 2;
    m_max_xcoord = INVALID_COORD_VALUE;
    m_min_xcoord = INVALID_COORD_VALUE;
    m_max_ycoord = INVALID_COORD_VALUE;
    m_min_ycoord = INVALID_COORD_VALUE;
    m_show_bin_count = false;
} 

bool WaferMapWidget::SetDataLog(DataLog* datalog)
{
    DeviceDataList.clear();
    ColorTableList.clear();
    SBinNumberList.clear();
    HBinNumberList.clear();
    HBin_SBinList.clear();
    HBinCountList.clear();
    unsigned int site_count = datalog->get_site_count();
    if(site_count == 0) return false;

    for(unsigned int s = 0; s < site_count; s++)
    {
        TestSite* site = datalog->get_site(s);
        unsigned int device_count = site->get_device_count();

        unsigned int hbin_count = site->get_hardbin_count();
        if(hbin_count > m_hardbin_count)
        {
            m_hardbin_count = hbin_count;
            m_hbin_site_index = s;
        }

        for(unsigned int d = 0; d < device_count; d++)
        {
            TestDevice device = site->get_device(d);
            WaferMapData one_device;
            int xcoord = device.get_xcoord();
            int ycoord = device.get_ycoord();

            if(xcoord == INVALID_COORD_VALUE || ycoord == INVALID_COORD_VALUE)
            {
                DeviceDataList.clear();
                return false;
            }

            if(s==0 && d == 0)
            {
                m_max_xcoord = xcoord;
                m_min_xcoord = xcoord;
                m_max_ycoord = ycoord;
                m_min_ycoord = ycoord;
            }
            else
            {
                if(m_max_xcoord < xcoord) m_max_xcoord = xcoord;
                if(m_min_xcoord > xcoord) m_min_xcoord = xcoord;
                if(m_max_ycoord < ycoord) m_max_ycoord = ycoord;
                if(m_min_ycoord > ycoord) m_min_ycoord = ycoord;
            }

            one_device.X = xcoord;
            one_device.Y = ycoord;
            one_device.HBin = device.get_hardbin();
            one_device.SBin = device.get_softbin();
            DeviceDataList.append(one_device);
            HBin_SBinList.insertMulti(one_device.HBin, one_device.SBin);
        }
    }
    int device_count = DeviceDataList.size();
    if(device_count == 0) return false;

    TestSite* bin_site = datalog->get_site(m_hbin_site_index);
    m_show_bin_count = datalog->is_hbin_for_all_sites();
    HBinNumberList.clear();
    for(unsigned int i = 0; i < m_hardbin_count; i++)
    {
        TestBin bin = bin_site->get_hardbin(i);
        unsigned short bin_number = bin.get_number();
        QColor bin_color = colorTable[i%12];
        ColorTableList.insert(bin_number, bin_color);
        HBinNumberList.append(bin_number);
        HBinCountList.insert(bin_number, bin.get_count() );
    }

    unsigned int sbin_count = bin_site->get_softbin_count();
    for(unsigned int i = 0; i < sbin_count; i++)
    {
        TestBin bin = bin_site->get_softbin(i);
        unsigned short bin_number = bin.get_number();
        QString bin_name = QString::fromLocal8Bit(bin.get_name());
        SBinNumberList.insertMulti(bin_number, bin_name);
    }


    update();

    return true;
}

unsigned int  WaferMapWidget::GetBinCount() const
{
    return HBinNumberList.size();
}

QColor  WaferMapWidget::GetBinColor(unsigned int bin_number) const
{
    return ColorTableList[bin_number];
}

unsigned int  WaferMapWidget::GetHBinNumber(unsigned int index) const
{
    return HBinNumberList[index];
}

bool WaferMapWidget::show_bin_count()
{
    return m_show_bin_count;
}

unsigned WaferMapWidget::GetHbinDeviceCount(unsigned int hbin_number) const
{
    return HBinCountList[hbin_number];
}

QString WaferMapWidget::GetSBinName(unsigned int hbin_number) const
{
    QList<unsigned short> sbin_numbers = HBin_SBinList.values(hbin_number);
    unsigned int sbin_count = sbin_numbers.count();
    QString sbin_name;
    for(unsigned int i = 0; i < sbin_count; i++)
    {
        QList<QString> sbin_names = SBinNumberList.values(sbin_numbers[i]);
        unsigned int sbin_name_count = sbin_names.count();
        for(unsigned int n = 0; n < sbin_name_count; n++)
        {
            if(i == 0 && n == 0)
            {
                sbin_name += sbin_names[0];
            }
            else
            {
                if(sbin_name.contains(sbin_names[n])) continue;
                else sbin_name = sbin_name + "|" + sbin_names[n];
            }
        }
    }
    return sbin_name;
}

void WaferMapWidget::UpdateSize()
{
    QRect rect = this->rect();
    m_span = rect.width();
    //if(rect.width() > rect.height()) m_span = rect.height();
    if(m_span > 200) m_span = int(m_span * 0.95);
}

void WaferMapWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    UpdateSize();
    DrawWaferMap(&painter);
}


void WaferMapWidget::DrawWaferMap(QPainter *painter)
{ 
    int device_count = DeviceDataList.size();
    if(device_count == 0) return;

    int xspan = m_max_xcoord - m_min_xcoord;
    int yspan = m_max_ycoord - m_min_ycoord;
    int max_span = xspan;
    if(yspan > max_span) max_span = yspan;
    int pix_count = std::round(1.0 * m_span / max_span); //
    if(pix_count > 2)
        m_pixcount = pix_count;
    else m_pixcount = 1;

    for(int i = 0; i < device_count; i++)
    {
        WaferMapData device = DeviceDataList[i];
        int x = (device.X - m_min_xcoord) * m_pixcount + 1;
        int y = (m_max_ycoord - device.Y) * m_pixcount + 1;

        unsigned short bin = device.HBin;
        QColor color = ColorTableList[bin];

        painter->fillRect(x + 1, y + 1, m_pixcount - 2, m_pixcount - 2,
                         color);

        painter->setPen(color.light());
        painter->drawLine(x, y + m_pixcount - 1, x, y);
        painter->drawLine(x, y, x + m_pixcount - 1, y);

        painter->setPen(color.dark());
        painter->drawLine(x + 1, y + m_pixcount - 1,
                         x + m_pixcount - 1, y + m_pixcount - 1);
        painter->drawLine(x + m_pixcount - 1, y + m_pixcount - 1,
                         x + m_pixcount - 1, y + 1);
    }
}
