#ifndef _DATALOG_H_
#define _DATALOG_H_

#include <iostream>
#include <fstream>

const short INVALID_COORD_VALUE = -32768;
const unsigned char SITE_NUMBER_ALL = 255;
const unsigned short INVALID_SOFTBIN_NUMBER = 65535;
const unsigned  short INVALID_HARDBIN_NUMBER = 0;

enum TestFlag :  char
{
    RESULT_PASS = 'P',
    RESULT_FAIL = 'F',
	RESULT_TESTED = 'T',
    RESULT_INVALID = ' ',
};

enum BinType : char
{
    PASSBIN = 'P',
    FAILBIN = 'F',
    REJECTBIN = 'R',
    UNKONWNBIN = 'X',
};

enum DataLogError : int
{
    DATALOG_OPERATE_OK = 0,
    READ_FILE_ERROR = -1,
    WRITE_FILE_ERROR = -2,
    FILE_FORMATE_ERROR = -3,
    DATALOG_STDF_VERSION_NOT_SUPPORT = -4,
    DATDALOG_STDF_CPU_TYPE_NOT_SUPPORT = -5,
    NO_DATA_TO_WRIET = -6,
    NO_DATA_TO_READ = -7,
};

class TestResult
{
private:
    unsigned int m_index;
    double m_value;
    TestFlag m_flag;

public:
     void set_index(unsigned int index);
     void set_flag(TestFlag flag);
     void set_value(double value);
     unsigned int get_index() const;
     TestFlag get_flag() const;
     double get_value() const;
public:
    TestResult();
    TestResult(unsigned int index, double value, TestFlag flag) ;
};

class TestDevice
{
private:
    unsigned short m_softbin;
    unsigned short m_hardbin;
    short m_xcoord;
    short m_ycoord;
    char* m_id;
public:
    TestDevice();
    TestDevice(const TestDevice& src);
    TestDevice& operator=(const TestDevice& src);
    ~TestDevice();

public:
    void set_xcoord(short x);
    void set_ycoord(short y);
    int get_xcoord() const;
    int get_ycoord() const;
    void set_softbin(unsigned short number);
    void set_hardbin(unsigned short number);
    unsigned short get_softbin() const;
    unsigned short get_hardbin() const;

    bool set_id(const char* id);
    const char* get_id() const;
};

class TestBin
{
private:
    unsigned short m_number;
    unsigned int m_count;
    BinType m_type;
    char* m_name;
public:
    TestBin();
    TestBin(const TestBin& src);
    TestBin& operator=(const TestBin& src);
    ~TestBin();

public:
    void set_number(unsigned short number);
    void set_count(unsigned int count);
    void add_count();
    void set_type(BinType type);
    bool set_name(const char* name);

    unsigned short get_number() const;
    unsigned int get_count() const;
    BinType get_type() const;
    const char* get_name() const;
};

class TestItem
{
public:
    TestItem();
    ~TestItem();
    void set_lowlimit(double limit);
    void set_highlimit(double limit);
    double get_lowlimit() const;
    double get_highlimit() const;
    bool highlimit_valid() const;
    bool lowlimit_valid() const;

    void set_number(unsigned int number);
    unsigned int get_number() const;

    void set_label(const char* lable);
    void set_unit(const char* unit);
    void set_itemnumber(const char* number);
    const char* get_label() const;
    const char* get_unit() const;
    const char* get_itemnumber() const;

    TestResult get_result(unsigned index) const;
    TestResult& get_result(unsigned index);
    unsigned int get_result_count() const;
    double get_average() const;
    double get_stdev() const;
    double get_max() const;
    double get_min() const;
    double get_range() const;
    double get_sum() const;
    double get_sum_squares() const;

    void add_result(TestResult& result);
    void add_result_flag(TestResult& result);
    void copy_item_without_result(const TestItem* src);

    unsigned int get_result_tested() const;
    unsigned int get_result_pass() const;
    bool is_all_pass() const;
private:
    TestItem(const TestItem& src);
    TestItem& operator=(const TestItem&);
    typedef class TestItemImpl Impl;
    Impl *impl;
};

class TestSite
{
public:
    TestSite();
    ~TestSite();
    void set_name(const char* name);
    const char* get_name() const;
    void set_number(unsigned char number);
    unsigned char get_number() const;

    void add_device(const TestDevice& device);
    void add_hardbin(const TestBin& hardbin);
    void add_softbin(const TestBin& softbin);
    void add_hardbin_bynumber(unsigned short hbin_number);
    void add_softbin_bynumber(unsigned short sbin_number);
    void add_item(TestItem* item);
    void add_item_by_itemnumber(TestItem* item);

    TestDevice get_device(unsigned int index) const;
    TestBin get_hardbin(unsigned int index) const;
    TestBin get_softbin(unsigned int index) const;

    TestItem* get_item_by_number(unsigned int number) const;
    TestItem* get_item_by_label(const char* label) const;
    TestItem* get_item_by_itemnumber(const char* itemnumber) const;
    TestItem* get_item(unsigned int index) const;

    unsigned int get_item_count() const;
    unsigned int get_softbin_count() const;
    unsigned int get_hardbin_count() const;
    unsigned int get_device_count() const;

	DataLogError save_data(const char* filename) const;
	DataLogError save_statistic(const char* filename) const;
    void save_data(std::ofstream& out, bool need_title) const;
    void save_statistic(std::ofstream& out) const;
private:
    TestSite(const TestSite&);
    TestSite& operator=(const TestSite&);
    typedef class TestSiteImpl Impl;
    Impl* impl;
};

class DataLog
{
public:
    DataLog();
    ~DataLog();
    DataLogError read_datalog(const char* filename);
    DataLogError write_data_to_csv(const char* filename);
    DataLogError write_statistic_to_csv(const char* filename);
    DataLogError write_summary_to_csv(const char* filename);
    TestSite* get_site(unsigned int index) const;
    unsigned int get_site_count() const;
    void set_name(const char* name);
    const char* get_name() const;
    bool is_sbin_for_all_sites();
    bool is_hbin_for_all_sites();
private:
    typedef class DataLogImpl Impl;
    Impl *impl;
};




#endif//_DATALOG_H_
