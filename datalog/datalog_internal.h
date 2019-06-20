#ifndef _DATALOG_INTERNAL_H_
#define _DATALOG_INTERNAL_H_

#include <vector>
#include <string>

#include "datalog.h"
#include "stdf_v4_api.h"
#include "csv_api.h"

enum DataLogType : int
{
    STDF_FILE = 0,
    LOG_FILE = 1,
    CSV_FILE = 2,
    TXT_FILE = 3,
    TAB_FILE = 4,
    OTHER_FILE = -1,
};

class TestItemImpl
{
private:
    unsigned int TestNumber;

    double LowLimit;
    double HighLimit;
    bool LowLimitFlag;
    bool HighLimitFlag;

    unsigned int ResultTestedCount;
    unsigned int ResultPassCount;

    double MinValue;
    double MaxValue;
    double SumOfValues;
    double SumOfSquares;

    std::string TestLabel;
    std::string TestUnit;
    std::string ItemNumber;

    std::vector<TestResult> ResultList;
public:
    TestItemImpl();
    ~TestItemImpl();

public:
    void SetLowLimit(double limit);
    void SetHighLimit(double limit);
    double GetLowLimit() const;
    double GetHighLimit() const;
    bool LowLimitValid() const;
    bool HighLimitValid() const;

    void SetNumber(unsigned int number);
    unsigned int GetNumber() const;

    void SetLabel(const char* lable);
    void SetUnit(const char* unit);
    void SetItemNumber(const char* number);
    const char* GetLabel() const;
    const char* GetUnit() const;
    const char* GetItemNumber() const;

    void AddResult(TestResult& result);
    void AddResultAndFlag( TestResult& result);
    TestResult GetResult(unsigned index) const;
    TestResult& GetResult(unsigned index);
    unsigned int GetResultCount() const;
    double GetAverage() const;
    double GetStdev() const;
    double GetMax() const;
    double GetMin() const;
    double GetRange() const;
    double GetSum() const;
    double GetSumSquares() const;

    unsigned int GetResultTested() const;
    unsigned int GetResultPass() const;
    bool IsResultAllPass() const;

private:
    TestItemImpl(const TestItemImpl&);
    TestItemImpl& operator=(const TestItemImpl&);
};


class TestSiteImpl
{
private:
    std::string Name;
    unsigned char Number;

    std::vector<TestDevice> DeviceList;
    std::vector<TestBin> HardBinList;
    std::vector<TestBin> SoftBinList;
    std::vector<TestItem*> ItemlList;

public:
    TestSiteImpl();
    ~TestSiteImpl();

public:
    void SetName(const char* name);
    const char* GetName() const;
    void SetNumber(unsigned char number);
    unsigned char GetNumber() const;

    void AddDevice(const TestDevice& device);
    void AddHardBin(const TestBin& hardbin);
    void AddSoftBin(const TestBin& softbin);

    void AddHardBinByNumber(unsigned short hbin_number);
    void AddSoftBinByNumber(unsigned short sbin_number);

    void AddItem(TestItem* item);
    void AddItemByItemNumber(TestItem* item);

    TestDevice GetDevice(unsigned int index) const;
    TestBin GetHardBin(unsigned int index) const;
    TestBin GetSoftBin(unsigned int index) const;

    TestItem* GetItemByNumber(unsigned int number);
    TestItem* GetItemByLabel(const char* label);
    TestItem* GetItemByItemNumber(const char* itemnumber);
    TestItem* GetItem(unsigned int index);

    unsigned int GetItemCount() const;
    unsigned int GetSoftBinCount() const;
    unsigned int GetHardBinCount() const;
    unsigned int GetDeviceCount() const;

	DataLogError SaveStatisticToFile(const char* filename) const;
	DataLogError SaveDataToFile(const char* filename) const;
    void SaveStatisticToFile(std::ofstream& out) const;
    void SaveDataToFile(std::ofstream& out, bool need_title) const;

private:
    TestSiteImpl(const TestSiteImpl& );
    TestSiteImpl& operator=(const TestSiteImpl& );
};

//========================================================
class TempDataHolder
{
public:
    std::vector<std::string> ItemNumberList;
    std::vector<double> ValueList;
    std::vector<TestFlag> ItemFlagList;
public:
    TempDataHolder();
    ~TempDataHolder();
    void Clear();
};

//=======================================================
class DataLogImpl
{
private:
    std::string Name;
    DataLogType Type;
    std::vector<TestSite*> SiteList;
    bool HBin_For_All_Sites;
    bool SBin_For_All_Sites;
public:
    DataLogImpl();
    ~DataLogImpl();
public:
    DataLogError ReadDataLog(const char* filename);
    DataLogError WriteDataToCSV(const char* filename);
    DataLogError WriteStatisticToCSV(const char* filename);
    DataLogError WriteSummaryToCSV(const char* filename);

    void SetName(const char* name);
    const char* GetName();
    TestSite* GetSiteByNumber(unsigned char number);
    TestSite* GetSite(unsigned int index);
    unsigned int GetSiteCount();
    bool IsSBinForAllSites();
    bool IsHBinForAllSites();
private:
    DataLogError ReadFromSTDF(const char* filename);
    DataLogError ReadFromLog(const char* filename);
    DataLogError ReadFromCSV(const char* filename);
    DataLogError ReadFromTXT(const char* filename);
    DataLogError ReadFromTabXLS(const char* filename);

    void SetSiteList(const StdfSDR& sdr);
    void SetItemValue(TestItem* item, const StdfPTR& ptr);
    void SetItemValue(TestItem* item, CSVParser& parser, unsigned int test_number);
    void SetDeviceValue(TestDevice& device, const StdfPRR& prr);
    void SetResultValue(TestResult& result, const StdfPTR& ptr, unsigned int index);
    void SetHardBinValue(TestBin& hardbin, const StdfHBR& hbr);
    void SetHardBinForAllSite(TestBin& hardbin);
    void SetSoftBinValue(TestBin& softbin, const StdfSBR& sbr);
    void SetSoftBinForAllSite(TestBin& softbin);
    void SetBinValue(TestBin& bin,const CSVParser& parser);
    void SetSoftBinForAllSite(const CSVParser& parser);
    void SetHardBinForAllSite(const CSVParser& parser);
    void SetDeviceVaue(TestDevice& device, const CSVParser& parser);
    void SetTempDataHolder(TempDataHolder& holder, const CSVParser& parser);
    void SetItemsForSite(TestSite* site, TempDataHolder& holder);
    DataLogType GetFileTypeBySuffix(const char* filename);
    TestSite* CreateNewSite(TestSite* old_site, unsigned int site_number);
    std::string ComposeSiteName(unsigned int site_number);
    void CopyGroupSiteItemsDetailToEachSite(TestSite* group_site);
    void ClearSiteList();
};

#endif//_DATALOG_INTERNAL_H_
