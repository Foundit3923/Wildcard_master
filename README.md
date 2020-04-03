# Wildcard_master
The main files needed are tests.c, KMP_arbitrary_length_wildcard.h, and the benchmarking folder.
tests.c is the executable file, it contains the main function. 

Within the main funciton are calls to an expect funciton. These calls are the tests.
the structure of the expect functions is text, query, expected result, message

The expect functions exist in a for loop, the first past tests Wildfilter, the second pass tests whatever algorithm we are testing against.

The expect function proper executes the preprocessing, and both of the algorithms a set number of times while recording the result,
as well as the time taken. It also prints out the result of the test.
The either algorithm may be replaced with any other algorithm that returns a boolean result.

