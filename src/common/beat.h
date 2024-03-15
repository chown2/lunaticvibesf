#pragma once
#include "types.h"
#include "fraction.h"
#include <string>
#include <chrono>
#include <variant>
#include <climits>
#include <iostream>

typedef unsigned Bar;
typedef double BPM;

class Metre : public fraction
{
protected:
	operator double() const { return fraction::operator double(); }

public:
	Metre() : fraction() {}
	Metre(long long division_level, long long multiple_level) : fraction(division_level, multiple_level, false) {}
	Metre(double value) : fraction(static_cast<long long>(value * 1e12), 1e12, true) {}
	~Metre() override {}

	double toDouble() const { return operator double(); }
	bool operator==(const Metre& rhs) const { return fraction(division_level(), multiple_level(), true) == fraction(rhs.division_level(), rhs.multiple_level(), true); }

	long long division_level() const { return _numerator; }
	long long multiple_level() const { return _denominator; }
};

typedef fraction Segment;  // Normal rhythm indicator, must be normalized (value: [0, 1)).

typedef std::chrono::milliseconds timeNormRes; // Regular time, expect millisecond (1/1e3 second). Used for general timing demand
typedef std::chrono::nanoseconds  timeHighRes; // High resolution time, expect nanosecond (1/1e9 second). Used for note timings

namespace lunaticvibes {

#pragma warning(push)
#pragma warning(disable:4244)
class Time
{
private:
	decltype(std::declval<timeNormRes>().count()) _regular;
	decltype(std::declval<timeHighRes>().count()) _highres;
public:
	// TODO: have a separate helper that would construct with current time.
	Time()
	{
		auto now = std::chrono::system_clock::now().time_since_epoch();
		_regular = std::chrono::duration_cast<timeNormRes>(now).count();
		_highres = std::chrono::duration_cast<timeHighRes>(now).count();
	}
	constexpr Time(long long n, bool init_with_high_resolution_timestamp = false) : _regular(), _highres()
	{
		if (init_with_high_resolution_timestamp || n > LLONG_MAX / 1000000)
		{
			_highres = n;
			_regular = std::chrono::duration_cast<timeNormRes>(timeHighRes(n)).count();
		}
		else
		{
			_regular = n;
			_highres = std::chrono::duration_cast<timeHighRes>(timeNormRes(n)).count();
		}
	}

	[[nodiscard]] static constexpr Time singleBeatLengthFromBPM(BPM bpm)
	{
		using namespace std::chrono;
		return Time(6e4 / bpm * duration_cast<timeHighRes>(1ms).count(), true);
	}

	constexpr Time operator- () const { return Time(-_highres, true); }
	constexpr Time operator+ (const Time& rhs) const { return {_highres + rhs._highres, true}; }
	constexpr Time operator- (const Time& rhs) const { return {_highres - rhs._highres, true}; }
	constexpr Time operator* (const double rhs) const { return {static_cast<long>(_highres * rhs), true}; }
	constexpr Time& operator+= (const Time& rhs) { _highres += rhs._highres; _regular = _highres / 1000000; return *this; }
	constexpr Time& operator-= (const Time& rhs) { _highres -= rhs._highres; _regular = _highres / 1000000; return *this; }
	constexpr bool operator< (const Time& rhs) const { return _highres < rhs._highres; }
	constexpr bool operator> (const Time& rhs) const { return _highres > rhs._highres; }
	constexpr bool operator<= (const Time& rhs) const { return _highres <= rhs._highres; }
	constexpr bool operator>= (const Time& rhs) const { return _highres >= rhs._highres; }
	constexpr bool operator== (const Time& rhs) const { return _highres == rhs._highres; }
	constexpr bool operator!= (const Time& rhs) const { return _highres != rhs._highres; }
	friend inline std::ostream& operator<< (std::ostream& os, const Time& t) { return os << t._regular << "ms / " << t._highres << "ns"; }

	[[nodiscard]] constexpr auto norm() const { return _regular; }  // ms
	[[nodiscard]] constexpr auto hres() const { return _highres; }  // ns
};
#pragma warning(pop)

} // namespace lunaticvibes

struct Note
{
    Bar measure;        // Which measure the note is placed
    Metre pos;        // Which metre the note is placed in visual (ignoring SV & STOP), can be above 1
    lunaticvibes::Time time;             // Timestamp

	enum Flags
	{
		LN_TAIL = 1 << 0,

		BGA_BASE = 0b01 << 1,
		BGA_LAYER = 0b10 << 1,
		BGA_MISS = 0b11 << 1,

		MINE = 1 << 4,
		INVS = 1 << 5,

		SCRATCH = 1 << 6,
		KEY_6_7 = 1 << 7,
	};
	size_t flags = 0;			// used for distinguishing plain notes

	long long dvalue;		// regular integer values
	double fvalue;			// used by #BPM, bar length, etc 
};
