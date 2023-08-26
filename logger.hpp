
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




enum class Log : std::uint32_t {
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
extern Log log_level;

class Trace final {
	const std::string id;
	Trace() = delete;

public:

	Trace(std::string name);
	std::ostream& operator <<(const Log& log) const;
	operator std::string() const { return id; }
};

namespace trace {
	template<typename T>
	inline std::stringstream& arglist_(std::stringstream& out, T value)
	{ out << value << ")"; return out; }

	template<typename T, typename ... R>
	inline std::stringstream& arglist_(std::stringstream& out, T value, R ... other)
	{ out << value << ", "; return arglist_(out, other...); }

	template<typename ... T>
	inline std::string arglist(T ... value) {
		std::stringstream ss; ss << "(";
		return arglist_(ss, value...).str();
	}
	inline std::string arglist( ) { return "( )"; }

	const Trace global(""); // any/none/generic/general

	void activate(std::string traceid);
	void shutdown(std::string traceid);

	std::ostream& call   (std::string name, const Trace trace = global);
	std::ostream& section(std::string name, const Trace trace = global);
	std::ostream& value  (std::string name, const Trace trace = global);
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
#include <algorithm>
#include <iomanip>
#include <unordered_set>

static std::string uppercase(std::string str) {
	std::transform(str.cbegin(), str.cend(), str.begin(),
		[](unsigned char c) -> char { return std::toupper(c); }
	);
	return str;
}

static std::unordered_set<std::string> active_traces;
static int trace_max = 0;

Log log_level = Log::never;
namespace logs {

	std::ostream  ignore(NULL);
	// std::ostream& out   (std::cout);
	// std::ostream& error (std::clog);

}


static std::string log2string(const Log& log) {
	switch(log) {
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
// operator std::string(const Log& log) { return log2string(log); }


static std::string getnow();
static std::ostream& log2stream(const Log& log) {
	// std::ostream& out = logs::ignore;
	uint32_t lvl = (std::uint32_t)(log);
	if (lvl <= (std::uint32_t)Log::never) return logs::ignore;
	if (lvl <= (std::uint32_t)Log::warn ) return std::clog;
	if (lvl <= (std::uint32_t)Log::all  ) return std::cout;
	return logs::ignore;

	// return out;
}
// operator std::ostream(const Log& log) { return log2stream(log); }


std::ostream& Trace::operator<<(const Log& log) const {
	if (active_traces.find(uppercase(this->id)) != active_traces.end())
		return logs::ignore;

	std::ostream& out = log2stream(log);
	out << getnow() << log2string(log) << ' ';

	if (this->id.empty())
		out << std::setw(trace_max+3) << " ";
	else
		out << '(' << std::setw(trace_max) << this->id << ") ";

	return out;
}

static std::unordered_set<std::string> all_traces;
Trace::Trace(std::string name) : id(name) {
	if (all_traces.find(uppercase(name)) != all_traces.end())
		throw std::runtime_error("trace '"+name+"' already exists. Traces have to be global.");
	all_traces.insert(uppercase(name));
	// trace_max = name.length() > trace_max ? name.length() : trace_max;
}


std::ostream& trace::call(std::string funcname, const Trace trace) {
	std::ostream& out = trace << Log::call << "-> " << funcname;
	if (logs::level >= Log::args)
		return out;
	out << std::endl;
	return logs::ignore;
}
std::ostream& trace::section(std::string name, const Trace trace) {
#if defined(DEBUG)
	std::ostream& out = trace << Log::section << sectname;
	if (logs::level >= Log::verbose)
		return out;
	out << std::endl;
#endif
	return logs::ignore;
}
std::ostream& trace::value  (std::string name, const Trace trace) {
#if defined(DEBUG)
	std::ostream& out = trace << Log::values << valname;
	if (logs::level >= Log::compute)
		return out << " = ";
	out << std::endl;
#else
	return logs::ignore;
#endif
}

void trace::activate(std::string name) {
	if (active_traces.find(uppercase(name)) != active_traces.end())
		return; // already in list

	active_traces.insert(uppercase(name));
	trace_max = name.length() > trace_max ? name.length() : trace_max;
}
void trace::shutdown(std::string name) {
	if (active_traces.find(uppercase(name)) == active_traces.end())
		return; // nothing found in list

	active_traces.erase(uppercase(name));
	if (trace_max > name.length())
		return; // no need to recalculate trace_max

	int max = 0;
	for (auto& trace : active_traces)
		max = trace.length() > max ? trace.length() : max;
	trace_max = max;
}

static std::string getnow() {
	namespace sc = std::chrono;
	using std::chrono::system_clock;

	std::string date(24, ' ');
	auto        cur  = system_clock::now();
	std::time_t time = system_clock::to_time_t(cur);
	std::tm*    now  = std::gmtime(&time);
	int         ms   = (sc::duration_cast<sc::milliseconds>(cur.time_since_epoch()).count() % 1000);

	std::strftime(&(date[0]), 24, "%Y-%m-%d %H:%M:%S.", now);
	sprintf(&(date.data()[20]), "%03d", ms);
	return date;
}

#endif /* LOGGER_IMPL */

