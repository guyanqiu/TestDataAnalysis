#ifndef _MY_GRR_CALCULATE_H_
#define _MY_GRR_CALCULATE_H_

#include "../datalog/datalog.h"
#include <vector>
#include <string>


class TestItem_GRR
{
private:
    int m_mode;
    unsigned int m_site_count;
    unsigned int m_data_count;

    double m_high_spec;
    double m_low_spec;
    bool m_highspec_valid;
    bool m_lowspec_valid;
    double m_default_high_spec;
    double m_default_low_spec;

    double m_max_average;
    double m_min_average;

    double m_sum_range;
    double m_sum_variance;

    unsigned int m_number;
    std::string m_label;
    std::string m_unit;

    double m_measure_guard_band;
    double m_tcs_error;
    double m_tcs;

public:
    bool add_item(const TestItem* item);
    void set_default_lowspec(double low_spec);
    void set_default_highspec(double high_spec);
    void set_calculate_mode(int mode);
    double get_highspec() const;
    double get_lowspec() const;
    unsigned int get_site_count() const;
    unsigned int get_data_count() const;
    double get_tolerance() const;
    double get_max_average() const;
    double get_min_average() const;
    double get_average_diff() const;
    double get_range_bar() const;
    double get_stdev_bar() const;
    double get_repeatability() const;
    double get_reproducibility() const;
    double get_r_r() const;
    double get_gr_r() const;
    unsigned int get_number() const;
    const char* get_label() const;
    const char* get_unit() const;

    double get_measure_guard_band() const;
    double get_tcs_error() const;
    double get_tcs() const;

public:
    TestItem_GRR();
    ~TestItem_GRR();

public:
    void set_tcs_error(double tcs_error);
};


enum GRR_CALCULATE_ERROR : int
{
    GRR_RESULT_IS_OK = 0,
    GRR_LABEL_NOT_SAME = 1,
    GRR_NUMBER_NOT_SAME = 2,
    GRR_LABEL_AND_NUMBER_NOT_SAME = 3,
    GRR_DATACOUNT_NOT_SAME = -1,
    GRR_ITEM_NOT_EXIST = -2,
    GRR_DATACOUNT_TOO_FEW = -3,
    GRR_DATA_NOT_ALL_PASS = -4,
    GRR_SITE_NOT_EXIST = -5,
    GRR_ITEMCOUNT_NOT_SAME = -6,
    GRR_SITECOUNT_TOO_FEW = -7,
    GRR_NOT_ALL_ITEM_TESTED = -8,
};

class TestSite_GRR
{
private:
    unsigned int m_site_count;
    unsigned int m_item_count;
    unsigned int m_data_count;

    double m_low_spec;
    double m_high_spec;
    int m_mode;
    bool m_calculate_ok;

    std::vector<const TestSite*> m_site_list;
    std::vector<TestItem_GRR> m_grr_list;
    std::vector<std::string> m_label_list;
    std::vector<unsigned int> m_number_list;

public:

    GRR_CALCULATE_ERROR add_site(const TestSite* site);
    GRR_CALCULATE_ERROR calculate();
    const TestSite* get_site(unsigned int index) const;
    TestItem_GRR get_grr(unsigned int index) const;
    unsigned int get_data_count() const;
    unsigned int get_site_count() const;
    unsigned int get_item_count() const;
    bool save_result_csv(const char* filename);
    bool save_result_xls(const char* filename);
    void set_default_limit(double default_low_spec, double default_high_spec);
    void set_calculate_mode(int mode);
    void clear();

public:
    TestSite_GRR(double default_low_spec, double default_high_spec, int mode = 0);
    TestSite_GRR();
    ~TestSite_GRR();
private:
    TestSite_GRR& operator=(const TestSite_GRR& );
    TestSite_GRR(const TestSite_GRR& );
};

#endif // _MY_GRR_CALCULATE_H_
