#include "RequestId.h"

    
void Meq::maskSubId (RequestId &id,int mask)
{
  // null mask: clear everything
  if( !mask )
  {
    id = AtomicID(0);
    return;
  }
  // start from end 
  HIID::reverse_iterator iter = id.rbegin();
  // ... until we run out of bits, or get to the start of BOTH ids
  for( int m1=1; 
       m1 < (1<<RQIDM_NBITS) && iter != id.rend(); 
       m1<<=1 )
  {
    if( !(mask&m1) )
      *iter = 0;
  }
}

void Meq::incrSubId (RequestId &id,int mask)
{
  // null mask: do nothing
  if( !mask )
    return;
  // start from end 
  HIID::reverse_iterator iter = id.rbegin();
  // ... until we run out of bits, or get to the start of BOTH ids
  for( int m1=1; 
       m1 < (1<<RQIDM_NBITS) && iter != id.rend(); 
       m1<<=1 )
  {
    if( mask&m1 )
      *iter = (*iter).id()+1;
  }
}
    
bool Meq::maskedCompare (const RequestId &id1,const RequestId &id2,int mask)
{
  // null mask: comparison always succeeds
  if( !mask )
    return true;
  // start comparing from end of each ID
  HIID::const_reverse_iterator iter1 = id1.rbegin();
  HIID::const_reverse_iterator iter2 = id2.rbegin();
  // ... until we run out of bits, or get to the start of BOTH ids
  for(  int m1=1; 
        m1 < (1<<RQIDM_NBITS) && (iter1 != id1.rend() || iter2 != id2.rend()); 
        m1<<=1 )
  {
    // once we run out of indices in either ID, assume 0
    AtomicID x1 = iter1 != id1.rend() ? *(iter1++) : AtomicID(0);
    AtomicID x2 = iter2 != id2.rend() ? *(iter2++) : AtomicID(0);
    if( mask&m1 && x1 != x2 )
      return false;
  }
  return true;
}
