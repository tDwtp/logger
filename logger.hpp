
#ifndef LOGGER_H
#define LOGGER_H

#include <ostream>
#include <sstream>
#include <string>



#if defined(__PRETTY_FUNC__)
#define TRACE(...) trace::call(__PRETTY_FUNC__) << trace::arglist(__VA_ARGS__) << std::endl
#endif

#if !defined(TRACE) && defined(__FUNCSIG__)
#define TRACE(...) trace::call(__FUNCSIG__) << trace::arglist(__VA_ARGS__) << std::endl
#endif

#if !defined(TRACE)
#define TRACE(...) trace::call(__func__) << trace::arglist(__VA_ARGS__) << std::endl
#endif

#if defined(DEBUG)
#define dbgdo(...) __VA_ARGS__
#else
#define dbgdo(...)
#endif




enum class Log : unsigned {
	never   =  0, // ignore all logging
	fatal   =  1, // app will terminate
	alert   =  2, // app wont terminate, requires live adjustment to prevent FATAL
	error   =  3, // error occured, reset state and keep running
	warn    =  4, // unwanted state detected, dont stop
	notice  =  5, // changes (nothing important; notable)
	info    =  6, // app-settings (nothing important; unnotable)
	debug   =  7, // operational changes (calls, object-settings, events...)
	// trace levels, use specific trace functions
	call    =  8, // trace entering a function
	args    =  9, // add arguments used for CALLBACK
	section = 10, // interleave trace with function section names
	verbose = 11, // add additional basic infos for interleaves
	values  = 12, // trace values between interleaves
	compute = 13, // add calculations for values or other additional info
	other   = 14, // trace unrelated values additionally to calculations
	all     = 15  // arbitrary information which does not seem relevant
};

class Trace final {

public:
	Trace() : id("") {};
	Trace(std::string name) : id(name) { };
	std::ostream& operator <<(const Log& log) const;
	operator std::string() const { return id; }

private:
	const std::string id;

};

namespace trace {
	extern const Trace global; // any/none/generic/general
	extern Log level;

	void activate(std::string traceid);
	void shutdown(std::string traceid);

	std::ostream& call   (std::string name, const Trace trace = global);
	std::ostream& section(std::string name, const Trace trace = global);
	std::ostream& value  (std::string name, const Trace trace = global);

	template<typename T, typename ... R>
	inline std::string arglist(T value, R... rest) {
		std::stringstream ss;
		ss << "(" << value;
		int x[sizeof...(R)] = {(ss << ", " << rest, 0)...};
		ss << ")";
		return ss.str();
	}
	inline std::string arglist( ) { return "( )"; }
}

#endif /* LOGGER_H */

#ifdef LOGGER_IMPL
#undef LOGGER_IMPL


// C
#include <ctime>
#include <cctype>

// C++
#include <chrono>
#include <iostream>
#include <iomanip>
#include <unordered_set>

Log trace::level = Log::never;

static std::ostream ignore(nullptr);
static std::unordered_set<std::string> active;
static int trace_max = 0;

static std::string& uppercase(std::string& str);
static std::string log2string(const Log& level);
static std::ostream& log2stream(const Log& level);
static std::string getnow();


std::ostream& Trace::operator<<(const Log& log) const {
	std::string trace = this->id;
	uppercase(trace);
	if (!this->id.empty() && active.find(trace) == active.end())
		return ignore;

	std::ostream& out = log2stream(log);
	out << getnow() << ' ' << log2string(log) << ' ';

	if (!this->id.empty())
		out << '(' << std::setw(trace_max) << this->id << ") ";
	else if (trace_max > 0)
		out << std::setw(trace_max+3) << " ";

	return out;
}


std::ostream& trace::call(std::string funcname, const Trace trace) {
	std::ostream& out = trace << Log::call << "-> " << funcname;
	if (trace::level >= Log::args)
		return out;
	out << std::endl;
	return ignore;
}

std::ostream& trace::section(std::string name, const Trace trace) {
	std::ostream& out = trace << Log::section << name;
	if (trace::level >= Log::verbose)
		return out << ' ';
	out << std::endl;
	return ignore;
}

std::ostream& trace::value  (std::string name, const Trace trace) {
	std::ostream& out = trace << Log::values << name;
	if (trace::level >= Log::compute)
		return out << " = ";
	out << std::endl;
	return ignore;
}

void trace::activate(std::string name) {
	if (name.empty())
		return; // global trace is always active
	uppercase(name);
	if (active.find(name) != active.end())
		return; // already in list

	active.insert(name);
	trace_max = name.length() > trace_max ? name.length() : trace_max;
}

void trace::shutdown(std::string name) {
	uppercase(name);
	if (active.find(name) == active.end())
		return; // nothing found in list

	active.erase(name);
	if (trace_max > name.length())
		return; // no need to recalculate trace_max

	int max = 0;
	for (auto& trace : active)
		max = trace.length() > max ? trace.length() : max;
	trace_max = max;
}



const Trace trace::global("");



static std::string& uppercase(std::string& str) {
	for (int i = 0; i < str.size(); ++i)
		str[i] = std::toupper(str[i]);
	return str;
}

static std::string log2string(const Log& level) {
	switch(level) {
	case Log::fatal  : return "\033[31mFATAL  \033[m";
	case Log::alert  : return "\033[31mALERT  \033[m";
	case Log::error  : return "\033[31mERROR  \033[m";
	case Log::warn   : return "\033[33mWARN   \033[m";
	case Log::notice : return "\033[36mNOTICE \033[m";
	case Log::info   : return "\033[36mINFO   \033[m";
	case Log::debug  : return "\033[32mDEBUG  \033[m";

	case Log::call   : return "\033[1;32mCALL   \033[m";
	case Log::args   : return "\033[1;32mARGS   \033[m";
	case Log::section: return "\033[38mSECTION\033[m";
	case Log::verbose: return "\033[38mVERBOSE\033[m";
	case Log::values : return "\033[38mVALUES \033[m";
	case Log::compute: return "\033[38mCOMPUTE\033[m";
	case Log::other  : return "\033[38mOTHER  \033[m";
	case Log::all    : return "\033[38mALL    \033[m";

	default: return "\033[38mUNKNOWN \033[m:";
	}
}

static std::ostream& log2stream(const Log& level) {
	if (level > trace::level) return ignore;
	if (level <=  Log::never) return ignore;
	if (level <=  Log::warn ) return std::clog;
	if (level <=  Log::all  ) return std::cout;
	return ignore;
}

static std::string getnow() {
	namespace sc = std::chrono;
	using std::chrono::system_clock;

	char date[25];
	auto        cur  = system_clock::now();
	std::time_t time = system_clock::to_time_t(cur);
	std::tm*    now  = std::gmtime(&time);
	int         ms   = (sc::duration_cast<sc::milliseconds>(cur.time_since_epoch()).count() % 1000);

	std::strftime(date, 24, "%Y-%m-%d %H:%M:%S.", now);
	sprintf(date + 20, "%03d", ms);
	return std::string(date);
}

#endif /* LOGGER_IMPL */

