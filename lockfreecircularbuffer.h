/* Copyright (C) 2013 Leonardo Bispo de Oliveira
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef LOCKFREECIRCULARBUFFER_H
#define LOCKFREECIRCULARBUFFER_H

#include <limits>
#include <atomic>

#include <thread>
#include <chrono>

/**
 * Implementation of a Circular Buffer using an Lock-Free algorithm.
 *
 * @author Leonardo Bispo de Oliveira.
 *
 */
template<class T>
class LockFreeCircularBuffer
{
  public:
    /**
     * Constructor.
     *
     * @param size Queue size. If it is not power of two, a next power of 2 number will be used.
     *
     */
    LockFreeCircularBuffer(uint64_t size) : ptrRead(0), ptrWrite(0), ptrAvailable(0)
    {
      size  = nextPowerOfTwo(size);
      mask  = size - 1;
      queue = new T[size];
    }

    /**
     * Destructor.
     *
     */
    virtual ~LockFreeCircularBuffer()
    {
      delete [] queue;
    }

    /**
     * Push a new element to the queue if there are enough room to it. If there is not enough space, the push will be ignored and the method will return false.
     *
     * @param element Element to be inserted in the buffer.
     *
     * @return True if the element was correctly inserted, otherwise false.
     *
     */
    bool push(const T &element)
    {
      uint64_t readIdx;
      uint64_t writeIdx;

      do {
        writeIdx = ptrWrite;
        readIdx  = ptrRead;

        if (((writeIdx + 1) & mask) == (readIdx & mask))
          return false;
      } while (!ptrWrite.compare_exchange_strong(writeIdx, (writeIdx + 1)));

      queue[writeIdx & mask] = element;

      while (!ptrAvailable.compare_exchange_strong(writeIdx, (writeIdx + 1)))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

      bufferLen.fetch_add(1);
      return true;
    }

    /**
     * Pop an element from the queue if there is an element available. If there is no element inside the buffer, the method will return false.
     *
     * @param element Element to be used to add the popped data.
     *
     * @return True if an element has been popped from the buffer, otherwise false.
     *
     */
    bool pop(T &element)
    {
      for (;;) {
        uint64_t availableIdx = ptrAvailable;
        uint64_t readIdx      = ptrRead;

        if ((readIdx & mask) == (availableIdx & mask))
          return false;

        element = queue[readIdx & mask];

        if (ptrRead.compare_exchange_strong(readIdx, (readIdx + 1))) {
          bufferLen.fetch_sub(1);
          return true;
        }
      }
    }

    /**
     * Check if the buffer is Empty.
     *
     * @return True if it is empty, otherwise false.
     *
     */
    bool empty() const
    {
      return bufferLen == 0;
    }

  private:
    T        *queue;
    uint64_t mask;

    std::atomic<uint64_t> ptrRead;
    std::atomic<uint64_t> ptrWrite;

    std::atomic<uint64_t> ptrAvailable;
    std::atomic<uint64_t> bufferLen;

    /**
     * Helper to return the next closest power of two number of inValue.
     *
     */
    template <class V>
    V nextPowerOfTwo(V inValue)
    {
      if (std::numeric_limits<V>::is_signed && inValue == 0)
        return 1;

      --inValue;
      for (size_t i = 1; i < std::numeric_limits<V>::digits; i <<= 1)
        inValue = static_cast<V>(inValue | (inValue >> i));

      return static_cast<V>(inValue + 1);
    }
};

#endif
