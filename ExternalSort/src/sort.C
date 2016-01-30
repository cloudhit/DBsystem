// file sort.C  --  implementation of Sort class.

#include "sort.h"
#include "heapfile.h"
#include "system_defs.h"
#include <string.h>
// Add you error message here
// static const char *sortErrMsgs[] {
// };
// static error_string_table sortTable(SORT, sortErrMsgs);

// constructor.
int Sort::tupleCmp (const void* t1, const void* t2 )
{
  int diff = strncmp((char*)t1 + offset,(char*)t2 + offset, key_len);

  if ( sortOrder == Ascending )
  {
    if ( diff > 0 ) return 1; 
    if ( diff < 0 ) return -1;
    if ( diff == 0 ) return 0;
  }
  
  else
  {
    if ( diff < 0 ) return 1;
    if ( diff > 0 ) return -1;
    if ( diff == 0 ) return 0;
  }
}
unsigned short Sort::offset  = 1;
unsigned short Sort::key_len = 1;
TupleOrder Sort::sortOrder = Ascending;
Sort::Sort ( char* inFile, char* outFile, int len_in, AttrType in[],
	     short str_sizes[], int fld_no, TupleOrder sort_order,
	     int amt_of_buf, Status& s )
{
  s = OK;
  offset = 0, rec_len = 0;
	for(int i = 0; i < len_in; i ++){
    if(i < fld_no)
      offset += str_sizes[i];
    rec_len += str_sizes[i];
  }
  key_len = str_sizes[fld_no];
  sortOrder = sort_order;
  int midnum, passNum = 1;
  outFileName = outFile;
  record = (char*)malloc(amt_of_buf * rec_len);
  firstPass(inFile, amt_of_buf, midnum);
  while(midnum != 1){
    int newnum;
    followingPass(++passNum, midnum, amt_of_buf, newnum);
    midnum = newnum;
  }
}

void Sort::makeHFname( char *name, int passNum, int HFnum ){

	sprintf(name, "s%d_%d", passNum, HFnum);

}
Status Sort::firstPass( char *inFile, int bufferNum, int& tempHFnum ){
	Status st;
	HeapFile* hf = new HeapFile(inFile, st);
	int buffersize = bufferNum * PAGESIZE, recLen, total_num = 0;
	Scan* scan = hf->openScan(st);
	tempHFnum = 0;
	while(1){
		char* buffer = (char*)malloc(buffersize);
		int cur = 0, cnt = 0;
    RID rid;
		while(cur + rec_len <= buffersize){
			st = scan->getNext(rid, buffer + cur, recLen);
			if(st != OK) break;
			cur += rec_len, cnt ++, total_num ++;
		}
    if(cur == 0 && tempHFnum == 0){
      tempHFnum ++;
      char* name = (char*)malloc(NAMELEN);
      sprintf(name, "%s", outFileName);
      Status tmpst;
      HeapFile* tmp = new HeapFile(name, tmpst);
      delete tmp;
      tmp = 0;
    }
		else if(cur != 0){
			tempHFnum ++;
			qsort((void*)buffer, cnt, rec_len, tupleCmp);
			char* name = (char*)malloc(NAMELEN);
      if(tempHFnum == 1 && total_num >= hf->getRecCnt()){
      sprintf(name, "%s", outFileName);
    }
    else
      makeHFname(name, 1, tempHFnum);
    Status tmpst;
    char test[NAMELEN];
    HeapFile* tmp = new HeapFile(name, tmpst);
    int cur_cnt = 0;
    while(cur_cnt < cnt){
      RID outRid;
      tmp->insertRecord(buffer + cur_cnt * rec_len, rec_len, outRid);
      cur_cnt ++;               
    }
    tmp = NULL;
  }
    if(st != OK) break;
  }
  delete scan;
  scan = NULL;
  //hf->deleteFile();
  delete hf;
  hf = NULL;
  return OK;

}


Status Sort::followingPass( int passNum, int oldHFnum, 
					int bufferNum, int& newHFnum ){
  int run_num = (int)ceil(oldHFnum / (bufferNum - 1)), cur = 1;   
  for(int i = 0; i < run_num; i ++){
      Scan* scan[bufferNum - 1];
      HeapFile* hf[bufferNum - 1];
      int runNum = 0;
      for(int j = 0; j < bufferNum - 1; j ++){
          if(cur <= oldHFnum){
              runNum ++;
              char* name = (char*)malloc(NAMELEN);
              makeHFname(name, passNum - 1, cur++);
              Status st;
              hf[j] = new HeapFile(name, st);
              scan[j] = hf[j]->openScan(st);
              //free(name);
          }else break;
      }
      char* out_name = (char*)malloc(NAMELEN);
      if(run_num == 1)
        sprintf(out_name, "%s", outFileName);
      else
        makeHFname(out_name, passNum, i + 1);
      Status st;
      HeapFile* outHF = new HeapFile(out_name, st);
      merge(scan, runNum, outHF);
      for(int j =0; j < runNum; j ++){
        hf[j]->deleteFile();
        delete hf[j];
        hf[j] = NULL;
      }
  }
  newHFnum = run_num;
  return OK;
}

  // merge.
Status Sort::merge( Scan* scan[], int runNum, HeapFile* outHF ){
  int runFlag = 0;
  int run_num = runNum, runId;
  for(int i = 0; i < run_num; i ++)
    runFlag |= (1 << i);
  int recLen;
  for(int i = 0; i < runNum; i ++){  
    Status st;
    RID rid;
    st = scan[i]->getNext(rid, record + i * rec_len, (int&)rec_len);
  }
  while(run_num > 0){
    popup(record, &runFlag, run_num, runId);
    RID rid;
    outHF->insertRecord(record + runId * rec_len, rec_len, rid);
    Status st;
    st = scan[runId]->getNext(rid, record + runId * rec_len, (int&)rec_len);
    if(st == DONE){
      run_num --;
      runFlag &= ~(0x01<<runId);
    }
  }
  for(int i = 0; i < runNum; i ++){     
    delete scan[i];
    scan[i] = 0;
  }   
      outHF = 0;
      return OK;

  }

  // find the "smallest" record from runs.
  /*Status Sort::popup( char* record, int *runFlag, int runNum, int& runId ){

  }*/
  Status Sort::popup( char* record, int *runFlag, int runNum, int& runId ){
    int cnt = 0, cur = 0;
    char* min_record = 0;
    while(cnt < runNum){
      if((*runFlag >> cur) & 0x01){
        cnt ++;
        if(0 == min_record){
          min_record = record + cur * rec_len;
          cur++;
          runId = cur - 1;
          continue;
        }
        int diff = strncmp(min_record + offset, record + cur * rec_len + offset, key_len);
        if(sortOrder == Ascending && diff > 0 || sortOrder == Descending && diff < 0){
          min_record = record + cur * rec_len;
          runId = cur;
        }
      }
      cur ++;
    }
    return OK;
  }