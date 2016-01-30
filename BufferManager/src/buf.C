/*****************************************************************************/
/*************** Implementation of the Buffer Manager Layer ******************/
/*****************************************************************************/


#include "buf.h"

// Define buffer manager error messages here
//enum bufErrCodes  {...};

// Define error message here
static const char* bufErrMsgs[] = { 
  "failed to find page in hashtable",
  "the buffer pool is full and can't find the replacement",
  "pin_count is 0 when unpin the page",
  "page can't be deallocated since it's pin_count is not 0",
  "no space in buffer pool for first page"
  // error message strings go here
};

// Create a static "error_string_table" object and register the error messages
// with minibase system 
static error_string_table bufTable(BUFMGR,bufErrMsgs);
Status st;

ML::ML(int size){
  ML_list = new node[size];
  for(int i = 0; i < size; i ++){
      free_pages.push_back(i);
      ML_list[i].index = i;
    }
    head = new node();
    tail = new node();
    head->next = tail;
    tail->pre = head;
}
void ML::insertToHead(int frame_id, bool hate){
  node* p = &ML_list[frame_id];
  if(hate){
    node* tmp = head->next;
    head->next = p;
    p->pre = head;
    p->next = tmp;
    tmp->pre = p;
  }else{
    node* tmp = tail->pre;
    tail->pre = p;
    p->next = tail;
    p->pre = tmp;
    tmp->next = p;
  }
}

void ML::deleteNode(int frame_id){
  node* p = &ML_list[frame_id];
  p->pre->next = p->next;
  p->next->pre = p->pre;
  p->pre = 0;
  p->next = 0;
}
void ML::addToFree(int frame_id){
  free_pages.push_back(frame_id);
}
void ML::remove(int frame_id){
  loveset.erase(frame_id);
  if(ML_list[frame_id].next != 0)
    ML::deleteNode(frame_id);
  ML::addToFree(frame_id);
}

int ML::allocateFrame(){
  if(free_pages.size() == 0) return -1;
  int x = free_pages[0];
  free_pages.erase(free_pages.begin());
  return x;
}

int ML::findReplacement(){
  node* p = head->next;
  if(p == tail)return -1;
  return p->index;
}
void ML::delReplacement(int frame_id){
  loveset.erase(frame_id);
  ML::deleteNode(frame_id);
}
void ML::addToMR(int frame_id, bool hate){
  if(hate){
    if(loveset.find(frame_id) != loveset.end())
      return;
    ML::insertToHead(frame_id, hate);
  }else{
    loveset.insert(frame_id);
    ML::insertToHead(frame_id, hate);
  }
}
ML::~ML(){
  delete []ML_list;
  ML_list = NULL;
  delete head;
  head = NULL;
  delete tail;
  tail = NULL;
  free_pages.clear();
  loveset.clear();
}

idpair::~idpair(){

}

head::~head(){

}
hashT::hashT(int size){
  keyArray = new head[size];
  a = 2, b =3;
}

hashT::~hashT(){
  //cout << "delete"<<endl;
  for(int i = 0; i < HTSIZE; i ++){
    idpair* p = keyArray[i].list;
    while(p != 0){
      idpair* tmp = p;
      p = p->next;
      delete tmp;
      tmp = NULL;
    }
  }

  delete []keyArray;
  keyArray = NULL;
  //cout << "delete"<<endl;
}
int hashT::search(PageId id){
  int h = (a * id + b) % HTSIZE;
  idpair* p = keyArray[h].list;
  while(p != 0){
    if(p -> id == id) return p -> frame_id;
    p = p -> next;
  }
  return -1;
}
void hashT::add(PageId id, int frame_id){
  int h = (a * id + b) % HTSIZE;
  idpair* tmp = keyArray[h].list;
  keyArray[h].list = new idpair(id, frame_id);
  keyArray[h].list->next = tmp;
}

void hashT::del(PageId id){
  int h = (a * id + b) % HTSIZE;
  idpair* p = keyArray[h].list;
  if(p ->id == id){
    keyArray[h].list = p->next;
    delete p;
    p = NULL;

  }else{
    while(p->next->id != id)
      p = p ->next;
    idpair* tmp = p->next;
    p->next = p->next->next;
    delete tmp;
    tmp = NULL;
  }
}


BufMgr::BufMgr(int numbuf, Replacer *replacer) {
  // put your code here
  bufPool = new Page[numbuf];
  ht = new hashT(HTSIZE);
  desc = new descriptor[numbuf];
  for(int i = 0; i < numbuf; i ++){
    desc[i].pageid = -1;
  }
  ml = new ML(numbuf);
}


Status BufMgr::pinPage(PageId PageId_in_a_DB, Page*& page, int emptyPage) {
  // put your code here
  int fid = ht->search(PageId_in_a_DB);
  if(fid != -1){
    if(desc[fid].pin_count == 0){
      ml->deleteNode(fid);
    }
    desc[fid].pin_count ++;
    page = &bufPool[fid];
  }else{
    int x = ml->allocateFrame();
    if(x != -1){
      page = &bufPool[x];
      st = MINIBASE_DB->read_page(PageId_in_a_DB, page);
      if(st != OK) {
        ml->addToFree(x);
        return MINIBASE_CHAIN_ERROR(BUFMGR, st);
      }
      ht->add(PageId_in_a_DB, x);
      desc[x].pageid = PageId_in_a_DB;
      desc[x].pin_count = 1;
      desc[x].dirtybit = false;
    }else{
      x = ml->findReplacement();
      if(x == -1) return MINIBASE_FIRST_ERROR(BUFMGR, BUFFERFUL);
      page = &bufPool[x];
      if(desc[x].dirtybit){
        st = BufMgr::flushPage(desc[x].pageid);
        MINIBASE_CHAIN_ERROR(BUFMGR, st);
      }
      st = MINIBASE_DB->read_page(PageId_in_a_DB, page);
      if(st != OK) return MINIBASE_CHAIN_ERROR(BUFMGR, st);
      ml->delReplacement(x);
      desc[x].pin_count = 1;
      desc[x].dirtybit = false;
      ht->del(desc[x].pageid);
      desc[x].pageid = PageId_in_a_DB;
      ht->add(PageId_in_a_DB, x);
    }
  }
  return OK;
}//end pinPage


Status BufMgr::newPage(PageId& firstPageId, Page*& firstpage, int howmany) {
  // put your code here
  st = MINIBASE_DB->allocate_page(firstPageId, howmany);
  if(st!=OK) return MINIBASE_CHAIN_ERROR(BUFMGR, st);
  st = BufMgr::pinPage(firstPageId, firstpage);
  if(st!= OK){
    Status st1 = MINIBASE_DB->deallocate_page(firstPageId, howmany);
    if(st1 != OK) MINIBASE_CHAIN_ERROR(BUFMGR, st1);
    return MINIBASE_CHAIN_ERROR(BUFMGR, st); 
  }
  return OK;
}

Status BufMgr::flushPage(PageId pageid) {
  // put your code here
  int x = ht->search(pageid);
  if(x == -1) return MINIBASE_FIRST_ERROR(BUFMGR,HTFOUNDFAIL);
  if(desc[x].dirtybit){
    st = MINIBASE_DB-> write_page(pageid, &bufPool[x]);
    if(st != OK) return MINIBASE_CHAIN_ERROR(BUFMGR, st);
  }
  return OK;
}
    
	  
//*************************************************************
//** This is the implementation of ~BufMgr
//************************************************************
BufMgr::~BufMgr(){
  // put your code here
  BufMgr::flushAllPages();
  delete []bufPool;
  bufPool = NULL;
  delete ht;
  ht = NULL;
  delete []desc;
  desc = NULL;
  delete ml;
  ml = NULL;
}


//*************************************************************
//** This is the implementation of unpinPage
//************************************************************

Status BufMgr::unpinPage(PageId page_num, int dirty=FALSE, int hate = FALSE){
  // put your code here
  int x = ht->search(page_num);
  if(x == -1)return MINIBASE_FIRST_ERROR(BUFMGR, HTFOUNDFAIL);
  if(desc[x].pin_count == 0) return MINIBASE_FIRST_ERROR(BUFMGR, UNPINERROR);
  desc[x].dirtybit = dirty;
  desc[x].pin_count --;
  if(desc[x].pin_count == 0){
    ml->addToMR(x, hate);
  }
  return OK;
}

//*************************************************************
//** This is the implementation of freePage
//************************************************************

Status BufMgr::freePage(PageId globalPageId){
  // put your code here
  int x = ht->search(globalPageId);
  if(x != -1 && desc[x].pin_count > 0){
    return MINIBASE_FIRST_ERROR(BUFMGR, PAGEFREEFAIL);
  }
  st = MINIBASE_DB -> deallocate_page(globalPageId);
  if(st!= OK) return MINIBASE_CHAIN_ERROR(BUFMGR, st);

  return OK;
}

Status BufMgr::flushAllPages(){
  //put your code here
  for(int i = 0; i < NUMBUF; i ++){
    if(desc[i].pageid != -1){
      st = BufMgr::flushPage(desc[i].pageid);
      if(st != OK) MINIBASE_CHAIN_ERROR(BUFMGR, st);
    }
  }
  return st;
}
