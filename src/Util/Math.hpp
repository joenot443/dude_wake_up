//
//  Math.h
//  dude_wake_up
//
//  Created by Joe Crozier on 8/31/22.
//

#ifndef Math_h
#define Math_h

struct Math {
  
  constexpr static const float RELEASE = 0.99;
  
  static int nearestOdd( float x )
  {
    return 2 * ( (int)(x / 2.0f) ) + 1;
  }
  
  static float release(float curr, float target) {
    if (target > curr) { return target; }
    
    if (curr * 0.95 < target) { return target; }
    
    return curr * 0.95;
  }
};
  
#endif /* Math_h */
