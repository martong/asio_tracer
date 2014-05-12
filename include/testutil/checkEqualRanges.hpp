#ifndef CPP_UTILITY_INCLUDE_TESTUTIL_CHECKEQUALRANGES_HPP
#define CPP_UTILITY_INCLUDE_TESTUTIL_CHECKEQUALRANGES_HPP

#define TESTUTIL_CHECK_EQUAL_RANGES(a, b) \
	BOOST_CHECK_EQUAL_COLLECTIONS( \
			std::begin((a)), std::end((a)), \
			std::begin((b)), std::end((b)))

#endif /* CPP_UTILITY_INCLUDE_TESTUTIL_CHECKEQUALRANGES_HPP */
