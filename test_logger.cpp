
#include "logi.hpp"
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>


const Trace sample("sample");


int main(int count, char *arg[]) {
	using std::endl;

	log_level = Log::never;

	if (count > 1)
		log_level = static_cast<Log>(atoi(arg[1]));
	if (count > 2) {
		for (int i = 2; i < count; ++i)
			trace::activate(arg[i]);
	}
	std::string traces = getenv("TRACE");
	std::stringstream immediate(traces);
	for (std::string trace; std::getline(immediate, trace, ',');) {
		trace::activate(trace);
	}

	TRACE(count,arg);


	trace::global << Log::fatal  << "fatal"  << endl; sample << Log::fatal  << "fatal  sample" << endl;
	trace::global << Log::alert  << "alert"  << endl; sample << Log::alert  << "alert  sample" << endl;
	trace::global << Log::error  << "error"  << endl; sample << Log::error  << "error  sample" << endl;
	trace::global << Log::warn   << "warn"   << endl; sample << Log::warn   << "warn   sample" << endl;
	trace::global << Log::notice << "notice" << endl; sample << Log::notice << "notice sample" << endl;
	trace::global << Log::info   << "info"   << endl; sample << Log::info   << "info   sample" << endl;
	trace::global << Log::debug  << "debug"  << endl; sample << Log::debug  << "debug  sample" << endl;

	dbgdo(trace::value("something") << endl);

	std::cout << std::endl;
	return 0;
}

#define LOGGER_IMPL
#include "./logger.hpp"