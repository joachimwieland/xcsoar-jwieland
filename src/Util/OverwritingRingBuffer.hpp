/*
 * Copyright (C) 2010 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef XCSOAR_OVERWRITING_RING_BUFFER_HPP
#define XCSOAR_OVERWRITING_RING_BUFFER_HPP

#include <algorithm>
#include <cassert>

/**
 * A fixed-size ring buffer which deletes the oldest item when it
 * overflows.  It stores up to "size-1" items (for the full/empty
 * distinction).
 *
 * Not thread safe.
 */
template<class T, unsigned size>
class OverwritingRingBuffer {
  friend class const_iterator;

public:
  class const_iterator {
    friend class OverwritingRingBuffer;

    const OverwritingRingBuffer &buffer;
    unsigned i;

    const_iterator(const OverwritingRingBuffer<T, size> &_buffer, unsigned _i)
      :buffer(_buffer), i(_i) {
      assert(i < size);
    }

  public:
    const T &operator*() const {
      return buffer.data[i];
    }

    typename OverwritingRingBuffer::const_iterator &operator++() {
      i = buffer.next(i);
      return *this;
    }

    bool operator==(const const_iterator &other) const {
      assert(&buffer == &other.buffer);
      return i == other.i;
    }

    bool operator!=(const const_iterator &other) const {
      assert(&buffer == &other.buffer);
      return i != other.i;
    }
  };

protected:
  T data[size];
  unsigned head, tail;

public:
  OverwritingRingBuffer()
    :head(0), tail(0) {}

  OverwritingRingBuffer(const OverwritingRingBuffer<T,size> &other) {
    head = other.head;
    tail = other.tail;

    assert(head < size);
    assert(tail < size);

    if (head < tail)
      std::copy(other.data + head, other.data + tail, data + head);
    else if (head > tail) {
      std::copy(other.data + head, other.data + size, data + head);
      std::copy(other.data, other.data + tail, data);
    }
  }

protected:
  static unsigned next(unsigned i) {
    assert(i < size);

    return (i + 1) % size;
  }
  static unsigned previous(unsigned i) {
    assert(i < size);
    if (i>0)
      return (i-1);
    else
      return size-1;
  }

public:
  bool empty() const {
    assert(head < size);
    assert(tail < size);

    return head == tail;
  }

  void clear() {
    head = tail = 0;
  }

  // returns last value added
  const T &last() const {
    assert(!empty());
    return data[previous(tail)];
  }

  const T &peek() const {
    assert(!empty());

    return data[head];
  }

  const T &shift() {
    /* this returns a reference to an item which is being invalidated
       - but that's okay, because it won't get purged yet */
    const T &value = peek();
    head = next(head);
    return value;
  }

  void push(const T &value) {
    assert(tail < size);

    data[tail] = value;
    tail = next(tail);
    if (tail == head)
      /* the ring buffer is full - delete the oldest item */
      head = next(head);
  }

  /**
   * Returns a pointer to the oldest item.
   */
  const_iterator begin() const {
    return const_iterator(*this, head);
  }

  /**
   * Returns a pointer to end of the buffer (one item after the newest
   * item).
   */
  const_iterator end() const {
    return const_iterator(*this, tail);
  }

  unsigned capacity() const {
    return size;
  }
};

#endif
