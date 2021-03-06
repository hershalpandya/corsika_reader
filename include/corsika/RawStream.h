/**
   \file
   Raw disk file as specified by the CORSIKA manual.

   \author Lukas Nellen
   \version $Id$
   \date 08 Dec 2003
*/

#ifndef _corsika_Corsika_RawStream_h_
#define _corsika_Corsika_RawStream_h_

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <corsika/CorsikaBlock.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <corsika/CorsikaIOException.h>
#include <boost/iostreams/filtering_stream.hpp>
#include <corsika/RawParticleIterator.h>

namespace corsika {

  namespace Corsika {

    enum Compression {
      eNone,
      eGZip,
      eBZip2
    };

    template <class Thinning, int Padding> class RawStream;

    struct FileIndex {
      int runNumber;
      std::vector<unsigned long int> eventHeaders;
      std::vector<unsigned long int> eventTrailers;
      std::vector<unsigned long int> longBlocks;
      std::map<unsigned int, unsigned int> IDToPosition;
      std::string String()
      {
        std::ostringstream str;
        str << "file index" << std::endl;
        for (unsigned int i = 0; i != eventHeaders.size(); ++i) {
          str << i << ", header " << eventHeaders[i] << ", trailer " << eventTrailers[i];
          if (longBlocks.size())
            str << ", long " << longBlocks[i];
          str<< std::endl;
        }
        return str.str();
      }
    };
    template <class Thinning, int Padding> FileIndex Scan(corsika::Corsika::RawStream<Thinning, Padding>& stream, bool force=false);
    boost::shared_ptr<std::istream> GetFilter(std::istream& in, Compression c);

    struct FormatSpec {
      FormatSpec(std::istream& in);
      bool thinned;
      bool size_32;
      bool invalid;

      std::string String()
      {
        std::ostringstream str;
        str << (thinned?"thinned, ":"not thinned, ")
            << (int(!size_32)+1)*32 << " bit,"
            << (invalid?"invalid, ":"valid, ");
        return str.str();
      }
    };

    class VRawStream: public boost::enable_shared_from_this<VRawStream> {
    public:
      typedef unsigned long int PositionType;

      virtual ~VRawStream()
      {
        while (fStreams.size()) fStreams.pop_back();
      }

      /// Read one block and advance
      //bool GetNextBlock(Block<Thinning>& theBlock);
      virtual bool GetNextBlock(Block<Thinned>& theBlock) = 0;
      virtual bool GetNextBlock(Block<NotThinned>& theBlock) = 0;

      /// Number of the block read by the next call to GetNextBlock
      virtual PositionType GetNextPosition() const = 0;

      /// Seek to a given block, the next block will be \a thePosition
      virtual void SeekTo(const PositionType thePosition) = 0;
      virtual bool IsSeekable() const = 0;

      virtual bool IsValid() = 0;
      virtual bool IsThinned() const = 0;

      virtual boost::shared_ptr<VRawStream> Clone() const
      { return const_cast<VRawStream*>(this)->shared_from_this(); } // I know... const_cast... argh...

      virtual boost::shared_ptr<VRawParticleIterator> GetVParticleIt(PositionType start=0) const = 0;

      virtual FileIndex Scan(bool force) = 0;
      virtual void Close() = 0;

      virtual void SetFile(std::string filename, boost::shared_ptr<std::ifstream> file) = 0;

      void Hold(boost::shared_ptr<std::istream> ptr)
      { fStreams.push_back(ptr); }

      std::string   fName;
    protected:
      virtual void Move(VRawStream& other)
      { fStreams = other.fStreams; other.fStreams.clear(); }
    private:
      std::vector<boost::shared_ptr<std::istream> > fStreams;

    };

    class RawStreamFactory {
    public:
      static boost::shared_ptr<VRawStream> Create(const std::string& theName);
      static boost::shared_ptr<VRawStream> Create(std::istream& in, bool randomAccess, Compression c=eNone);
    };

    template<class Stream, class BlockThinning>
    struct NextBlockDispatch {
      static bool GetNextBlock(Stream& stream, Block<BlockThinning>& theBlock)
      {
        return false;
      }
    };
    template<class Stream>
    struct NextBlockDispatch<Stream, typename Stream::ThinningType> {
      static bool GetNextBlock(Stream& stream, Block<typename Stream::ThinningType>& theBlock)
      {
        return stream.GetNextBlockImpl(theBlock);
      }
    };

    /**
     \class RawStream RawCorsikaFile.h "Corsika/RawCorsikaFile.h"

     \brief Raw disk file.

     This class provides block-wise read access to a Corsika ground
     particles file on disk. Simple random access is supported.

     This class handles the grouping of individual blocks into a disk
     block with padding. It doesn't provide tools for unpacking the
     individual particles from a block.

     \author Lukas Nellen
     \author Javier Gonzalez
     \date 08 Dec 2003
     \ingroup corsika
    */

    template <class Thinning, int Padding=1>
    class RawStream: public VRawStream {
    public:
      typedef unsigned long int PositionType;
      typedef Thinning ThinningType;

      static const unsigned int kBlocksInDiskBlock = Thinning::kSubBlocksPerBlock;

      /// Padding bytes at the beginning of a raw block
      static const unsigned int kPaddingBeginning  = Padding;
      /// Padding bytes at the end of a raw block
      static const unsigned int kPaddingEnd        = Padding;


      class DiskBlock {
      public:
        /// initial padding - works also for size 0
        int fPaddingBeginning[kPaddingBeginning];
        Block<Thinning>  fBlock[kBlocksInDiskBlock];
        /// final padding - works also for size 0
        int fPaddingEnd[kPaddingEnd];

      };


      RawStream(std::istream& in, bool randomAccess=true);
      RawStream(std::istream& in, boost::shared_ptr<std::ifstream> file, bool randomAccess=true);

      ~RawStream()
      { Close(); }

      /// Read one block and advance
      bool GetNextBlock(Block<Thinned>& theBlock)
      { return NextBlockDispatch<RawStream<Thinning, Padding>, Thinned>::GetNextBlock(*this, theBlock); }
      bool GetNextBlock(Block<NotThinned>& theBlock)
      { return NextBlockDispatch<RawStream<Thinning, Padding>, NotThinned>::GetNextBlock(*this, theBlock); }
      bool GetNextBlockImpl(Block<Thinning>& theBlock);

      /// Number of the block read by the next call to GetNextBlock
      PositionType GetNextPosition() const
      { return fIndexInDiskBlock + kBlocksInDiskBlock * fCurrentBlockNumber; }

      bool IsSeekable() const
      { return fRandomAccess || fFile; }

      /// Seek to a given block, the next block will be \a thePosition
      void SeekTo(const PositionType thePosition);

      bool IsValid();

      bool IsThinned() const
      { return Thinning::kBytesPerBlock == Corsika::Thinned::kBytesPerBlock; }

      boost::shared_ptr<VRawParticleIterator> GetVParticleIt(PositionType start=0) const
      { return boost::dynamic_pointer_cast<VRawParticleIterator>(GetParticleIt(start)); }

      boost::shared_ptr<RawParticleIterator<Thinning> > GetParticleIt(PositionType start=0) const
      {
        if (start == 0) {// if there is something we KNOW, it is that particles are not in block zero.
          start = GetNextPosition();
        }
        return boost::shared_ptr<RawParticleIterator<Thinning> >(new RawParticleIterator<Thinning>(*this, start));
      }

      const DiskBlock& DiskBlockBuffer() const
      { return fDiskBlockBuffer; }

      FileIndex Scan(bool force)
      { return corsika::Corsika::Scan(*this, force); }

      void Close();

      virtual void SetFile(std::string filename, boost::shared_ptr<std::ifstream> file)
      { fName = filename; fFile = file; }

    private:
      RawStream(const RawStream& other);
      RawStream& operator=(const RawStream& other)
      {
        throw CorsikaIOException("This stream is not copiable");
        return *this;
      }
      RawStream& Move(RawStream& other)
      {
        Close();
        VRawStream::Move(other);
        // take ownership of other's stuff and invalidate other
        fFilterStream.swap(other.fFilterStream);
        fFile.swap(other.fFile);
        fDiskStream         = other.fDiskStream;
        fCurrentBlockNumber = other.fCurrentBlockNumber;
        fDiskBlockBuffer    = other.fDiskBlockBuffer;
        fIndexInDiskBlock   = other.fIndexInDiskBlock;
        fBlockBufferValid   = false;
        fRandomAccess       = other.fRandomAccess;
        // the following should have no effect, only the shared_ptr but just to be consistent
        other.fDiskStream = NULL;
        other.fCurrentBlockNumber = 0;
        other.fIndexInDiskBlock = 0;
        other.fBlockBufferValid = false;
        other.fRandomAccess = false;
        return *this;
      }


      /// Read the block at the current position from disk
      bool ReadDiskBlock();

      std::istream* fDiskStream;
      boost::shared_ptr<boost::iostreams::filtering_istream> fFilterStream;
      boost::shared_ptr<std::ifstream> fFile;
      PositionType  fCurrentBlockNumber;

      DiskBlock fDiskBlockBuffer;

      unsigned int  fIndexInDiskBlock;
      bool          fBlockBufferValid;
      bool          fRandomAccess;

    };


  } // Corsika
} // corsika


#endif // _corsika_Corsika_RawStream_h_

// Configure (x)emacs for this file ...
// Local Variables:
// mode:c++
// compile-command: "make -C .. -k"
// End:
