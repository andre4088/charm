/*****************************************************************************
 * $Source$
 * $Author$
 * $Date$
 * $Revision$
 *****************************************************************************/

/**
 * \addtogroup CkPerf
*/
/*@{*/

#ifndef _PROJECTIONS_H
#define _PROJECTIONS_H

#include "trace.h"
#include "ck.h"
#include "stdio.h"
#include "errno.h"

#if CMK_PROJECTIONS_USE_ZLIB
#include <zlib.h>
#endif

#include "trace-common.h"
#include "trace-projector.h"

#define PROJECTION_VERSION  "5.0"

/// a log entry in trace projection
class LogEntry {
  public:
    double time;
    int event;
    int pe;
    UShort mIdx;
    UShort eIdx;
    UChar type; 
    int msglen;
    double recvTime;
    CmiObjId   id;
  public:
    LogEntry() {}
    LogEntry(double tm, UChar t, UShort m=0, UShort e=0, int ev=0, int p=0, int ml=0, CmiObjId *d=NULL, double rt=0.) {
      type = t; mIdx = m; eIdx = e; event = ev; pe = p; time = tm; msglen = ml;
      if (d) id = *d; else {id.id[0]=id.id[1]=id.id[2]=0; };
      recvTime = rt; 
    }
    void *operator new(size_t s) {void*ret=malloc(s);_MEMCHECK(ret);return ret;}
    void *operator new(size_t, void *ptr) { return ptr; }
    void operator delete(void *ptr) { free(ptr); }
#ifdef WIN32
    void operator delete(void *, void *) { }
#endif
    // **CW** For backward compatibility, the previous signature is provided.
    void write(FILE *fp);
    // **CW** Simple delta encoding implementation
    double write(FILE *fp, double prevTime, double *timeErr);
    void writeBinary(FILE *fp);
#if CMK_PROJECTIONS_USE_ZLIB
    // **CW** for backward compatibility, the previous signature is provided.
    void writeCompressed(gzFile fp);
    // **CW** Simple delta encoding implementation
    double writeCompressed(gzFile fp, double prevTime, double *timeErr);
#endif
};

/// log pool in trace projection
class LogPool {
  friend class TraceProjections;
  private:
    UInt poolSize;
    UInt numEntries;
    LogEntry *pool;
    FILE *fp;
    FILE *deltafp;
    FILE *stsfp;
    char *fname;
    char *dfname;
    char *pgmname;
    int binary;
#if CMK_PROJECTIONS_USE_ZLIB
    gzFile deltazfp;
    gzFile zfp;
    int compressed;
#endif
    // **CW** prevTime stores the timestamp of the last event
    // written out to log. This allows the implementation of
    // simple delta encoding and should only be used when
    // writing out logs.
    double prevTime;
    double timeErr;
  public:
    LogPool(char *pgm);
    ~LogPool();
    void setBinary(int b) { binary = b; }
#if CMK_PROJECTIONS_USE_ZLIB
    void setCompressed(int c) { compressed = c; }
#endif
    void creatFiles(char *fix="");
    void openLog(const char *mode);
    void closeLog(void);
    void writeLog(void);
    void write(void);
    void writeBinary(void);
#if CMK_PROJECTIONS_USE_ZLIB
    void writeCompressed(void);
#endif
    void writeSts(void);
    void add(UChar type,UShort mIdx,UShort eIdx,double time,int event,int pe, int ml=0, CmiObjId* id=0, double recvT=0.);
    void postProcessLog();
};

/// class for recording trace projections events 
/**
  TraceProjections will log Converse/Charm++ events and write into .log files;
  events descriptions will be written into .sts file.
*/
class TraceProjections : public Trace {
    LogPool* _logPool;        /**<  logpool for all events */
    int curevent;
    int execEvent;
    int execEp;
    int execPe;
    int isIdle;
    int cancel_beginIdle, cancel_endIdle;
  public:
    TraceProjections(char **argv);
    void userEvent(int e);
    void userBracketEvent(int e, double bt, double et);
    void creation(envelope *e, int epIdx, int num=1);
    void creationDone(int num=1);
    void beginExecute(envelope *e);
    void beginExecute(CmiObjId  *tid);
    void beginExecute(int event,int msgType,int ep,int srcPe,int ml,CmiObjId *idx=NULL);
    void endExecute(void);
    void messageRecv(char *env, int pe);
    void beginIdle(void);
    void endIdle(void);
    void beginPack(void);
    void endPack(void);
    void beginUnpack(void);
    void endUnpack(void);
    void enqueue(envelope *e);
    void dequeue(envelope *e);
    void beginComputation(void);
    void endComputation(void);

    int traceRegisterUserEvent(const char*, int);
    void traceClearEps();
    void traceWriteSts();
    void traceClose();
    void traceBegin();
    void traceEnd();
};


#endif

/*@}*/
