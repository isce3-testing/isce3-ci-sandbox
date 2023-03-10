//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Piyush Agram, Bryan Riel
// Copyright 2017-2018

#include "DateTime.h"

#include <limits>
#include <regex>

#include <pyre/journal.h>

#include <isce3/except/Error.h>

#include "TimeDelta.h"

// Handful of utility functions
bool isce3::core::_is_leap(int yy)
{
    unsigned int year = (unsigned int) yy;
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int isce3::core::_days_in_month(int yy, int mm)
{
    assert((mm >= 1) && (mm <= 12));
    if (mm == 2 && _is_leap(yy))
        return 29;
    else
        return DaysInMonths[mm - 1];
}

int isce3::core::_days_before_year(int year)
{
    assert(year >= 1);
    int yy = year - 1;
    return yy * 365 + yy / 4 - yy / 100 + yy / 400;
}

int isce3::core::_days_before_month(int yy, int mm)
{
    assert((mm >= 1) && (mm <= 12));
    int dd = DaysBeforeMonths[mm - 1];
    if (mm > 2 && _is_leap(yy))
        dd++;
    return dd;
}

int isce3::core::_ymd_to_ord(int yy, int mm, int dd)
{
    return _days_before_year(yy) + _days_before_month(yy, mm) + dd;
}

void isce3::core::_ord_to_ymd(int ord, int& y, int& m, int& d)
{
    int n, n1, n4, n100, n400, leapyear, preceding;

    assert(ord >= 1);
    ord--;
    n400 = ord / DAYSPER400;
    n = ord % DAYSPER400;

    y = n400 * 400 + 1;

    n100 = n / DAYSPER100;
    n = n % DAYSPER100;

    n4 = n / DAYSPER4;
    n = n % DAYSPER4;

    n1 = n / DAY_TO_YEAR;
    n = n % DAY_TO_YEAR;

    y += n100 * 100 + n4 * 4 + n1;

    if (n1 == 4 || n100 == 4) {
        assert(n == 0);
        y -= 1;
        m = 12;
        d = 31;
        return;
    }

    leapyear = n1 == 3 && (n4 != 24 || n100 == 3);
    assert(leapyear == _is_leap(y));
    m = (n + 50) >> 5;
    preceding = DaysBeforeMonths[m - 1] + (m > 2 && leapyear);
    if (preceding > n) {
        m--;
        preceding -= _days_in_month(y, m);
    }

    n -= preceding;
    assert(0 <= n);
    assert(n < _days_in_month(y, m));
    d = n + 1;
}

// Normalize function
void isce3::core::DateTime::_normalize_time()
{
    // Adjust fractional part
    {
        int ipart = frac - (frac < 0);
        frac -= ipart;
        seconds += ipart;
    }

    {
        int ipart = (seconds / MIN_TO_SEC) - (seconds < 0);
        seconds -= ipart * MIN_TO_SEC;
        minutes += ipart;
    }

    {
        int ipart = (minutes / HOUR_TO_MIN) - (minutes < 0);
        minutes -= ipart * HOUR_TO_MIN;
        hours += ipart;
    }

    {
        int ipart = (hours / DAY_TO_HOUR) - (hours < 0);
        hours -= ipart * DAY_TO_HOUR;
        days += ipart;
    }
}

void isce3::core::DateTime::_normalize_date()
{
    int daylimit;
    daylimit = _days_in_month(year, months);

    if (days < 1 || days > daylimit) {
        if (days == 0) {
            months--;

            if (months > 0) {
                days = _days_in_month(year, months);
            } else {
                year--;
                months = 12;
                days = 31;
            }
        } else if (days == daylimit + 1) {
            months++;
            days = 1;
            if (months == 13) {
                months = 1;
                year++;
            }
        } else {
            int y, m, d;
            int ord = _ymd_to_ord(year, months, 1) + days - 1;
            assert(ord < 1 || ord > MAXORDINAL);
            _ord_to_ymd(ord, y, m, d);
            year = y;
            months = m;
            days = d;
        }
    }

    assert((months > 0) && (days > 0));
}

void isce3::core::DateTime::_normalize()
{
    _normalize_time();
    _normalize_date();
}

// Constructors
void isce3::core::DateTime::_init(int yy, int mm, int dd, int hh, int mn,
                                  int ss, double ff)
{
    year = yy;
    months = mm;
    days = dd;
    hours = hh;
    minutes = mn;
    seconds = ss;
    frac = ff;
    _normalize();
}

isce3::core::DateTime::DateTime(double ord)
{
    int iord = ord;
    double secs = (ord - iord) * DAY_TO_SEC;
    int y, m, d;
    _ord_to_ymd(ord, y, m, d);
    _init(y, m, d, 0, 0, 0, secs);
}

isce3::core::DateTime::DateTime(int yy, int mm, int dd)
{
    _init(yy, mm, dd, 0, 0, 0, 0);
}

isce3::core::DateTime::DateTime(int yy, int mm, int dd, int hh, int mn, int ss)
{
    _init(yy, mm, dd, hh, mn, ss, 0);
}

isce3::core::DateTime::DateTime(int yy, int mm, int dd, int hh, int mn,
                                double ss)
{
    int ipart = ss;
    double fpart = ss - ipart;
    _init(yy, mm, dd, hh, mn, ipart, fpart);
}

isce3::core::DateTime::DateTime(int yy, int mm, int dd, int hh, int mn, int ss,
                                double ff)
{
    _init(yy, mm, dd, hh, mn, ss, ff);
}

isce3::core::DateTime::DateTime(const DateTime& ts)
{
    _init(ts.year, ts.months, ts.days, ts.hours, ts.minutes, ts.seconds,
          ts.frac);
}

// From string
isce3::core::DateTime::DateTime(const std::string& datestr)
{
    strptime(datestr);
}

// Assignment from another DateTime
isce3::core::DateTime& isce3::core::DateTime::operator=(const DateTime& ts)
{
    _init(ts.year, ts.months, ts.days, ts.hours, ts.minutes, ts.seconds,
          ts.frac);
    return *this;
}

// Comparison operators
bool isce3::core::DateTime::operator<(const DateTime& ts) const
{
    return ((*this) - ts) < 0.0;
}

bool isce3::core::DateTime::operator>(const DateTime& ts) const
{
    return ((*this) - ts) > 0.0;
}

bool isce3::core::DateTime::operator<=(const DateTime& ts) const
{
    return !((*this) > ts);
}

bool isce3::core::DateTime::operator>=(const DateTime& ts) const
{
    return !(*this < ts);
}

bool isce3::core::DateTime::operator==(const DateTime& ts) const
{
    return (*this) - ts == 0.0;
}

bool isce3::core::DateTime::operator!=(const DateTime& ts) const
{
    return ((*this) - ts) != 0.0;
}

// Math operators
isce3::core::DateTime
isce3::core::DateTime::operator+(const TimeDelta& ts) const
{
    int ord = _ymd_to_ord(year, months, days) + ts.days;
    int y, m, d;
    _ord_to_ymd(ord, y, m, d);

    return DateTime(y, m, d, hours + ts.hours, minutes + ts.minutes,
                    seconds + ts.seconds, frac + ts.frac);
}

isce3::core::DateTime isce3::core::DateTime::operator+(const double& ts) const
{
    return (*this) + TimeDelta(ts);
}

isce3::core::DateTime
isce3::core::DateTime::operator-(const TimeDelta& ts) const
{
    int ord = _ymd_to_ord(year, months, days) - ts.days;
    int y, m, d;
    _ord_to_ymd(ord, y, m, d);

    return DateTime(y, m, d, hours - ts.hours, minutes - ts.minutes,
                    seconds - ts.seconds, frac - ts.frac);
}

isce3::core::DateTime isce3::core::DateTime::operator-(const double& ts) const
{
    return (*this) - TimeDelta(ts);
}

// In-place update operators
isce3::core::DateTime& isce3::core::DateTime::operator+=(const TimeDelta& ts)
{
    int ord = _ymd_to_ord(year, months, days) + ts.days;
    int y, m, d;
    _ord_to_ymd(ord, y, m, d);
    _init(y, m, d, hours + ts.hours, minutes + ts.minutes, seconds + ts.seconds,
          frac + ts.frac);
    return *this;
}

isce3::core::DateTime& isce3::core::DateTime::operator+=(const double& ts)
{
    (*this) += TimeDelta(ts);
    return *this;
}

isce3::core::DateTime& isce3::core::DateTime::operator-=(const TimeDelta& ts)
{
    int ord = _ymd_to_ord(year, months, days) - ts.days;
    int y, m, d;
    _ord_to_ymd(ord, y, m, d);
    _init(y, m, d, hours - ts.hours, minutes - ts.minutes, seconds - ts.seconds,
          frac - ts.frac);
    return *this;
}

isce3::core::DateTime& isce3::core::DateTime::operator-=(const double& ts)
{
    (*this) -= TimeDelta(ts);
    return *this;
}

isce3::core::TimeDelta
isce3::core::DateTime::operator-(const DateTime& ts) const
{
    int delta_days = _ymd_to_ord(year, months, days) -
                     _ymd_to_ord(ts.year, ts.months, ts.days);
    return TimeDelta(delta_days, hours - ts.hours, minutes - ts.minutes,
                     seconds - ts.seconds, frac - ts.frac);
}

bool isce3::core::DateTime::isClose(const DateTime& ts) const
{
    TimeDelta dt = (*this) - ts;
    return std::abs(dt.getTotalSeconds()) < TOL_SECONDS;
}

bool isce3::core::DateTime::isClose(const DateTime& ts,
                                    const TimeDelta& errtol) const
{
    TimeDelta dt = (*this) - ts;
    return std::abs(dt.getTotalSeconds()) < errtol.getTotalSeconds();
}

// Get seconds of day
double isce3::core::DateTime::secondsOfDay() const
{
    // Make a midnight datetime
    DateTime midnight(year, months, days, 0, 0, 0);
    // Timedelta
    TimeDelta dt = (*this) - midnight;
    // Return the total seconds
    return dt.getTotalSeconds();
}

// Get seconds since epoch at provided datetime
double isce3::core::DateTime::secondsSinceEpoch(const DateTime& epoch) const
{
    // Timedelta
    TimeDelta dt = (*this) - epoch;
    // Return the total seconds
    return dt.getTotalSeconds();
}

// Get seconds since fixed 1970-01-01 epoch
double isce3::core::DateTime::secondsSinceEpoch() const
{
    TimeDelta dt = (*this) - MIN_DATE_TIME;
    return dt.getTotalSeconds();
}

// Set time based on seconds since 1970-01-01 epoch
void isce3::core::DateTime::secondsSinceEpoch(double sec)
{
    _init(1970, 1, 1, 0, 0, 0, 0);
    (*this) += TimeDelta(sec);
}

// Formatting output options
std::string isce3::core::DateTime::isoformat() const
{
    // Buffer for holding output
    char buffer[100];
    // Convert seconds and fraction into decimal seconds
    double decimal_seconds = this->seconds + this->frac;
    // Fill the buffer
    snprintf(buffer, 100, "%04d-%02d-%02dT%02d:%02d:%012.9f", this->year,
            this->months, this->days, this->hours, this->minutes,
            decimal_seconds);
    // Convert to string and output
    std::string outputStr {buffer};
    return outputStr;
}

// Parse string to fill date time contents
void isce3::core::DateTime::strptime(std::string datetime_str)
{
    if (!isIsoFormat(datetime_str))
        throw isce3::except::InvalidArgument(
                ISCE_SRCINFO(),
                "ISO-8601 incompatible or bad value for datetime string!");
    // if time section is missing then append time
    if (datetime_str.find(':') == std::string::npos)
        datetime_str = datetime_str.substr(0, 10) + "T00:00:00";
    // replace decimal point ":" or "," by "." if any
    if (datetime_str.find_last_of(':') == 19 ||
        datetime_str.find_last_of(',') == 19)
        datetime_str.replace(19, 1, ".");
    // supports any desired sep for string format!
    std::string str_fmt {"%d-%d-%d%d:%d:%lf"};
    str_fmt.insert(8, 1, datetime_str[10]);
    // Parse the string
    double decimal_seconds = std::numeric_limits<double>::min();
    std::sscanf(datetime_str.c_str(), str_fmt.c_str(), &this->year,
                &this->months, &this->days, &this->hours, &this->minutes,
                &decimal_seconds);
    // Convert decimal seconds to integer and fraction
    int integer_seconds = int(decimal_seconds);
    this->seconds = integer_seconds;
    this->frac = decimal_seconds - integer_seconds;
}

bool isce3::core::DateTime::isIsoFormat(const std::string& datetime_str)
{
    return std::regex_match(datetime_str, std::regex(ISOFMT8601));
}

namespace isce3 { namespace core {

std::ostream& operator<<(std::ostream& os, const DateTime& datetime)
{
    return os << std::string(datetime);
}

}} // namespace isce3::core

// end of file
