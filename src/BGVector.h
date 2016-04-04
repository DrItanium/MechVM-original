////////////////////////////////////////////////////////////////////////////////
// BGVector.h - main advantage is function getRawBuf, and non-const operator[] 
// increases size dynamically
// Class Declaration
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef BGVector__H
#define BGVector__H
#include "ExceptionBase.h"

////////////////////////////////////////////////////////////////////////////////
// Template BGVector

template <typename T, int bufID>
class BGVector: VirtualClass
{
public:
   // Default constructor
   BGVector ()
      : VirtualClass (BGVectorID),
      m_reserve (0),
      m_size (0),
      m_data (NULL)
   { 
   }

   // Alternative constructor: Create vector of given size
   BGVector (size_t initialReserve)
      : VirtualClass (BGVectorID),
      m_reserve (initialReserve),
      m_size (0),
      m_data (new T[initialReserve])
   { 
   }

   // Copy constructor
   BGVector (const BGVector<T, bufID>& other)
      : VirtualClass (BGVectorID),
      m_reserve (other.m_size),
      m_size (other.m_size),
      m_data (new T[other.m_size])
   {
      for (size_t i = 0; i < m_size; ++i)
         m_data[i] = other->m_data[i];
   }

   // Destructor
   virtual ~BGVector ()
   { 
      reserve(0); 
   }

   // Array access: const operator does not change size
   inline const T& operator[] (size_t index) const
   {
      if (index < m_size)
         return m_data[index];
      else
         throw ExceptionBase ("BGVector: const operator[] access too high");
   }

   // Array access: non-const access increases size when necessary
   inline T& operator[] (size_t index)
   { 
      if (index < m_size)
         return m_data[index]; 
      else if (index < m_reserve) {
         m_size = index+1;
         return m_data[index];
      } else { // index >= reserve
         reserve (2*(index+1));
         m_size = index+1;
         return m_data[index];
      }
   }

   // Set size
   bool reserve (size_t newReserve)
   {
      if (m_reserve == newReserve)
         return true;

      T* newData = NULL;
      if (newReserve != 0) {
         newData = (T*) traceAlloc (newReserve*sizeof(T), bufID);
         if (m_data != NULL) {
            if (m_size != 0) {
               m_size = bgmin (m_size, newReserve);
               for (size_t i = 0; i < m_size; ++i)
                  newData[i] = m_data[i];
            }
            traceFree (m_data, bufID, m_reserve*sizeof(T));
         }
      } else if (m_reserve != 0) {
         traceFree (m_data, bufID, m_reserve*sizeof(T));
         m_size = 0;
      }

      m_reserve = newReserve;
      m_data = newData;
      return true;
   }

   // Get size
   inline size_t getSize () const
   {
      return m_size; 
   }

   // Get raw data buffer (use cautiously)
   inline T* getRawBuf (size_t index = 0) 
   {
      return &m_data[index];
   }

   // Add copy of an element
   inline size_t add (const T& newElement)
   {
      T& copy = operator[](m_size); 
      copy = newElement; 
      return m_size-1;
   }

   // Add copy of an element if it is not yet in the mesh, returns false otherwise
   inline bool addNoDups (const T& elem)
   {
      bool notFound = true;
      size_t i = 0;

      while (i < m_size && notFound) {
         if (m_data[i] == elem)
            notFound = false;
         else
            ++i;
      }

      if (notFound) {
         add(elem);
         return true;
      } else 
         return false;
   }

   // Add new default element
   inline T& add ()
   {
      return operator[](m_size);
   }

   // Clear contents
   inline void clear()
   { m_size = 0; }

   // Remove element by swapping with last element and then removing it
   // (does not maintain element order)
   inline bool remove (size_t i)
   {
      if (i < m_size) {
         if (i+1 < m_size)
            m_data[i] = m_data[m_size-1];
         --m_size;
         return true;
      } else
         return false;
   }

   // Remove last element
   inline bool removeLast ()
   {
      if (m_size > 0) {
         --m_size;
         return true;
      } else
         return false;
   }

   // Set new size directly
   inline void setSize(size_t newSize)
   {
      if (newSize < m_reserve) {
         m_size = newSize;
      } else if (newSize > m_reserve) {
         reserve (newSize);
         m_size = newSize;
      }
   }

   void permutate()
   {
      // Swap every element with a random other one
      for (size_t i = 0; i < m_size; ++i) {
         size_t j = random(0, m_size-1);
         T backup = m_data[i];
         m_data[i] = m_data[j];
         m_data[j] = backup;
      }
   }
private:
   size_t m_reserve, m_size;
   T* m_data;
};

#endif
