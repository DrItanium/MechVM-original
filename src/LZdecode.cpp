////////////////////////////////////////////////////////////////////////////////
// Lempel-Ziv decoder for MW2
// Copyright Bjoern Ganster 2008
////////////////////////////////////////////////////////////////////////////////

#include "LZdecode.h"

////////////////////////////////////////////////////////////////////////////////

class LZdecodeState {
private:
   MemoryBlock * in, * out;
   size_t iOffset, oOffset;
public:
   // Constructor
   LZdecodeState ()
   {
      in = NULL;
      out = NULL;
      iOffset = 4;
      oOffset = 0;
   }

   // Destructor
   ~LZdecodeState ()
   {
      // in, out are destroyed by caller - nothing to do
   }

   // Decode code word
   void decodeCodeWord ()
   {
      if (iOffset+2 > in->getSize() || oOffset > out->getSize()) 
         return;

      WORD code = in->getWord(iOffset);
      size_t outCount = (code >> 12)+3;
      short int dist = (code & 4095);

      size_t i = 0;
      while (i < outCount && oOffset < out->getSize()) {
         size_t maskedOOffset = 0;
         if (oOffset > 4096) {
            maskedOOffset = (oOffset & (4096+8192+16384+32768+65536+131072+262144));
            if (((int) oOffset)-((int) maskedOOffset) < dist)
               maskedOOffset -= 4096;
         }
         size_t cpOfs = maskedOOffset + dist + i;
         unsigned char b = 0;
         if (cpOfs < oOffset)
            b = out->getByte (cpOfs);
         if (oOffset < out->getSize())
            out->setByte(oOffset, b);
         oOffset++;
         i++;
      }

      iOffset += 2;
   }

   // Copy input to output
   void copy (size_t count)
   {
      if (iOffset > in->getSize() || oOffset > out->getSize())
         return;
      if (iOffset+count > in->getSize())
         count = in->getSize()-iOffset;
      if (oOffset+count > out->getSize())
         count = out->getSize()-oOffset;
      out->copy (oOffset, in->getPtr (iOffset), count);
      iOffset += count;
      oOffset += count;
   }

   // Decode control nibble
   void decodeControlByte (unsigned char cb)
   {
      unsigned minRestSize[] = {7, 7, 7, 5, 6, 5, 4, 5, 6, 5, 5, 5, 4, 4, 4, 3};
      //if (iOffset+minRestSize[cb] > in->getSize())
      //   return;

      switch (cb) {
         case 0:
            // Decode 8 bytes
            decodeCodeWord ();
            decodeCodeWord ();
            decodeCodeWord ();
            decodeCodeWord ();
            break;
         case 1:
            // Copy one, decode six (totally read: 7)
            copy (1);
            decodeCodeWord ();
            decodeCodeWord ();
            decodeCodeWord ();
            break;
         case 2:
            // Decode two, copy one, decode 4
            decodeCodeWord ();
            copy (1);
            decodeCodeWord ();
            decodeCodeWord ();
            break;
         case 3:
            // Copy two, decode four
            copy (2);
            decodeCodeWord ();
            decodeCodeWord ();
            break;
         case 4:
            // Decode four bytes, copy one, decode two
            decodeCodeWord ();
            decodeCodeWord ();
            copy (1);
            decodeCodeWord ();
            break;
         case 5:
            // Copy one, decode two, copy one, decode two
            copy (1);
            decodeCodeWord ();
            copy (1);
            decodeCodeWord ();
            break;
         case 6:
            //  Decode two, copy two, decode two
            decodeCodeWord ();
            copy (2);
            decodeCodeWord ();
            break;
         case 7:
            // Copy 3 bytes, then decompress final two bytes
            copy (3);
            decodeCodeWord ();
            break;
         case 8:
            // Decode six, copy one
            decodeCodeWord ();
            decodeCodeWord ();
            decodeCodeWord ();
            copy (1);
            break;
         case 9:
            // Copy one, decode four, copy one
            copy (1);
            decodeCodeWord ();
            decodeCodeWord ();
            copy (1);
            break;
         case 0xA:
            // Decode two, copy one, decode two, copy one
            decodeCodeWord ();
            copy (1);
            decodeCodeWord ();
            copy (1);
            break;
         case 0xB:
            // Copy 2 bytes, decode two, copy the last byte
            copy (2);
            decodeCodeWord ();
            copy (1);
            break;
         case 0xC:
            // Decode four, copy two
            decodeCodeWord ();
            decodeCodeWord ();
            copy (2);
            break;
         case 0xD:
            // Copy 1 byte, decode 2, copy another 2
            copy (1);
            decodeCodeWord ();
            copy (2);
            break;
         case 0xE:
            // Decode 2, then copy 3
            decodeCodeWord ();
            copy (3);
            break;
         case 0xF:
            // Copy 4 bytes to output
            copy (4);
            break;
      }
   }

   // Decode Lempel-Ziv
   MemoryBlock* decode(MemoryBlock* _in)
   {
      in = _in;
      DWORD outSize = in->getDWord(0);
      out = new MemoryBlock (outSize);

      try {
         if (getFileTypeFromContent (in) != FT_EFA)
            return NULL;

         while (iOffset < in->getSize()) {
            unsigned char cb = in->getByte(iOffset);
            iOffset++;
            decodeControlByte (cb & 15);
            decodeControlByte (cb >> 4);
         }

         /*BGString str = "PCX-Decoding stopped with control byte";
         //str.appendInt (cb, 16);
         str += " at ";
         str.appendUInt (iOffset, 16);
         str.printLn();*/

         out->resize (oOffset);
         return out;
      } catch (...) {
         BGString str = "Failed to decode file at offset ";
         str.appendUInt(iOffset, 16);
         str.printLn();
         return NULL;
      }
   }
};

///////////////////////////////////////////////////////////////////////////////

MemoryBlock* LZdecode(MemoryBlock* in)
{
   LZdecodeState state;
   return state.decode(in);
}
