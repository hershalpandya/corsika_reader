/**
   \file
   Implementation of the VShowerFileParticleIterator for an Corsika generated
   shower file
   \author Troy Porter
   \author Lukas Nellen
   \version $Id$
   \date 22 May 2003
*/

#ifndef _io_RawParticleIterator_h_
#define _io_RawParticleIterator_h_

static const char CVSId__RawParticleIterator[] =
"$Id$";

#include <corsika/CorsikaBlock.h>
#include <corsika/CorsikaParticle.h>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional.hpp>

namespace corsika {

  namespace Corsika {
    class VRawStream;
  }

  class CorsikaParticle;


  class VRawParticleIterator {
  public:
    virtual boost::optional<CorsikaParticle> GetCorsikaParticle() const = 0;
    virtual void Rewind() = 0;
    virtual bool IsValid() const = 0;
  };

  /**
    \class RawParticleIterator
    \brief Iterator for particles in a corsika generated shower file
    \author Troy Porter
    \author Lukas Nellen
    \author Javier Gonzalez
    \version $Id$
    \date 22 May 2003
  */

  template <class Thinning>
  class RawParticleIterator: public VRawParticleIterator
  {

  public:
    typedef unsigned long int PositionType;

    //typedef corsika::Corsika::VRawStream::PositionType PositionType;

    RawParticleIterator();
    RawParticleIterator(const Corsika::VRawStream& rawStream, const PositionType startPosition);

    ~RawParticleIterator() { }

    virtual boost::optional<CorsikaParticle> GetCorsikaParticle() const
    {
      const typename corsika::Corsika::Block<Thinning>::ParticleData* d = GetOneParticle();
      if(d) {
        CorsikaParticle p(d);
        return boost::optional<CorsikaParticle>(p);
      }
      return boost::optional<CorsikaParticle>();
    }

    const typename corsika::Corsika::Block<Thinning>::ParticleData* GetOneParticle();

    const typename corsika::Corsika::Block<Thinning>::ParticleData* GetOneParticle() const
    { return const_cast<RawParticleIterator<Thinning>*>(this)->GetOneParticle(); }

    void Rewind();

    bool operator==(const RawParticleIterator& other) const;

    bool IsValid() const
    { return fIteratorValid; }

  private:

    boost::shared_ptr<corsika::Corsika::VRawStream> fRawStream;

    PositionType fStartPosition;
    PositionType fCurrentBlockIndex;
    int fParticleInBlock;

    corsika::Corsika::Block<Thinning> fCurrentBlock;

    bool fIteratorValid;
    bool fBlockBufferValid;
  };

}


#endif // _io_RawParticleIterator_h_

// Configure (x)emacs for this file ...
// Local Variables:
// mode: c++
// compile-command: "make -C .. -k"
// End:
