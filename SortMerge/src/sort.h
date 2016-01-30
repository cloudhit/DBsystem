#ifndef __SORT__
#define __SORT__

#include "minirel.h"
#include "new_error.h"
#include "scan.h"

#define    PAGESIZE    MINIBASE_PAGESIZE

class Sort
{
 public:

   Sort(char*    inFile,        // Name of unsorted heapfile.

    char*        outFile,    // Name of sorted heapfile.

    int          len_in,        // Number of fields in input records.

    AttrType     in[],        // Array containing field types of input records.
                    // i.e. index of in[] ranges from 0 to (len_in - 1)

    short        str_sizes[],  // Array containing field sizes of input records.

    int          fld_no,     // The number of the field to sort on.
                            // fld_no ranges from 0 to (len_in - 1).

    TupleOrder   sort_order,   // ASCENDING, DESCENDING

    int          amt_of_buf,   // Number of buffer pages available for sorting.

    Status&     s
       );

    ~Sort(){}
 private: 
    Status _pass_one(int& numtempfile);
    Status _merge_many_to_one(unsigned int numtempfile, 
        HeapFile **source, HeapFile* dest);
    Status _one_later_pass(int numberTempFiles, int passNum, int &numDest);
    Status _merge(int numFiles);
    
    char* _temp_name(int pass, int run, char* out_file);

    int _rec_length;
    int _amt_of_buf;
    char* _in_file;
    char* _out_file;
    int _fld_no;
    TupleOrder     _sort_order;
    short* _str_sizes;

};


#endif
