
#include "logger.hpp"
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>


const Trace sample("sample");


int main(int count, char *arg[]) {
	using std::endl;

	trace::level = Log::never;

	// arg[1] is the loglevel
	if (count > 1)
		trace::level = static_cast<Log>(atoi(arg[1]));
	
	// arg[2+] are the traces
	if (count > 2) {
		for (int i = 2; i < count; ++i)
			trace::activate(arg[i]);
	}

	// use environemnt variable TRACE
	const char * traces = getenv("TRACE");
	if (traces != NULL) {
		std::stringstream immediate(traces);
		std::string trace;
		// comma seperate list
		while (std::getline(immediate, trace, ','))
			trace::activate(trace);
	}

	TRACE(count,arg);


	trace::global << Log::fatal   << "fatal"   << endl;
	trace::global << Log::alert   << "alert"   << endl;
	trace::global << Log::error   << "error"   << endl;
	trace::global << Log::warn    << "warn"    << endl;
	trace::global << Log::notice  << "notice"  << endl;
	trace::global << Log::info    << "info"    << endl;
	trace::global << Log::debug   << "debug"   << endl;
	trace::global << Log::call    << "call"    << endl;
	trace::global << Log::args    << "args"    << endl;
	trace::global << Log::section << "section" << endl;
	trace::global << Log::verbose << "verbose" << endl;
	trace::global << Log::values  << "values"  << endl;
	trace::global << Log::compute << "compute" << endl;
	trace::global << Log::other   << "other"   << endl;
	trace::global << Log::all     << "all"     << endl;

	sample << Log::fatal   << "fatal   sample" << endl;
	sample << Log::alert   << "alert   sample" << endl;
	sample << Log::error   << "error   sample" << endl;
	sample << Log::warn    << "warn    sample" << endl;
	sample << Log::notice  << "notice  sample" << endl;
	sample << Log::info    << "info    sample" << endl;
	sample << Log::debug   << "debug   sample" << endl;
	sample << Log::call    << "call    sample" << endl;
	sample << Log::args    << "args    sample" << endl;
	sample << Log::section << "section sample" << endl;
	sample << Log::verbose << "verbose sample" << endl;
	sample << Log::values  << "values  sample" << endl;
	sample << Log::compute << "compute sample" << endl;
	sample << Log::other   << "other   sample" << endl;
	sample << Log::all     << "all     sample" << endl;

	dbgdo(trace::value("something") << endl);

	std::cout << std::endl;
	return 0;
}

#define LOGGER_IMPL
#include "./logger.hpp"