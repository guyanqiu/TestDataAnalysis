#include "correlation.h"
#include <fstream>
#include <math.h>
#include <QString>
#include "xlsxdocument.h"
#include "xlsxdocument.h"
#include "xlsxabstractsheet.h"
#include "xlsxformat.h"


//static std::wstring convert(const char* str)
//{
//    QString temp = QString::fromLocal8Bit(str);
//    return std::wstring(reinterpret_cast<const wchar_t *>(temp.utf16()));
//}

TestItem_CORR::TestItem_CORR()
{
    m_calculate_mode = 0;
	m_set_ref_item = false;
    m_corr_spec = 0.1;
	m_site_count = 0;
    m_data_count = 0;

    m_high_spec = 9999.9;
    m_low_spec = -9999.9;
    m_highspec_valid = false;
    m_lowspec_valid = false;

    m_number = 0;
	m_ref_average = 0.0;
}

void TestItem_CORR::set_calculate_mode(int mode)
{
    m_calculate_mode = mode;
}

TestItem_CORR::~TestItem_CORR()
{
	m_average_list.clear();
    m_delta_value_list.clear();
}

bool TestItem_CORR::set_corr_sepc(double spec_value)
{
    if( (spec_value > 1.0) || (spec_value < 0.001))
        return false;
    else
    {
        m_corr_spec = spec_value;
        return true;
    }
}

bool TestItem_CORR::set_ref_item(const TestItem* item, double default_lowlimit, double default_highlimit)
{
	if(!item) return false;

	unsigned int data_count = item->get_result_count();
    if(data_count == 0) return false;

    m_low_spec = default_lowlimit;
    m_high_spec = default_highlimit;

	m_data_count = data_count;

	m_ref_average = item->get_average();
	m_label.assign(item->get_label());
    m_number = item->get_number();

    const char* str_unit = item->get_unit();
    if(str_unit) m_unit.assign(str_unit);

    if(item->lowlimit_valid())
    {
        m_low_spec = item->get_lowlimit();
        m_lowspec_valid = true;
    }
	else
		m_low_spec = default_lowlimit;
    if(item->highlimit_valid())
    {
        m_high_spec = item->get_highlimit();
        m_highspec_valid = true;
    }
	else
		m_high_spec = default_highlimit;

	m_set_ref_item = true;
	return true;
}

bool TestItem_CORR::add_item(const TestItem* item)
{
	if(!m_set_ref_item) return false;
	if(!item) return false;
	unsigned int data_count = item->get_result_count();

    if(data_count == 0) return false;

    double average = item->get_average();
	m_average_list.push_back(average);

	double err = m_ref_average - average;
    m_delta_value_list.push_back(err);

	m_site_count ++;
	return true;
}

double TestItem_CORR::get_highspec() const
{
	return m_high_spec;
}

double TestItem_CORR::get_lowspec() const
{
	return m_low_spec;
}

unsigned int TestItem_CORR::get_site_count() const
{
	return m_site_count;
}

unsigned int TestItem_CORR::get_data_count() const
{
	return m_data_count;
}

double TestItem_CORR::get_tolerance() const
{
	return (m_high_spec - m_low_spec);
}

double TestItem_CORR::get_ref_average() const
{
	return m_ref_average;
}

double TestItem_CORR::get_corr_average(unsigned int index) const
{
	return m_average_list[index];
}

double TestItem_CORR::get_corr_value(unsigned int index) const
{
    double err = m_delta_value_list[index];
    double tolerance = m_high_spec - m_low_spec;
    if(m_calculate_mode != 0)
    {
        tolerance = m_ref_average;
    }

    if(tolerance != 0)
        return err/tolerance;
    else if(tolerance == 0 && err != 0)
        return 1.0;
    else
        return 0.0;
}

double TestItem_CORR::get_delta_value(unsigned int index) const
{
    return m_delta_value_list[index];
}

unsigned int TestItem_CORR::get_number() const
{
	return m_number;
}

const char* TestItem_CORR::get_label() const
{
	return m_label.c_str();
}

double TestItem_CORR::get_corr_spec() const
{
    return m_corr_spec;
}

const char* TestItem_CORR::get_unit() const
{
	if(m_unit.empty()) return nullptr;
    return m_unit.c_str();
}

TestSite_CORR::TestSite_CORR()
{
    m_calculate_mode = 0;
	m_add_ref_site = false;
    m_site_count = 0;
    m_item_count = 0;
    m_data_count = 0;

    m_low_spec = -9999.9;
    m_high_spec = 9999.9;
    m_calculate_ok = false;
	m_ref_site = nullptr;
}

TestSite_CORR::~TestSite_CORR()
{
	clear();
}

void TestSite_CORR::set_calcualte_mode(int mode)
{
    m_calculate_mode = mode;
}

CORR_CALCULATE_ERROR
TestSite_CORR::set_ref_site(const TestSite* site, double default_low_spec, double default_high_spec)
{
    if(!site) return CORR_SITE_NOT_EXIST;
    m_low_spec = default_low_spec;
    m_high_spec = default_high_spec;

    unsigned int item_count = site->get_item_count();
    if(item_count == 0) return CORR_ITEM_NOT_EXIST;

    unsigned int device_count = site->get_device_count();
    if(device_count == 0) return CORR_DATACOUNT_TOO_FEW;

    CORR_CALCULATE_ERROR return_value = CORR_RESULT_IS_OK;

    for(unsigned int i = 0; i < item_count; i++)
    {
        TestItem* item = site->get_item(i);
        if(!item->is_all_pass())
            return_value = CORR_DATA_NOT_ALL_PASS;

        unsigned int result_count = item->get_result_count();
        if(result_count == 0) return CORR_NOT_ALL_ITEM_TESTED;
    }

    for(unsigned int i = 0; i < item_count; i++)
    {
        TestItem* item = site->get_item(i);
        std::string label(item->get_label());
        unsigned int number = item->get_number();

        m_number_list.push_back(number);
        m_label_list.push_back(label);
    }

    m_data_count = device_count;
    m_item_count = item_count;
    m_ref_site = site;
    m_add_ref_site = true;

    return return_value;
}

CORR_CALCULATE_ERROR TestSite_CORR::add_site(const TestSite* site)
{
   if(!site) return CORR_SITE_NOT_EXIST;

    unsigned int item_count = site->get_item_count();
    if(item_count == 0) return CORR_ITEM_NOT_EXIST;

    unsigned int device_count = site->get_device_count();
    if(device_count == 0) return CORR_DATACOUNT_TOO_FEW;

    int data_faild_count = 0;
    for(unsigned int i = 0; i < item_count; i++)
    {
        TestItem* item = site->get_item(i);
        bool all_pass = item->is_all_pass();
        if(!all_pass) data_faild_count++;
        unsigned int result_count = item->get_result_count();
        if(result_count == 0) return CORR_NOT_ALL_ITEM_TESTED;
    }

    unsigned int number_not_same_count = 0;
    unsigned int label_not_same_count = 0;

    if(m_item_count != item_count)
        return CORR_ITEMCOUNT_NOT_SAME;

    for(unsigned int i = 0; i < item_count; i++)
    {
        TestItem* item = site->get_item(i);
        std::string label(item->get_label());
        unsigned int number = item->get_number();

        if(number != m_number_list[i]) number_not_same_count++;
        if(label != m_label_list[i]) label_not_same_count++;
    }

    m_site_list.push_back(site);
    m_site_count++;

    if(number_not_same_count > 0 && label_not_same_count > 0)
        return CORR_LABEL_AND_NUMBER_NOT_SAME;
    else if(number_not_same_count > 0)
        return CORR_NUMBER_NOT_SAME;
    else if(label_not_same_count > 0)
        return CORR_LABEL_NOT_SAME;
    else if(data_faild_count > 0)
        return CORR_DATA_NOT_ALL_PASS;
    else if(m_data_count != device_count)
        return CORR_DATACOUNT_NOT_SAME;
    else
        return CORR_RESULT_IS_OK;
}

CORR_CALCULATE_ERROR TestSite_CORR::calculate()
{
	if(m_site_count < 1) return CORR_SITECOUNT_TOO_FEW;
    if(m_item_count == 0) return CORR_ITEM_NOT_EXIST;
    if(m_data_count == 0) return CORR_DATACOUNT_TOO_FEW;
	if(!m_add_ref_site) return CORR_REF_SITE_NOT_SET;

    for(unsigned int i = 0; i < m_item_count; i++)
    {
		TestItem_CORR* corr = new TestItem_CORR();
		TestItem* ref = m_ref_site->get_item(i);
        bool ret = corr->set_ref_item(ref, m_low_spec, m_high_spec);
        if(ret == false)
        {
            return CORR_DATACOUNT_TOO_FEW;
        }
        corr->set_corr_sepc(m_corr_spec);
        corr->set_calculate_mode(m_calculate_mode);
        for(unsigned int s = 0; s < m_site_count; s++)
        {
            const TestSite* site = m_site_list[s];
            TestItem* item = site->get_item(i);
            corr->add_item(item);
        }
        m_corr_list.push_back(corr);
    }
    m_calculate_ok = true;
    return CORR_RESULT_IS_OK;
}

const TestSite* TestSite_CORR::get_site(unsigned int index) const
{
	return m_site_list[index];
}

TestItem_CORR* TestSite_CORR::get_corr(unsigned int index) const
{
	return m_corr_list[index];
}

unsigned int TestSite_CORR::get_data_count() const
{
	return m_data_count;
}

unsigned int TestSite_CORR::get_site_count() const
{
	return m_site_count;
}

unsigned int TestSite_CORR::get_item_count() const
{
	return m_item_count;
}

bool TestSite_CORR::set_corr_spec(double spec_value)
{
    if(spec_value > 1.0 || spec_value < 0.001)
        return false;
    else
    {
        m_corr_spec = spec_value;
        return true;
    }
}

double TestSite_CORR::get_corr_spec() const
{
    return m_corr_spec;
}

bool TestSite_CORR::save_result_csv(const char* filename)
{
	if(!m_calculate_ok) return false;
    std::ofstream out(filename, std::ios::out );
    if(!out) return false;

    out<<"TestNumber,TestLabel,HighSpec,LowSpec,Tolerance,Unit,Ref-"<<m_ref_site->get_name()<<" Average,,";
	for(unsigned int s = 0; s < m_site_count; s++)
	{
		const TestSite* site = m_site_list[s];
        out<<site->get_name()<<" Average,Delta,Corr,Pass/Fail,,";
	}
	out<<"\n";

    for(unsigned int i = 0; i < m_item_count; i++)
    {
        TestItem_CORR* corr = m_corr_list[i];
        out<<corr->get_number()<<",";
        out<<corr->get_label()<<",";
        out<<corr->get_highspec()<<",";
        out<<corr->get_lowspec()<<",";
        out<<corr->get_tolerance()<<",";
        const char* str_unit = corr->get_unit();
        if(str_unit) out<<str_unit<<",";
        else out<<",";
        out<< corr->get_ref_average()<<",,";

        double temp_item_corr_spec = corr->get_corr_spec();
		for(unsigned int s = 0; s < m_site_count; s++)
		{
			out<<corr->get_corr_average(s)<<",";
            out<<corr->get_delta_value(s)<<",";
            double corr_value = corr->get_corr_value(s);
            out<<100.0*corr_value<<"%"<<",";
            if(fabs(corr_value) > temp_item_corr_spec)
                out<<"F,,";
            else
                out<<"P,,";
		}
		out<<"\n";
    }
    out.close();
	return true;
}

bool TestSite_CORR::save_result_xls(const char* filename)
{
    if(!m_calculate_ok) return false;
    QXlsx::Document xlsx;
    xlsx.renameSheet("Sheet1", "Correlation Result");
    QString qfilename = QString::fromLocal8Bit(filename);
    QString wsite_name = QString::fromLocal8Bit(m_ref_site->get_name());

    unsigned int row = 1, col = 1;
    xlsx.write(row, col++, "TestNumber");
    xlsx.write(row, col++, "TestLable");
    xlsx.write(row, col++, "HighSpec");
    xlsx.write(row, col++, "LowSpec");
    xlsx.write(row, col++, "Tolerance");
    xlsx.write(row, col++, "Unit");
    xlsx.write(row, col++, wsite_name+"-Average");
    col++;

    for(unsigned int s = 0; s < m_site_count; s++)
    {
        const TestSite* site = m_site_list[s];
        QString wtemp_name = QString::fromLocal8Bit(site->get_name());
        xlsx.write(row, col++, wtemp_name + "-Average");
        xlsx.write(row, col++, "Delta");
        xlsx.write(row, col++, "Corr");
        col++;
    }
    row++;
    for(unsigned int i = 0; i < m_item_count; i++)
    {
        col = 1;
        TestItem_CORR* corr = m_corr_list[i];
        QString wlabel = QString::fromLocal8Bit(corr->get_label());
        QString wunit = QString::fromLocal8Bit(corr->get_unit());

        xlsx.write(row, col++, corr->get_number());
        xlsx.write(row, col++, wlabel);
        xlsx.write(row, col++, corr->get_highspec());
        xlsx.write(row, col++, corr->get_lowspec());
        xlsx.write(row, col++, corr->get_tolerance());
        xlsx.write(row, col++, wunit);
        xlsx.write(row, col++, corr->get_ref_average());
        col++;

        double temp_item_corr_spec = corr->get_corr_spec();
        for(unsigned int s = 0; s < m_site_count; s++)
        {
            xlsx.write(row, col++, corr->get_corr_average(s));
            xlsx.write(row, col++, corr->get_delta_value(s));
            double corr_value = corr->get_corr_value(s);

            QXlsx::Format format_red;
            format_red.setPatternBackgroundColor(QColor(Qt::red));
            format_red.setNumberFormat("0.00%");

            QXlsx::Format format_green;
            format_green.setPatternBackgroundColor(QColor(Qt::green));
            format_green.setNumberFormat("0.00%");

            if(fabs(corr_value) > temp_item_corr_spec)
                xlsx.write(row, col++, corr_value, format_red);
            else
                xlsx.write(row, col++, corr_value, format_green);
            col++;
        }
        row++;
    }
    bool result = xlsx.saveAs(qfilename);
    return result;
}

void TestSite_CORR::clear()
{
	m_add_ref_site = false;
    m_site_count = 0;
    m_item_count = 0;
    m_data_count = 0;

    m_calculate_ok = false;
	m_ref_site = nullptr;

    m_site_list.clear();
    m_label_list.clear();
    m_number_list.clear();
    unsigned int corr_count = m_corr_list.size();
    for(unsigned int i = 0; i < corr_count; i++)
    {
        delete m_corr_list[i];
    }
    m_corr_list.clear();
}
