#include <exception>
#include <string>

#include "phonenumbers/phonenumberutil.h"

extern "C" {
	#include "postgres.h"
}

#include "error_handling.h"

using namespace i18n::phonenumbers;

static const char* parseErrorMessage(PhoneNumberUtil::ErrorType error) {
	using PNU = i18n::phonenumbers::PhoneNumberUtil;
	switch(error) {
	case PNU::NO_PARSING_ERROR:
		return "Parsed successfully";
	case PNU::NOT_A_NUMBER:
		return "String does not appear to contain a phone number.";
	case PNU::INVALID_COUNTRY_CODE_ERROR:
		return "Invalid country code";
	//TODO: handle more error cases specifically.
	default:
		//We have some generic parsing error.
		return "Unable to parse number";
	}
}

/*
 * Utility functions for error handling
 */

void reportOutOfMemory() {
	ereport(ERROR,
		(errcode(ERRCODE_OUT_OF_MEMORY),
		 errmsg("Out of memory")));
}

void reportException(const std::exception& exception) {
	{
		const std::bad_alloc* bad_alloc = reinterpret_cast<const std::bad_alloc*>(&exception);
		if(bad_alloc != nullptr) {
			reportOutOfMemory();
			return;
		}
	}

	//If we don't have a special way to handle this exception, report
	//a generic error.
	ereport(ERROR,
			(errcode(ERRCODE_EXTERNAL_ROUTINE_INVOCATION_EXCEPTION),
			 errmsg("C++ exception: %s", typeid(exception).name()),
			 errdetail("%s", exception.what())));
}

void reportParseError(const char* phone_number, PhoneNumberUtil::ErrorType err) {
	ereport(ERROR,
			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			 errmsg("unable to parse '%s' as a phone number", phone_number),
			 errdetail("%s", parseErrorMessage(err))));
}

void logInfo(const char* msg) {
	ereport(INFO,
			(errcode(ERRCODE_SUCCESSFUL_COMPLETION),
			errmsg("%s", msg)));
}
