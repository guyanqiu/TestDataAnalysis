#include <algorithm>
#include <cmath>
#include "statistics.h"

MySPC::MySPC(TestItem* test_item)
{
	mTestItem = test_item;

	mCount = mTestItem->get_result_count();
	mStDev = mTestItem->get_stdev();
	mAverage = mTestItem->get_average();

	mDefaultLowSpec = -9999.9;
	mDefaultHighSpec = 9999.9;

	mLowSpec = mDefaultLowSpec;
	mHighSpec = mDefaultHighSpec;

	if(mTestItem->lowlimit_valid()) mLowSpec = mTestItem->get_lowlimit();
	if(mTestItem->highlimit_valid()) mHighSpec = mTestItem->get_highlimit();
}

MySPC::~MySPC()
{

}

void MySPC::SetDefaltSpec(double low_spec, double high_spec)
{
	mDefaultLowSpec = low_spec;
	mDefaultHighSpec = high_spec;

	if(!(mTestItem->lowlimit_valid()))
		mLowSpec = mDefaultLowSpec;

	if(!(mTestItem->highlimit_valid()))
		mHighSpec = mDefaultHighSpec;
}

double MySPC::GetCp()
{
	if(mStDev == 0) return MAX_SPC_VALUE;

	double T = mHighSpec - mLowSpec;
	double cp =  T / (6.0 * mStDev);
	if(cp < MAX_SPC_VALUE) return cp;
	else return MAX_SPC_VALUE;
}

double MySPC::GetCpu()
{
	if(mStDev == 0) return MAX_SPC_VALUE;

	double cpu = (mHighSpec - mAverage) /(3.0 * mStDev) ;
	if(cpu < MAX_SPC_VALUE) return cpu;
	else return MAX_SPC_VALUE;
}

double MySPC::GetCpl()
{
	if(mStDev == 0) return MAX_SPC_VALUE;

	double cpl = (mAverage - mLowSpec) / (3.0 * mStDev);
	if(cpl < MAX_SPC_VALUE) return cpl;
	else return MAX_SPC_VALUE;
}

double MySPC::GetCpk()
{
	if(mStDev == 0) return MAX_SPC_VALUE;

	double cpu = (mHighSpec - mAverage) /(3.0 * mStDev) ;
	double cpl = (mAverage - mLowSpec) / (3.0 * mStDev);

	double cpk = std::min(cpu, cpl);
	if(cpk > MAX_SPC_VALUE) return MAX_SPC_VALUE;
	return cpk;
}

double MySPC::GetCa()
{
	double U = (mHighSpec + mLowSpec) / 2.0;
	double offset = mAverage - U;
	if(mStDev == 0) 
	{
		if(offset >= 0)
			return MAX_SPC_VALUE;
		else
			return MIN_SPC_VALUE;
	}

	double T = mHighSpec - mLowSpec;
	double ca = offset * 2.0 / T;

	if(ca > MAX_SPC_VALUE) return MAX_SPC_VALUE;
	else if( ca < MIN_SPC_VALUE) return MIN_SPC_VALUE;
	else return ca;
}

//////////////////////////////////////////////////////////////////////////

MyHistogram::MyHistogram(unsigned int min_step, unsigned int max_step)
{
    mMaxStep = max_step;
    mMinStep = min_step;
    mCount = 0;
	mStDev = 0.0;
    mMaxResult = 0.0;
    mMinResult = 0.0;
	mAverage = 0.0;
    mMinRange = 0.0;
}

MyHistogram::~MyHistogram()
{

}

void MyHistogram::AddItem(TestItem* item)
{
	double stdev = item->get_stdev();
	unsigned int result_count = item->get_result_count();
	double min_value = item->get_min();
	double max_value = item->get_max();
    double range = item->get_range();

	if(mCount == 0)
	{
		mCount = result_count;
		mStDev = stdev;
        mMaxResult= max_value;
        mMinResult = min_value;
		mAverage = item->get_average();
        mMinRange = item->get_range();
	}
	else
	{
		if(mCount < result_count) mCount = result_count;
		if(mStDev < stdev) mStDev = stdev;
        if(mMaxResult < max_value) mMaxResult = max_value;
        if(min_value < mMinResult) mMinResult = min_value;
        if(range < mMinRange) mMinRange = range;
	}
}

double MyHistogram::NormalDistribution(double x, double average, double stdev)
{
	double natural_base  = 2.718281828459045;
	double root2pi = 2.506628274631;

	double diff_square = (x - average) * (x - average);
	double delta_square = stdev * stdev;
	double result = std::pow(natural_base , -0.5 * diff_square/delta_square) / (stdev * root2pi);
	return result ;
}

double MyHistogram::StandardNormalDist(double x_coord, double x_average)
{
	double natural_base  = 2.718281828459045;
	double root2pi = 2.506628274631;
	double x = (x_coord - x_average) / mStDev;
	double result = std::pow(natural_base, -0.5 * x * x) / root2pi;

	return result ;
}


unsigned int MyHistogram::GetIntervalCount(TestItem* item, double low_threshold, double high_threshold)
{
	unsigned int count = 0;
	unsigned int result_count = item->get_result_count();
	for(unsigned int i = 0; i < result_count; i++)
	{
		TestResult result = item->get_result(i);
		double value = result.get_value();
		if(value > low_threshold && value <= high_threshold)
			count++;
	}
	return count;
}

unsigned int MyHistogram::GetHistogramGroupCount()
{
    double max_range = mMaxResult - mMinResult;
    double min_range = mMinRange;

    if(min_range != 0)
    {
        double step_value = min_range / (mMinStep - 1);
        unsigned int step_ount = std::ceil(max_range/step_value);
        if(step_ount < mMaxStep) return step_ount;
        else return mMaxStep;
    }
    else
    {
        return mMinStep;
    }
}

double MyHistogram::GetMaxStDev()
{
	return mStDev;
}

double MyHistogram::GetMaxCount()
{
	return mCount;
}

double MyHistogram::GetMaxValue()
{
    return mMaxResult;
}

double MyHistogram::GetMinValue()
{
    return mMinResult;
}
