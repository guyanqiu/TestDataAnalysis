#ifndef _MY_DATALOG_STATISTICS_H_
#define _MY_DATALOG_STATISTICS_H_

#include "datalog/datalog.h"

#define MAX_SPC_VALUE 999.9
#define MIN_SPC_VALUE -999.9

class MySPC
{
public:
	explicit MySPC(TestItem* test_item);
	~MySPC();
	void SetDefaltSpec(double low_spec, double high_spec);
	double GetCp();
	double GetCpk();
	double GetCpu();
	double GetCpl();
	double GetCa();

private:
	MySPC(const MySPC& );
	MySPC& operator=(const MySPC& );
	
	unsigned int mCount;
	double mDefaultLowSpec;
	double mDefaultHighSpec;
	double mAverage;
	double mStDev;
	double mLowSpec;
	double mHighSpec;

	TestItem* mTestItem;
};

class MyHistogram
{
public:
    MyHistogram(unsigned int min_step, unsigned int max_step);
	~MyHistogram();	
	void AddItem(TestItem* item);

	static double NormalDistribution(double x, double average, double stdev);
	double StandardNormalDist(double x,  double x_average);
	unsigned int GetHistogramGroupCount();
	static unsigned int GetIntervalCount(TestItem* item, double low_threshold, double high_threshold);
	double GetMaxStDev();
	double GetMaxCount();
	double GetMaxValue();
	double GetMinValue();
	
private:
	MyHistogram(const MyHistogram& );
	MyHistogram& operator=(const MyHistogram& );

	unsigned int mCount;
    unsigned int mMaxStep;
    unsigned int mMinStep;
	double mStDev;
    double mMaxResult;
    double mMinResult;
	double mAverage;
    double mMinRange;
};

#endif//_MY_DATALOG_STATISTICS_H_
