// -*- c++ -*-
//
// Super Standard Template Library
//
// Faster and more optimized than stock STL implementation,
// plus includes various customized features
//
// Copyright 1997-2005 Matt T. Yourst <yourst@yourst.com>
//
// This program is free software; it is licensed under the
// GNU General Public License, Version 2.
//

#ifndef _SUPERSTL_H
#define _SUPERSTL_H

#include <unistd.h>

namespace superstl {

  //
  // ostream class
  //
  static const char endl[] = "\n";
  static class iosflush { } flush;

  class ostream {
  public:
    inline ostream() { fd = NULL; }

    void open(const char* filename, bool append = false) {
      assert(!fd);
      fd = fopen(filename, (append) ? "a" : "w");
    }

    void open(FILE* f) {
      assert(!fd);
      fd = f;
    }

    void close() {
      if (fd) fclose(fd);
      fd = NULL;
    }

    ~ostream() {
      close();
    }

    ostream(const char* filename, bool append = false) {
      fd = null;
      open(filename, append);
    }

    ostream(FILE* f) {
      fd = null;
      open(f);
    }

    int write(const void* buf, int count) {
      if (!fd) return 0;
      int n = fwrite(buf, 1, count, fd);
      return n;
    }

    operator FILE*() const {
      return fd;
    }

    operator bool() const {
      return ok();
    }

    bool ok() const {
      return (fd != NULL);
    }

    int filehandle() const {
      return fileno(fd);
    }

    void flush() const {
      fflush(fd);
    }

  public:
    FILE* fd;
  };
  
  //
  // Manipulators
  //      
  inline ostream& operator <<(ostream& os, const iosflush& v) {
    if (os.ok()) fflush(os.fd);
    return os;
  }
  
  //
  // Inserters
  //
  
#define DefineInserter(T, format, expr) inline ostream& operator <<(ostream& os, const T v) { if (os.ok()) fprintf(os.fd, format, expr); return os; }
  DefineInserter(char*, "%s", v);
  DefineInserter(char, "%c", v);
  DefineInserter(signed short, "%hd", v);
  DefineInserter(signed int, "%d", v);
  DefineInserter(signed long, "%ld", v);
  DefineInserter(signed long long, "%lld", v);
  DefineInserter(unsigned short, "%hu", v);
  DefineInserter(unsigned int, "%u", v);
  DefineInserter(unsigned long, "%lu", v);
  DefineInserter(unsigned long long, "%llu", v);

  DefineInserter(float, "%f", v);
  DefineInserter(double, "%f", v);
  DefineInserter(bool, "%s", (v) ? "true" : "false");
#undef DefineInserter

  template <class T>
  inline ostream& operator <<(ostream& os, const T* v) {
    if (os.ok()) fprintf(os.fd, "%#llx", (void*)v);
    return os;
  }

  //
  // A much more intuitive syntax than STL provides:
  //
  template <class T>
  inline ostream& operator ,(ostream& os, const T& v) {
    return os << v;
  }

  //
  // String buffer
  //

#define stringbuf_smallbufsize 32

  class stringbuf {
  public:
    stringbuf() { buf = null; reset(); }
    stringbuf(int length) {
      buf = null;
      reset(length);
    }

    void reset(int length = stringbuf_smallbufsize);

    ~stringbuf();

    int remaining() const {
      return (buf + length) - p;
    }

    operator char*() const {
      return buf;
    }

    void resize(int newlength);

    void expand() {
      resize(length*2);
    }

    void reserve(int extra);

  public:
    char smallbuf[stringbuf_smallbufsize];
    char* buf;
    char* p;
    int length;
  };
  
  //
  // Inserters
  //
  stringbuf& operator <<(stringbuf& os, const char* v);
  stringbuf& operator <<(stringbuf& os, const char v);

#define DefineInserter(T, format) \
  inline stringbuf& operator <<(stringbuf& os, const T v) { \
    char buf[128]; \
    snprintf(buf, sizeof(buf), format, v); \
    return os << buf; \
  }

  DefineInserter(signed short, "%hd");
  DefineInserter(signed int, "%d");
  DefineInserter(signed long, "%ld");
  DefineInserter(signed long long, "%lld");
  DefineInserter(unsigned short, "%hu");
  DefineInserter(unsigned int, "%u");
  DefineInserter(unsigned long, "%lu");
  DefineInserter(unsigned long long, "%llu");
  DefineInserter(float, "%f");
  DefineInserter(double, "%f");

  inline stringbuf& operator <<(stringbuf& os, const bool v) {
    return os << (int)v;
  }

#undef DefineInserter

  inline stringbuf& operator <<(stringbuf& os, const stringbuf& sb) {
    //
    // This is an exceptional case for constructs like:
    //
    // stringbuf& func(stringbuf& sb) { sb << "xyz" }
    // os << func(sb);      // os is an ostream
    //
    // The user usually wants to take the contents of whatever was added
    // into sb and write the whole thing to os. However, if we add to
    // sb here, then return the entire sb, problems arise: the entire
    // stringbuf gets printed a second time (via it being cast to char
    // by the compiler).
    //
    // There is no generally workable solution to this problem, so we
    // just warn the user to be careful if they try this.
    //
    abort();
  }

  template <class T>
  inline stringbuf& operator <<(stringbuf& os, const T* v) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%p", v);
    return os << buf;
  }

  //
  // A much more intuitive syntax than STL provides:
  //
  template <class T>
  inline stringbuf& operator ,(stringbuf& os, const T& v) {
    return os << v;
  }

#define DeclareStringBufToStream(T) inline ostream& operator <<(ostream& os, const T& arg) { stringbuf sb; sb << arg; os << sb; return os; }

  // Print bits as a string:
  struct bitstring {
    W64 bits;
    int n;
    bool reverse;
    
    bitstring() { }
    
    bitstring(const W64 bits, const int n, bool reverse = false) {
      assert(n <= 64);
      this->bits = bits;
      this->n = n;
      this->reverse = reverse;
    }
  };

  stringbuf& operator <<(stringbuf& os, const bitstring& bs);

  DeclareStringBufToStream(bitstring);

  struct bitmaskstring {
    W64 bits;
    W64 mask;
    int n;
    bool reverse;
    
    bitmaskstring() { }
    
    bitmaskstring(const W64 bits, W64 mask, const int n, bool reverse = false) {
      assert(n <= 64);
      this->bits = bits;
      this->mask = mask;
      this->n = n;
      this->reverse = reverse;
    }
  };
  
  stringbuf& operator <<(stringbuf& os, const bitmaskstring& bs);

  DeclareStringBufToStream(bitmaskstring);

  struct hexstring {
    W64 value;
    int n;
    
    hexstring() { }
    
    hexstring(const W64 value, const int n) {
      this->value = value;
      this->n = n;
    }
  };
  
  stringbuf& operator <<(stringbuf& os, const hexstring& hs);

  DeclareStringBufToStream(hexstring);

  struct bytemaskstring {
    const byte* bytes;
    W64 mask;
    int n;
    int splitat;

    bytemaskstring() { }

    bytemaskstring(const byte* bytes, W64 mask, int n, int splitat = 16) {
      assert(n <= 64);
      this->bytes = bytes;
      this->mask = mask;
      this->n = n;
      this->splitat = splitat;
    }
  };
  
  stringbuf& operator <<(stringbuf& os, const bytemaskstring& bs);

  DeclareStringBufToStream(bytemaskstring);

  struct intstring {
    W64s value;
    int width;

    intstring() { }

    intstring(W64s value, int width) {
      this->value = value;
      this->width = width;
    }
  };

  stringbuf& operator <<(stringbuf& os, const intstring& is);

  DeclareStringBufToStream(intstring);

  struct floatstring {
    double value;
    int width;
    int precision;
    
    floatstring() { }

    floatstring(double value, int width = 0, int precision = 6) {
      this->value = value;
      this->width = width;
      this->precision = precision;
    }
  };
  
  stringbuf& operator <<(stringbuf& os, const floatstring& fs);

  DeclareStringBufToStream(floatstring);

  struct padstring {
    const char* value;
    int width;

    padstring() { }

    padstring(const char* value, int width) {
      this->value = value;
      this->width = width;
    }
  };

  stringbuf& operator <<(stringbuf& os, const padstring& s);

  DeclareStringBufToStream(padstring);

  //
  // Output stream for raw data
  //
  class odstream {
  public:
    inline odstream() { fd = NULL; }

    void open(const char* filename, bool append = false) {
      assert(!fd);
      fd = fopen(filename, (append) ? "a" : "w");
    }

    void open(FILE* f) {
      assert(!fd);
      fd = f;
    }

    ~odstream() {
      close();
    }

    void close() {
      if (fd) fclose(fd);
      fd = NULL;
    }

    odstream(const char* filename, bool append = false) {
      fd = null;
      open(filename, append);
    }

    odstream(FILE* f) {
      open(f);
    }

    int write(const void* buf, int count) {
      if (!fd) return 0;
      int n = fwrite(buf, 1, count, fd);
      return n;
    }

    operator FILE*() {
      return fd;
    }

    operator bool() {
      return ok();
    }

    bool ok() {
      return (fd != NULL);
    }

    int filehandle() {
      return fileno(fd);
    }

    void flush() {
      fflush(fd);
    }

  public:
    FILE* fd;
  };

  template <typename T>
  inline odstream& operator <<(odstream& os, const T& v) {
    os.write(&v, sizeof(T));
    return os;
  }

  //
  // A much more intuitive syntax than STL provides:
  //
  template <typename T>
  inline odstream& operator ,(odstream& os, const T& v) {
    return os << v;
  }

  //
  // istream class
  //
  class istream {
  public:
    istream() {
      fd = null;
    }

    bool open(const char* filename) {
      assert(!fd);
      fd = fopen(filename, "r");
      ok = (fd != NULL);
      setwidth(0);
      return ok;
    }

    istream(const char* filename) {
      fd = null;
      open(filename);
    }

    istream(FILE* f) {
      assert(!fd);
      fd = f;
      ok = (fd != NULL);
      setwidth(0);
    }
    
    void close() {
      if (fd) fclose(fd);
      fd = null;
    }

    ~istream() {
      close();
    }

    void reset() { ok = true; }
    operator bool() { return ok; }

    void setwidth(int maxwidth) {
      snprintf(strformat, sizeof(strformat), "%%%ds", maxwidth);
    }

    inline char* readln() {
      char* v = NULL;
      int n = fscanf(fd, "%as\n", &v);
      ok = (n == 1);
      return (ok) ? v : NULL;
    }

    int read(void* buf, int count) {
      int n = fread(buf, 1, count, fd);
      ok = (n == count);
      return n;
    }

    operator FILE*() {
      return fd;
    }

    int filehandle() {
      return fileno(fd);
    }

  public:
    FILE* fd;
    bool ok;

    char strformat[16];
  };

  class readline { 
  public:
    readline(char* p, size_t l): buf(p), len(l) { }
    char* buf;
    size_t len;
  };

  /*
  template <int size>
  class readline { 
  public:
    readline(char (&)[size]): buf(p), len(size) { }
    char* buf;
    size_t len;
  };
  */

#define DefineExtractor(T, format) inline istream& operator >>(istream& is, T& v) { int n = fscanf(is.fd, format, &v); if (is.ok) is.ok = (n == 1); return is; }
  DefineExtractor(char, "%c");
  DefineExtractor(signed short, "%hd");
  DefineExtractor(signed int, "%d");
  DefineExtractor(signed long, "%d");
  DefineExtractor(unsigned short, "%hd");
  DefineExtractor(unsigned int, "%d");
  DefineExtractor(unsigned long, "%d");

  DefineExtractor(float, "%f");
  DefineExtractor(double, "%lf");
#undef DefineExtractor

  istream& operator >>(istream& is, char* v);
  istream& operator >>(istream& is, const readline& v);

  inline istream& operator ,(istream& is, const readline& v) { return is >> v; }

  //
  // A much more intuitive syntax than STL provides:
  //
  template <class T>
  inline istream& operator ,(istream& is, T& v) {
    return is >> v;
  }

  //
  // idstream class (binary data stream)
  //
  class idstream {
  public:
    idstream() {
      fd = NULL;
    }

    idstream(const char* filename) {
      fd = null;
      open(filename);
    }

    idstream(FILE* f) { 
      fd = null;
      assert(!fd);
      fd = f;
      ok = (fd != NULL);
    }

    ~idstream() {
      if (fd) fclose(fd);
      fd = NULL;
    }

    int open(const char* filename) {
      fd = null;
      assert(!fd);
      fd = fopen(filename, "r");
      ok = (fd != NULL);
      return ok;
    }

    void close() {
      if (fd) fclose(fd);
      fd = NULL;
    }

    int read(void* v, int size) {
      int n = fread(v, 1, size, fd);
      ok = (n == size);
      return n;
    }

    void reset() { ok = true; }
    operator bool() { return ok; }

    unsigned long long size() const;

    operator FILE*() {
      return fd;
    }

    int filehandle() {
      return fileno(fd);
    }

    void* mmap(long long size);

  public:
    FILE* fd;
    bool ok;
  };

  template <typename T>
  inline idstream& operator >>(idstream& is, T& v) { 
    is.read(&v, sizeof(T)); 
    return is; 
  }

  //
  // A much more intuitive syntax than STL provides:
  //
  template <typename T>
  inline idstream& operator ,(idstream& is, T& v) {
    return is >> v;
  }

  //
  // Global streams:
  //
  extern istream cin;
  extern ostream cout;
  extern ostream cerr;

  template <typename T>
  T* renew(T* p, size_t oldcount, size_t newcount) {
    if (newcount <= oldcount) return p;

    T* pp = new T[newcount];

    if (!p) assert(oldcount == 0);

    if (p) {
      memcpy(pp, p, oldcount * sizeof(T));
      delete[] p;
    }

    return pp;
  }


  /*
   * Simple array class with optional bounds checking
   */  
  template <typename T, int size>
  class array {
  public:
    array() { }
    static const int length = size;

    T data[size];
    const T& operator [](int i) const { 
#ifdef CHECK_BOUNDS
      assert((i >= 0) && (i < size));
#endif
      return data[i]; 
    }

    T& operator [](int i) { 
#ifdef CHECK_BOUNDS
      assert((i >= 0) && (i < size));
#endif
      return data[i]; 
    }

    void clear() {
      foreach(i, size) data[i] = T();
    }
  };

  template <typename T, int size>
  inline ostream& operator <<(ostream& os, const array<T, size>& v) {
    os << "Array of ", size, " elements:", endl;
    for (int i = 0; i < size; i++) {
      os << "  [", i, "]: ", v[i], endl;
    }
    return os;
  }

  /*
   * Simple STL-like dynamic array class.
   */
  template <class T>
  class dynarray {
  protected:
  public:
    T* data;
    int length;
    int reserved;
    int granularity;
    
  public:
    inline T& operator [](int i) { return data[i]; }
    inline T operator [](int i) const { return data[i]; }

    // NOTE: g *must* be a power of two!
    dynarray() {
      length = reserved = 0;
      granularity = 16;
      data = NULL;
    }
    
    dynarray(int initcap, int g = 16) {
      length = 0;
      reserved = 0;
      granularity = g;
      data = NULL;
      reserve(initcap);
    }
    
    ~dynarray() {
      delete[] data;
      length = 0;
      reserved = 0;
    }
    
    inline int capacity() const { return reserved; }
    inline bool empty() const { return (size == 0); }
    inline void clear() { resize(0); }
    inline int size() const { return length; }
    
    void push(const T& obj) {
      T& pushed = push();
      pushed = obj;
    }
    
    T& push() {
      reserve(length + 1);
      length++;
      return data[length-1];
    }

    T& pop() {
      length--;
      return data[length];
    }   

    void resize(int newsize) {
      if (newsize > length) reserve(newsize);
      length = newsize;
    }

    void resize(int newsize, const T& emptyvalue) {
      int oldlength = length;
      resize(newsize);
      if (newsize <= oldlength) return;
      for (int i = oldlength; i < reserved; i++) { data[i] = emptyvalue; }
    }
    
    void reserve(int newsize) {
      if (newsize <= reserved) return;
      newsize = (newsize + (granularity-1)) & ~(granularity-1);
      data = renew(data, length, newsize);
      reserved = newsize;
    }
    
    void trim() {
      //++MTY FIXME
      //reserved = count;
      //data = (T*)realloc(data, count * sizeof(T));
      //data = renew(data, count, newsize);
    }
  };

  template <class T>
  inline ostream& operator <<(ostream& os, const dynarray<T>& v) {
    os << "Array of ", v.size(), " elements (", v.capacity(), " reserved): ", endl;
    for (int i = 0; i < v.size(); i++) {
      os << "  [", i, "]: ", v[i], endl;
    }
    return os;
  }

  /*
   * Simple type-safe temporary buffer with overflow protection.
   */
  template <class T>
  class tempbuf {
  protected:
  public:
    T* data;
    T* endp;
    T* base;

  public:
    inline T& operator [](int i) { return base[i]; }
    inline T operator [](int i) const { return base[i]; }
    inline operator T*() { return data; }
    T& operator ->() { return *data; }

    inline operator const T&() { return *data; }

    inline const T& operator =(const T& v) { *data = v; return *this; }
    //inline const T& operator +=(const T& v) { *data++ = v; return v; }
    inline T* operator +=(int n) { return (data += n); }
    inline T* operator -=(int n) { return (data -= n); }

    inline T* operator ++() { return ++data; }
    inline T* operator ++(int postfix) { return data++; }

    inline T* operator --() { return --data; }
    inline T* operator --(int postfix) { return data--; }

    tempbuf() {
      data = endp = base = NULL;
    }

    void free() {
      if (!base)
        return;
      assert(data <= endp);
      munmap(base, ((char*)endp - (char*)base) + PAGE_SIZE);
      base = endp = data = NULL;
    }

    ~tempbuf() {
      free();
    }

    void resize(int size) {
      free();

      int realsize = ceil(size * sizeof(T), PAGE_SIZE);
      
      base = (T*)mmap(NULL, realsize + 2*PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, 0, 0);
      assert(base != MAP_FAILED);

      base = (T*)((char*)base + PAGE_SIZE);
      endp = (T*)((char*)base + realsize);

      assert(mprotect(((char*)base) - PAGE_SIZE, PAGE_SIZE, PROT_NONE) == 0);
      assert(mprotect((char*)endp, PAGE_SIZE, PROT_NONE) == 0);
      data = base;
    }

    tempbuf(int size) {
      resize(size);
    }

    inline bool empty() const { return (data == base); }
    inline void clear() { data = base; }
    inline int capacity() const { return (endp - base); }
    inline int setcount(int newcount) {
      data = base + newcount; 
      return newcount;
    }
    inline int size() const { return (data - base); }
    inline int count() const { return (data - base); }
    inline operator bool() const { return empty(); }

    inline T* start() const { return base; }
    inline T* end() const { return data; }
    inline T* dup() const {
      T* t = new T[count()];
      memcpy(t, base, sizeof(T) * count());
      return t;
    }

    T* reserve(int n = 1) {
      T* p = data;
      data = data + n;
      if (data <= end)
        return p;

      data = p;
      return NULL;
    }

    const T& push(const T& obj) {
      *data++ = obj;
      return obj;
    }

    T& push() {
      return *data++;
    }

    T& pop() {
      return *data--;
    }
  };

  template <class T> inline const T& operator <<(tempbuf<T>& buf, const T& v) { return buf.push(v); }
  template <class T> inline const T& operator >>(tempbuf<T>& buf, T& v) { return (v = buf.pop()); }

  /*
   * CRC32
   */
  struct CRC32 {
    static const W32 crctable[256];
    W32 crc;
    
    inline W32 update(byte value) {
      crc = crctable[(crc ^ value) & 0xff] ^ (crc >> 8);
      return crc;
    }

    inline W32 update(byte* data, int count) {
      foreach (i, count) {
        update(data[i]);
      }
      return crc;
    }

    CRC32() {
      reset();
    }
    
    CRC32(W32 newcrc) {
      reset(newcrc);
    }
    
    inline void reset(W32 newcrc = 0xffffffff) {
      crc = newcrc;
    }

    operator W32() const {
      return crc;
    }
  };

  template <typename T>
  CRC32& operator <<(CRC32& crc, const T& t) {
    crc.update((byte*)&t, sizeof(T));
    return crc;
  }

  template <class T>
  inline CRC32& operator ,(CRC32& crc, const T& v) {
    return crc << v;
  }

  /*
   * listlink class
   * Double linked list with self-referential operations from single root
   */
  template <class T>
  class listlink {
  public:
    listlink<T>* next;
    listlink<T>** prevnp;
    T* data;
  public:
    void reset() { next = null; prevnp = null; data = null; }
    listlink() { reset(); data = null; }
    listlink(const T& t) { reset(); data = &t; }
    listlink(const T* t) { reset(); data = t; }
    listlink<T>& operator ()(T* t) { data = t; return *this; }

    T& unlink() {
      if (prevnp) *prevnp = next;
      if (next) next->prevnp = prevnp;
      prevnp = NULL;
      next = NULL;  
      return *data;
    }

    void addto(listlink<T>*& root) {
      this->next = root;
      if (root)
        root->prevnp = &this->next;
      root = this;
      this->prevnp = &root;
    }

    bool linked() const {
      return (next || prevnp);
    }

    bool unlinked() const {
      return !linked();
    }

    //T* operator->*() const { return data; }
    T* operator->() const { return data; }
    operator T*() const { return data; }
    operator T&() const { return *data; }
  };

  class selfqueuelink {
  public:
    selfqueuelink* next;
    selfqueuelink* prev;
  public:
    void reset() { next = this; prev = this; }
    selfqueuelink() { }

    selfqueuelink& unlink() {
      // No effect if next = prev = this (i.e., unlinked)
      next->prev = prev;
      prev->next = next;
      prev = this;
      next = this;
      return *this;
    }

    void addhead(selfqueuelink& root) {
      addlink(&root, root.next);
    }

    void addhead(selfqueuelink* root) {
      addhead(*root);
    }

    void addto(selfqueuelink& root) {
      addhead(root);
    }

    void addto(selfqueuelink* root) {
      addto(*root);
    }

    void addtail(selfqueuelink& root) {
      addlink(root.prev, &root);
    }

    void addtail(selfqueuelink* root) {
      addtail(*root);
    }

    selfqueuelink* removehead() {
      if (empty()) return null;
      selfqueuelink* link = next;
      link->unlink();
      return link;
    }

    selfqueuelink* removetail() {
      if (empty()) return null;
      selfqueuelink* link = prev;
      link->unlink();
      return link;
    }

    selfqueuelink* head() const {
      return next;
    }

    selfqueuelink* tail() const {
      return prev;
    }

    bool empty() const {
      return (next == this);
    }

    bool unlinked() const {
      return ((!prev && !next) || ((prev == this) && (next == this)));
    }

    bool linked() const {
      return !unlinked();
    }

    operator bool() const { return (!empty()); }

  protected:
    void addlink(selfqueuelink* prev, selfqueuelink* next) {
      next->prev = this;
      this->next = next;
      this->prev = prev;
      prev->next = this;      
    }
  };

  template <class T>
  class queuelink {
  public:
    queuelink<T>* next;
    queuelink<T>* prev;
    T* data;
  public:
    void reset() { next = this; prev = this; data = null; }
    queuelink() { }
    queuelink(const T& t) { reset(); data = &t; }
    queuelink(const T* t) { reset(); data = t; }
    queuelink<T>& operator ()(T* t) { reset(); data = t; return *this; }

    T& unlink() {
      // No effect if next = prev = this (i.e., unlinked)
      next->prev = prev;
      prev->next = next;
      prev = this;
      next = this;
      return *data;
    }

    void addhead(queuelink<T>& root) {
      addlink(&root, root.next);
    }

    void addto(queuelink<T>& root) {
      addhead(root);
    }

    void addtail(queuelink<T>& root) {
      addlink(root.prev, &root);
    }

    queuelink<T>* removehead() {
      queuelink<T>* link = next;
      link->unlink();
      return link;
    }

    queuelink<T>* removetail() {
      queuelink<T>* link = prev;
      link->unlink();
      return link;
    }

    queuelink<T>* head() const {
      return next;
    }

    queuelink<T>* tail() const {
      return prev;
    }

    bool empty() const {
      return (next == this);
    }

    bool unlinked() const {
      return ((!prev && !next) || ((prev == this) && (next == this)));
    }

    bool linked() const {
      return !unlinked();
    }

    operator bool() const { return (!empty()); }

    T* operator->() const { return data; }
    operator T*() const { return data; }
    operator T&() const { return *data; }

  protected:
    void addlink(queuelink<T>* prev, queuelink<T>* next) {
      next->prev = this;
      this->next = next;
      this->prev = prev;
      prev->next = this;      
    }
  };

  //
  // Index References (indexrefs) work exactly like pointers but always
  // index into a specific structure. This saves considerable space and
  // can allow aliasing optimizations not possible with pointers.
  //
  
  // null allowed:
  template <typename T>
  struct indexrefnull {
    W16s index;

    indexrefnull() { }

    indexrefnull<T>& operator =(const T& obj) { 
      index = (&obj) ? obj.index() : -1;
      return *this;
    }

    indexrefnull<T>& operator =(const T* obj) {
      index = (obj) ? obj->index() : -1;
      return *this;
    }

    indexrefnull<T>& operator =(int i) {
      index = i;
      return *this;
    }

    T* operator ->() const {
      return (index >= 0) ? &(get(index)) : null;
    }

    T& operator *() const {
      return (index >= 0) ? get(index) : *(T*)null;
    }

    operator T*() const { return &(get(index)); }

    T& get(int index) const;
  };

  template <typename T>
  struct indexref {
    W16s index;

    indexref() { }

    indexref<T>& operator =(const T& obj) { 
      index = obj.index();
      return *this;
    }

    indexref<T>& operator =(const T* obj) {
      index = obj->index();
      return *this;
    }

    indexref<T>& operator =(int i) {
      index = i;
      return *this;
    }

    T* operator ->() const {
      return &(get(index));
    }

    T& operator *() const {
      return get(index);
    }

    operator T*() const { return &(get(index)); }

    T& get(int index) const;
  };

  //
  // Convenient list iterator
  //
#define foreachlink(list, type, iter) \
  for (type* iter = (type*)((list)->first); (iter != NULL); prefetch(iter->next), iter = (type*)(iter->next)) \

  template <typename A, typename B>
  struct KeyValuePair {
    A key;
    B value;
  };

  template <typename K>
  struct HashtableKeyManager {
    static inline int hash(K key);
    static inline bool equal(K a, K b);
    static inline K dup(K key);
    static inline void free(K key);
  };

  template <typename K, typename T, int setcount = 64, typename KM = HashtableKeyManager<K> >
  class Hashtable {
  protected:
    struct Entry: public KeyValuePair<K, T> {
      listlink<Entry> link;

      Entry() { }

      Entry(const K key, const T& value) {
        link(this);
        this->key = KM::dup(key);
        this->value = value;
      }

      ~Entry() {
        link.unlink();
        KM::free(this->key);
      }
    };

    listlink<Entry>* sets[setcount];

    Entry* findentry(const K key) {
      listlink<Entry>* tlink = sets[lowbits(KM::hash(key), log2(setcount))];
      while (tlink) {
        Entry* entry = tlink->data;
        if (KM::equal(entry->key, key)) return entry;
        tlink = tlink->next;
      }

      return null;
    }

  public:
    int count;

    dynarray<KeyValuePair<K, T> >& getentries() const {
      dynarray<KeyValuePair<K, T> >& a = *(new dynarray<KeyValuePair<K, T> >(count, 1));
      a.resize(count);
      int n = 0;
      foreach (i, setcount) {
        listlink<Entry>* tlink = sets[i];
        while (tlink) {
          assert(n < count);
          Entry* entry = tlink->data;
          KeyValuePair<K, T>& kvp = a[n];
          kvp.key = entry->key;
          kvp.value = entry->value;
          tlink = tlink->next;
          n++;
        }
      }
      return a;
    }

    Hashtable() {
      count = 0;
      foreach (i, setcount) { sets[i] = null; }
    }

    void clear() {
      foreach (i, setcount) {
        listlink<Entry>* tlink = sets[i];
        while (tlink) {
          listlink<Entry>* tnext = tlink->next;
          delete tlink->data;
          tlink = tnext;
        }
        sets[i] = null;
      }
      count = 0;
    }

    T* get(const K key) {
      Entry* entry = findentry(key);
      if (!entry) return null;
      return &entry->value;
    }

    T* operator ()(const K key) {
      return get(key);
    }

    T add(const K key, const T& value) { 
      Entry* entry = findentry(key);
      if (entry) {
        T oldvalue = entry->value;
        entry->value = value;
        return oldvalue;
      }

      entry = new Entry(key, value);
      entry->link.addto(sets[lowbits(KM::hash(key), log2(setcount))]);
      count++;
      return 0;
    }

    T remove(const K key) {
      Entry* entry = findentry(key);
      if (!entry) return false;

      T value = entry->value;
      delete entry;
      count--;
      return value;
    }

    ostream& print(ostream& os) const {
      os << "Hashtable of ", setcount, " sets containing ", count, " entries:", endl;
      foreach (i, setcount) {
        listlink<Entry>* tlink = sets[i];
        if (!tlink)
          continue;
        os << "  Set ", i, ":", endl;
        int n = 0;
        while (tlink) {
          Entry* entry = tlink->data;
          os << "    ", entry->key, " -> ", entry->value, endl;
          tlink = tlink->next;
          n++;
        }
      }
      return os;
    }
  };

  template <typename K, typename T, int setcount, typename KM>
  ostream& operator <<(ostream& os, const Hashtable<K, T, setcount, KM>& ht) {
    return ht.print(os);
  }

  //
  // HashtableKeyManager<const char*>
  //
  inline int HashtableKeyManager<const char*>::hash(const char* key) {
    int len = strlen(key);
    CRC32 h;
    
    foreach (i, len) { h << key[i]; }

    return h;
  }

  inline bool HashtableKeyManager<const char*>::equal(const char* a, const char* b) {
    return (strcmp(a, b) == 0);
  }

  inline const char* HashtableKeyManager<const char*>::dup(const char* key) {
    return strdup(key);
  }

  inline void HashtableKeyManager<const char*>::free(const char* key) {
    ::free((void*)key);
  }

  //
  // HashtableKeyManager<W64>
  //
  inline int HashtableKeyManager<W64>::hash(W64 key) {
    return key;
  }

  inline bool HashtableKeyManager<W64>::equal(W64 a, W64 b) {
    return (a == b);
  }

  inline W64 HashtableKeyManager<W64>::dup(W64 key) {
    return key;
  }

  inline void HashtableKeyManager<W64>::free(W64 key) { }

  //
  // ChunkHashtable
  //
  template <typename T, int entries_per_chunk>
  struct ChunkHashtableBlock {
    listlink< ChunkHashtableBlock<T, entries_per_chunk> > link;
    W64 freemap;
    T data[entries_per_chunk];
    static const W64 ALL_FREE = (1LL << entries_per_chunk)-1LL;

    ChunkHashtableBlock() {
      link(this);
      setzero(data);
      freemap = ALL_FREE;
    }

    bool full() const {
      return (freemap == 0);
    }

    bool add(const T& entry) {
      int idx = lsbindex64(freemap);
      if ((!freemap) || (idx >= entries_per_chunk)) return false;
      freemap &= ~(1LL << idx);
      data[idx] = entry;
      return true;
    }

    T* operator ()(const T& entry) {
      W64 match = 0;
      foreach (i, entries_per_chunk) {
        match |= (W64)(data[i] == entry) << (W64)i;
      }
      match &= ~freemap;
      int idx = lsbindex64(match);
      return (match) ? &data[idx] : null;
    }

    bool remove(const T& entry) {
      T* slot = (*this)(entry);
      if (!slot) return (freemap == ALL_FREE);
      int idx = (slot - data); 
      freemap |= (1LL << idx);
      return (freemap == ALL_FREE);
    }

    void print(ostream& os) const {
      os << "    ChunkHashtableBlock<", entries_per_chunk, ">: prev ", link.prevnp, ", next ", link.next, ", freemap ", bitstring(freemap, entries_per_chunk, true), ":", endl;
      foreach (i, entries_per_chunk) {
        if (!bit(freemap, i)) os << "      ", intstring(i, 2), ": ", data[i], endl;
      }
    }
  };

  template <typename T, int entries_per_chunk>
  ostream& operator <<(ostream& os, ChunkHashtableBlock<T, entries_per_chunk>& chunk) {
    chunk.print(os);
    return os;
  }

#define ChunkHashtableBlock_fit_in_bytes(T, bytes) ((bytes - (sizeof(ChunkHashtableBlock<W64, 1>) - sizeof(W64))) / sizeof(T))

  template <typename T, int setcount, int entries_per_chunk>
  struct ChunkHashtable {
    typedef ChunkHashtableBlock<T, entries_per_chunk> chunk_t;
    int count;
    listlink<chunk_t>* sets[setcount];

    int setof(const T& entry); // (implement in subclasses)

    void add(const T& entry) {
      chunk_t* base = (chunk_t*)sets[setof(entry)];
      if ((!base) || base->full()) {
        base = new chunk_t();
        base->link.addto(sets[setof(entry)]);
      }

      assert(base->add(entry));
    }

    void remove(const T& entry) {
      chunk_t* chunk = (chunk_t*)sets[setof(entry)];
      while (chunk) {
        chunk_t* nextchunk = (chunk_t*)chunk->link.next;
        bool empty = chunk->remove(entry);
        if (empty) {
          chunk->link.unlink();
          delete chunk;
        }
        chunk = nextchunk;
      }
    }

    T* operator ()(const T& entry) {
      chunk_t* chunk = (chunk_t*)sets[setof(entry)];
      T* found = null;
      while (chunk) {
        found = (*chunk)(entry);
        if (found) break;
        chunk = (chunk_t*)chunk->link.next;
      }
      return found;
    }

    ChunkHashtable() {
      init();
    }

    void init() {
      // Free all traces:
      foreach (i, setcount) {
        sets[i] = null;
      }
      count = 0;
    }

    void clear() {
      foreach (i, setcount) {
        chunk_t* chunk = (chunk_t*)sets[i];
        while (chunk) {
          chunk_t* nextchunk = (chunk_t*)chunk->link.next;
          chunk->link.unlink();
          delete chunk;
          chunk = nextchunk;
        }
        sets[i] = null;
      }
      count = 0;
    }

    void print(ostream& os) {
      os << "ChunkHashtable<", setcount, " sets, ", entries_per_chunk, " entries per chunk>: ", count, " entries:", endl;
      foreach (i, setcount) {
        if (!sets[i]) continue;
        os << "  Set ", i, endl;
        chunk_t* chunk = (chunk_t*)sets[i];
        T* found = null;
        while (chunk) {
          os << (*chunk);
          chunk = (chunk_t*)chunk->link.next;
        }
      }
    }
  };

  template <typename T, int setcount, int entries_per_chunk>
  ostream& operator <<(ostream& os, ChunkHashtable<T, setcount, entries_per_chunk>& cht) {
    cht.print(os);
    return os;
  }

#define BITS_PER_WORD ((sizeof(unsigned long) == 8) ? 64 : 32)
#define BITVEC_WORDS(n) ((n) < 1 ? 0 : ((n) + BITS_PER_WORD - 1)/BITS_PER_WORD)

#ifdef __x86_64__
#define __builtin_ctzl(t) lsbindex64(t)
#define __builtin_clzl(t) msbindex64(t)
#else
#define __builtin_ctzl(t) lsbindex32(t)
#define __builtin_clzl(t) msbindex32(t)
#endif

  template<size_t N>
  struct bitvecbase {
    typedef unsigned long T;

    T w[N];

    bitvecbase() { resetop(); }

    bitvecbase(const bitvecbase<N>& vec) { foreach (i, N) w[i] = vec.w[i]; }

    bitvecbase(unsigned long long val) {
      resetop();
      w[0] = val;
    }

    static size_t wordof(size_t index) { return index / BITS_PER_WORD; }
    static size_t byteof(size_t index) { return (index % BITS_PER_WORD) / __CHAR_BIT__; }
    static size_t bitof(size_t index) { return index % BITS_PER_WORD; }
    static T maskof(size_t index) { return (static_cast<T>(1)) << bitof(index); }

    T& getword(size_t index) { return w[wordof(index)]; }
    T getword(size_t index) const { return w[wordof(index)]; }
    T& hiword() { return w[N - 1]; }
    T hiword() const { return w[N - 1]; }

    void andop(const bitvecbase<N>& x) {
      for (size_t i = 0; i < N; i++) w[i] &= x.w[i];
    }

    void orop(const bitvecbase<N>& x) {
      foreach (i, N) w[i] |= x.w[i];
    }

    void xorop(const bitvecbase<N>& x) {
      foreach (i, N) w[i] ^= x.w[i];
    }

    void shiftleftop(size_t shift) {
      if (__builtin_expect(shift != 0, 1)) {
        const size_t wshift = shift / BITS_PER_WORD;
        const size_t offset = shift % BITS_PER_WORD;
    
        if (offset == 0) {
          for (size_t i = N - 1; i >= wshift; --i) { w[i] = w[i - wshift]; }
        } else {
          const size_t suboffset = BITS_PER_WORD - offset;
          for (size_t i = N - 1; i > wshift; --i) { w[i] = (w[i - wshift] << offset) | (w[i - wshift - 1] >> suboffset); }
          w[wshift] = w[0] << offset;
        }

        // memset(w, static_cast<T>(0), wshift);
        foreach (i, wshift) { w[i] = 0; }
      }
    }

    void shiftrightop(size_t shift) {
      if (__builtin_expect(shift != 0, 1)) {
        const size_t wshift = shift / BITS_PER_WORD;
        const size_t offset = shift % BITS_PER_WORD;
        const size_t limit = N - wshift - 1;
      
        if (offset == 0) {
          for (size_t i = 0; i <= limit; ++i) { w[i] = w[i + wshift]; }
        } else {
          const size_t suboffset = BITS_PER_WORD - offset;
          for (size_t i = 0; i < limit; ++i) { w[i] = (w[i + wshift] >> offset) | (w[i + wshift + 1] << suboffset); }
          w[limit] = w[N-1] >> offset;
        }

        //memset(w + limit + 1, static_cast<T>(0), N - (limit + 1));
        foreach (i, N - (limit + 1)) { w[limit + 1 + i] = 0; }
      }
    }

    void maskop(size_t count) {
      if (bitof(count))
        w[wordof(count)] &= bitmask(bitof(count));

      for (size_t i = wordof(count)+1; i < N; i++) {
        w[i] = 0;
      }
    }

    void invertop() {
      foreach (i, N) w[i] = ~w[i];
    }

    void setallop() {
      foreach (i, N) w[i] = ~static_cast<T>(0);
    }

    void resetop() { memset(w, 0, N * sizeof(T)); }

    bool equalop(const bitvecbase<N>& x) const {
      T t = 0;
      foreach (i, N) { t |= (w[i] ^ x.w[i]); }
      return (t == 0);
    }

    bool nonzeroop() const {
      T t = 0;
      foreach (i, N) { t |= w[i]; }
      return (t != 0);
    }

    size_t popcountop() const {
      size_t result = 0;
      foreach (i, N)
        result += __builtin_popcountl(w[i]);
      return result;
    }

    unsigned long integerop() const { return w[0]; }

    void insertop(size_t i, size_t n, T v) {
      T& lw = w[wordof(i)];
      T lm = (bitmask(n) << bitof(i));
      lw = (lw & ~lm) | ((v << i) & lm);

      if ((bitof(i) + n) > BITS_PER_WORD) {
        T& hw = w[wordof(i+1)];
        T hm = (bitmask(n) >> (BITS_PER_WORD - bitof(i)));
        hw = (hw & ~hm) | ((v >> (BITS_PER_WORD - bitof(i))) & hm);
      }
    }

    void accumop(size_t i, size_t n, T v) {
      w[wordof(i)] |= (v << i);

      if ((bitof(i) + n) > BITS_PER_WORD)
        w[wordof(i+1)] |= (v >> (BITS_PER_WORD - bitof(i)));
    }

    // find index of first "1" bit starting from low end
    size_t lsbop(size_t notfound) const {
      foreach (i, N) {
        T t = w[i];
        if (t) return (i * BITS_PER_WORD) + __builtin_ctzl(t);
      }
      return notfound;
    }

    // find index of last "1" bit starting from high end
    size_t msbop(size_t notfound) const {
      for (int i = N-1; i >= 0; i--) {
        T t = w[i];
        if (t) return (i * BITS_PER_WORD) + __builtin_clzl(t);
      }
      return notfound;
    }

    // assume value is nonzero
    size_t lsbop() const {
      return lsbop(0);
    }

    // assume value is nonzero
    size_t msbop() const {
      return msbop(0);
    }

    // find the next "on" bit that follows "prev"

    size_t nextlsbop(size_t prev, size_t notfound) const {
      // make bound inclusive
      ++prev;

      // check out of bounds
      if (prev >= N * BITS_PER_WORD)
        return notfound;

      // search first word
      size_t i = wordof(prev);
      T t = w[i];

      // mask off bits below bound
      t &= (~static_cast<T>(0)) << bitof(prev);

      if (t != static_cast<T>(0))
        return (i * BITS_PER_WORD) + __builtin_ctzl(t);

      // check subsequent words
      i++;
      for ( ; i < N; i++ ) {
        t = w[i];
        if (t != static_cast<T>(0))
          return (i * BITS_PER_WORD) + __builtin_ctzl(t);
      }
      // not found, so return an indication of failure.
      return notfound;
    }
  };

  template <>
  struct bitvecbase<1> {
    typedef unsigned long T;
    T w;

    bitvecbase(void): w(0) {}
    bitvecbase(unsigned long long val): w(val) {}

    static size_t wordof(size_t index) { return index / BITS_PER_WORD; }
    static size_t byteof(size_t index) { return (index % BITS_PER_WORD) / __CHAR_BIT__; }
    static size_t bitof(size_t index) { return index % BITS_PER_WORD; }
    static T maskof(size_t index) { return (static_cast<T>(1)) << bitof(index); }

    T& getword(size_t) { return w; }
    T getword(size_t) const { return w; }
    T& hiword() { return w; }
    T hiword() const { return w; }
    void andop(const bitvecbase<1>& x) { w &= x.w; }
    void orop(const bitvecbase<1>& x)  { w |= x.w; }
    void xorop(const bitvecbase<1>& x) { w ^= x.w; }
    void shiftleftop(size_t __shift) { w <<= __shift; }
    void shiftrightop(size_t __shift) { w >>= __shift; }
    void invertop() { w = ~w; }
    void setallop() { w = ~static_cast<T>(0); }
    void resetop() { w = 0; }
    bool equalop(const bitvecbase<1>& x) const { return w == x.w; }
    bool nonzeroop() const { return (!!w); }
    size_t popcountop() const { return __builtin_popcountl(w); }
    unsigned long integerop() const { return w; }
    size_t lsbop() const { return __builtin_ctzl(w); }
    size_t msbop() const { return __builtin_clzl(w); }
    size_t lsbop(size_t notfound) const { return (w) ? __builtin_ctzl(w) : notfound; }
    size_t msbop(size_t notfound) const { return (w) ? __builtin_clzl(w) : notfound; }
    void maskop(size_t count) { w &= bitmask(bitof(count)); }

    void insertop(size_t i, size_t n, T v) {
      T m = (bitmask(n) << bitof(i));
      w = (w & ~m) | ((v << i) & m);
    }

    void accumop(size_t i, size_t n, T v) {
      w |= (v << i);
    }

    // find the next "on" bit that follows "prev"
    size_t nextlsbop(size_t __prev, size_t notfound) const {
      ++__prev;
      if (__prev >= ((size_t) BITS_PER_WORD))
        return notfound;

      T x = w >> __prev;
      if (x != 0)
        return __builtin_ctzl(x) + __prev;
      else
        return notfound;
    }
  };

  template <>
  struct bitvecbase<0> {
    typedef unsigned long T;

    bitvecbase() { }
    bitvecbase(unsigned long long) { }

    static size_t wordof(size_t index) { return index / BITS_PER_WORD; }
    static size_t byteof(size_t index) { return (index % BITS_PER_WORD) / __CHAR_BIT__; }
    static size_t bitof(size_t index) { return index % BITS_PER_WORD; }
    static T maskof(size_t index) { return (static_cast<T>(1)) << bitof(index); }

    T& getword(size_t) const { return *new T;  }
    T hiword() const { return 0; }
    void andop(const bitvecbase<0>&) { }
    void orop(const bitvecbase<0>&)  { }
    void xorop(const bitvecbase<0>&) { }
    void shiftleftop(size_t) { }
    void shiftrightop(size_t) { }
    void invertop() { }
    void setallop() { }
    void resetop() { }
    bool equalop(const bitvecbase<0>&) const { return true; }
    bool nonzeroop() const { return false; }
    size_t popcountop() const { return 0; }
    void maskop(size_t count) { }
    void accumop(int i, int n, T v) { }
    void insertop(int i, int n, T v) { }
    unsigned long integerop() const { return 0; }
    size_t lsbop() const { return 0; }
    size_t msbop() const { return 0; }
    size_t lsbop(size_t notfound) const { return notfound; }
    size_t msbop(size_t notfound) const { return notfound; }
    size_t nextlsbop(size_t, size_t) const { return 0; }
  };

  // Helper class to zero out the unused high-order bits in the highest word.
  template <size_t extrabits>
  struct bitvec_sanitizer {
    static void sanitize(unsigned long& val) { 
      val &= ~((~static_cast<unsigned long>(0)) << extrabits); 
    }
  };

  template <>
  struct bitvec_sanitizer<0> { 
    static void sanitize(unsigned long) { }
  };

  template<size_t N>
  class bitvec: private bitvecbase<BITVEC_WORDS(N)> {
  private:
    typedef bitvecbase<BITVEC_WORDS(N)> base_t;
    typedef unsigned long T;

    bitvec<N>& sanitize() {
      bitvec_sanitizer<N % BITS_PER_WORD>::sanitize(this->hiword());
      return *this;
    }

  public:
    class reference {
      friend class bitvec;

      T *wp;
      size_t bpos;

      // left undefined
      reference();

    public:
      reference(bitvec& __b, size_t index) {
        wp = &__b.getword(index);
        bpos = base_t::bitof(index);
      }

      ~reference() { }

      // For b[i] = x;
      reference& operator=(bool x) {
        if (x)
          *wp |= base_t::maskof(bpos);
        else
          *wp &= ~base_t::maskof(bpos);
        return *this;
      }

      // For b[i] = b[j];
      reference& operator=(const reference& j) {
        if ((*(j.wp) & base_t::maskof(j.bpos)))
          *wp |= base_t::maskof(bpos);
        else
          *wp &= ~base_t::maskof(bpos);
        return *this;
      }

      // For b[i] = 1;
      reference& operator++(int postfixdummy) {
        *wp |= base_t::maskof(bpos);
        return *this;
      }

      // For b[i] = 0;
      reference& operator--(int postfixdummy) {
        *wp &= ~base_t::maskof(bpos);
        return *this;
      }

      // Flips the bit
      bool operator~() const { return (*(wp) & base_t::maskof(bpos)) == 0; }

      // For x = b[i];
      operator bool() const { return (*(wp) & base_t::maskof(bpos)) != 0; }

      // For b[i].invert();
      reference& invert() {
        *wp ^= base_t::maskof(bpos);
        return *this;
      }
    };

    friend class reference;

    bitvec() { }

    bitvec(const bitvec<N>& vec): base_t(vec) { }

    bitvec(unsigned long long val): base_t(val) { sanitize(); }

    bitvec<N>& operator&=(const bitvec<N>& rhs) {
      this->andop(rhs);
      return *this;
    }

    bitvec<N>& operator|=(const bitvec<N>& rhs) {
      this->orop(rhs);
      return *this;
    }

    bitvec<N>& operator^=(const bitvec<N>& rhs) {
      this->xorop(rhs);
      return *this;
    }

    bitvec<N>& operator <<=(int index) {
      if (__builtin_expect(index < N, 1)) {
        this->shiftleftop(index);
        this->sanitize();
      } else this->resetop();
      return *this;
    }

    bitvec<N>& operator>>=(int index) {
      if (__builtin_expect(index < N, 1)) {
        this->shiftrightop(index);
        this->sanitize();
      } else this->resetop();
      return *this;
    }

    bitvec<N>& set(size_t index) {
      this->getword(index) |= base_t::maskof(index);
      return *this;
    }

    bitvec<N>& reset(size_t index) {
      this->getword(index) &= ~base_t::maskof(index);
      return *this;
    }

    bitvec<N>& assign(size_t index, int val) {
      if (val)
        this->getword(index) |= base_t::maskof(index);
      else
        this->getword(index) &= ~base_t::maskof(index);
      return *this;
    }

    bitvec<N>& invert(size_t index) {
      this->getword(index) ^= base_t::maskof(index);
      return *this;
    }

    bool test(size_t index) const {
      return (this->getword(index) & base_t::maskof(index)) != static_cast<T>(0);
    }

    bitvec<N>& setall() {
      this->setallop();
      this->sanitize();
      return *this;
    }

    bitvec<N>& reset() {
      this->resetop();
      return *this;
    }

    bitvec<N>& operator++(int postfixdummy) { return setall(); }
    bitvec<N>& operator--(int postfixdummy) { return reset(); }

    bitvec<N>& invert() {
      this->invertop();
      this->sanitize();
      return *this;
    }

    bitvec<N> operator ~() const { return bitvec<N>(*this).invert(); }

    reference operator [](size_t index) { return reference(*this, index); }

    bool operator [](size_t index) const { return test(index); }

    bool operator *() const { return nonzero(); }
    bool operator !() const { return iszero(); }

    unsigned long integer() const { return this->integerop(); }

    // Returns the number of bits which are set.
    size_t popcount() const { return this->popcountop(); }

    // Returns the total number of bits.
    size_t size() const { return N; }

    bool operator ==(const bitvec<N>& rhs) const { return this->equalop(rhs); }
    bool operator !=(const bitvec<N>& rhs) const { return !this->equalop(rhs); }
    bool nonzero() const { return this->nonzeroop(); }
    bool iszero() const { return !this->nonzeroop(); }
    bool allset() const { return (~(*this)).iszero(); }
    bool all() const { return allset(N); }

    bitvec<N> operator <<(size_t shift) const { return bitvec<N>(*this) <<= shift; }

    bitvec<N> operator >>(size_t shift) const { return bitvec<N>(*this) >>= shift; }

    size_t lsb() const { return this->lsbop(); }
    size_t msb() const { return this->msbop(); }
    size_t lsb(int notfound) const { return this->lsbop(notfound); }
    size_t msb(int notfound) const { return this->msbop(notfound); }
    size_t nextlsb(size_t prev, int notfound = -1) const { return this->nextlsbop(prev, notfound); }

    bitvec<N> insert(int i, int n, T v) const {
      bitvec<N> b(*this);
      b.insertop(i, n, v);
      b.sanitize();
      return b;
    }

    bitvec<N> accum(size_t i, size_t n, T v) const {
      bitvec<N> b(*this);
      b.accumop(i, n, v);
      return b;
    }

    bitvec<N> mask(size_t count) const { 
      bitvec<N> b(*this);
      b.maskop(count);
      return b;
    }

    bitvec<N> operator %(size_t b) {
      return mask(b);
    }
 
    bitvec<N> extract(size_t index, size_t count) const {
      return (bitvec<N>(*this) >> index) % count;
    }

    bitvec<N> operator ()(size_t index, size_t count) {
      return extract(index, count);
    }

    bitvec<N> operator &(const bitvec<N>& y) {
      return bitvec<N>(*this) &= y;
    }

    bitvec<N> operator |(const bitvec<N>& y) {
      return bitvec<N>(*this) |= y;
    }

    bitvec<N> operator ^(const bitvec<N>& y) {
      return bitvec<N>(*this) ^= y;
    }

    bitvec<N> remove(size_t index, size_t count = 1) {
      return (((*this) >> (index + count)) << index) | ((*this) % index);
    }

    template <int S> bitvec<S> subset(int i) const {
      return bitvec<S>((*this) >> i);
    }

    // This introduces ambiguity:
    // explicit operator unsigned long long() const { return integer(); }

    ostream& print(ostream& os) const {
      foreach (i, N) {
        os << (((*this)[i]) ? '1' : '0');
      }
      return os;
    }

    stringbuf& print(stringbuf& sb) const {
      foreach (i, N) {
        sb << (((*this)[i]) ? '1' : '0');
      }
      return sb;
    }
  };

  template <size_t N>
  inline ostream& operator <<(ostream& os, const bitvec<N>& v) {
    return v.print(os);
  }

  template <size_t N>
  inline stringbuf& operator <<(stringbuf& sb, const bitvec<N>& v) {
    return v.print(sb);
  }

#undef BITVEC_WORDS
#undef BITS_PER_WORD
#undef __builtin_ctzl
#undef __builtin_clzl

  class CycleTimer {
  public:
    CycleTimer() { total = 0; tstart = 0; iterations = 0; title = "(generic)"; running = 0; }
    CycleTimer(const char* title) { total = 0; tstart = 0; iterations = 1; this->title = title; running = 0; }

    inline void start() { W64 t = rdtsc(); if (running) return; iterations++; tstart = t; running = 1; }
    inline W64 stop() {
      W64 t = rdtsc() - tstart;

      if (!running) return total;

      tstart = 0;
      total += t;
      running = 0;
      return t;
    }

    inline W64 cycles() const {
      return total;
    }

    inline double seconds() const {
      return (double)total / hz;
    }

  public:
    W64 total;
    W64 tstart;
    int iterations;
    const char* title;
    bool running;

    static inline W64 rdtsc() {
      W32 lo, hi;
      asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
      return ((W64)lo) | (((W64)hi) << 32);
    }

    static double gethz();

  protected:
    static double hz;
  };

  ostream& operator <<(ostream& os, const CycleTimer& ct);

} // namespace superstl

#endif // _SUPERSTL_H