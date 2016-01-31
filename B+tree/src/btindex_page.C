/*
 * btindex_page.cc - implementation of class BTIndexPage
 *
 */

#include "btindex_page.h"

// Define your Error Messge here
const char* BTIndexErrorMsgs[] = {
  "newpage", 
   "can't find record"
};
static error_string_table btree_table(BTINDEXPAGE, BTIndexErrorMsgs);

Status BTIndexPage::insertKey (const void *key,
                               AttrType key_type,
                               PageId pageNo,
                               RID& rid)
{
      Status st = OK;
      rid.slotNo = -2;
      int length = (key_type == attrInteger)? 4:MAX_KEY_SIZE1; 
      int record_size = length + sizeof(PageId);
      char recPtr[record_size];
      memcpy(recPtr, key, length);
      memcpy(recPtr + length, &pageNo, sizeof(PageId));
      int space_need = record_size + sizeof(slot_t);
      if(space_need <= SortedPage::free_space())
        st = SortedPage::insertRecord(key_type, recPtr, record_size, rid);
      else{ 
        Page* page;
        PageId pageid;
        MINIBASE_BM->newPage(pageid, page);
        BTIndexPage* next = (BTIndexPage*)page;
        next->init(pageid);
        int tmp = nextPage;
        nextPage = pageid;
        next->prevPage = curPage;
        next->nextPage = tmp;
        if(tmp != -1){
          Page* tmp_page;
          Status st1 = MINIBASE_BM->pinPage(tmp, tmp_page);
          if(st1 == OK){
            ((BTIndexPage*)page)->prevPage = pageid;
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

        rid.pageNo = next->curPage, rid.slotNo = 0;
        PageId splitkey;
        memcpy(&splitkey, &(next->data[next->slot[0].offset + length]), sizeof(PageId)); 
        next->SortedPage::deleteRecord(rid);
        next->setLeftLink(splitkey);
        rid.slotNo = -1;
        MINIBASE_BM->unpinPage(pageid, TRUE);
      }
      return st;
}

Status BTIndexPage::deleteKey (const void *key, AttrType key_type, RID& curRid)
{
  return OK;
}

Status BTIndexPage::get_page_no(const void *key,
                                AttrType key_type,
                                PageId & pageNo)
{
      int l = 0, r = SortedPage::numberOfRecords() - 1;
      int length = (key_type == attrInteger)? 4:MAX_KEY_SIZE1;   
      int found = 0;
      for(int i = 0; i < slotCnt; i ++){
        char* comparekey = data + slot[i].offset;
        int judge = keyCompare(key, (void*)comparekey, key_type);
        if(judge < 0){
          if(i == 0) pageNo = getLeftLink();
          else{
            char* prekey = data + slot[i - 1].offset;
            memcpy(&pageNo, prekey + length, sizeof(PageId));
          }
          found = 1;
          break;
        }
      }
      if(found == 0){
            char* prekey = data + slot[slotCnt - 1].offset;
            memcpy(&pageNo, prekey + length, sizeof(PageId));
      }
      return OK;
}

    
Status BTIndexPage::get_first(RID& rid,
                              void *key,
                              PageId & pageNo)
{
      Status st ;
      st = HFPage::firstRecord(rid);
      if(st != OK) return MINIBASE_FIRST_ERROR(BTINDEXPAGE, NOMORERECS);
      char* context = data + slot[rid.slotNo].offset;
      int length = slot[rid.slotNo].length - sizeof(PageId);
      memcpy(key, context, length);
      memcpy(&pageNo, context + length, sizeof(PageId));
      return OK;
}

Status BTIndexPage::get_next(RID& rid, void *key, PageId & pageNo)
{
      Status st;
      RID nextRid;
      st = HFPage::nextRecord (rid, nextRid);
      if(st != OK) return MINIBASE_FIRST_ERROR(BTINDEXPAGE, NOMORERECS);
      char* context = data + slot[nextRid.slotNo].offset;
      int length = slot[rid.slotNo].length - sizeof(PageId);
      memcpy(key, context, length);
      memcpy(&pageNo, context + length, sizeof(PageId));
      rid = nextRid;
      return OK;
}
