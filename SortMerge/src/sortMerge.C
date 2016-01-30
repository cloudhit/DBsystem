
#include <string.h>
#include <assert.h>
#include "sortMerge.h"
#include <cstdlib>

// Error Protocall:

enum ErrCodes {};

static const char* ErrMsgs[] = 	{};

static error_string_table ErrTable( JOINS, ErrMsgs );
sortMerge::sortMerge(
    char*           filename1,      // Name of heapfile for relation R
    int             len_in1,        // # of columns in R.
    AttrType        in1[],          // Array containing field types of R.
    short           t1_str_sizes[], // Array containing size of columns in R
    int             join_col_in1,   // The join column of R 

    char*           filename2,      // Name of heapfile for relation S
    int             len_in2,        // # of columns in S.
    AttrType        in2[],          // Array containing field types of S.
    short           t2_str_sizes[], // Array containing size of columns in S
    int             join_col_in2,   // The join column of S

    char*           filename3,      // Name of heapfile for merged results
    int             amt_of_mem,     // Number of pages available
    TupleOrder      order,          // Sorting order: Ascending or Descending
    Status&         s               // Status of constructor
){
    char* outFile1 = "outFile1";
    char* outFile2 = "outFile2";
    Sort(filename1, outFile1, len_in1, in1, t1_str_sizes, join_col_in1,order,amt_of_mem,s);
    if(s != OK) return;
    Sort(filename2, outFile2, len_in2, in2, t2_str_sizes, join_col_in2,order,amt_of_mem,s);
    if(s != OK) return;
    HeapFile* hp1 = new  HeapFile((const char*)outFile1, s);
    if(s != OK) return;
    HeapFile* hp2 = new  HeapFile((const char*)outFile2, s); 
    if(s != OK) return;
    Scan*  scan1 = hp1->openScan(s);
    if(s != OK) return;
    Scan*  scan2 = hp2->openScan(s);
    if(s != OK) return;
    HeapFile* out = new  HeapFile((const char*)filename3, s);
    RID rid1, rid2, first, second;
    int recLen1 = 0, recLen2 = 0, offset1 = 0, offset2 = 0, judge = 0, cmp;
    for(int i = 0; i < len_in1; i ++) {
        if(i < join_col_in1)
            offset1 += t1_str_sizes[i];
        recLen1 += t1_str_sizes[i];
    }
    for(int i = 0; i < len_in2; i ++){
        if(i < join_col_in2)
            offset2 += t2_str_sizes[i];
        recLen2 += t2_str_sizes[i];
    }
    char* pre_record = 0;
    char* recPtr1 = (char*)malloc(recLen1 * sizeof(char));
    char* recPtr2 = (char*)malloc(recLen2 * sizeof(char));
    char* recPtr = (char*)malloc((recLen1 + recLen2) * sizeof(char));
    while(1){
        s = scan1->getNext(rid1, recPtr1, recLen1);
        if(s != DONE){
            if(pre_record == 0){
                pre_record = (char*)malloc(recLen1 * sizeof(char));
                memcpy(pre_record, recPtr1, recLen1);
            }else{
                cmp = tupleCmp((const void*)(pre_record + offset1), (const void*)(recPtr1 + offset1));
                if(cmp == 0 && judge == 1){
                    scan2->position(first);
                }else if(cmp == 0) continue;
                else scan2->position(second);
                memcpy(pre_record,recPtr1,recLen1);

                char test[recLen1];
                memcpy(test, recPtr1, recLen1);
                cout<<test<<endl;
            }
            Status s2;
            judge = 0;
            while((s2 = scan2 -> getNext(rid2, recPtr2, recLen2)) != DONE){
                int res = tupleCmp((const void*)(recPtr1 + offset1), (const void*)(recPtr2 + offset2));
                if(res == 0){
                    if(!judge){
                        first = rid2;
                        judge = 1;
                    }
                    RID outRid;
                    memcpy(recPtr, recPtr1, recLen1);
                    memcpy(recPtr + recLen2, recPtr2, recLen2);
                    out->insertRecord(recPtr, recLen1 + recLen2, outRid); 
                }else if(res > 0 && order == Ascending || res < 0 && order == Descending) 
                    continue;
                else{
                    second = rid2; 
                    break;
                }
                
            }
        if(s2 == DONE && judge == 0) break;

        }else break;
    }
    hp1->deleteFile();
    hp2->deleteFile();
    delete hp1;
    delete hp2;
    delete scan1;
    delete scan2;
    delete out;
}

sortMerge::~sortMerge()
{

}
