#ifndef _MRU_
#define _MRU_

#include "buf.h"
#include "new_error.h"
#include "db.h"
#include "page.h"

class MRU : public Replacer {

  public:

    MRU();
   ~MRU();
  
    int pin(int frameNo);
    int pick_victim();
  
    const char* name() { return "MRU"; }
    void info();
  
  
  private:
    int* frames;

    void update(int frameNo);
    void setBufferManager( BufMgr *mgr );
};


#endif

