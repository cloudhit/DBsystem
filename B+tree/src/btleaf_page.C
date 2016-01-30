/*
 * btleaf_page.cc - implementation of class BTLeafPage
 *
 */

#include "btleaf_page.h"

#include <string.h>
const char* BTLeafErrorMsgs[] = {
 
};
static error_string_table btree_table(BTLEAFPAGE, BTLeafErrorMsgs);



Status BTLeafPage::insertRec(const void *key, AttrType key_type, RID dataRid, RID& rid)
{
  Status st = OK;
  rid.slotNo = -2;
  int length = (key_type == attrInteger)? 4:MAX_KEY_SIZE1; 
  int record_size = length + sizeof(RID);
  char recPtr[record_size];
  memcpy(recPtr, key, length);
  memcpy(recPtr + length, &dataRid, sizeof(RID));
  //int testkey;
  //memcpy(&testkey, recPtr, length);
  //cout<<"testkey"<<testkey<<endl;
  int space_need = record_size + sizeof(slot_t);
  bool found = 0;
  for(int i = 0; i < slotCnt; i ++){
    char* xdata = &(data[slot[i].offset]);
    int judge1 = keyCompare((void*)key, xdata, key_type);
    if(judge1 == 0){
      found = 1;
      break;
    }
  }
  if(found == 1) return st;


  if(space_need <= SortedPage::free_space()){
    st = SortedPage::insertRecord(key_type,recPtr, record_size, rid);
  }
  else{ 
    PageId pageid;
    Page* page;
    MINIBASE_BM->newPage(pageid, page);
    BTLeafPage* next = (BTLeafPage*)page;
    next->init(pageid);
    int tmp = nextPage;
    next->prevPage = curPage;
    next->nextPage = tmp;
    nextPage = pageid;
    if(tmp != -1){
      Page* tmp_page;
      Status st1 = MINIBASE_BM->pinPage(tmp, tmp_page);
      if(st1 == OK){
        ((BTLeafPage*)tmp_page)->prevPage = pageid;
        MINIBASE_BM->unpinPage(tmp, TRUE);
      }
    }
    int start;
    if(keyCompare((void*)(data + slot[(1+slotCnt)/2].offset), key, key_type) <= 0)
      start = (1 + slotCnt) / 2;
    else
      start = keyCompare((void*)(data + slot[(slotCnt - 1)/2].offset), key, key_type) <= 0? (1 + slotCnt) / 2:(slotCnt - 1) /2;
    int count = slotCnt;
    for(int i = count - 1; i >= start; i --){
      RID x;
      next->SortedPage::insertRecord (key_type, (data + slot[i].offset), slot[i].length, x);
      x.pageNo = curPage;
      x.slotNo = i;
      SortedPage::deleteRecord(x);
    }
    if(start == (1 + count) / 2){
      next->SortedPage::insertRecord(key_type, recPtr, record_size, rid);
    }else SortedPage::insertRecord(key_type, recPtr, record_size, rid);

    //st = MINIBASE_FIRST_ERROR(BTLEAFPAGE, NEWPAGE);
    rid.slotNo = -1;
    MINIBASE_BM->unpinPage(pageid, TRUE);
  }
  //delete recPtr;
  //MINIBASE_BM->unpin(curPage, TRUE);
  return st;
}

Status BTLeafPage::get_data_rid(void *key,
                                AttrType key_type,
                                RID & dataRid)
{
  int length = (key_type == attrInteger)? 4:MAX_KEY_SIZE1; 
  int l = 0, r = SortedPage::numberOfRecords() - 1, found = 0;
  while(l <= r){
    int mid = (l + r) / 2;
    char* comparekey = data + slot[mid].offset;
    int judge = keyCompare(key, (void*)comparekey, key_type);
    if(judge == 0){
      memcpy(&dataRid, comparekey + length, slot[mid].length - length);
      found = 1;
      break;
    }else if(judge > 0) l = mid + 1;
    else r = mid - 1;
  }
  if(found == 0)
  return MINIBASE_FIRST_ERROR(BTLEAFPAGE, FAILEDTOFIND);
  return OK;
}
Status BTLeafPage::get_first (RID& rid,
                              void *key,
                              RID & dataRid)
{ 
  Status st;
  st = HFPage::firstRecord(rid);
  if(st != OK) return MINIBASE_FIRST_ERROR(BTLEAFPAGE, NOMORERECS);
  char* context = data + slot[rid.slotNo].offset;
  int length = slot[rid.slotNo].length - sizeof(dataRid);
  memcpy(key, context, length);
  memcpy(&dataRid, context + length, sizeof(dataRid));
  return OK;
}

Status BTLeafPage::get_next (RID& rid,
                             void *key,
                             RID & dataRid)
{
  Status st;
  RID nextRid;
  st = HFPage::nextRecord(rid, nextRid);
  if(st != OK) return MINIBASE_FIRST_ERROR(BTLEAFPAGE, NOMORERECS);
  char* context = data + slot[rid.slotNo].offset;
  int length = slot[rid.slotNo].length - sizeof(dataRid);
  memcpy(key, context, length);
  memcpy(&dataRid, context + length, sizeof(dataRid));
  rid = nextRid;
  return OK;
}
