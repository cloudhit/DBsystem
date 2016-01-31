/*
 * btfile.C - function members of class BTreeFile 
 * 
 */

#include "minirel.h"
#include "buf.h"
#include "db.h"
#include "new_error.h"
#include "btfile.h"
#include "btreefilescan.h"
#include <stack>

// Define your error message here
const char* BtreeErrorMsgs[] = {
  "keytype is not attrString nor attrInteger",
  "can't fine the record",
  "no more record"
};

static error_string_table btree_table( BTREE, BtreeErrorMsgs);
//MINIBASE_FIRST_ERROR(BTREE, BUFFERFUL);
//MINIBASE_CHAIN_ERROR(BUFMGR, st);

BTreeFile::BTreeFile (Status& returnStatus, const char *filename)
{
      this->filename = filename;
      returnStatus = MINIBASE_DB->get_file_entry(filename, root);
      if(returnStatus != OK){
        MINIBASE_CHAIN_ERROR(BUFMGR, returnStatus);
        return;
      }
      returnStatus = MINIBASE_BM->pinPage(root, (Page*&)root_page);
      if(returnStatus != OK){
        MINIBASE_CHAIN_ERROR(BUFMGR, returnStatus);
      }
}

BTreeFile::BTreeFile (Status& returnStatus, const char *filename, 
                      const AttrType keytype,
                      const int keysize)
{
      // put your code here
      this->filename = filename;
      if(keytype != attrString && keytype != attrInteger || (keytype == attrString && keysize != MAX_KEY_SIZE1)){
        returnStatus = MINIBASE_FIRST_ERROR(BTREE, TYPEERROR);
        return;
      }
      MINIBASE_BM->newPage(root, (Page*&)root_page);
      root_page->root = -1;
      root_page->key_type = keytype;
      root_page->keysize = keysize;
      returnStatus = MINIBASE_DB -> add_file_entry(filename, root);
      if(returnStatus != OK)
        MINIBASE_CHAIN_ERROR(BUFMGR, returnStatus);
}

BTreeFile::~BTreeFile ()
{
      root_page = 0;
      MINIBASE_BM->unpinPage(root, TRUE);
}

bool BTreeFile::DFS(PageId pageid){
      Page* page;
      MINIBASE_BM->pinPage(pageid, page);
      SortedPage* tmp = (SortedPage*)page;
      if(tmp -> get_type() == LEAF) {
        MINIBASE_BM->unpinPage(pageid);
        return 0;
      }
      int judge = 0;
      PageId pageno = ((BTIndexPage*)page)->getLeftLink();
      if(!DFS(pageno)) judge = 1;
      MINIBASE_BM->freePage(pageno);
      for(int i = 0; i < tmp->numberOfRecords(); i ++){
        char* data = &tmp->data[tmp->slot[i].offset];
        PageId pageno;
        memcpy(&pageno, data + root_page->keysize, sizeof(PageId));
        if(!judge){
          DFS(pageno);
        }
        MINIBASE_BM->freePage(pageno);
      }
      MINIBASE_BM->unpinPage(pageid);
      return 1;
}
Status BTreeFile::destroyFile ()
{
      DFS(root_page->root);
      MINIBASE_BM->freePage(root_page->root);
      MINIBASE_BM->freePage(root);
      MINIBASE_DB->delete_file_entry(this->filename);
      return OK;
}

char* BTreeFile::getmidkey(SortedPage* leaf, const void* key){
      int mid = (leaf->numberOfRecords() + 1) / 2;
      char* midkey = &leaf->data[leaf->slot[mid].offset];
      char* prekey = &leaf->data[leaf->slot[mid - 1].offset];
      char* result_key = new char[root_page->keysize];
      int judge1 = keyCompare(key, (const void*)midkey, root_page->key_type);
      int judge2 = keyCompare(key, (const void*)prekey, root_page->key_type);
      if(judge1 >= 0){
        memcpy(result_key, midkey, root_page->keysize);
      }else if(judge1 < 0 && judge2 >= 0){
        memcpy(result_key, key, root_page->keysize);
      }else{
        memcpy(result_key, prekey, root_page->keysize);
      }
      return result_key;
}

void BTreeFile::addRoot(SortedPage* leaf, char* result_key){
      Page* indexpage;
      PageId indexpageid;
      MINIBASE_BM->newPage(indexpageid, indexpage);
      BTIndexPage* index = (BTIndexPage*)indexpage;
      index->init(indexpageid);
      index->setLeftLink(leaf->curPage);
      RID id;
      index->insertKey((const void*)result_key, root_page->key_type, leaf->nextPage, id);
      root_page->root = indexpageid;
      MINIBASE_BM->unpinPage(indexpageid, TRUE);
}

Status BTreeFile::insert(const void *key, const RID rid) {
      if(root_page->root == -1){
        Page* page;
        PageId pageid;
        MINIBASE_BM->newPage(pageid, page);
        BTLeafPage* leafpage = (BTLeafPage*)page;
        leafpage->init(pageid);
        RID id;
        root_page->root = pageid;
        leafpage->insertRec(key, root_page->key_type, rid, id);
        MINIBASE_BM->unpinPage(pageid, TRUE);
        leafpage = 0;
      }else{
        Page* page;
        MINIBASE_BM->pinPage(root_page->root,page);
        SortedPage* tmp = (SortedPage*)page;
        if(tmp->get_type() == LEAF){
          BTLeafPage* leaf = (BTLeafPage*)tmp;
          Status st;
          char* result_key = getmidkey((SortedPage*)leaf, key);
          RID id;
          st = leaf->insertRec(key, root_page->key_type, rid, id);
          if(id.slotNo == -1){
            addRoot((SortedPage*)leaf, result_key);
          }
          delete []result_key;
          MINIBASE_BM->unpinPage(tmp->curPage, TRUE);      
        }else{
          stack<BTIndexPage*> mystack;
          while(tmp->get_type() == INDEX){
            BTIndexPage* index = (BTIndexPage*)tmp;
            mystack.push(index);
            PageId indexpageid;
            index->get_page_no(key,root_page->key_type, indexpageid);
            Page* indexpage;
            MINIBASE_BM->pinPage(indexpageid,indexpage);
            tmp = (SortedPage*)indexpage;
          }
          BTLeafPage* leaf = (BTLeafPage*)tmp;
          char* result_key = getmidkey(leaf, key);
          RID id;
          Status st = leaf->insertRec(key, root_page->key_type, rid, id);
          BTIndexPage* cur_page;
          PageId pre_page_id = leaf->nextPage;
          while(id.slotNo == -1 && !mystack.empty()){
            char* tmp_result_key = result_key;
            cur_page = mystack.top();
            mystack.pop();
            result_key = getmidkey(cur_page, tmp_result_key);
            st = cur_page->insertKey((const void*)tmp_result_key, root_page->key_type, pre_page_id, id);
            pre_page_id = cur_page->nextPage;
            MINIBASE_BM->unpinPage(cur_page->curPage, TRUE);
            delete []tmp_result_key;
          }
          if(mystack.empty() && id.slotNo == -1){
            addRoot((SortedPage*)cur_page, result_key);
          }else if(!mystack.empty()){
            while(!mystack.empty()){
              cur_page = mystack.top();
              mystack.pop();
              MINIBASE_BM->unpinPage(cur_page->curPage, TRUE);
            }
          }
          delete []result_key;
          MINIBASE_BM->unpinPage(leaf->curPage, TRUE);
        }
      }
      return OK;
}

Status BTreeFile::Delete(const void *key, const RID rid) {
      Page* page;
      MINIBASE_BM->pinPage(root_page->root, page);
      while(((SortedPage*)page)->get_type() == INDEX){
        BTIndexPage* tmp = (BTIndexPage*)page;
        PageId pageno;
        tmp->get_page_no(key, root_page->key_type,pageno);
        MINIBASE_BM->unpinPage(tmp->curPage);
        MINIBASE_BM->pinPage(pageno, page);
      }
      BTLeafPage* leaf = (BTLeafPage*)page;
      RID dataRid;
      bool found = 0;
      for(int i = 0; i < leaf->slotCnt; i ++){
        char* data = &(leaf->data[leaf->slot[i].offset]);
        int length = leaf->slot[i].length - sizeof(RID);
        int judge1 = keyCompare((void*)key, data, root_page->key_type);
        memcpy(&dataRid, data + length, sizeof(RID));
        if(judge1 == 0 && dataRid.pageNo == rid.pageNo && dataRid.slotNo == rid.slotNo){
          found = 1;
          dataRid.pageNo = leaf->curPage;
          dataRid.slotNo = i;
          leaf->SortedPage::deleteRecord(dataRid);
          break;
        }
      }
      Status st = OK;
      if(found == 0) st = MINIBASE_FIRST_ERROR(BTREE, LOSTTOFIND);
      MINIBASE_BM->unpinPage(leaf->curPage, TRUE);
      return st;
}
PageId BTreeFile::getLeafPageNo(void* key, bool high){
      Page* page;
      MINIBASE_BM->pinPage(root_page->root, page);
      PageId next = root_page->root;
      PageId pre;
      while(((SortedPage*)page)->get_type() == INDEX){
        pre = next;
        BTIndexPage* index = (BTIndexPage*)page;
        char* data;
        if(key == 0 && high == TRUE){
          data = &(index->data[index->slot[index->numberOfRecords() - 1].offset]);
          memcpy(&next, data + root_page->keysize, sizeof(PageId));
        }
        else if(key == 0 && high == FALSE){
          next = index->getLeftLink();
        }
        else
          index->get_page_no(key, root_page->key_type, next);       
        MINIBASE_BM->unpinPage(pre);
        MINIBASE_BM->pinPage(next, page);
      }
      MINIBASE_BM->unpinPage(next);
      return  next;
}    
IndexFileScan *BTreeFile::new_scan(const void *lo_key, const void *hi_key) {
  // put your code here
      // create a scan with given keys
    // Cases:
    //      (1) lo_key = NULL, hi_key = NULL
    //              scan the whole index
    //      (2) lo_key = NULL, hi_key!= NULL
    //              range scan from min to the hi_key
    //      (3) lo_key!= NULL, hi_key = NULL
    //              range scan from the lo_key to max
    //      (4) lo_key!= NULL, hi_key!= NULL, lo_key = hi_key
    //              exact match ( might not unique)
    //      (5) lo_key!= NULL, hi_key!= NULL, lo_key < hi_key
    //              range scan from lo_key to hi_key
      BTreeFileScan* scan = new BTreeFileScan();
      scan->filename = filename;
      scan->setkeysize(root_page->keysize);
      PageId low, high;
      if(lo_key == 0 && hi_key == 0){
        low = getLeafPageNo(0, FALSE);
        high = getLeafPageNo(0, TRUE);
      }else if(lo_key == 0 && hi_key != 0){
        low = getLeafPageNo(0, FALSE);
        high = getLeafPageNo((void*)hi_key);    
      }else if(lo_key != 0 && hi_key == 0){
        low = getLeafPageNo((void*)lo_key);
        high = getLeafPageNo(0, TRUE);    
      }else if(lo_key != 0 && hi_key != 0){
        if(keyCompare(lo_key, hi_key, root_page->key_type) > 0) 
          return (IndexFileScan *)scan;
        low = getLeafPageNo((void*)lo_key);
        high = getLeafPageNo((void*)hi_key);
      }
      Page* page;
      BTLeafPage* leaf;
      while(1){
        MINIBASE_BM->pinPage(low, page);
        leaf = (BTLeafPage*)page;
        for(int i = 0; i < leaf->numberOfRecords(); i ++){
          char* data = &(leaf->data[leaf->slot[i].offset]);
          if(lo_key != 0){
            if(keyCompare(lo_key, data, root_page->key_type) > 0)
              continue;  
          }
          if(hi_key != 0){
            if(keyCompare(hi_key, data, root_page->key_type) < 0)
              continue;   
          }        
          char* key = new char[root_page->keysize];
          memcpy(key, data, root_page->keysize);
          RID rid;

          memcpy(&rid, data + root_page->keysize, sizeof(RID));
          if(scan->head == 0){
            scan->head = new metadata(rid, key);
            scan->tail = scan->head;

          }else{
            scan->tail->next = new metadata(rid, key);
            scan->tail = scan->tail->next;
          }
        }
        MINIBASE_BM->unpinPage(low);
        if(low == high) break;
        low = leaf->nextPage;
      }
      Page* xpage;
      MINIBASE_BM->pinPage(root_page->root, xpage);
      BTIndexPage* index = (BTIndexPage*)xpage;

      metadata* head = scan->head;

      return (IndexFileScan *)scan;
  }
