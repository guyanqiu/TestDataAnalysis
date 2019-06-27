#include "datalog_internal.h"
#include "datalog.h"
#include "csv_api.h"
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <locale>
#include <map>

//#define _DEBUG_FOR_DEBUG_
#ifdef _DEBUG_FOR_DEBUG_
#define dprintf(msg, ...)      { printf( msg,  ##__VA_ARGS__ );}
#else
#define dprintf(msg, ...)
#endif

/********************  Define for Log File *******************************/
#define LOG_FILE_MARK       120     // The First Line of Log File
#define LOG_ITEMDESCR_MARK  10      // Test Item Description Line Mark for LOG File
#define LOG_TESTDATAS_MARK  100     // Test Result Record Line Mark for LOG File
#define LOG_BINRESULT_MARK  130     // Bin Information Line Mark for LOG File
#define LOG_HBININFO_MARK   60      // HardBin Information Line Mark for LOG File
#define LOG_SBININFO_MARK   50      // SoftBin Information Line Mark for LOG File
#define LOG_UPPER_INDEX     3       // High Limit Index in the Line
#define LOG_LOWER_INDEX     4       // Low Limit Index in the Line
#define LOG_UNITS_INDEX     5       // Test Unit Index in the Line
#define LOG_TESTLABEL_INDEX 6       // Test Label Index in the Line
#define LOG_TESTDATAS_INDEX 4       // Test Result Index in the Line
#define LOG_DEVICE_NUMBER   3       // Test Device Index in the Line
#define LOG_ITEM_NUMBER     1       // The Item Number Index int the Line

#define LOG_SITE_NUMBER     1       // The Site Number Index in the Line
#define LOG_ITEM_PASS       3       // The Item Pass or Fail Flag Index in the Line
#define LOG_DEVICE_PASS     4       // The Device Pass or Fail Flag Index in the Line
#define LOG_DEVICE_XCoord   5       // The X Coordinate Index of the Device Under Test in the Line
#define LOG_DEVICE_YCoord   6       // The Y Coordinate Index of the Device Under Test in the Line
#define LOG_SBIN_NUMBER     7       // SoftBin Number Index
#define LOG_HBIN_NUMBER     8       // HardBin Number Index int the Line

#define LOG_BININFO_NUMBER  1       // Bin Number Index int he line
#define LOG_BININFO_TYPE    2       // Bin type Index int he line
#define LOG_BININFO_COUNT   3       // Bin Count Index int he line
#define LOG_BININFO_NAME    4       // Bin Name Index int he line


//////////////////////////////////////////////////////////////////////////

static double convert_units(const std::string& exit_unit,const std::string& new_unit, bool& need_convert)
{
    if(exit_unit == new_unit)
    {
        need_convert = false;
        return 1.0;
    }

    std::map<std::string, double> Unit_Convert_Ratio;
    std::map<std::string, double>::iterator it;
    Unit_Convert_Ratio["u"] = 1.0e-6;
    Unit_Convert_Ratio["m"] = 1.0e-3;
    Unit_Convert_Ratio["n"] = 1.0e-9;
    Unit_Convert_Ratio["p"] = 1.0e-12;
    Unit_Convert_Ratio["f"] = 1.0e-15;
    Unit_Convert_Ratio["k"] = 1.0e3;
    Unit_Convert_Ratio["M"] = 1.0e6;
    Unit_Convert_Ratio["G"] = 1.0e9;

    int exit_length = exit_unit.length();
    int new_length = new_unit.length();

    if(exit_length > new_length)
    {
        std::size_t found = exit_unit.find(new_unit);
        if(found != std::string::npos)
        {
            std::string prefix_unit = exit_unit.substr(0, found);
            it = Unit_Convert_Ratio.find(prefix_unit);
            if(it != Unit_Convert_Ratio.end())
            {
                need_convert = true;
                return 1.0/Unit_Convert_Ratio[prefix_unit];
            }
        }
        else
        {
            need_convert = false;
            return 1.0;
        }
    }

    if(new_length > exit_length)
    {
        std::size_t found = new_unit.find(exit_unit);
        if(found != std::string::npos)
        {
            std::string prefix_unit = new_unit.substr(0, found);
            it = Unit_Convert_Ratio.find(prefix_unit);
            if(it != Unit_Convert_Ratio.end())
            {
                need_convert = true;
                return Unit_Convert_Ratio[prefix_unit];
            }
        }
        else
        {
            need_convert = false;
            return 1.0;
        }
    }

    if(new_length == exit_length)
    {
        double ratio_new = 1.0;
        double ratio_exit = 1.0;
        std::string prefix_new = new_unit.substr(0, 1);
        it = Unit_Convert_Ratio.find(prefix_new);
        if(it != Unit_Convert_Ratio.end())
        {
            std::string prefix_exit = exit_unit.substr(0, 1);
            it = Unit_Convert_Ratio.find(prefix_exit);
            {
                need_convert = true;
                ratio_new = Unit_Convert_Ratio[prefix_new];
                ratio_exit = Unit_Convert_Ratio[prefix_exit];
                return  ratio_exit/ratio_new;
            }
        }
    }

    need_convert = false;
    return 1.0;
}


TestItemImpl::TestItemImpl()
{
	TestNumber = 0;

	HighLimit = 0.0;
	LowLimit = 0.0;
	HighLimitFlag = false;
	LowLimitFlag = false;

	ResultPassCount = 0;
	ResultTestedCount = 0;

	MinValue = 0.0;
	MaxValue = 0.0;

	SumOfValues = 0.0;
	SumOfSquares = 0.0;
}

TestItemImpl::~TestItemImpl()
{
	if(!ResultList.empty()) ResultList.clear();
}

void TestItemImpl::SetLowLimit( double limit )
{
	LowLimitFlag = true;
	LowLimit = limit;
}

void TestItemImpl::SetHighLimit( double limit )
{
	HighLimitFlag = true;
	HighLimit = limit;
}

double TestItemImpl::GetLowLimit() const
{
	return LowLimit;
}

double TestItemImpl::GetHighLimit() const
{
	return HighLimit;
}

void TestItemImpl::SetNumber( unsigned int number )
{
	TestNumber = number;
}

unsigned int TestItemImpl::GetNumber() const
{
	return TestNumber;
}

void TestItemImpl::SetLabel( const char* lable )
{
	if(lable) TestLabel.assign(lable);
}

void TestItemImpl::SetUnit( const char* unit )
{
	if(unit) TestUnit.assign(unit);
}

void TestItemImpl::SetItemNumber( const char* number )
{
	if(number) ItemNumber.assign(number);
}

const char* TestItemImpl::GetLabel() const
{
	if(TestLabel.empty()) return nullptr;
	return TestLabel.c_str();
}

const char* TestItemImpl::GetUnit() const
{
	if(TestUnit.empty()) return nullptr;
	return TestUnit.c_str();
}

const char* TestItemImpl::GetItemNumber() const
{
	if(ItemNumber.empty()) return nullptr;
	return ItemNumber.c_str();
}

void TestItemImpl::AddResult(TestResult& result )
{
	if(result.get_flag() == RESULT_INVALID) return;
	double value = result.get_value();

	if(ResultTestedCount == 0)
	{
		MinValue = value;
		MaxValue = value;
	}
	else
	{
		if(value < MinValue) MinValue = value;
		if(MaxValue < value) MaxValue = value;
	}

	SumOfValues += value;
	SumOfSquares += (value*value);

	ResultTestedCount++;
	if(result.get_flag() == RESULT_PASS) ResultPassCount++;
    //if(result.get_index() > ResultTestedCount) result.set_index(ResultTestedCount);

	ResultList.push_back(result);
}

TestResult TestItemImpl::GetResult( unsigned index ) const
{
	TestResult result;
	unsigned int range = ResultList.size();
	if(range <= index)
	{
#ifdef _DEBUG_API_H_
		SHOW_INDEX_ERROR(range, index);
		throw INDEX_OVER_RANGE(range, index);
#endif
		return result;
	}
	else
	{
		return ResultList[index];
	}
}

TestResult& TestItemImpl::GetResult(unsigned index)
{
    return ResultList.at(index);
}

unsigned int TestItemImpl::GetResultCount() const
{
	return ResultList.size();
}

bool TestItemImpl::LowLimitValid() const
{
	return LowLimitFlag;
}

bool TestItemImpl::HighLimitValid() const
{
	return HighLimitFlag;
}

void TestItemImpl::AddResultAndFlag( TestResult& result )
{
	if(result.get_flag() == RESULT_INVALID) return;

	double value = result.get_value();
	if(HighLimitFlag)
	{
		if(LowLimitFlag)
		{
			if(value <= HighLimit && value >= LowLimit)
				result.set_flag(RESULT_PASS);
			else
				result.set_flag(RESULT_FAIL);
		}
		else
		{
			if(value <= HighLimit)
				result.set_flag(RESULT_PASS);
			else
				result.set_flag(RESULT_FAIL);
		}
	}
	else if(LowLimitFlag)
	{
		if(value >= LowLimit)
			result.set_flag(RESULT_PASS);
		else
			result.set_flag(RESULT_FAIL);
	}
	else
	{
        result.set_flag(RESULT_PASS);
	}
	AddResult(result);
}

double TestItemImpl::GetMax() const
{
	return MaxValue;
}

double TestItemImpl::GetMin() const
{
	return MinValue;
}

double TestItemImpl::GetRange() const
{
	return (MaxValue - MinValue);
}

double TestItemImpl::GetSum() const
{
    return SumOfValues;
}
double TestItemImpl::GetSumSquares() const
{
    return SumOfSquares;
}

unsigned int TestItemImpl::GetResultTested() const
{
	return ResultTestedCount;
}

unsigned int TestItemImpl::GetResultPass() const
{
	return ResultPassCount;
}

bool TestItemImpl::IsResultAllPass() const
{
	if(ResultPassCount == ResultTestedCount)
		return true;
	else
		return false;
}


double TestItemImpl::GetAverage() const
{
	unsigned int count = ResultList.size();
	if(count == 0) return 0;

	return (SumOfValues/count);
}


double TestItemImpl::GetStdev() const
{
	unsigned int count = ResultList.size();
	if(count <= 1) return 0.0;

	double temp = SumOfSquares - (SumOfValues/count)*SumOfValues;
	if(temp < 0.0) return 0.0;
	else return std::sqrt(temp/(count-1));
}

//////////////////////////////////////////////////////////////////////////
TestSiteImpl::TestSiteImpl()
{
	Number = 1;
}

TestSiteImpl::~TestSiteImpl()
{
	DeviceList.clear();
	HardBinList.clear();
	SoftBinList.clear();

	unsigned int count = ItemlList.size();
	for(unsigned int i = 0; i < count; i++)
	{
		TestItem* temp = ItemlList[i];
		delete temp;
		ItemlList[i] = nullptr;
	}
	ItemlList.clear();
}

void TestSiteImpl::SetName( const char* name )
{
	if(name) Name.assign(name);
}

const char* TestSiteImpl::GetName() const
{
	if(Name.empty()) return nullptr;
	return Name.c_str();
}

void TestSiteImpl::SetNumber(unsigned char number)
{
	Number = number;
}

unsigned char TestSiteImpl::GetNumber() const
{
	return Number;
}

void TestSiteImpl::AddDevice( const TestDevice& device )
{
	//DeviceList.push_back(device);
	const char* id = device.get_id();
	unsigned int device_count = DeviceList.size();
	if(!id || device_count == 0)
	{
		DeviceList.push_back(device);
	}
	else
	{
		const char* current_id = DeviceList[device_count-1].get_id();
		if(!current_id)
		{
			DeviceList.push_back(device);
			return;
		}

		if(std::strcmp(current_id, id) != 0)
		{
			DeviceList.push_back(device);
		}
	}
}

void TestSiteImpl::AddHardBin( const TestBin& hardbin )
{
	HardBinList.push_back(hardbin);
}

void TestSiteImpl::AddSoftBin( const TestBin& softbin )
{
	SoftBinList.push_back(softbin);
}

void TestSiteImpl::AddHardBinByNumber(unsigned short hbin_number)
{
	unsigned int count  = HardBinList.size();
	bool number_is_exsit = false;
	for(unsigned int i = 0; i < count ; i++)
	{
		TestBin& bin = HardBinList[i];
		if(bin.get_number() == hbin_number)
		{
			number_is_exsit = true;
			bin.add_count();
			break;
		}
	}
	if(!number_is_exsit)
	{
		//std::string number = std::to_string((unsigned long long)hbin_number);
		char number[8];
		std::sprintf(number, "%u", hbin_number);
		std::string str_bin = "HBIN";
		str_bin.append(number);
		TestBin newbin;
		newbin.set_name(str_bin.c_str());
		newbin.set_number(hbin_number);
		newbin.set_type(UNKONWNBIN);
		newbin.add_count();
		HardBinList.push_back(newbin);
	}
}

void TestSiteImpl::AddSoftBinByNumber(unsigned short sbin_number)
{
	unsigned int count  = SoftBinList.size();
	bool number_is_exsit = false;
	for(unsigned int i = 0; i < count ; i++)
	{
		TestBin& bin = SoftBinList[i];
		if(bin.get_number() == sbin_number)
		{
			number_is_exsit = true;
			bin.add_count();
			break;
		}
	}
	if(!number_is_exsit)
	{
		//std::string number = std::to_string((unsigned long long)sbin_number);
		char number[8];
		std::sprintf(number, "%u", sbin_number);
		std::string str_bin = "SBIN";
		str_bin.append(number);
		TestBin newbin;
		newbin.set_name(str_bin.c_str());
		newbin.set_number(sbin_number);
		newbin.set_type(UNKONWNBIN);
		newbin.add_count();
		SoftBinList.push_back(newbin);
	}
}

void TestSiteImpl::AddItem(TestItem* item)
{
	ItemlList.push_back(item);
}

void TestSiteImpl::AddItemByItemNumber(TestItem* item)
{
    TestItem* temp_item = GetItemByItemNumber(item->get_itemnumber());
    if(!temp_item) ItemlList.push_back(item);
}

TestDevice TestSiteImpl::GetDevice( unsigned int index ) const
{
	TestDevice device;
	unsigned int range = DeviceList.size();
	if(range <= index)
	{
#ifdef _DEBUG_API_H_
		SHOW_INDEX_ERROR(range, index);
		throw INDEX_OVER_RANGE(range, index);
#endif
		return device;
	}
	return DeviceList[index];
}

TestBin TestSiteImpl::GetHardBin( unsigned int index ) const
{
	TestBin bin;
	unsigned int range = HardBinList.size();
	if(range <= index)
	{
#ifdef _DEBUG_API_H_
		SHOW_INDEX_ERROR(range, index);
		throw INDEX_OVER_RANGE(range, index);
#endif
		return bin;
	}
	return HardBinList[index];
}

TestBin TestSiteImpl::GetSoftBin( unsigned int index ) const
{
	TestBin bin;
	unsigned int range = SoftBinList.size();
	if(range <= index)
	{
#ifdef _DEBUG_API_H_
		SHOW_INDEX_ERROR(range, index);
		throw INDEX_OVER_RANGE(range, index);
#endif
		return bin;
	}
	return SoftBinList[index];
}

TestItem* TestSiteImpl::GetItemByNumber( unsigned int number )
{
	unsigned int count = ItemlList.size();
	if(number && number < count)
	{
		TestItem* temp = ItemlList[number-1];
		if(temp->get_number() == number)
			return temp;
	}
	for(unsigned int i = 0; i < count; i++)
	{
		TestItem* temp = ItemlList[i];
		if(temp->get_number() == number)
			return temp;
	}
	return nullptr;
}

TestItem* TestSiteImpl::GetItemByLabel(const char* label)
{
	if(!label) return nullptr;
	unsigned int count = ItemlList.size();
	for(unsigned int i = 0; i < count; i++)
	{
		TestItem* temp = ItemlList[i];
		const char* temp_label = temp->get_label();
		if(temp_label && std::strcmp(label, temp_label) == 0)
			return temp;
	}
	return nullptr;
}

TestItem* TestSiteImpl::GetItemByItemNumber(const char* itemnumber)
{
	if(!itemnumber) return nullptr;
	unsigned int count = ItemlList.size();
	for(unsigned int i = 0; i < count; i++)
	{
		TestItem* temp = ItemlList[i];
		const char* temp_itemnumber = temp->get_itemnumber();
		if(temp_itemnumber && std::strcmp(itemnumber, temp_itemnumber) == 0)
			return temp;
	}
	return nullptr;
}

unsigned int TestSiteImpl::GetItemCount() const
{
	return ItemlList.size();
}

unsigned int TestSiteImpl::GetSoftBinCount() const
{
	return SoftBinList.size();
}

unsigned int TestSiteImpl::GetHardBinCount() const
{
	return HardBinList.size();
}

unsigned int TestSiteImpl::GetDeviceCount() const
{
	return DeviceList.size();
}

TestItem* TestSiteImpl::GetItem( unsigned int index )
{
	unsigned int range = ItemlList.size();
	if(index >= range)
	{
#ifdef _DEBUG_API_H_
		SHOW_INDEX_ERROR(range, index);
		throw INDEX_OVER_RANGE(range, index);
#endif
		return nullptr;
	}

	return ItemlList[index];
}

DataLogError TestSiteImpl::SaveStatisticToFile(const char* filename) const
{
	std::ofstream out(filename, std::ios::out );
	if(!out) return WRITE_FILE_ERROR;

    SaveStatisticToFile(out);

	out.close();
    return DATALOG_OPERATE_OK;
}

void TestSiteImpl::SaveStatisticToFile(std::ofstream& out) const
{
    out<<"Test Number,Test Label,Low Limit,High Limit,Unit,Device Tested,Device Pass,Yield,Min,Average,Max,Range,StDev\n";
    unsigned int item_count = ItemlList.size();
    for(unsigned int n = 0; n < item_count; n++)
    {
        TestItem* item = ItemlList[n];

        const char* lable = item->get_label();
        unsigned int test_number = item->get_number();
        const char* unit = item->get_unit();
        double hight_limit = item->get_highlimit();
        double low_limit = item->get_lowlimit();
        bool has_lowlimit = item->lowlimit_valid();
        bool has_highlimit = item->highlimit_valid();

        unsigned int test_count = item->get_result_tested();
        unsigned int pass_count = item->get_result_pass();
        double average = item->get_average();
        double stdev = item->get_stdev();
        double max_val = item->get_max();
        double min_val = item->get_min();
        double range = item->get_range();
        double yield = 0.0;
        if(test_count != 0) yield = 100.0*pass_count/test_count;

        out<<test_number<<",";
        out<< CSVWriter(lable) <<",";

        if(has_lowlimit) out<<low_limit<<","; else out<<",";
        if(has_highlimit) out<<hight_limit<<","; else out<<",";
        if(unit) out<< CSVWriter(unit) <<","; else out<<",";
        out<<test_count<<",";
        out<<pass_count<<",";
        out<<yield<<"%,";
        out<<min_val<<",";
        out<<average<<",";
        out<<max_val<<",";
        out<<range<<",";
        out<<stdev<<std::endl;
    }
    out<<std::endl;
}

DataLogError TestSiteImpl::SaveDataToFile(const char* filename) const
{
	std::ofstream out(filename, std::ios::out );
	if(!out) return WRITE_FILE_ERROR;

	unsigned int item_count = ItemlList.size();
	if(item_count == 0) return NO_DATA_TO_WRIET;

    SaveDataToFile(out, true);

	out<<"\n";
	out.close();
    return DATALOG_OPERATE_OK;
}

void TestSiteImpl::SaveDataToFile(std::ofstream& out, bool need_title) const
{
    unsigned int item_count = ItemlList.size();
    if(need_title)
    {
        out<<"Test Name,,,,,,";
        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = ItemlList[n];
            const char* label = item->get_label();
            out<< CSVWriter(label) <<",";
        }
        out<<"\n";

        out<<"Test Number,,,,,,";
        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = ItemlList[n];
            const char* itemlabel = item->get_itemnumber();
            if(!itemlabel)
                out<<item->get_number()<<",";
            else
                out<<itemlabel<<",";
        }
        out<<"\n";

        out<<"Low Limit,,,,,,";
        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = ItemlList[n];
            if(item->lowlimit_valid())
                out<<item->get_lowlimit()<<",";
            else out<< ",";
        }
        out<<"\n";

        out<<"Upper Limit,,,,,,";
        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = ItemlList[n];
            if(item->highlimit_valid())
                out<<item->get_highlimit()<<",";
            else out<< ",";
        }
        out<<"\n";

        out<<"Units,,,,,,";
        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = ItemlList[n];
            const char* unit = item->get_unit();
            if(unit) out<< CSVWriter(unit)<<",";
            else out<<",";
        }
        out<<"\n";
        out<<"Site #,Serial#,SBin,HBin,XCoord,YCoord\n";
    }

    unsigned int site_number = Number;
    unsigned int device_count = DeviceList.size();
    std::vector<unsigned int> count_of_result(item_count, 0);
    for(unsigned int d = 0; d < device_count; d++)
    {
        TestDevice device = DeviceList[d];
        out<<site_number<<",";
        out<<d+1<<",";

        unsigned short soft_bin = device.get_softbin();
        unsigned short hard_bin = device.get_hardbin();

        if(soft_bin == INVALID_SOFTBIN_NUMBER || hard_bin == INVALID_HARDBIN_NUMBER)
            out<<"N/A,N/A,";
        else
            out<<soft_bin<<","<<hard_bin<<",";

        short x = device.get_xcoord();
        short y = device.get_ycoord();

        if(x == INVALID_COORD_VALUE || y == INVALID_COORD_VALUE)
            out<<"N/A,N/A,";
        else
            out<<x<<","<<y<<",";

        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = ItemlList[n];
            unsigned int result_count = item->get_result_count();
            if(count_of_result[n] < result_count)
            {
                TestResult result = item->get_result(count_of_result[n]);
                unsigned int result_index = result.get_index();

                if(d+1 == result_index)
                {
                    out<<result.get_value()<<",";
                    count_of_result[n]++;
                }
                else
                    out<<",";
            }
            else out<<",";
        }
        out<<"\n";
    }
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
TempDataHolder::TempDataHolder()
{

}

TempDataHolder::~TempDataHolder()
{
	Clear();
}

void TempDataHolder::Clear()
{
	ItemNumberList.clear();
	ValueList.clear();
	ItemFlagList.clear();
}

//////////////////////////////////////////////////////////////////////////
DataLogImpl::DataLogImpl()
{
	Type = OTHER_FILE;
    HBin_For_All_Sites = false;
    SBin_For_All_Sites = false;
}

DataLogImpl::~DataLogImpl()
{
	ClearSiteList();
}

void DataLogImpl::ClearSiteList()
{
	unsigned int count = SiteList.size();
	for(unsigned int i = 0; i < count; i++ )
	{
		TestSite* temp = SiteList[i];
		delete temp;
	}
	SiteList.clear();
}

void DataLogImpl::SetName(const char* name)
{
	if(!name) return;
	std::string temp_name(name);
	std::string name_only;
	std::string::size_type pos = temp_name.find_last_of('\\');
	if(pos != std::string::npos)
		name_only = temp_name.substr(pos+1);
	else
	{
		pos = temp_name.find_last_of('/');
		if(pos != std::string::npos)
			name_only = temp_name.substr(pos+1);
		else
			name_only.assign(name);
	}

	Name.assign(name_only);
}

const char* DataLogImpl::GetName()
{
	if(Name.empty()) return nullptr;
	return Name.c_str();
}

TestSite* DataLogImpl::GetSiteByNumber(unsigned char number)
{
	unsigned int count = SiteList.size();
	for(unsigned int i = 0; i < count; i++)
	{
		TestSite* temp = SiteList[i];
		if(temp->get_number() == number)
			return temp;
	}
	return nullptr;
}

TestSite* DataLogImpl::GetSite( unsigned int index )
{
	unsigned int range = SiteList.size();
	if(index >= range)
	{
#ifdef _DEBUG_API_H_
		SHOW_INDEX_ERROR(range, index);
		throw INDEX_OVER_RANGE(range, index);
#endif
		return nullptr;
	}

	return SiteList[index];
}

std::string DataLogImpl::ComposeSiteName(unsigned int site_number)
{
    std::string site_name = Name;
    site_name.append("-site");
    //std::string number = std::to_string((unsigned long long)site_number);
    char number[8];
    std::sprintf(number, "%u", site_number);
    site_name.append(number);
    return site_name;
}

void DataLogImpl::SetSiteList(const StdfSDR& sdr)
{
	unsigned int site_count = sdr.get_site_count();
	for(unsigned int i = 0; i < site_count; i++)
	{
		TestSite* site = new TestSite();
		if(!site)
		{
#ifdef _DEBUG_API_H_
			SHOW_MEMORY_ERROR();
			throw MEMORY_ALLOCATED_ERROR();
#endif // _DEBUG_API_H_
			return;
		}
		unsigned char site_number = sdr.get_site_number(i);
		site->set_number(site_number);
        std::string site_name = ComposeSiteName(site_number);
		site->set_name(site_name.c_str());
		SiteList.push_back(site);
	}
}

void DataLogImpl::SetItemValue(TestItem* item, const StdfPTR& ptr)
{
	if(!item) return;
    if(! ptr.no_low_limit())
	{
		double low_limit = ptr.get_low_limit();
		item->set_lowlimit(low_limit);
	}
    if(! ptr.no_high_limit())
	{
		double high_limit = ptr.get_high_limit();
		item->set_highlimit(high_limit);
	}
	item->set_label(ptr.get_test_text());
	item->set_unit(ptr.get_unit());
	item->set_number(ptr.get_test_number());
}

void DataLogImpl::SetDeviceValue(TestDevice& device, const StdfPRR& prr)
{
	device.set_xcoord(prr.get_x_coordinate());
	device.set_ycoord(prr.get_y_coordinate());
	device.set_softbin(prr.get_softbin_number());
	device.set_hardbin(prr.get_hardbin_number());
	device.set_id(prr.get_part_id());
}

void DataLogImpl::SetDeviceVaue(TestDevice& device,const CSVParser& parser)
{
	device.set_hardbin(std::atoi(parser.get_string(LOG_HBIN_NUMBER)));
	device.set_softbin(std::atoi(parser.get_string(LOG_SBIN_NUMBER)));
	const char* x = parser.get_string(LOG_DEVICE_XCoord);
	if(x && std::strlen(x) > 0) device.set_xcoord(std::atoi(x));
	const char* y = parser.get_string(LOG_DEVICE_YCoord);
	if(y && std::strlen(y) > 0) device.set_ycoord(std::atoi(y));
}

void DataLogImpl::SetResultValue(TestResult& result, const StdfPTR& ptr, unsigned int index)
{
	result.set_index(index);
	result.set_value(ptr.get_result());
	bool is_fail = ptr.test_failed();
	if(is_fail) result.set_flag(RESULT_FAIL);
	else result.set_flag(RESULT_PASS);
}

void DataLogImpl::SetHardBinValue(TestBin& hardbin, const StdfHBR& hbr)
{
	hardbin.set_count(hbr.get_hardbin_count());
	hardbin.set_name(hbr.get_hardbin_name());
	hardbin.set_number(hbr.get_hardbin_number());
	char bin_type = hbr.get_hardbin_indication();
	if(bin_type == 'P') hardbin.set_type(PASSBIN);
	else if(bin_type == 'F') hardbin.set_type(FAILBIN);
    else if(bin_type == 'R') hardbin.set_type(REJECTBIN);
	else hardbin.set_type(UNKONWNBIN);
}

void DataLogImpl::SetHardBinForAllSite(TestBin& hardbin)
{
	unsigned int scount = SiteList.size();
	for(unsigned int s = 0; s < scount; s++)
	{
		TestSite* site_temp = GetSite(s);
		if(!site_temp) break;
		site_temp->add_hardbin(hardbin);
	}
}

void DataLogImpl::SetSoftBinValue(TestBin& softbin, const StdfSBR& sbr)
{
	softbin.set_count(sbr.get_softbin_count());
	softbin.set_name(sbr.get_softbin_name());
	softbin.set_number(sbr.get_softbin_number());
	char bin_type = sbr.get_softbin_indication();
	if(bin_type == 'P') softbin.set_type(PASSBIN);
	else if(bin_type == 'F') softbin.set_type(FAILBIN);
    else if(bin_type == 'R') softbin.set_type(REJECTBIN);
	else softbin.set_type(REJECTBIN);
}

void DataLogImpl::SetSoftBinForAllSite(TestBin& softbin)
{
	unsigned int scount = SiteList.size();
	for(unsigned int s = 0; s < scount; s++)
	{
		TestSite* site_temp = GetSite(s);
		if(!site_temp) break;
		site_temp->add_softbin(softbin);
	}
}

void DataLogImpl::SetItemValue(TestItem* item, CSVParser& parser, unsigned int test_number)
{
	if(!item) return;
	item->set_number(test_number);
	item->set_itemnumber(parser.get_string(LOG_ITEM_NUMBER));
	item->set_label(parser.get_string(LOG_TESTLABEL_INDEX));
	item->set_unit(parser.get_string(LOG_UNITS_INDEX));
	if(std::strcmp("?", parser.get_string(LOG_UPPER_INDEX)) != 0)
	{
		item->set_highlimit(std::atof(parser.get_string(LOG_UPPER_INDEX)));
	}

	if(std::strcmp("?", parser.get_string(LOG_LOWER_INDEX)) != 0)
	{
		item->set_lowlimit(std::atof(parser.get_string(LOG_LOWER_INDEX)));
	}
}

void DataLogImpl::SetBinValue(TestBin& bin,const CSVParser& parser)
{
	bin.set_number(std::atoi(parser.get_string(LOG_BININFO_NUMBER)));
    bin.set_count(std::atoi(parser.get_string(LOG_BININFO_COUNT)));
	bin.set_name(parser.get_string(LOG_BININFO_NAME));
	const char* bin_type = parser.get_string(LOG_BININFO_TYPE);
	if(!bin_type) return;

	if(std::strcmp("P", bin_type) == 0)
		bin.set_type(PASSBIN);
	else if(std::strcmp("F", bin_type) == 0)
		bin.set_type(FAILBIN);
    else if(std::strcmp("R", bin_type) == 0)
        bin.set_type(REJECTBIN);
	else bin.set_type(UNKONWNBIN);
}

void DataLogImpl::SetSoftBinForAllSite(const CSVParser& parser)
{
	TestBin softbin;
	SetBinValue(softbin, parser);
	unsigned int site_count = SiteList.size();
	for(unsigned int i = 0; i < site_count; i++)
	{
		TestSite* site = SiteList[i];
		if(!site) break;
		site->add_softbin(softbin);
	}
    //qDebug("%u, %s", softbin.get_number(), softbin.get_name());
}

void DataLogImpl::SetHardBinForAllSite(const CSVParser& parser)
{
	TestBin hardbin;
	SetBinValue(hardbin, parser);
	unsigned int site_count = SiteList.size();
	for(unsigned int i = 0; i < site_count; i++)
	{
		TestSite* site = SiteList[i];
		if(!site) break;
		site->add_hardbin(hardbin);
	}
}

void DataLogImpl::SetTempDataHolder(TempDataHolder& holder, const CSVParser& parser)
{
	std::string item_number(parser.get_string(LOG_ITEM_NUMBER));
	double value = std::atof(parser.get_string(LOG_TESTDATAS_INDEX));
	TestFlag flag;
	if(std::strcmp("P", parser.get_string(LOG_ITEM_PASS)) == 0)
		flag = RESULT_PASS;
	else flag = RESULT_FAIL;

	holder.ItemFlagList.push_back(flag);
	holder.ItemNumberList.push_back(item_number);
	holder.ValueList.push_back(value);
}

void DataLogImpl::SetItemsForSite(TestSite* current_site, TempDataHolder& holder)
{
	unsigned int value_count = holder.ValueList.size();
	unsigned int device_index = current_site->get_device_count();
	for(unsigned int i = 0; i < value_count; i++)
	{
		TestItem* current_item = current_site->get_item_by_itemnumber(holder.ItemNumberList[i].c_str());
		TestResult result;
		result.set_flag(holder.ItemFlagList[i]);
		result.set_value(holder.ValueList[i]);
		result.set_index(device_index+1);
		current_item->add_result(result);
	}
	holder.Clear();
}

unsigned int DataLogImpl::GetSiteCount()
{
	return SiteList.size();
}

TestSite* DataLogImpl::CreateNewSite(TestSite* old_site, unsigned int site_number)
{
	TestSite* current_site = new TestSite();

	if(current_site == nullptr)
	{
#ifdef _DEBUG_API_H_
		SHOW_MEMORY_ERROR();
		throw MEMORY_ALLOCATED_ERROR();
#endif
		return nullptr;
	}

	current_site->set_number(site_number);

	std::string site_name = Name;
	site_name.append("-site");
	//std::string number = std::to_string((unsigned long long)site_number);
	char number[8];
	std::sprintf(number, "%u", site_number);
	site_name.append(number);
	current_site->set_name(site_name.c_str());

	for(unsigned int i = 0; i < old_site->get_item_count(); i++)
	{
		TestItem* temp = old_site->get_item(i);
		TestItem* new_item = new TestItem();
		if(new_item == nullptr)
		{
#ifdef _DEBUG_API_H_
			SHOW_MEMORY_ERROR();
			throw MEMORY_ALLOCATED_ERROR();
#endif
			delete current_site;
			return nullptr;
		}
		new_item->copy_item_without_result(temp);
		current_site->add_item(new_item);
	}
	SiteList.push_back(current_site);
	return current_site;
}

void DataLogImpl::CopyGroupSiteItemsDetailToEachSite(TestSite* group_site)
{
    if(!group_site) return;
    unsigned int group_item_count = group_site->get_item_count();
    if(group_item_count == 0) return;

    for(unsigned int s = 0; s < SiteList.size(); s++)
    {
        TestSite* current_site = SiteList[s];
        if(!current_site) return ;

        for(unsigned int i = 0; i < group_item_count; i++)
        {
            TestItem* group_site_item = group_site->get_item(i);
            if(!group_site_item) return;

            unsigned int item_number = group_site_item->get_number();
            TestItem* current_site_item = current_site->get_item_by_number(item_number);
            if(!current_site_item) continue;

            const char* label = current_site_item->get_label();
            if(!label)
            {
                current_site_item->copy_item_without_result(group_site_item);
            }
            else
            {
                break;
            }
        }
    }
}

DataLogType DataLogImpl::GetFileTypeBySuffix(const char* filename)
{
	std::string tem(filename);
	std::string::size_type pos = tem.find_last_of('.');
	if(pos == std::string::npos) return OTHER_FILE;
	std::string suffix = tem.substr(pos);
    std::locale loc;
    std::string new_suffix(suffix);
    for (size_t i=0; i<suffix.length(); ++i)
    {
        new_suffix[i]=(std::tolower(suffix[i],loc));
    }
    dprintf("new_suffix=%s\n", new_suffix.c_str());

	DataLogType type = OTHER_FILE;
	if(new_suffix == std::string(".std") || new_suffix == std::string(".stdf"))
		type = STDF_FILE;
	else if(new_suffix == std::string(".log"))
		type = LOG_FILE;
	else if(new_suffix == std::string(".csv"))
		type = CSV_FILE;
    else if(new_suffix == std::string(".txt"))
        type = TXT_FILE;
    else if(new_suffix == std::string(".xls"))
        type = TAB_FILE;
	else
		type = OTHER_FILE;

	Type = type;
	return type;
}

//======================== STDF ============================
DataLogError DataLogImpl::ReadFromSTDF(const char* filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if(!in) return READ_FILE_ERROR;

	StdfHeader header;
	STDF_TYPE type;
	bool is_sdr_exist = false;
	TestSite* current_site = nullptr;
	unsigned int device_index = 0;
	bool has_all_site_softbin = false;
	bool for_single_site_softbin = false;
	bool has_all_site_hardbin = false;
	bool for_single_site_hardbin = false;

	StdfPRR prr;
	StdfPIR pir;
	StdfPTR ptr;

    TestSite *group_site = nullptr;
    TestItem *last_item = nullptr;

	while(!in.eof())
	{
		type = header.read(in);
		switch(type)
		{
		case FAR_TYPE:
			{
				StdfFAR far;
				unsigned char cpu_type = far.get_cpu_type();
				if(cpu_type != 2)
				{
					in.close();
                    return DATDALOG_STDF_CPU_TYPE_NOT_SUPPORT;
				}
				unsigned char stdf_ver = far.get_stdf_version();
				if(stdf_ver != 4)
				{
					in.close();
                    return DATALOG_STDF_VERSION_NOT_SUPPORT;
				}
			}
			break;
		case SDR_TYPE:
			{
				is_sdr_exist = true;
				StdfSDR sdr;
				sdr.parse(header);
				SetSiteList(sdr);

                group_site = new TestSite();
                group_site->set_number(sdr.get_site_group_number());
			}
			break;

		case PIR_TYPE:
			{
				//StdfPIR pir;
				pir.parse(header);
                unsigned char current_site_number = pir.get_site_number();
                current_site = GetSiteByNumber(current_site_number);
				if(!current_site)
				{
					if(!is_sdr_exist)
					{
						current_site = new TestSite();
                        current_site->set_number(current_site_number);
                        std::string temp_site_name = ComposeSiteName(current_site_number);
                        current_site->set_name(temp_site_name.c_str());

						SiteList.push_back(current_site);
					}
					else
					{
                        if(group_site->get_number() == current_site_number)
                        {
                            current_site = group_site;
                        }
                        else
                        {
                            ClearSiteList();
                            return READ_FILE_ERROR;
                        }
					}
				}
                device_index = current_site->get_device_count();
			}
			break;

		case PTR_TYPE:
			{
				//StdfPTR ptr;
				ptr.parse(header);
				unsigned int test_number = ptr.get_test_number();

				TestItem *current_item = current_site->get_item_by_number(test_number);
				if(!current_item)
				{
					current_item = new TestItem();
					SetItemValue(current_item, ptr);
					current_site->add_item(current_item);
				}

                TestResult result;
				SetResultValue(result, ptr, device_index+1);
				current_item->add_result(result);
                last_item = current_item;
			}
			break;

		case PRR_TYPE:
			{
				//StdfPRR prr;
				prr.parse(header);
				TestDevice device;
				SetDeviceValue(device, prr);
				current_site->add_device(device);

                if(device_index == current_site->get_device_count())
                {
                    TestResult& last_result = last_item->get_result(last_item->get_result_count()-1);
                    last_result.set_index(device_index);
                }
			}
			break;

		case HBR_TYPE:
			{
				StdfHBR hbr;
				hbr.parse(header);

				TestBin hardbin;
				SetHardBinValue(hardbin, hbr);

				unsigned char site_number = hbr.get_site_number();
				if(site_number == SITE_NUMBER_ALL)
				{
					has_all_site_hardbin = true;
					if(!for_single_site_hardbin)
						SetHardBinForAllSite(hardbin);
				}
				else
				{
					for_single_site_hardbin = true;
					if(!has_all_site_hardbin)
					{
						current_site = GetSiteByNumber(site_number);
						current_site->add_hardbin(hardbin);
					}
				}
			};
			break;

		case SBR_TYPE:
			{
				StdfSBR sbr;
				sbr.parse(header);

				TestBin softbin;
				SetSoftBinValue(softbin, sbr);

				unsigned char site_number = sbr.get_site_number();
				if(site_number == SITE_NUMBER_ALL)
				{
					has_all_site_softbin = true;
					if(!for_single_site_softbin)
						SetSoftBinForAllSite(softbin);
				}
				else
				{
					for_single_site_softbin = true;
					if(!has_all_site_softbin)
					{
						current_site = GetSiteByNumber(site_number);
						current_site->add_softbin(softbin);
					}
				}
			};
			break;

		default: break;
		}
	}
	in.close();
    //SetName(filename);
    if(has_all_site_hardbin ) HBin_For_All_Sites = true;
    if(has_all_site_softbin ) SBin_For_All_Sites = true;
    CopyGroupSiteItemsDetailToEachSite(group_site);
    if(group_site) delete group_site;

    return DATALOG_OPERATE_OK;
}

//==================================== LOG =========================
DataLogError DataLogImpl::ReadFromLog( const char* filename )
{
	std::ifstream in(filename, std::ios::in);
	if(!in) return READ_FILE_ERROR;

	CSVLine line(1024, 512);
	CSVParser parser(1024, 16);

	const char* first_line = line.read_line(in);
	unsigned int count = parser.parse(first_line, ',');
	if(count == 0)
	{
		in.close();
		return FILE_FORMATE_ERROR;
	}
	if(std::atoi(parser.get_string(0)) != LOG_FILE_MARK)
	{
		in.close();
		return FILE_FORMATE_ERROR;
	}

	unsigned int test_number = 0;
	TempDataHolder holder;
	TestSite* hold_items_site = new TestSite();
	while(in.good() && !in.eof())
	{
		const char* a_line = line.read_line(in);
		if(a_line)
		{
			unsigned int count = parser.parse(a_line, ',');
			if(count == 0) break;

			int line_mark = std::atoi(parser.get_string(0));
			switch(line_mark)
			{
			case LOG_ITEMDESCR_MARK:
				{
					if(count <= LOG_TESTLABEL_INDEX) return FILE_FORMATE_ERROR;

					TestItem* item = new TestItem();
					test_number++;
					SetItemValue(item, parser, test_number);
					hold_items_site->add_item(item);
				}
				break;

			case LOG_TESTDATAS_MARK:
				{
					if(count <= LOG_TESTDATAS_INDEX) return FILE_FORMATE_ERROR;
					SetTempDataHolder(holder, parser);
				}
				break;

			case LOG_BINRESULT_MARK:
				{
					if(count <= LOG_HBIN_NUMBER) return FILE_FORMATE_ERROR;
					unsigned int site_number = (unsigned int)(std::atoi(parser.get_string(LOG_SITE_NUMBER)));
					TestSite* current_site = GetSiteByNumber(site_number);
					if(!current_site)
						current_site = CreateNewSite(hold_items_site, site_number);
					SetItemsForSite(current_site, holder);

					TestDevice device;
					SetDeviceVaue(device, parser);
					current_site->add_device(device);
				}
				break;

			case LOG_SBININFO_MARK:
				{
					if(count <= LOG_BININFO_NAME) return FILE_FORMATE_ERROR;
					SetSoftBinForAllSite(parser);
				}
				break;

			case LOG_HBININFO_MARK:
				{
					if(count <= LOG_BININFO_NAME) return FILE_FORMATE_ERROR;
					SetHardBinForAllSite(parser);
				}
				break;
			}
		}
	}
	in.close();
	delete hold_items_site;
    //SetName(filename);
    HBin_For_All_Sites = true;
    SBin_For_All_Sites = true;
    return DATALOG_OPERATE_OK;
}

//==================================== CSV =========================
DataLogError DataLogImpl::ReadFromCSV(const char* filename)
{
	std::ifstream in(filename, std::ios::in);
	if(!in) return READ_FILE_ERROR;

	CSVLine line;
	CSVParser parser;
	bool is_strict = false;

	const unsigned int sbin_index = 2;
	const unsigned int hbin_index = 3;
    const unsigned int xcoord_index = 4;
    const unsigned int ycoord_index = 5;
    const char* invalid_cord_string = "N/A";

	TestSite* hold_items_site = new TestSite();
	bool reach_test_name = false;
	bool reach_data_value = false;
	unsigned int data_start_index = 0;
	unsigned int device_index = 0;
	while(in.good() && !in.eof())
	{
		const char* a_line = line.read_line(in, is_strict);
		if(!a_line || std::strlen(a_line) == 0) continue;

		if(!reach_test_name)
		{
			unsigned int count = parser.parse(a_line, ',');
			if(count == 0) continue;
			const char *test_name = parser.get_string(0);
            if(std::strcmp(test_name, "Test Name") == 0 && count > 2)
			{
				reach_test_name = true;
				for(unsigned int n = 1; n < count; n++)
				{
					const char* temp = parser.get_string(n);
					if(temp && std::strlen(temp) > 0)
					{
						data_start_index = n;
						break;
					}
				}
				if(data_start_index > 0)
				{
					// Test Name
					unsigned int test_number = 0;
					for(unsigned int i = data_start_index; i < count; i++)
					{
						const char* lable = parser.get_string(i);
						test_number++;

						TestItem *item = new TestItem();
						item->set_label(lable);
						item->set_number(test_number);
						hold_items_site->add_item(item);
					}
					// Test Number
					a_line = line.read_line(in, is_strict);
					count = parser.parse(a_line, ',');
					unsigned int item_index = 0;
					for(unsigned int i = data_start_index; i < count; i++)
					{
						const char* item_number = parser.get_string(i);
						TestItem *item = hold_items_site->get_item(item_index);
						item->set_itemnumber(item_number);
						item_index++;
					}

					// Lower Limit
					a_line = line.read_line(in, is_strict);
					count = parser.parse(a_line, ',');
					item_index = 0;
					for(unsigned int i = data_start_index; i < count; i++)
					{
						const char* lowlimit = parser.get_string(i);
						if(lowlimit && std::strlen(lowlimit) > 0)
						{
							TestItem *item = hold_items_site->get_item(item_index);
							item->set_lowlimit(std::atof(lowlimit));
						}
						item_index++;
					}

					// Upper Limit
					a_line = line.read_line(in, is_strict);
					count = parser.parse(a_line, ',');
					item_index = 0;
					for(unsigned int i = data_start_index; i < count; i++)
					{
						const char* highlimit = parser.get_string(i);
						if(highlimit && std::strlen(highlimit) > 0)
						{
							TestItem *item = hold_items_site->get_item(item_index);
							item->set_highlimit(std::atof(highlimit));
						}
						item_index++;
					}

					// Units
					a_line = line.read_line(in, is_strict);
					count = parser.parse(a_line, ',');
					item_index = 0;
					for(unsigned int i = data_start_index; i < count; i++)
					{
						const char* units = parser.get_string(i);
						if(units && std::strlen(units) > 0)
						{
							TestItem *item = hold_items_site->get_item(item_index);
							item->set_unit(units);
						}
						item_index++;
					}
				}
				else
				{
					in.close();
					return FILE_FORMATE_ERROR;
				}
			}
		}
		if(!reach_data_value)
		{
			unsigned int count = parser.parse(a_line, ',');
			if(count == 0) continue;
			const char *site_begin = parser.get_string(0);
			if(std::strcmp(site_begin, "Site #") == 0)
			{
				reach_data_value = true;
				continue;
			}
		}
		else
		{
			unsigned int count = parser.parse(a_line, ',');
			if(count == 0) continue;
			const char *str_site_number = parser.get_string(0);
			unsigned int site_number = (unsigned int)std::atoi(str_site_number);
            if(site_number == 0) continue;
			TestSite *current_site = GetSiteByNumber(site_number);
			if(!current_site)
			{
				current_site = CreateNewSite(hold_items_site, site_number);
			}
			unsigned int item_count = current_site->get_item_count();
			unsigned int vindex = data_start_index;
			device_index = current_site->get_device_count();
			for(unsigned int i = 0; i < item_count; i++)
			{
				TestItem* vitem = current_site->get_item(i);
				const char* str_value = parser.get_string(vindex);
				if(!str_value || std::strlen(str_value) == 0)
				{
					vindex++;
					if(vindex == count) break;
					continue;
				}

				TestResult result;
				result.set_flag(RESULT_TESTED);
				result.set_index(device_index+1);
				result.set_value(std::atof(str_value));
				//TestResult result(device_index+1, std::atof(str_value), RESULT_TESTED);
				vitem->add_result_flag(result);

				vindex++;
				if(vindex == count) break;
			}
			TestDevice device;
			const char* str_sbin_number = parser.get_string(sbin_index);
			const char* str_hbin_number = parser.get_string(hbin_index);
			unsigned short sbin_number = (unsigned short)std::atoi(str_sbin_number);
			unsigned short hbin_number = (unsigned short)std::atoi(str_hbin_number);
            const char* xcoord_string = parser.get_string(xcoord_index);
            const char* ycoord_string = parser.get_string(ycoord_index);
            if(xcoord_string && std::strcmp(xcoord_string, invalid_cord_string))
            {
                short temp_xcoord = (short)std::atoi(xcoord_string);
                device.set_xcoord(temp_xcoord);
            }
            if(ycoord_string && std::strcmp(ycoord_string, invalid_cord_string))
            {
                short temp_ycoord = (short)std::atoi(ycoord_string);
                device.set_ycoord(temp_ycoord);
            }

			device.set_hardbin(hbin_number);
			device.set_softbin(sbin_number);
			current_site->add_device(device);

			current_site->add_hardbin_bynumber(hbin_number);
			current_site->add_softbin_bynumber(sbin_number);
		}
	}
	in.close();
	delete hold_items_site;
	if(!reach_test_name || !reach_data_value) return FILE_FORMATE_ERROR;
    if(SiteList.size() == 0) return NO_DATA_TO_READ;
    HBin_For_All_Sites = false;
    SBin_For_All_Sites = false;
    return DATALOG_OPERATE_OK;
}

DataLogError DataLogImpl::ReadFromTXT(const char* filename)
{
    const std::string start_str = "Datalog for Serial#";
    const std::string pass_str = "PASSED";
    std::ifstream in(filename, std::ios::in);
    if(!in) return READ_FILE_ERROR;

    TestSite *current_site = nullptr;
    while(in.good() && !in.eof())
    {
        std::string current_line;
        std::getline(in, current_line);
        if(current_line.find(start_str) != std::string::npos)
        {
            unsigned pos_comma = current_line.find(',');
            std::string serial_index_str = current_line.substr(21, pos_comma - 21);

            TestFlag result_flag = RESULT_FAIL;
            if(current_line.find(pass_str) != std::string::npos) result_flag = RESULT_PASS;

            std::string rest_line = current_line.substr(pos_comma + 1);
            unsigned pos_pound = rest_line.find('#');
            unsigned pos_star = rest_line.find('*');
            std::string site_index_str = rest_line.substr(pos_pound + 1, pos_star - pos_pound - 1);

            unsigned char site_number = (unsigned char)std::atoi(site_index_str.c_str());

            current_site = GetSiteByNumber(site_number);
            if(!current_site)
            {
                current_site = new TestSite();
                current_site->set_number(site_number);
                std::string temp_site_name = ComposeSiteName(site_number);
                current_site->set_name(temp_site_name.c_str());
                SiteList.push_back(current_site);
            }

            std::getline(in, current_line);//Re-Test
            std::getline(in, current_line);//SBin
            std::string soft_bin_str = current_line.substr(21);
            std::getline(in, current_line);//HBin
            std::string hard_bin_str = current_line.substr(21);
            std::getline(in, current_line);//TEST#
            std::getline(in, current_line);//----

            unsigned short soft_bin_number = (unsigned short)std::atoi(soft_bin_str.c_str());
            unsigned short hard_bin_number = (unsigned short)std::atoi(hard_bin_str.c_str());
            current_site->add_hardbin_bynumber(hard_bin_number);
            current_site->add_softbin_bynumber(soft_bin_number);
            TestDevice temp_device;
            temp_device.set_id(serial_index_str.c_str());
            temp_device.set_hardbin(hard_bin_number);
            temp_device.set_softbin(soft_bin_number);
            current_site->add_device(temp_device);
            while(in.good() && !in.eof())
            {
                std::getline(in, current_line);
                int length = current_line.length();
                if(length < 5) break;
                std::string item_number_str = current_line.substr(0, 6);
                std::string result_str = current_line.substr(7, 9);
                TestItem* item = current_site->get_item_by_itemnumber(item_number_str.c_str());
                if(!item)
                {
                    item = new TestItem();
                    std::string item_units_str = current_line.substr(17, 5);
                    std::string item_lower_str = current_line.substr(23, 9);
                    std::string item_upper_str = current_line.substr(33, 9);
                    std::string item_label_str = current_line.substr(49, length-49);

                    std::size_t fount_not_space = item_units_str.find_first_not_of(' ');
                    if(fount_not_space != std::string::npos)
                    {
                        std::string item_units_str2 =item_units_str.substr(fount_not_space);
                        item->set_unit(item_units_str2.c_str());
                    }

                    std::size_t found_h = item_upper_str.find_first_of("+-0123456789.");
                    if(found_h != std::string::npos) item->set_highlimit(std::atof(item_upper_str.c_str()));

                    std::size_t found_l = item_lower_str.find_first_of("+-0123456789.");
                    if(found_l != std::string::npos) item->set_lowlimit(std::atof(item_lower_str.c_str()));

                    item->set_label(item_label_str.c_str());
                    item->set_itemnumber(item_number_str.c_str());
                    current_site->add_item_by_itemnumber(item);
                }
                TestResult result;
                result.set_flag(result_flag);
                result.set_value(std::atof(result_str.c_str()));
                result.set_index(current_site->get_device_count());
                item->add_result(result);
                //item->add_result_flag(result);
            }
        }
    }
    in.close();
    if(SiteList.size() == 0) return NO_DATA_TO_READ;
    HBin_For_All_Sites = false;
    SBin_For_All_Sites = false;
    for(unsigned int i = 0; i < SiteList.size(); i++)
    {
        TestSite* site = SiteList[i];
        if(site)
        {
            for(unsigned int n = 0; n < site->get_item_count(); n++)
            {
                TestItem* item = site->get_item(n);
                if(item) item->set_number(n+1);
            }
        }
    }
    return DATALOG_OPERATE_OK;
}

DataLogError DataLogImpl::ReadDataLog(const char* filename)
{
	DataLogType type = GetFileTypeBySuffix(filename);
	switch(type)
	{
	case STDF_FILE: return ReadFromSTDF(filename);
	case LOG_FILE: return ReadFromLog(filename);
	case CSV_FILE:
	    {
	        DataLogError ret =  ReadFromCSV(filename);
	        dprintf("Error in ReadFromCSV =%d\n", ret);
	        if(ret == FILE_FORMATE_ERROR)
            {
                dprintf("in ReadFromTabXLS\n");
                ret = ReadFromTabXLS(filename, ',');
                dprintf("Error in ReadFromTabXLS =%d\n", ret);
            }
            return ret;
	    }
    case TXT_FILE: return ReadFromTXT(filename);
    case TAB_FILE: return ReadFromTabXLS(filename, '\t');
	default: return FILE_FORMATE_ERROR;
	}
}

DataLogError DataLogImpl::WriteDataToCSV(const char* filename)
{
	unsigned int site_count = SiteList.size();
	if(site_count == 0) return NO_DATA_TO_WRIET;

	std::ofstream out(filename, std::ios::out );
	if(!out) return WRITE_FILE_ERROR;

	// Header
	bool write_header_flag = false;
	for(unsigned int i = 0; i < site_count; i++)
	{
		TestSite *site = SiteList[i];
		if(!site) break;

		unsigned int item_count = site->get_item_count();
		if(item_count == 0) continue;

		if(!write_header_flag)
		{
			write_header_flag = true;
            site->save_data(out, true);
		}
        else
        {
            if(Type == TXT_FILE)
                site->save_data(out, true);
            else
                site->save_data(out, false);
        }
		out<<"\n";
	}

	out.close();
    return DATALOG_OPERATE_OK;
}


DataLogError DataLogImpl::WriteStatisticToCSV(const char* filename)
{
	unsigned int site_count = SiteList.size();
	if(site_count == 0) return NO_DATA_TO_WRIET;

	std::ofstream out(filename, std::ios::out );
	if(!out) return WRITE_FILE_ERROR;

	for(unsigned int i = 0; i < site_count; i++)
	{
		TestSite *site = SiteList[i];
		if(!site) break;

		out<<site->get_name()<<std::endl;

        site->save_statistic(out);
		out<<"\n";
	}
	out.close();
    return DATALOG_OPERATE_OK;
}

DataLogError DataLogImpl::WriteSummaryToCSV(const char* filename)
{
    unsigned int site_count = SiteList.size();
    if(site_count == 0) return NO_DATA_TO_WRIET;

    std::ofstream out(filename, std::ios::out );
    if(!out) return WRITE_FILE_ERROR;

    unsigned int total_device_count = 0;
    for(unsigned int i = 0; i < site_count; i++)
    {
        TestSite *site = SiteList[i];
        if(!site) break;
        total_device_count += site->get_device_count();
    }

    TestSite *first_site = NULL;
    for(unsigned int i = 0; i < site_count; i++)
    {
        TestSite *site = SiteList[i];
        if(site)
        {
            first_site = site;
            break;
        }
    }

    if(SBin_For_All_Sites)
    {
        out<<"Software Bin for All Sites:"<<"\n";
        out<<"Bin Number,Bin Type,Bin Description,Bin Count,Bin Percentage"<<"\n";
        unsigned int soft_bin_count = first_site->get_softbin_count();
        for(unsigned int n = 0; n < soft_bin_count; n++)
        {
            TestBin bin = first_site->get_softbin(n);
            out<<bin.get_number()<<",";
            out<<char(bin.get_type())<<",";
            out<< CSVWriter(bin.get_name()) <<",";
            unsigned int bin_count = bin.get_count();
            double percent = 100.0 * bin_count / total_device_count;
            out<<bin_count<<",";
            out<<percent<<"%\n";
        }
        out<<"\n\n";
    }

    if(HBin_For_All_Sites)
    {
         out<<"Hardware Bin for All Sites:"<<"\n";
         out<<"Bin Number,Bin Type,Bin Description,Bin Count,Bin Percentage"<<"\n";
         unsigned int hard_bin_count = first_site->get_hardbin_count();
         for(unsigned int n = 0; n < hard_bin_count; n++)
         {
             TestBin bin = first_site->get_hardbin(n);
             out<<bin.get_number()<<",";
             out<<char(bin.get_type())<<",";
             out<< CSVWriter(bin.get_name()) <<",";
             unsigned int bin_count = bin.get_count();
             double percent = 100.0 * bin_count / total_device_count;
             out<<bin_count<<",";
             out<<percent<<"%\n";
         }
         out<<"\n\n";
    }

    if(LOG_FILE == Type)
    {
        out<<"Fail Items for All Sites:"<<"\n";
        out<<"Index,Test Number,Test Label,Fail Count,Fail Percentage"<<"\n";

        unsigned int item_count = first_site->get_item_count();
        std::vector<unsigned int> Fail_counts(item_count,0), Device_counts(item_count,0);
        for(unsigned int n = 0; n < item_count; n++)
        {
            for(unsigned int i = 0; i < site_count; i++)
            {
                TestSite *temp_site = SiteList[i];
                if(!temp_site) continue;
                TestItem* item = temp_site->get_item(n);
                unsigned int result_tested = item->get_result_tested();
                Fail_counts[n] += (result_tested - item->get_result_pass());
                Device_counts[n] += result_tested;
            }
        }
        unsigned int fail_item_index = 0;
        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = first_site->get_item(n);
            if(Fail_counts[n] > 0)
            {
                fail_item_index++;
                out<<fail_item_index<<",";
                const char* item_number = item->get_itemnumber();
                out<<item_number<<",";
                out<< CSVWriter(item->get_label()) <<",";
                out<<Fail_counts[n]<<",";
                out<<100.0*Fail_counts[n]/Device_counts[n]<<"%\n";
            }
        }
        out<<"\n\n";
        out<<"\n\n";
    }

    for(unsigned int i = 0; i < site_count; i++)
    {
        TestSite *site = SiteList[i];
        if(!site) break;

        out<<"Summary: "<<site->get_name()<<"\n";

        unsigned int device_count = site->get_device_count();
        if(device_count == 0)
        {
            out<<"No Device Under Test."<<"\n\n";
            continue;
        }

        out<<"Software Bin for Site"<<(unsigned int)(site->get_number())<<":\n";
        out<<"Bin Number,Bin Type,Bin Description,Bin Count,Bin Percentage"<<"\n";
        unsigned int soft_bin_count = site->get_softbin_count();

        for(unsigned int n = 0; n < soft_bin_count; n++)
        {
            TestBin bin = site->get_softbin(n);
            unsigned short bin_number = bin.get_number();
            unsigned int bin_count = 0;
            for(unsigned int m = 0; m < device_count; m++)
            {
                TestDevice device = site->get_device(m);
                if(device.get_softbin() == bin_number)
                    bin_count++;
            }

            out<<bin_number<<",";
            out<<char(bin.get_type())<<",";
            out<< CSVWriter(bin.get_name()) <<",";
            double percent = 100.0 * bin_count / device_count;
            out<<bin_count<<",";
            out<<percent<<"%\n";
        }
        out<<"\n\n";


        out<<"Hardware Bin for Site"<<(unsigned int)(site->get_number())<<":\n";
        out<<"Bin Number,Bin Type,Bin Description,Bin Count,Bin Percentage"<<"\n";
        unsigned int hard_bin_count = site->get_hardbin_count();
        for(unsigned int n = 0; n < hard_bin_count; n++)
        {
            TestBin bin = site->get_hardbin(n);
            unsigned short bin_number = bin.get_number();
            unsigned int bin_count = 0;
            for(unsigned int m = 0; m < device_count; m++)
            {
                TestDevice device = site->get_device(m);
                if(device.get_hardbin() == bin_number)
                    bin_count++;
            }

            out<<bin.get_number()<<",";
            out<<char(bin.get_type())<<",";
            out<< CSVWriter(bin.get_name()) <<",";
            double percent = 100.0 * bin_count / device_count;
            out<<bin_count<<",";
            out<<percent<<"%\n";
        }
        out<<"\n\n";

        out<<"Index,Test Number,Test Label,Fail Count,Fail Percentage"<<"\n";
        unsigned int item_count = site->get_item_count();
        int fail_item_index = 0;
        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = site->get_item(n);
            unsigned int result_tested = item->get_result_tested();
            unsigned fail_count = result_tested - item->get_result_pass();
            double percent = 0.0;
            if(result_tested != 0)
            percent = 100.0 * fail_count / device_count;

            if(fail_count > 0)
            {
                fail_item_index++;
                out<<fail_item_index<<",";
                const char* item_number = item->get_itemnumber();
                if(item_number)
                    out<<item_number<<",";
                else
                    out<<item->get_number()<<",";
                out<< CSVWriter(item->get_label()) <<",";
                out<<fail_count<<",";
                out<<percent<<"%\n";
            }
        }
        out<<"\n\n";
    }

    out.close();
    return DATALOG_OPERATE_OK;
}

bool DataLogImpl::IsSBinForAllSites()
{
    return SBin_For_All_Sites;
}

bool DataLogImpl::IsHBinForAllSites()
{
    return HBin_For_All_Sites;
}

//////////////////////////////////////////////////////////////////////////
static void get_limits(const char* str, std::string& low, std::string& high, const char sepatator)
{
    std::string str_separator(1, sepatator);
    if(str)
    {
        std::string temp_left(str);
        std::size_t fount_number = temp_left.find_first_of("+-0123456789");
        if(fount_number == std::string::npos)
        {
            low = sepatator;
            high = sepatator;
            return;
        }

        std::size_t found_left = temp_left.find("<=");
        if(found_left == std::string::npos)
        {
            low = sepatator;
            high = sepatator;
            return;
        }
        std::string str1 = temp_left.substr(0, found_left);
        std::string temp_right = temp_left.substr(found_left+2);
        std::size_t fount_right =temp_right.find("<=");
        if(fount_right == std::string::npos)
        {
            if(fount_number == 0)
            {
                low = str1 + str_separator;
                high = str_separator;
            }
            else
            {
                low = str_separator;
                high = temp_right + str_separator;
            }
        }
        else
        {
            low = str1 + str_separator;
            high = temp_right.substr(fount_right+2)+str_separator;
        }
    }
    else
    {
        low = sepatator;
        high = sepatator;
    }
}

DataLogError DataLogImpl::ReadFromTabXLS(const char* filename, const char seprator)
{
    std::ifstream in(filename, std::ios::in);
	if(!in) return READ_FILE_ERROR;

	CSVLine line;
	bool is_strict = false;
	const char* company ="PowerTECH";
	const char* test_file_name_str = "TestFileName";
	const char* max_limit_str = "Max Limit";
	const char* min_limit_str = "Min Limit";
	const char* low_limit_str = "Lower Limit";
	const char* upper_limit_str = "Upper Limit";
	const char* units_str = "Units";
	const char* label_begin_str = "Result:";
	const char* item_name_str = "Item Name";
	const char* test_name_str = "Test Name";
	const char* datalog_begin_str = "Serial#";
	const char* number_digits = "0123456789";
	const char* sbin_str = "S#";
	const char* hbin_str = "Bin#";
	const char* xcoord_str = "XCoord";
	//const char* ycoord_str = "YCoord";
	const char* site_str = "Site #";
	std::string min_limits;
	std::string max_limits;

	// Check first line contain the company name
	const char* find_str = nullptr;
	for(int i = 0; i < 100; i++)
    {
        if((in.good() && !in.eof()))
        {
            const char* a_line = line.read_line(in, is_strict);
            if(!a_line) continue;
            find_str = std::strstr(a_line, company);
            if(find_str != NULL) break;

            find_str = std::strstr(a_line, test_file_name_str);
            if(find_str != NULL) break;
        }
    }
    if(find_str == NULL)
    {
        in.close();
        return FILE_FORMATE_ERROR;
    }

    // check compare for limits
    std::vector<std::string> d_low_limits;
    std::vector<std::string> d_high_limits;
    unsigned int compare_index = 0;
    std::string str_separator(1, seprator);

    in.clear();
    in.seekg(0, std::ios::beg);

    CSVParser parser;

    bool find_compare_flag = false;
    for(int i = 0; i < 100; i++)
    {
        if((in.good() && !in.eof()))
        {
            const char* a_line = line.read_line(in, is_strict);
            if(!a_line || std::strlen(a_line) == 0) continue;
            else
            {
                unsigned int count = parser.parse(a_line, seprator);
                const char *temp_string = parser.get_string(0);
                const char *find_compare = std::strstr(temp_string, "Compare");
                if(find_compare)
                {
                    if(compare_index == 0)
                    {
                        for(unsigned int i = 0; i < count; i++)
                        {
                            temp_string = parser.get_string(i);
                            std::string low, high;
                            get_limits(temp_string, low, high, seprator);
                            d_low_limits.push_back(low);
                            d_high_limits.push_back(high);
                            find_compare_flag = true;
                        }
                        compare_index++;
                    }
                    else
                    {
                        unsigned int length_low = d_low_limits.size();
                        unsigned int length_high = d_high_limits.size();
                        for(unsigned int i = 0; i < count; i++)
                        {
                            temp_string = parser.get_string(i);
                            std::string low, high;
                            get_limits(temp_string, low, high, seprator);
                            if(low != str_separator)
                            {
                                if(i+compare_index < length_low) d_low_limits[i+compare_index]=low;
                                else d_low_limits.push_back(low);
                            }
                            if(high != str_separator)
                            {
                                if(i+compare_index < length_high) d_high_limits[i+compare_index]=low;
                                else d_high_limits.push_back(high);
                            }
                        }
                        compare_index++;
                    }
                }
            }
        }
    }

    if(find_compare_flag)
    {
        for(unsigned int i = 0; i < d_low_limits.size(); i++) min_limits += d_low_limits[i];
        for(unsigned int i = 0; i < d_high_limits.size(); i++) max_limits += d_high_limits[i];
    }

	unsigned int data_start_index = 0;
	unsigned int sbin_index = 2;
	unsigned int hbin_index = 3;
	unsigned int xcoord_index = 4;
	unsigned int ycoord_index = 5;
	unsigned int site_number_index = 1;

	TestSite* hold_items_site = new TestSite();
	hold_items_site->set_number(1);
	bool reach_test_name = false;
	bool reach_data_value = false;
	bool reach_min_limit = false;
	bool reach_max_limit = false;
	bool reach_item_name = false;
	bool has_coord_value = false;
	bool has_site_number = false;
	bool has_units_line = false;

	unsigned int device_index = 0;
	std::string all_labels;
	std::string all_units;

	in.clear();
    in.seekg(0, std::ios::beg);
	while(in.good() && !in.eof())
	{
		const char* a_line = line.read_line(in, is_strict);
		if(!a_line || std::strlen(a_line) == 0) continue;

		if(!reach_min_limit)
		{
            unsigned int count = parser.parse(a_line, seprator);
			if(count == 0) continue;
			const char *temp_string = parser.get_string(0);
			const char *find_min_limit = std::strstr(temp_string, min_limit_str);
			if(!find_min_limit) find_min_limit = std::strstr(temp_string, low_limit_str);
            if(find_min_limit)
            {
                // Low Limits
                reach_min_limit = true;
                min_limits.assign(a_line);
                dprintf("min limit ok\n");
                //continue;
            }
		}

		if(!reach_max_limit)
		{
            unsigned int count = parser.parse(a_line, seprator);
			if(count == 0) continue;
			const char *temp_string = parser.get_string(0);
			const char *find_max_limit = std::strstr(temp_string, max_limit_str);
			if(!find_max_limit) find_max_limit = std::strstr(temp_string, upper_limit_str);
            if(find_max_limit)
            {
                // High Limits
                reach_max_limit = true;
                max_limits.assign(a_line);
                dprintf("max limit ok\n");
                //continue;
            }
		}

		if(!has_units_line)
        {
            unsigned int count = parser.parse(a_line, seprator);
			if(count == 0) continue;
			const char *temp_str = parser.get_string(0);
			const char *find_units = std::strstr(temp_str, units_str);
            if(find_units)
            {
                // set units
                has_units_line = true;
                all_units.assign(a_line);
                dprintf("all_units ok: %s\n", a_line);
                //continue;
            }
        }

		if(!reach_test_name)
		{
			unsigned int count = parser.parse(a_line, seprator);
			if(count == 0) continue;
			const char *test_name = parser.get_string(0);
			const char *find_labels = std::strstr(test_name, label_begin_str);
			if(!find_labels)
            {
                if(std::strcmp(test_name, "Result") == 0)
                {
                    reach_test_name = true;
                    reach_item_name = true;
                    all_labels.assign(a_line);
                    dprintf("Reach Result ok: %s\n", a_line);
                }
            }
            if(find_labels)
            {
                // set labels
                reach_test_name = true;
                reach_item_name = true;
                all_labels.assign(a_line);
                dprintf("all_labels ok: %s\n", a_line);
                //continue;
            }
		}

		if(!reach_item_name)
		{
            unsigned int count = parser.parse(a_line, seprator);
			if(count == 0) continue;
			const char *item_name = parser.get_string(0);
			const char *find_labels = std::strstr(item_name, item_name_str);
			if(!find_labels) find_labels = std::strstr(item_name, test_name_str);
            if(find_labels && (count > 3))
            {
                unsigned int check_begin = 1;
                for(unsigned int mm = 1; mm < count; mm++)
                {
                    const char* temp_check = parser.get_string(mm);
                    if(std::strlen(temp_check) > 1)
                    {
                        check_begin = mm;
                        break;
                    }
                }

                bool check_have_all_items = true;
                for(unsigned int mm = check_begin; mm < count; mm++)
                {
                    const char* temp_check = parser.get_string(mm);
                    if(std::strlen(temp_check) <= 1)
                    {
                        check_have_all_items = false;
                        break;
                    }
                }

                if(check_have_all_items)
                {
                    // set labels
                    reach_test_name = true;
                    reach_item_name = true;
                    all_labels.assign(a_line);
                    dprintf("all_labels ok: %s\n", a_line);
                }
                //continue;
            }
		}


		if(reach_item_name && (!reach_data_value) )
		{
			unsigned int count = parser.parse(a_line, seprator);
			if(count == 0) continue;
			const char *temp = parser.get_string(0);
            const char *find_data_begin = std::strstr(temp, datalog_begin_str);
            if(!find_data_begin)
            {
                find_data_begin = std::strstr(temp, site_str);
                if(find_data_begin)
                {
                    has_site_number = true;
                    site_number_index = 0;
                }
            }
			if(find_data_begin)
			{
                reach_data_value = true;
                dprintf("find data begin: %s\n", a_line);

                // must have Serial#	S#	Bin#
                int all_labels_length = all_labels.length();
                if(count < 3 || all_labels_length < 3)
                {
                    in.close();
                    delete hold_items_site;
                    dprintf("find data begin error\n");
                    return FILE_FORMATE_ERROR;
                }

                for(unsigned int i = 1; i < count; i++)
                {
                    temp = parser.get_string(i);
                    const char *find_xcoord = std::strstr(temp, xcoord_str);
                    if(find_xcoord)
                    {
                        xcoord_index = i;
                        dprintf("xcoord_index=%u\n", xcoord_index);
                        has_coord_value = true;
                        break;
                    }
                }

                // set SBin column index
                for(unsigned int i = 1; i < count; i++)
                {
                    temp = parser.get_string(i);
                    const char *find_sbin = std::strstr(temp, sbin_str);
                    if(find_sbin)
                    {
                        sbin_index = i;
                        dprintf("sbin_index=%u\n", sbin_index);
                        break;
                    }
                }

                // set HBin column index
                for(unsigned int i = 1; i < count; i++)
                {
                    temp = parser.get_string(i);
                    const char* find_hbin = std::strstr(temp, hbin_str);
                    if(find_hbin)
                    {
                        hbin_index = i;
                        dprintf("hbin_index=%u\n", hbin_index);
                        break;
                    }
                }

                // parset the test label and set the TestItem
                CSVParser parser_items, parser_low_limit, parser_high_limit, parser_units;
                unsigned int count_items = parser_items.parse(all_labels.c_str(), seprator);
                unsigned int count_low_limit = parser_low_limit.parse(min_limits.c_str(), seprator);
                unsigned int count_hihg_limit = parser_high_limit.parse(max_limits.c_str(), seprator);
                unsigned int count_units = 0;
                if(has_units_line) count_units = parser_units.parse(all_units.c_str(), seprator);

                // set the data start column index
                for(unsigned int m = 1; m < count_items; m++)
                {
                    if(std::strlen(parser_items.get_string(m)) > 1)
                    {
                        data_start_index = m;
                        dprintf("data_start_index=%u\n", data_start_index);
                        break;
                    }
                }

                // delete more tab tailed
                for(unsigned int m = data_start_index; m < count_items; m++)
                {
                    if(std::strlen(parser_items.get_string(m)) < 1)
                    {
                        count_items = m;
                        dprintf("count_items=%u\n", data_start_index);
                        break;
                    }
                }

                unsigned int test_number = 0;

                for(unsigned int m = data_start_index; m < count_items; m++)
                {
                    const char* label = parser_items.get_string(m);

                    std::string current_high_limit_str, current_low_limit_str;
                    TestItem * item = new TestItem();
                    dprintf("current lable is %s\n", label);
                    std::string  label_string = std::string(label);
                    std::size_t found_space = label_string.find_first_of(' ');
                    if(found_space != std::string::npos)
                    {
                        std::string test_number_str = label_string.substr(0, found_space);
                        test_number = (unsigned int)( std::atoi(test_number_str.c_str()) );

                        std::size_t found_bracket = label_string.find_last_of('(');
                        std::string new_babel;
                        if(found_space != std::string::npos)
                        {
                            new_babel = label_string.substr(found_space+1, found_bracket-found_space-1);
                        }
                        else
                        {
                            new_babel = label_string.substr(found_space+1);
                        }

                        item->set_label(new_babel.c_str());
                        item->set_number(test_number);

                    }
                    else
                    {
                        std::size_t found_bracket1 = label_string.find_last_of('(');
                        std::string new_babel;
                        if(found_bracket1 != std::string::npos && found_bracket1 != 0)
                        {
                            std::size_t found_bracket2 = label_string.find_last_of(')');
                            if(found_bracket2 != std::string::npos)
                            {
                                new_babel = label_string.substr(0, found_bracket1);
                                std::string unit = label_string.substr(found_bracket1+1, found_bracket2-found_bracket1-1);
                                dprintf("new label=%s, unit=%s\n", new_babel.c_str(), unit.c_str());
                                item->set_unit(unit.c_str());
                            }
                        }
                        else
                        {
                            new_babel = label_string;
                        }

                        test_number++;
                        item->set_label(new_babel.c_str());
                        item->set_number(test_number);
                    }

                    // set low first
                    double low_limit_value;
                    if(m < count_low_limit)
                    {
                        const char* temp_low = parser_low_limit.get_string(m);
                        std::string temp_low_str = std::string(temp_low);
                        unsigned int length = temp_low_str.length();
                        if(length > 0)
                        {
                            std::size_t found = temp_low_str.find_last_of(number_digits);
                            std::string low_str = temp_low_str.substr(0, found+1);
                            low_limit_value = std::atof(low_str.c_str());
                            item->set_lowlimit(low_limit_value);
                            if(found < length-1)
                            {
                                std::string units = temp_low_str.substr(found+1);
                                current_low_limit_str = units;
                                item->set_unit(units.c_str());
                            }
                        }
                    }

                    // set high overwrite
                    double high_limit_value;
                    if(m < count_hihg_limit)
                    {
                        const char* temp_high = parser_high_limit.get_string(m);
                        std::string temp_high_str = std::string(temp_high);
                        unsigned int length = temp_high_str.length();
                        if(length > 0)
                        {
                            std::size_t found = temp_high_str.find_last_of(number_digits);
                            std::string high_str = temp_high_str.substr(0, found+1);
                            high_limit_value = std::atof(high_str.c_str());
                            dprintf("temp_high=%s, found=%u, high_str=%s\n",temp_high,found, high_str.c_str());
                            item->set_highlimit(high_limit_value);
                            if(found < length-1)
                            {
                                std::string units = temp_high_str.substr(found+1);
                                current_high_limit_str = units;
                                item->set_unit(units.c_str());
                            }
                        }
                    }


                    // overwrite unit again
                    if(has_units_line)
                    {
                        if(m < count_units)
                        {
                            const char* temp_unit = parser_units.get_string(m);
                            const char* exit_unit = item->get_unit();
                            if(exit_unit)
                            {
                                bool need_convert_low = false;
                                double convert_ratio_low = convert_units(temp_unit, current_low_limit_str, need_convert_low);
                                if(need_convert_low)
                                {
                                    if(item->lowlimit_valid()) item->set_lowlimit(convert_ratio_low * item->get_lowlimit());
                                }

                                bool need_convert_hihg = false;
                                double convert_ratio_high = convert_units(temp_unit, current_high_limit_str, need_convert_hihg);
                                if(need_convert_hihg)
                                {
                                    if(item->highlimit_valid()) item->set_highlimit(convert_ratio_high * item->get_highlimit());
                                }
                            }

                            item->set_unit(temp_unit);
                        }
                    }

                    if(find_compare_flag)
                    {
                        if(item->highlimit_valid() && item->lowlimit_valid())
                        {
                            double low_value = item->get_lowlimit();
                            double high_value = item->get_highlimit();
                            if(low_value > high_value)
                            {
                                item->set_lowlimit(high_value);
                                item->set_highlimit(low_value);
                            }
                        }
                        if(!item->lowlimit_valid() && item->highlimit_valid())
                        {
                            double high_value = item->get_highlimit();
                            if(high_value < 0)
                            {
                                item->set_lowlimit(high_value);
                                item->set_highlimit(0);
                            }
                        }
                    }


                    hold_items_site->add_item(item);
                }
                continue;
			}
		}

		// Read data
		if(reach_data_value)
		{
            //a_line = line.read_line(in, is_strict);
			unsigned int count = parser.parse(a_line, seprator);
			if(count == 0) break;

			TestSite *current_site = nullptr;
			unsigned int site_number = 1;
			if(has_site_number)
            {
                const char *str_site_number = parser.get_string(site_number_index);
                site_number = (unsigned int)std::atoi(str_site_number);
            }

            current_site = GetSiteByNumber(site_number);
            if(!current_site)
            {
                current_site = CreateNewSite(hold_items_site, site_number);
            }


			unsigned int item_count = current_site->get_item_count();
			unsigned int vindex = data_start_index;
			device_index = current_site->get_device_count();
			for(unsigned int i = 0; i < item_count; i++)
			{
				TestItem* vitem = current_site->get_item(i);
				const char* str_value = parser.get_string(vindex);
				if(!str_value || std::strlen(str_value) == 0)
				{
					vindex++;
					if(vindex == count) break;
					continue;
				}

				unsigned int result_count = vitem->get_result_count();
				if(result_count == 0)
				{
                    std::string str_value_unit(str_value);
                    unsigned int found = str_value_unit.find_last_of(number_digits);
                    if(found < str_value_unit.length()-1)
                    {
                        std::string str_unit = str_value_unit.substr(found+1);
                        vitem->set_unit(str_unit.c_str());
                    }
				}

				TestResult result;
				result.set_flag(RESULT_TESTED);
				result.set_index(device_index+1);
				result.set_value(std::atof(str_value));
				vitem->add_result_flag(result);

				vindex++;
				if(vindex == count) break;
			}
			TestDevice device;
			const char* str_sbin_number = parser.get_string(sbin_index);
			const char* str_hbin_number = parser.get_string(hbin_index);
			unsigned short sbin_number = (unsigned short)std::atoi(str_sbin_number);
			unsigned short hbin_number = (unsigned short)std::atoi(str_hbin_number);

			if(has_coord_value)
            {
                ycoord_index = xcoord_index + 1;
                const char* temp_xcoord_str = parser.get_string(xcoord_index);
                const char* temp_ycoord_str = parser.get_string(ycoord_index);
                short xcoord = (short)std::atoi(temp_xcoord_str);
                short ycoord = (short)std::atoi(temp_ycoord_str);
                device.set_xcoord(xcoord);
                device.set_ycoord(ycoord);
            }

			device.set_hardbin(hbin_number);
			device.set_softbin(sbin_number);

			current_site->add_device(device);

			current_site->add_hardbin_bynumber(hbin_number);
			current_site->add_softbin_bynumber(sbin_number);
		}
	}
	in.close();
	delete hold_items_site;
    if(SiteList.size() == 0) return NO_DATA_TO_READ;

    HBin_For_All_Sites = false;
    SBin_For_All_Sites = false;
    return DATALOG_OPERATE_OK;
}
