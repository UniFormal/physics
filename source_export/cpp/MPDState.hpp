#ifndef MPD_STATE_H
#define MPD_STATE_H

namespace MPD
{
  class MPDState
  {
  public:
    MPDState(MPDBase &mpd);

    ~MPDState();

    operator std::string() const{ return toString()};

    std::string toString() const;

    double operator[](std::string quantityName) const;

    
    
  private:
  };
}

#endif // MPD_STATE_H
