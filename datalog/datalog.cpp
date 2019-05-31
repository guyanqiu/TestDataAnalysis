#include <cstring>
#include <cstdlib>
#include <cmath>

#include "csv_api.h"
#include "stdf_v4_api.h"
#include "datalog_internal.h"
#include "csv_api.h"
#include "debug_api.h"

//////////////////////////////////////////////////////////////////////////

TestResult::TestResult():
    m_index(0), m_value(0.0), m_flag(RESULT_INVALID)
{

}

TestResult::TestResult(unsigned int index, double value, TestFlag flag) :
    m_index( index), m_value( value), m_flag(flag)
{

}

void TestResult::set_index(unsigned int index)
{
    m_index = index;
}

void TestResult::set_flag(TestFlag flag)
{
    m_flag = flag;
}

void TestResult::set_value(double value)
{
    m_value = value;
}

unsigned int TestResult::get_index() const
{
    return m_index;
}

TestFlag TestResult::get_flag() const
{
    return m_flag;
}

double TestResult::get_value() const
{
    return m_value;
}

//////////////////////////////////////////////////////////////////////////
TestDevice::TestDevice()
{
    m_softbin = 1;
    m_hardbin = 1;
    m_xcoord = INVALID_COORD_VALUE;
    m_ycoord = INVALID_COORD_VALUE;
    m_id = nullptr;
}

TestDevice::TestDevice(const TestDevice& src)
{
    m_softbin = src.m_softbin;
    m_hardbin = src.m_hardbin;
    m_xcoord = src.m_xcoord;
    m_ycoord = src.m_ycoord;

    if(!src.m_id)
    {
        m_id = nullptr;
    }
    else
    {
        unsigned int length = std::strlen(src.m_id);
        m_id = new char[length+1];
        if(m_id)
        {
            std::strcpy(m_id, src.m_id);
        }
        else
        {
#ifdef _DEBUG_API_H_
            SHOW_MEMORY_ERROR();
            throw MEMORY_ALLOCATED_ERROR();
#endif
        }
    }
}

TestDevice& TestDevice::operator=(const TestDevice& src)
{
    if(this == &src) return *this;
    m_softbin = src.m_softbin;
    m_hardbin = src.m_hardbin;
    m_xcoord = src.m_xcoord;
    m_ycoord = src.m_ycoord;

    if(m_id) delete[] m_id;
    if(!src.m_id)
    {
        m_id = nullptr;
    }
    else
    {
        unsigned int length = std::strlen(src.m_id);
        if(m_id) delete[] m_id;
        m_id = new char[length+1];

        if(!m_id)
        {
#ifdef _DEBUG_API_H_
            SHOW_MEMORY_ERROR();
            throw MEMORY_ALLOCATED_ERROR();
#endif
        }
        else
        {
            std::strcpy(m_id, src.m_id);
        }
    }
    return *this;
}

TestDevice::~TestDevice()
{
    if(m_id)
    {
        delete[] m_id;
        m_id = nullptr;
    }
}

void TestDevice::set_xcoord( short x )
{
    m_xcoord = x;
}

void TestDevice::set_ycoord( short y )
{
    m_ycoord = y;
}

int TestDevice::get_xcoord() const
{
    return m_xcoord;
}

int TestDevice::get_ycoord() const
{
    return m_ycoord;
}

void TestDevice::set_softbin( unsigned short number )
{
    m_softbin = number;
}

void TestDevice::set_hardbin( unsigned short number )
{
    m_hardbin = number;
}

unsigned short TestDevice::get_softbin() const
{
    return m_softbin;
}

unsigned short TestDevice::get_hardbin() const
{
    return m_hardbin;
}


bool TestDevice::set_id(const char* id)
{
    if(!id) return true;
    if(m_id) delete [] m_id;

    unsigned int length = std::strlen(id);
    m_id = new char[length+1];
    if(m_id)
    {
        std::strcpy(m_id, id);
        return true;
    }
    else
    {
        return false;
    }
}

const char* TestDevice::get_id() const
{
    return m_id;
}

//////////////////////////////////////////////////////////////////////////
TestBin::TestBin()
{
    m_number = 1;
    m_count = 0;
    m_type = UNKONWNBIN;
    m_name = nullptr;
}

TestBin::TestBin(const TestBin& src)
{
    m_number = src.m_number;
    m_count = src.m_count;
    m_type = src.m_type;

    if(!src.m_name)
    {
        m_name = nullptr;
    }
    else
    {
        unsigned int length = std::strlen(src.m_name);
        m_name = new char[length+1];
        if(!m_name)
        {
#ifdef _DEBUG_API_H_
            SHOW_MEMORY_ERROR();
            throw MEMORY_ALLOCATED_ERROR();
#endif
        }
        else
        {
            std::strcpy(m_name, src.m_name);
        }
    }
}

TestBin& TestBin::operator=(const TestBin& src)
{
    if(this == &src) return *this;

    m_number = src.m_number;
    m_type = src.m_type;
    m_count = src.m_count;

    if(m_name) delete[] m_name;
    if(!src.m_name)
    {
        m_name = nullptr;
    }
    else
    {
        unsigned int length = std::strlen(src.m_name);
        if(m_name) delete[] m_name;
        m_name = new char[length+1];

        if(!m_name)
        {
#ifdef _DEBUG_API_H_
            SHOW_MEMORY_ERROR();
            throw MEMORY_ALLOCATED_ERROR();
#endif
        }
        else
        {
            std::strcpy(m_name, src.m_name);
        }
    }
    return *this;
}

TestBin::~TestBin()
{
    if(m_name)
    {
        delete [] m_name;
        m_name = nullptr;
    }
}

void TestBin::set_number( unsigned short number )
{
    m_number = number;
}

void TestBin::set_count( unsigned int count )
{
    m_count = count;
}

void TestBin::add_count()
{
    m_count++;
}

void TestBin::set_type( BinType type )
{
    m_type = type;
}

bool TestBin::set_name( const char* name )
{
    if(!name) return true;
    if(m_name) delete [] m_name;

    unsigned int length = std::strlen(name);
    m_name = new char[length+1];
    if(m_name)
    {
        std::strcpy(m_name, name);
        return true;
    }
    else
    {
        return false;
    }
}

unsigned short TestBin::get_number() const
{
    return m_number;
}

unsigned int TestBin::get_count() const
{
    return m_count;
}

BinType TestBin::get_type() const
{
    return m_type;
}

const char* TestBin::get_name() const
{
    return m_name;
}



//////////////////////////////////////////////////////////////////////////
TestItem::TestItem()
{
    impl = new TestItemImpl();
#ifdef _DEBUG_API_H_
    if(impl == nullptr)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

TestItem::~TestItem()
{
    delete impl;
    impl = nullptr;
}

void TestItem::set_lowlimit(double limit)
{
    impl->SetLowLimit(limit);
}

void TestItem::set_highlimit(double limit)
{
    impl->SetHighLimit(limit);
}

double TestItem::get_lowlimit() const
{
    return impl->GetLowLimit();
}

double TestItem::get_highlimit() const
{
    return impl->GetHighLimit();
}

bool TestItem::highlimit_valid() const
{
    return impl->HighLimitValid();
}

bool TestItem::lowlimit_valid() const
{
    return impl->LowLimitValid();
}

void TestItem::set_number(unsigned int number)
{
    impl->SetNumber(number);
}

unsigned int TestItem::get_number() const
{
    return impl->GetNumber();
}

void TestItem::set_label(const char* lable)
{
    impl->SetLabel(lable);
}
void TestItem::set_unit(const char* unit)
{
    impl->SetUnit(unit);
}

void TestItem::set_itemnumber(const char* number)
{
    impl->SetItemNumber(number);
}

const char* TestItem::get_label() const
{
    return impl->GetLabel();
}

const char* TestItem::get_unit() const
{
    return impl->GetUnit();
}

const char* TestItem::get_itemnumber() const
{
    return impl->GetItemNumber();
}

TestResult TestItem::get_result(unsigned index) const
{
    return impl->GetResult(index);
}

TestResult& TestItem::get_result(unsigned index)
{
    return impl->GetResult(index);
}

unsigned int TestItem::get_result_count() const
{
    return impl->GetResultCount();
}

double TestItem::get_average() const
{
    return impl->GetAverage();
}

double TestItem::get_stdev() const
{
    return impl->GetStdev();
}

double TestItem::get_max() const
{
    return impl->GetMax();
}

double TestItem::get_min() const
{
    return impl->GetMin();
}

double TestItem::get_range() const
{
    return impl->GetRange();
}

double TestItem::get_sum() const
{
    return impl->GetSum();
}
double TestItem::get_sum_squares() const
{
    return impl->GetSumSquares();
}

void TestItem::add_result(TestResult& result)
{
    impl->AddResult(result);
}

void TestItem::add_result_flag(TestResult& result)
{
    impl->AddResultAndFlag(result);
}

void TestItem::copy_item_without_result(const TestItem* src)
{
    if(!src) return;
    if(src->highlimit_valid())
        set_highlimit(src->get_highlimit());

    if(src->lowlimit_valid())
        set_lowlimit(src->get_lowlimit());

    set_itemnumber(src->get_itemnumber());
    set_label(src->get_label());
    set_number(src->get_number());
    set_unit(src->get_unit());
}

unsigned int TestItem::get_result_tested() const
{
    return impl->GetResultTested();
}

unsigned int TestItem::get_result_pass() const
{
    return impl->GetResultPass();
}

bool TestItem::is_all_pass() const
{
    return impl->IsResultAllPass();
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
TestSite::TestSite()
{
    impl = new TestSiteImpl();
#ifdef _DEBUG_API_H_
    if(impl == nullptr)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

TestSite::~TestSite()
{
    delete impl;
    impl = nullptr;
}

void TestSite::set_name(const char* name)
{
    impl->SetName(name);
}

const char* TestSite::get_name() const
{
    return impl->GetName();
}

void TestSite::set_number(unsigned char number)
{
    impl->SetNumber(number);
}

unsigned char TestSite::get_number() const
{
    return impl->GetNumber();
}

TestDevice TestSite::get_device(unsigned int index) const
{
    return impl->GetDevice(index);
}

TestBin TestSite::get_hardbin(unsigned int index) const
{
    return impl->GetHardBin(index);
}

TestBin TestSite::get_softbin(unsigned int index) const
{
    return impl->GetSoftBin(index);
}

TestItem* TestSite::get_item_by_number(unsigned int number) const
{
    return impl->GetItemByNumber(number);
}

TestItem* TestSite::get_item_by_label(const char* label) const
{
    return impl->GetItemByLabel(label);
}

TestItem* TestSite::get_item_by_itemnumber(const char* itemnumber) const
{
    return impl->GetItemByItemNumber(itemnumber);
}

TestItem* TestSite::get_item(unsigned int index) const
{
    return impl->GetItem(index);
}

unsigned int TestSite::get_item_count() const
{
    return impl->GetItemCount();
}

unsigned int TestSite::get_softbin_count() const
{
    return impl->GetSoftBinCount();
}

unsigned int TestSite::get_hardbin_count() const
{
    return impl->GetHardBinCount();
}

unsigned int TestSite::get_device_count() const
{
    return impl->GetDeviceCount();
}

void TestSite::add_device(const TestDevice& device)
{
    impl->AddDevice(device);
}

void TestSite::add_hardbin(const TestBin& hardbin)
{
    impl->AddHardBin(hardbin);
}

void TestSite::add_softbin(const TestBin& softbin)
{
    impl->AddSoftBin(softbin);
}

void TestSite::add_hardbin_bynumber(unsigned short hbin_number)
{
    impl->AddHardBinByNumber(hbin_number);
}

void TestSite::add_softbin_bynumber(unsigned short sbin_number)
{
    impl->AddSoftBinByNumber(sbin_number);
}

void TestSite::add_item(TestItem* item)
{
    impl->AddItem(item);
}

void TestSite::add_item_by_itemnumber(TestItem* item)
{
    impl->AddItemByItemNumber(item);
}

DataLogError TestSite::save_data(const char* filename) const
{
	return impl->SaveDataToFile(filename);
}
DataLogError TestSite::save_statistic(const char* filename) const
{
	return impl->SaveStatisticToFile(filename);
}

void TestSite::save_data(std::ofstream& out, bool need_title) const
{
    return impl->SaveDataToFile(out, need_title);
}
void TestSite::save_statistic(std::ofstream& out) const
{
    return impl->SaveStatisticToFile(out);
}

//////////////////////////////////////////////////////////////////////////

DataLog::DataLog()
{
    impl = new DataLogImpl();
#ifdef _DEBUG_API_H_
    if(impl == nullptr)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

DataLog::~DataLog()
{
    delete impl;
    impl = nullptr;
}

TestSite* DataLog::get_site(unsigned int index) const
{
    return impl->GetSite(index);
}

unsigned int DataLog::get_site_count() const
{
    return impl->GetSiteCount();
}

void DataLog::set_name(const char* name)
{
    impl->SetName(name);
}

const char* DataLog::get_name() const
{
    return impl->GetName();
}

DataLogError DataLog::read_datalog( const char* filename )
{
    impl->SetName(filename);
    return impl->ReadDataLog(filename);
}

DataLogError DataLog::write_data_to_csv( const char* filename )
{
    return impl->WriteDataToCSV(filename);
}

DataLogError DataLog::write_statistic_to_csv(const char* filename)
{
    return impl->WriteStatisticToCSV(filename);
}

DataLogError DataLog::write_summary_to_csv(const char* filename)
{
    return impl->WriteSummaryToCSV(filename);
}


bool DataLog::is_sbin_for_all_sites()
{
    return impl->IsSBinForAllSites();
}

bool DataLog::is_hbin_for_all_sites()
{
    return impl->IsHBinForAllSites();
}

//////////////////////////////////////////////////////////////////////////
