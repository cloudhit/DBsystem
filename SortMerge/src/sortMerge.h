#ifndef __SORT_MERGE_  
#define __SORT_MERGE_

#include "minirel.h"
#include "sort.h"
#include "scan.h"
#include "heapfile.h"
#include "new_error.h"
#include "system_defs.h"

extern global_errors* minirel_error;
extern SystemDefs* minibase_globals;
/*extern int tupleCmp(const int len_in1, const AttrType** in1, const int* t1_str_sizes, const int join_col_in1, const in len_in2, const AttrType** in2, const in* t2_str_sizes, const int join_col_in2, TupleOrder order, const char* RecordR, const char* RecordS);
*/

extern int tupleCmp(const void* t1, const void* t2);
class sortMerge 
{

 public:
   sortMerge(
	char*		filename1,	// Name of heapfile for relation R
	int     	len_in1,	// # of columns in R.
	AttrType    	in1[],		// Array containing field types of R.
	short   	t1_str_sizes[], // Array containing size of columns in R
	int     	join_col_in1,	// The join column of R 

	char*		filename2,	// Name of heapfile for relation S
	int     	len_in2,	// # of columns in S.
	AttrType    	in2[],		// Array containing field types of S.
	short   	t2_str_sizes[], // Array containing size of columns in S
	int     	join_col_in2,	// The join column of S

	char*		filename3,	// Name of heapfile for merged results
	int     	amt_of_mem,	// Number of pages available
	TupleOrder 	order,		// Sorting order: Ascending or Descending
	Status& 	s		// Status of constructor
   );

~sortMerge();

private:
	// Error handling protocal:

	enum ErrCodes 	{
			SORT_FAILED,
			HEAPFILE_FAILED
			};
};

#endif

