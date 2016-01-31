/*
 * btreefilescan.cc - function members of class BTreeFileScan
 *
 */

#include "minirel.h"
#include "buf.h"
#include "db.h"
#include "new_error.h"
#include "btfile.h"
#include "btreefilescan.h"

/*
 * Note: BTreeFileScan uses the same errors as BTREE since its code basically 
 * BTREE things (traversing trees).
 */

BTreeFileScan::~BTreeFileScan ()
{
  if(head != 0){
  	while(head != 0){
  		metadata* tmp = head;
  		head = head->next;
  		delete tmp;
  		tmp = 0;
  	}
  }
}

int BTreeFileScan::keysize() 
{
  return key_size;
}

Status BTreeFileScan::get_next (RID & rid, void* keyptr)
{
	if(cur == 0){
		cur = head;
	}else cur = cur -> next;
	if(cur == 0) return DONE;
	rid = cur->rid;
	memcpy(keyptr, cur->key, key_size);
	return OK;
}
void BTreeFileScan::setkeysize(int keysize){
	key_size = keysize;
}
Status BTreeFileScan::delete_current ()
{
  if(cur == 0)return DONE;
  Status st = OK;
  BTreeFile* btf = new BTreeFile(st, filename);
  btf->Delete((const void*)cur->key, cur->rid);
  delete btf; 
  return st; 
}
