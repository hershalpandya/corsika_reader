/**
   \file
   Implementation of the VShowerFileParticleIterator for an Corsika generated
   shower file
   \author Troy Porter
   \author Lukas Nellen
   \version $Id$
   \date 22 May 2003
*/

#ifndef _io_CorsikaShowerFileParticleIterator_h_
#define _io_CorsikaShowerFileParticleIterator_h_

static const char CVSId__CorsikaShowerFileParticleIterator[] =
"$Id$";

#include <corsika/CorsikaBlock.h>
#include <corsika/RawCorsikaFile.h>
#include <corsika/CorsikaParticle.h>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional.hpp>

namespace corsika {

  namespace Corsika {
    class VRawStream;
  }

  class CorsikaParticle;


  /**
    \class CorsikaShowerFileParticleIterator
    \brief Implementation of the VShowerFileParticleIterator for an Corsika generated
           shower file
    \author Troy Porter
    \author Lukas Nellen
    \version $Id$
    \date 22 May 2003
    \ingroup corsika particles
  */

  class CorsikaShowerFileParticleIterator: public boost::iterator_facade<CorsikaShowerFileParticleIterator,
                                                                         corsika::CorsikaParticle,
                                                                         boost::single_pass_traversal_tag,
                                                                         corsika::CorsikaParticle&
                                                                         >
  {

  public:
    typedef corsika::Corsika::VRawStream::PositionType PositionType;
    // the following things implement an iterator interface using iterator_facade. This might be removed.
    typedef CorsikaParticle& result_type;

    void increment();

    CorsikaParticle& dereference() const {
      return value_.get();
    }

    bool equal(CorsikaShowerFileParticleIterator const & other) const {
      return (*this == other);
    }

    CorsikaShowerFileParticleIterator();
    CorsikaShowerFileParticleIterator(const Corsika::VRawStream& rawStream,
				      Corsika::EventHeader event_header, // passed by value
                                      unsigned long int start,
                                      const double timeOffset,
                                      const unsigned int observationLevel,
                                      const bool isThinned,
                                      const bool keepMuProd);

    virtual ~CorsikaShowerFileParticleIterator() { }

    virtual void Rewind()
    { iterator_->Rewind(); }

    bool operator==(const CorsikaShowerFileParticleIterator& other) const;


  private:

    bool IsValid() const
    { return iterator_ && iterator_->IsValid(); }

    mutable boost::optional<CorsikaParticle> value_;
    Corsika::EventHeader event_header_;
    boost::shared_ptr<VRawParticleIterator> iterator_;

    double fTimeOffset;
    unsigned int fObservationLevel;
    bool fIsThinned;
    bool fKeepMuProd;
  };

}


#endif // _io_CorsikaShowerFileParticleIterator_h_

// Configure (x)emacs for this file ...
// Local Variables:
// mode: c++
// compile-command: "make -C .. -k"
// End:
