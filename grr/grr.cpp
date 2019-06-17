#include "grr.h"
#include <cmath>
#include <fstream>
#include <QString>

#include "xlsxdocument.h"
#include "xlsxdocument.h"
#include "xlsxabstractsheet.h"
#include "xlsxformat.h"

const double k_factor = 6.0; //99.73%
static double get_c4(int n)
{
    if(n > 100) return 1.0;
    double x1 = (n - 1)/2.0;
    double x2 = n / 2.0;
    double cn =  sqrt(x1)* exp(lgamma(x1) - lgamma(x2));
    return 1.0/cn;
}

static double get_d2(int n)
{
    double table_d2[50] = {
    1.4142, 1.128 , 1.693 , 2.059 , 2.326 , 2.534 , 2.704 , 2.847 , 2.97  , 3.078 ,
    3.173 , 3.258 , 3.336 , 3.407 , 3.472 , 3.532 , 3.588 , 3.64  , 3.689 , 3.735 ,
    3.778 , 3.819 , 3.858 , 3.895 , 3.931 , 3.964 , 3.997 , 4.027 , 4.057 , 4.086 ,
    4.113 , 4.139 , 4.165 , 4.189 , 4.213 , 4.236 , 4.259 , 4.28  , 4.301 , 4.322 ,
    4.341 , 4.361 , 4.379 , 4.398 , 4.415 , 4.433 , 4.45  , 4.466 , 4.482 , 4.498 };

    if(n <= 50) return table_d2[n-1];
    if(n > 50 && n <= 100) return (3.4873 + 0.0250141*n - 0.00009823*n*n);
    else return 5.0064;
}

static double get_d3(int n)
{
    double table_d3[25]={
    0.0000, 0.8525, 0.8884, 0.8798, 0.8641, 0.8480, 0.8332, 0.8198, 0.8078, 0.7971,
    0.7873, 0.7785, 0.7704, 0.7630, 0.7562, 0.7499, 0.7441, 0.7386, 0.7335, 0.7287,
    0.7242, 0.7199, 0.7159, 0.7121, 0.7084 };
    if(n <= 25) return table_d3[n-1];
    if(n > 25 && n <= 100) return (0.80818 - 0.0051871*n + 0.00005098*n*n - 0.00000019*n*n*n);
    else return 0.609;
}

static double get_d2_star(int g, int m)
{
    double dd2 = get_d2(m);
    double dd3 = get_d3(m);
    return sqrt(dd2*dd2 + dd3*dd3/g);
}


static double betacdf(double x, double a, double b)
{
    double myeps = 2.2204e-16;
    double tiny = 1.0e-30;
    if (x < 0.0 || x > 1.0) return 1.0/0.0;
    if (x > (a+1.0)/(a+b+2.0)) return (1.0-betacdf(1.0-x, b,a));

    double temp = lgamma(a)+lgamma(b)-lgamma(a+b);
    double y = exp(log(x)*a+log(1.0-x)*b-temp) / a;
    double f = 1.0, c = 1.0, d = 0.0;

    int i, m;
    for (i = 0; i <= 400; ++i)
    {
        m = i/2;
        double numerator;
        if (i == 0) numerator = 1.0;
        else if (i % 2 == 0) numerator = (m*(b-m)*x)/((a+2.0*m-1.0)*(a+2.0*m));
        else  numerator = -((a+m)*(a+b+m)*x)/((a+2.0*m)*(a+2.0*m+1));

        d = 1.0 + numerator * d;
        if (fabs(d) < tiny) d = tiny;
        d = 1.0 / d;
        c = 1.0 + numerator / c;
        if (fabs(c) < tiny) c = tiny;

        const double cd = c*d;
        f *= cd;

        if (fabs(1.0-cd) < myeps)  return y * (f-1.0);
    }
    return 1.0e35;
}

static  double betapdf(double x, double a, double b)
{
     double temp = lgamma(a+b) - lgamma(a) - lgamma(b);
     if(x>0 && x< 1 && a > 0 && b > 0 && (a!=1 || b!= 1))
     return exp((a-1)*log(x) + (b-1)*log(1-x) + temp);

     if(x == 0 && a == 1 && b > 0 && b != 1)
     return exp(temp);

     if(x==1 && b == 1 && a>0 && a!=1)
     return exp(temp);

     if(x >= 0 && x <= 1 && a==1 && b== 1)
     return 1.0;

     else return 1.0;
}

static double betainv(double x, double a, double b)
{
     double myeps = 2.2204e-16;
     double y = a / (a+b);

     if(y < myeps) y = sqrt(myeps);
     if(y > 1- myeps) y = 1 - sqrt(myeps);

     double y_new = y;
     double h = 0.0;
     double y_old = y_new;
     for(int i = 0; i <= 40; i++)
     {
        y_old = y_new;
        double cdf = betacdf(y_old, a, b);
        double pdf = betapdf(y_old,a,b);
        h = (cdf - x)/ pdf;
        y_new = y_old - h;
        if(y_new <= myeps)
            y_new = y_old / 10;
        if(y_new >= 1.0 - myeps)
            y_new = 1-(1-y_old)/10;
        h = y_old - y_new;
        if(fabs(h) < sqrt(myeps)) break;
     }
     return y_new;
}

static double finv(double x, int m, int n)
{
    return ((1.0/betainv(1-x, n/2.0, m/2.0)-1)*1.0*n/m);
}


TestItem_GRR::TestItem_GRR()
{
    m_mode = 0;
    m_site_count = 0;
    m_data_count = 0;
    m_number = 0;
    m_high_spec = 0.0;
    m_low_spec = 0.0;

    m_max_average = 0.0;
    m_min_average = 0.0;

    m_sum_range = 0.0;
    m_sum_variance = 0.0;

    m_highspec_valid = false;
    m_lowspec_valid = false;

    m_default_high_spec = 9999.9;
    m_default_low_spec = -9999.9;

    m_measure_guard_band = 0.0;
    m_tcs_error = 0.0;
    m_tcs = 0.0;

    m_anova_mse = 0.0;
    m_anova_msa = 0.0;
    m_anova_ev = 0.0;
    m_anova_av = 0.0;
    m_anova_f = 0.0;
    m_anova_ftest = 0.0;
    m_anova_alpha = 0.0;
}

bool TestItem_GRR::add_item(const TestItem* item)
{
    if(!item) return false;

    double average = item->get_average();
    double stdev = item->get_stdev();
    double range = item->get_range();
    unsigned int data_count = item->get_result_count();
    if(data_count <= 2) return false;

    if(m_site_count == 0)
    {
        m_label.assign(item->get_label());
        m_number = item->get_number();

        const char* str_unit = item->get_unit();
        if(str_unit) m_unit.assign(str_unit);

        if(item->lowlimit_valid())
        {
            m_low_spec = item->get_lowlimit();
            m_lowspec_valid = true;
        }
        if(item->highlimit_valid())
        {
            m_high_spec = item->get_highlimit();
            m_highspec_valid = true;
        }
        m_max_average = average;
        m_min_average = average;
        m_data_count = data_count;
    }
    else
    {
        if(data_count != m_data_count) return false;

        if(m_max_average < average) m_max_average = average;
        if(m_min_average > average) m_min_average = average;
    }
    m_sum_range += range;
    m_sum_variance += (stdev*stdev);
    m_site_count ++;

    return true;
}

void TestItem_GRR::set_calculate_mode(int mode)
{
    m_mode = mode;
}

unsigned int TestItem_GRR::get_number() const
{
    return m_number;
}
const char* TestItem_GRR::get_label() const
{
    return m_label.c_str();
}

const char* TestItem_GRR::get_unit() const
{
    if(m_unit.empty()) return nullptr;
    return m_unit.c_str();
}

void TestItem_GRR::set_default_lowspec(double low_spec)
{
       m_default_low_spec = low_spec;
}

void TestItem_GRR::set_default_highspec(double high_spec)
{
        m_default_high_spec = high_spec;
}

double TestItem_GRR::get_highspec() const
{
    if(m_highspec_valid)  return m_high_spec;
    else return m_default_high_spec;
}

double TestItem_GRR::get_lowspec() const
{
    if(m_lowspec_valid) return m_low_spec;
    else return m_default_low_spec;
}

unsigned int TestItem_GRR::get_site_count() const
{
    return m_site_count;
}

unsigned int TestItem_GRR::get_data_count() const
{
    return m_data_count;
}

double TestItem_GRR::get_tolerance() const
{
    double high_spec = get_highspec();
    double low_spec = get_lowspec();
    return (high_spec - low_spec);
}

double TestItem_GRR::get_max_average() const
{
    return m_max_average;
}

double TestItem_GRR::get_min_average() const
{
    return m_min_average;
}

double TestItem_GRR::get_average_diff() const
{
    return (m_max_average - m_min_average);
}

double TestItem_GRR::get_range_bar() const
{
    if(m_site_count == 0) return 0.0;
    else return (m_sum_range/m_site_count);
}

double TestItem_GRR::get_stdev_bar() const
{
    if(m_site_count == 0) return 0.0;
    else return std::sqrt(m_sum_variance/m_site_count);
}

double TestItem_GRR::get_repeatability() const
{
    if(m_site_count <= 1) return 0.0;

    if(m_mode == 0)
    {
        double stdev_avg = get_stdev_bar();
        return k_factor * stdev_avg;
    }
    else if(m_mode == 1)
    {
        unsigned int g = m_site_count;
        unsigned int m = m_data_count;
		double temp_d2 = get_d2_star(g,m);
		double k1 = 1.0/ temp_d2 ;                                                                                                                                                                                   
        double range_avg = get_range_bar();
        return (k_factor * k1 * range_avg);
    }
    else
    {
        return m_anova_ev;
    }
}

double TestItem_GRR::get_reproducibility() const
{
    if(m_site_count <= 1) return 0.0;

    if(m_mode == 0 || m_mode == 1)
    {
        unsigned int g = 1;
        unsigned int m = m_site_count;
		double temp_d2 = get_d2_star(g, m);
		double k2 = 1.0/temp_d2;

        double ev = get_repeatability();
        double avg_diff = get_average_diff();
        double av = (k_factor * avg_diff * k2) * (k_factor * avg_diff * k2) ;
        av = av - ev * ev / m_data_count;
        if(av < 0) return 0.0;
        else return std::sqrt(av);
    }
    else
    {
        return m_anova_av;
    }
}

double TestItem_GRR::get_r_r() const
{
    double av = get_reproducibility();
    double ev = get_repeatability();
    return std::sqrt(av * av + ev * ev);
}

double TestItem_GRR::get_gr_r() const
{
        double t = get_tolerance();
        if(t == 0) return 1.0;
        double rr = get_r_r();
        double gr_r = rr/t;
        if(gr_r > 1.0) gr_r = 1.0;
        return gr_r;
}

void TestItem_GRR::set_anova(double anova_mse, double anova_msa, double anova_alpha)
{
    m_anova_msa = anova_msa;
    m_anova_mse = anova_mse;
    m_anova_alpha = anova_alpha;
    if(m_anova_msa < 0) m_anova_msa = 0.0;
    if(m_anova_mse < 0) m_anova_mse = 0.0;
    m_anova_ev = k_factor * sqrt(m_anova_mse);
    m_anova_av = k_factor * sqrt(m_anova_msa/m_data_count);
    if(m_anova_mse == 0) m_anova_f = 0.0;
    else m_anova_f = m_anova_msa / m_anova_mse;
    m_anova_ftest = finv(1.0- m_anova_alpha, m_site_count-1, m_site_count*m_data_count - m_site_count);
}

double TestItem_GRR::get_anova_f() const
{
    return m_anova_f;
}
double TestItem_GRR::get_anova_ftest() const
{
    return m_anova_ftest;
}


void TestItem_GRR::set_tcs_error(double tcs_error)
{
    m_tcs_error = tcs_error;
    m_measure_guard_band = 3.0 * tcs_error;


    double t = 0.0;
    if(m_highspec_valid && m_lowspec_valid)
    {
        t = m_high_spec - m_low_spec;
        if(t == 0) m_tcs = 1.0;
        else m_tcs = 6.0 * tcs_error / t;
    }
    else if(m_highspec_valid)
    {
        t = fabs(m_high_spec);
        if(t == 0) m_tcs = 1.0;
        else m_tcs = 3.0 * tcs_error / t;
    }
    else if(m_lowspec_valid)
    {
        t = fabs(m_low_spec);
        if(t == 0) m_tcs = 1.0;
        else m_tcs = 3.0 * tcs_error / t;
    }
    else
    {
        m_tcs = 0.0;
    }
}

double TestItem_GRR::get_measure_guard_band() const
{
    return m_measure_guard_band;
}

double TestItem_GRR::get_tcs_error() const
{
    return m_tcs_error;
}
double TestItem_GRR::get_tcs() const
{
    return m_tcs;
}

TestItem_GRR::~TestItem_GRR()
{
}

TestSite_GRR::TestSite_GRR(double default_low_spec, double default_high_spec, int mode)
{
    m_high_spec = default_high_spec;
    m_low_spec = default_low_spec;
    m_mode = mode;
    m_site_count = 0;
    m_data_count = 0;
    m_calculate_ok = false;
    m_anova_alpha = 0.05;
}

TestSite_GRR::TestSite_GRR()
{
    m_high_spec = 9999.9;
    m_low_spec = -9999.9;
    m_mode = 0;
    m_site_count = 0;
    m_data_count = 0;
    m_calculate_ok = false;
    m_anova_alpha = 0.05;
}

TestSite_GRR::~TestSite_GRR()
{
    clear();
}

void TestSite_GRR::set_default_limit(double default_low_spec, double default_high_spec)
{
    m_high_spec = default_high_spec;
    m_low_spec = default_low_spec;
}

void TestSite_GRR::set_calculate_mode(int mode)
{
    m_mode = mode;
}

void TestSite_GRR::clear()
{
    m_site_list.clear();
    m_grr_list.clear();
    m_site_count = 0;
    m_data_count = 0;
    m_item_count = 0;
    m_calculate_ok = false;
}

GRR_CALCULATE_ERROR TestSite_GRR::add_site(const TestSite* site)
{
    if(!site) return GRR_SITE_NOT_EXIST;

    unsigned int item_count = site->get_item_count();
    if(item_count == 0) return GRR_ITEM_NOT_EXIST;

    unsigned int device_count = site->get_device_count();
    if(device_count <= 2) return GRR_DATACOUNT_TOO_FEW;


    for(unsigned int i = 0; i < item_count; i++)
    {
        TestItem* item = site->get_item(i);
        bool all_pass = item->is_all_pass();
        if(!all_pass) return GRR_DATA_NOT_ALL_PASS;

        unsigned int result_count = item->get_result_count();
        if(result_count != device_count) return GRR_NOT_ALL_ITEM_TESTED;
    }

    unsigned int number_not_same_count = 0;
    unsigned int label_not_same_count = 0;
    if(m_site_count == 0)
    {
        m_data_count = device_count;
        m_item_count = item_count;
        for(unsigned int i = 0; i < item_count; i++)
        {
            TestItem* item = site->get_item(i);
            std::string label(item->get_label());
            unsigned int number = item->get_number();

            m_number_list.push_back(number);
            m_label_list.push_back(label);
        }
    }
    else
    {
        if(m_data_count != device_count) return GRR_DATACOUNT_NOT_SAME;
        if(m_item_count != item_count) return GRR_ITEMCOUNT_NOT_SAME;

        for(unsigned int i = 0; i < item_count; i++)
        {
            TestItem* item = site->get_item(i);
            std::string label(item->get_label());
            unsigned int number = item->get_number();

            if(number != m_number_list[i]) number_not_same_count++;
            if(label != m_label_list[i]) label_not_same_count++;
        }
    }
    m_site_list.push_back(site);
    m_site_count++;

    if(number_not_same_count > 0 && label_not_same_count > 0)
        return GRR_LABEL_AND_NUMBER_NOT_SAME;
    else if(number_not_same_count > 0)
        return GRR_NUMBER_NOT_SAME;
    else if(label_not_same_count > 0)
        return GRR_LABEL_NOT_SAME;
    else
        return GRR_RESULT_IS_OK;
}

GRR_CALCULATE_ERROR TestSite_GRR::calculate()
{
    if(m_site_count < 2) return GRR_SITECOUNT_TOO_FEW;
    if(m_item_count == 0) return GRR_ITEM_NOT_EXIST;
    if(m_data_count < 2) return GRR_DATACOUNT_TOO_FEW;

    double c4_coeff = get_c4(m_site_count);
    for(unsigned int i = 0; i < m_item_count; i++)
    {
        TestItem_GRR grr;
        grr.set_calculate_mode(m_mode);
        grr.set_default_highspec(m_high_spec);
        grr.set_default_lowspec(m_low_spec);

        std::vector<double> sums(m_data_count, 0.0);
        std::vector<double> sums_sq(m_data_count, 0.0);

        double ANOVA_R = 0.0, ANOVA_P=0.0, ANOVA_Q=0.0;

        for(unsigned int s = 0; s < m_site_count; s++)
        {
            const TestSite* site = m_site_list[s];
            TestItem* item = site->get_item(i);
            grr.add_item(item);

            ANOVA_R += item->get_sum_squares();
            double sum_per_site = item->get_sum();
            ANOVA_Q += (sum_per_site*sum_per_site/m_data_count);
            ANOVA_P += sum_per_site;

            for(unsigned int d = 0; d <m_data_count; d++)
            {
                TestResult result = item->get_result(d);
                double temp_value = result.get_value();
                sums[d] += temp_value;
                sums_sq[d] += (temp_value*temp_value);
            }
        }
        ANOVA_P = (ANOVA_P*ANOVA_P)/(m_data_count*m_site_count);

        double sum_stdev = 0.0;
        for(unsigned int d = 0; d < m_data_count; d++)
        {
            double temp = sums_sq[d] - sums[d]*sums[d]/m_site_count;
            if(temp < 0.0) temp = 0.0;
            else temp = std::sqrt(temp/(m_site_count-1));
            sum_stdev += temp;
        }
        double tcs_error = sum_stdev / (m_data_count*c4_coeff);
        grr.set_tcs_error(tcs_error);

        double ANOVA_Se = ANOVA_R - ANOVA_Q;
        double ANOVA_Sa = ANOVA_Q - ANOVA_P;
        double ANOVA_MSe = ANOVA_Se/(m_data_count*m_site_count - m_site_count);
        double ANOVA_MSa = ANOVA_Sa /(m_site_count - 1);

        grr.set_anova(ANOVA_MSe,ANOVA_MSa,m_anova_alpha );

        m_grr_list.push_back(grr);
    }
    m_calculate_ok = true;

    return GRR_RESULT_IS_OK;
}

const TestSite* TestSite_GRR::get_site(unsigned int index) const
{
    return m_site_list[index];
}

TestItem_GRR TestSite_GRR::get_grr(unsigned int index) const
{
    return m_grr_list[index];
}

unsigned int TestSite_GRR::get_data_count() const
{
    return m_data_count;
}

unsigned int TestSite_GRR::get_site_count() const
{
    return m_site_count;
}

unsigned int TestSite_GRR::get_item_count() const
{
    return m_item_count;
}

bool TestSite_GRR::save_result_xls(const char* filename)
{
    if(!m_calculate_ok) return false;
    QXlsx::Document xlsx;
    xlsx.addSheet("GRR Result");
    QString qfilename = QString::fromLocal8Bit(filename);

    unsigned int row = 1, col = 1;
    xlsx.write(row, col++, "TestNumber");
    xlsx.write(row, col++, "TestLabel");
    xlsx.write(row, col++, "Unit");
    xlsx.write(row, col++, "HighSpec");
    xlsx.write(row, col++, "LowSpec");
    xlsx.write(row, col++, "MaxAvg");
    xlsx.write(row, col++, "MinAvg");
    xlsx.write(row, col++, "RxDiff");
    xlsx.write(row, col++, "StDevAvg");
    xlsx.write(row, col++, "EV");
    xlsx.write(row, col++, "AV");
    xlsx.write(row, col++, "R&R");
    xlsx.write(row, col++, "GR&R");
    xlsx.write(row, col++, "---");
    xlsx.write(row, col++, "TCS-Error");
    xlsx.write(row, col++, "MGB");
    xlsx.write(row, col++, "TCS");
    xlsx.write(row, col++, "---");
    xlsx.write(row, col++, "F(alpha=0.05)");
    xlsx.write(row, col++, "F");

    row = 2;
    for(unsigned int i = 0; i < m_item_count; i++)
    {
        TestItem_GRR grr = m_grr_list[i];
        QString label = QString::fromLocal8Bit(grr.get_label());
        QString unit = QString::fromLocal8Bit(grr.get_unit());
        col = 1;
        double grr_value = grr.get_gr_r();
        if(grr_value >= 1.0) grr_value = 1.0;
        xlsx.write(row, col++, grr.get_number());
        xlsx.write( row, col++, label);
        xlsx.write( row, col++, unit);
        xlsx.write(row, col++, grr.get_highspec());
        xlsx.write(row, col++, grr.get_lowspec());
        xlsx.write(row, col++, grr.get_max_average());
        xlsx.write(row, col++, grr.get_min_average());
        xlsx.write(row, col++, grr.get_average_diff());
        xlsx.write(row, col++, grr.get_stdev_bar());
        xlsx.write(row, col++, grr.get_repeatability());
        xlsx.write(row, col++, grr.get_reproducibility());
        xlsx.write(row, col++, grr.get_r_r());

        QXlsx::Format format_red;
        format_red.setPatternBackgroundColor(QColor(Qt::red));
        format_red.setNumberFormat("0.00%");

        QXlsx::Format format_green;
        format_green.setPatternBackgroundColor(QColor(Qt::green));
        format_green.setNumberFormat("0.00%");

        QXlsx::Format format_yellow;
        format_yellow.setPatternBackgroundColor(QColor(Qt::yellow));
        format_yellow.setNumberFormat("0.00%");

        QXlsx::Format format_yellow2;
        format_yellow2.setPatternBackgroundColor(QColor(Qt::yellow));
        format_yellow2.setNumberFormat("0.00");

        if(grr_value <= 0.1) xlsx.write(row, col++, grr_value, format_green);
        else if(grr_value > 0.3) xlsx.write(row, col++, grr_value, format_red);
        else xlsx.write(row, col++, grr_value, format_yellow);

        col++;
        xlsx.write(row, col++, grr.get_tcs_error());
        xlsx.write(row, col++, grr.get_measure_guard_band());

        double tcs_value = grr.get_tcs();
        if(tcs_value <= 0.1) xlsx.write(row, col++, tcs_value, format_green);
        else if(tcs_value > 0.3) xlsx.write(row, col++, tcs_value, format_red);
        else xlsx.write(row, col++, tcs_value, format_yellow);

        col++;
        double ftest = grr.get_anova_ftest();
        double f = grr.get_anova_f();
        xlsx.write(row, col++, ftest);
        if(f > ftest) xlsx.write(row, col++, f, format_yellow2 );
        else xlsx.write(row, col++, f);

        row++;
        col++;
    }
    for(unsigned int i = 0; i < m_site_count; i++)
    {
        const TestSite *site = m_site_list[i];
        QString wname = QString::fromLocal8Bit(site->get_name());

        int start_col = col;
        row = 1;
        xlsx.write(row, col++, wname);
        xlsx.write(row, col++, "Max");
        xlsx.write(row, col++, "Min");
        xlsx.write(row, col++, "Range");
        xlsx.write(row, col++, "Average");
        xlsx.write(row, col++, "Stdev");

        row = 2;

        QXlsx::Format formats[5];
        formats[0].setPatternBackgroundColor(QColor(147, 197, 251));
        formats[1].setPatternBackgroundColor(QColor(218, 254, 251));
        formats[2].setPatternBackgroundColor(QColor(249, 187, 248));
        formats[3].setPatternBackgroundColor(QColor(249, 249, 183));
        formats[4].setPatternBackgroundColor(QColor(203, 207, 214));

        for(unsigned int n = 0; n < m_item_count; n++)
        {
            const TestItem* item = site->get_item(n);

            col = start_col + 1;

            xlsx.write(row, col++, item->get_max(), formats[0]);
            xlsx.write(row, col++, item->get_min(), formats[1]);
            xlsx.write(row, col++, item->get_range(), formats[2]);
            xlsx.write(row, col++, item->get_average(), formats[3]);
            xlsx.write(row, col++, item->get_stdev(), formats[4]);


            row++;
        }
    }
    bool result = xlsx.saveAs(qfilename);
    return result;
}

bool TestSite_GRR::save_result_csv(const char* filename)
{
    if(!m_calculate_ok) return false;
    std::ofstream out(filename, std::ios::out );
    if(!out) return false;

    out<<"TestNumber,TestLabel,HighSpec,LowSpec,Tolerance,Unit,MaxAvg,MinAvg,Xdiff,StDevAvg,EV,AV,R&R,GR&R,,";
    out<<"TCS-Error,MGB,TCS,,";
    out<<"F(alpha=0.05),F"<<std::endl;
    for(unsigned int i = 0; i < m_item_count; i++)
    {
        TestItem_GRR grr = m_grr_list[i];
        out<<grr.get_number()<<",";
        out<<"\""<<grr.get_label()<<"\",";
        out<<grr.get_highspec()<<",";
        out<<grr.get_lowspec()<<",";
        out<<grr.get_tolerance()<<",";
        const char* str_unit = grr.get_unit();
        if(str_unit) out<<"\""<<str_unit<<"\",";
        else out<<",";
        out<< grr.get_max_average()<<",";
        out<<grr.get_min_average()<<",";
        out<<grr.get_average_diff()<<",";
        out<<grr.get_stdev_bar()<<",";
        out<<grr.get_repeatability()<<",";
        out<<grr.get_reproducibility()<<",";
        out<<grr.get_r_r()<<",";
        out<<100.0*grr.get_gr_r()<<"%"<<",,";
        out<<grr.get_tcs_error()<<",";
        out<<grr.get_measure_guard_band()<<",";
        out<<100.0*grr.get_tcs()<<"%"<<",,";
        out<<grr.get_anova_ftest()<<",";
        out<<grr.get_anova_f()<<std::endl;
    }

    out<<std::endl;

    for(unsigned int i = 0; i < m_site_count; i++)
    {
        const TestSite *site = m_site_list[i];
        out<< site->get_name()<<std::endl;
        out<<"TestNumber,TestLabel,Max,Min,Range,Average,Stdev"<<std::endl;

        for(unsigned int n = 0; n < m_item_count; n++)
        {
            const TestItem* item = site->get_item(n);
            out<<item->get_number()<<",";
            out<<"\""<<item->get_label()<<"\",";
            out<<item->get_max()<<",";
            out<<item->get_min()<<",";
            out<<item->get_range()<<",";
            out<<item->get_average()<<",";
            out<<item->get_stdev()<<std::endl;
        }
    }
    out.close();
    return true;
}
