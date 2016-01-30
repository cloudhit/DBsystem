#ifndef _LRU_
#define _LRU_

#include "buf.h"
#include "new_error.h"
#include "db.h"
#include "page.h"

class LRU : public Replacer {

  public:

    LRU();
   ~LRU();
  
    int pin(int frameNo);
    int pick_victim();
  
    const char* name() { return "LRU"; }
    void info();
  
  private:
    unsigned* frames;
    unsigned nframes;

    void update(int frameNo);
    void setBufferManager( BufMgr *mgr );
};



#endif

