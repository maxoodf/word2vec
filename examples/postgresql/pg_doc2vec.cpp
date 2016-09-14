#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>

#include <string>
#include <thread>
#include <mutex>
#include <random>
#include <limits>

#include <word2vec.h>
#include <doc2vec.h>
#include <wrErrors.h>

extern "C" {
#include <postgres.h>
#include <miscadmin.h>
#include <postmaster/bgworker.h>
#include <storage/ipc.h>
#include <storage/latch.h>
#include <fmgr.h>
#include <utils/builtins.h>
#include <utils/array.h>
#include <catalog/pg_type.h>
    
    PG_MODULE_MAGIC;
}

namespace pg_d2v {
#ifndef DOC_LEN_MAX
#error "DOC_LEN_MAX must be defined"
#else
    static const uint32_t docLengthMax = DOC_LEN_MAX;
#endif
    
#ifndef NEAREST_RESULT_MAX
#error "NEAREST_RESULT_MAX must be defined"
#else
    static const uint32_t nearestResultMax = NEAREST_RESULT_MAX;
#endif
    
    static const useconds_t queueWaitTimeoutMax = 1000L; // wait for a record, milliseconds
    
    class inOutQueue_t {
    public:
        static const uint16_t queueRecordsMax;
        
        struct insertQueueRecord_t {
            int64_t m_id;
            char m_text[docLengthMax];
        };
        
        struct nearestInQueueRecord_t {
            int64_t m_id;
            float m_distance;
            char m_text[docLengthMax];
        };
        
        struct nearestOutQueueRecord_t {
            int64_t m_idWhat;
            int64_t m_idWith[nearestResultMax];
        };
        
    private:
        static const char *insertQueueSemName;
        static const char *insertQueueShmName;
        
        static const char *nearestInQueueSemName;
        static const char *nearestInQueueShmName;
        
        static const char *nearestOutQueueSemName;
        static const char *nearestOutQueueShmName;
        
        static const char *delimChars;
        
        sem_t *m_insertQueueSem;
        sem_t *m_nearestInQueueSem;
        sem_t *m_nearestOutQueueSem;
        
        int m_insertQueueShm;
        int m_nearestInQueueShm;
        int m_nearestOutQueueShm;
        
        insertQueueRecord_t *m_insertQueue;
        nearestInQueueRecord_t *m_nearestInQueue;
        nearestOutQueueRecord_t *m_nearestOutQueue;
        
        bool m_OK;
        int m_errCode;
        
    public:
        static bool init() {
            sem_t *insertQueueSem = sem_open(insertQueueSemName, O_CREAT, 0600, 0);
            if (insertQueueSem == SEM_FAILED) {
                elog(LOG, "DOC2VEC: inOutQueue(1) init error = %d, %s", errno, strerror(errno));
                return false;
            }
            
            sem_t *nearestInQueueSem = sem_open(nearestInQueueSemName, O_CREAT, 0600, 0);
            if (nearestInQueueSem == SEM_FAILED) {
                elog(LOG, "DOC2VEC: inOutQueue(2) init error = %d, %s", errno, strerror(errno));
                return false;
            }
            
            sem_t *nearestOutQueueSem = sem_open(nearestOutQueueSemName, O_CREAT, 0600, 0);
            if (nearestOutQueueSem == SEM_FAILED) {
                elog(LOG, "DOC2VEC: inOutQueue(3) init error = %d, %s", errno, strerror(errno));
                return false;
            }
            
            int insertQueueShm = shm_open(insertQueueShmName, O_CREAT | O_RDWR, 0600);
            if (insertQueueShm == -1) {
                elog(LOG, "DOC2VEC: inOutQueue(4) init error = %d, %s", errno, strerror(errno));
                return false;
            }
            
            int nearestInQueueShm = shm_open(nearestInQueueShmName, (O_CREAT | O_RDWR), 0600);
            if (nearestInQueueShm == -1) {
                elog(LOG, "DOC2VEC: inOutQueue(5) init error = %d, %s", errno, strerror(errno));
                return false;
            }
            
            int nearestOutQueueShm = shm_open(nearestOutQueueShmName, (O_CREAT | O_RDWR), 0600);
            if (nearestOutQueueShm == -1) {
                elog(LOG, "DOC2VEC: inOutQueue(6) init error = %d, %s", errno, strerror(errno));
                return false;
            }
            
            if (ftruncate(insertQueueShm, (off_t) sizeof(insertQueueRecord_t) * queueRecordsMax) != 0) {
                elog(LOG, "DOC2VEC: inOutQueue(7) init error = %d, %s", errno, strerror(errno));
            }
            
            if (ftruncate(nearestInQueueShm, (off_t) sizeof(nearestInQueueRecord_t) * queueRecordsMax) != 0) {
                elog(LOG, "DOC2VEC: inOutQueue(8) init error = %d, %s", errno, strerror(errno));
            }
            
            if (ftruncate(nearestOutQueueShm, (off_t) sizeof(nearestOutQueueRecord_t) * queueRecordsMax) != 0) {
                elog(LOG, "DOC2VEC: inOutQueue(9) init error = %d, %s", errno, strerror(errno));
            }
            
            insertQueueRecord_t *insertQueue =
            (insertQueueRecord_t *) mmap((void *) 0, sizeof(insertQueueRecord_t) * queueRecordsMax,
                                         PROT_WRITE, MAP_SHARED, insertQueueShm, (off_t) 0);
            if (insertQueue == MAP_FAILED) {
                elog(LOG, "DOC2VEC: inOutQueue(10) init error = %d, %s", errno, strerror(errno));
                return false;
            }
            
            nearestInQueueRecord_t *nearestInQueue =
            (nearestInQueueRecord_t *) mmap((void *) 0, sizeof(nearestInQueueRecord_t) * queueRecordsMax,
                                            PROT_WRITE, MAP_SHARED, nearestInQueueShm, (off_t) 0);
            if (nearestInQueue == MAP_FAILED) {
                elog(LOG, "DOC2VEC: inOutQueue(11) init error = %d, %s", errno, strerror(errno));
                return false;
            }
            
            nearestOutQueueRecord_t *nearestOutQueue =
            (nearestOutQueueRecord_t *) mmap((void *) 0, sizeof(nearestOutQueueRecord_t) * queueRecordsMax,
                                             PROT_WRITE, MAP_SHARED, nearestOutQueueShm, (off_t) 0);
            if (nearestOutQueue == MAP_FAILED) {
                elog(LOG, "DOC2VEC: inOutQueue(12) init error = %d, %s", errno, strerror(errno));
                return false;
            }
            
            bzero((void *) insertQueue, sizeof(insertQueueRecord_t) * queueRecordsMax);
            bzero((void *) nearestInQueue, sizeof(nearestInQueueRecord_t) * queueRecordsMax);
            bzero((void *) nearestOutQueue, sizeof(nearestOutQueueRecord_t) * queueRecordsMax);
            
            munmap((void *) insertQueue, sizeof(insertQueueRecord_t) * queueRecordsMax);
            close(insertQueueShm);
            sem_post(insertQueueSem);
            
            munmap((void *) nearestInQueue, sizeof(nearestInQueueRecord_t) * queueRecordsMax);
            close(nearestInQueueShm);
            sem_post(nearestInQueueSem);
            
            munmap((void *) nearestOutQueue, sizeof(nearestOutQueueRecord_t) * queueRecordsMax);
            close(nearestOutQueueShm);
            sem_post(nearestOutQueueSem);
            
            return true;
        }
        
        static void release() {
            sem_unlink(insertQueueSemName);
            shm_unlink(insertQueueShmName);
            
            sem_unlink(nearestInQueueSemName);
            shm_unlink(nearestInQueueShmName);
            
            sem_unlink(nearestOutQueueSemName);
            shm_unlink(nearestOutQueueShmName);
        }
        
        inOutQueue_t(): m_insertQueueSem(SEM_FAILED), m_nearestInQueueSem(SEM_FAILED), m_nearestOutQueueSem(SEM_FAILED),
        m_insertQueueShm(-1), m_nearestInQueueShm(-1), m_nearestOutQueueShm(-1), m_insertQueue(nullptr),
        m_nearestInQueue(nullptr), m_nearestOutQueue(nullptr), m_OK(false), m_errCode(0) {
            m_insertQueueSem = sem_open(insertQueueSemName, 0);
            if (m_insertQueueSem == SEM_FAILED) {
                m_errCode = errno;
                return;
            }
            
            m_nearestInQueueSem = sem_open(nearestInQueueSemName, 0);
            if (m_nearestInQueueSem == SEM_FAILED) {
                m_errCode = errno;
                return;
            }
            
            m_nearestOutQueueSem = sem_open(nearestOutQueueSemName, 0);
            if (m_nearestOutQueueSem == SEM_FAILED) {
                m_errCode = errno;
                return;
            }
            
            m_insertQueueShm = shm_open(insertQueueShmName, O_RDWR, 0600);
            if (m_insertQueueShm == -1) {
                m_errCode = errno;
                return;
            }
            
            m_nearestInQueueShm = shm_open(nearestInQueueShmName, O_RDWR, 0600);
            if (m_nearestInQueueShm == -1) {
                m_errCode = errno;
                return;
            }
            
            m_nearestOutQueueShm = shm_open(nearestOutQueueShmName, O_RDWR, 0600);
            if (m_nearestOutQueueShm == -1) {
                m_errCode = errno;
                return;
            }
            
            
            m_insertQueue =
            (insertQueueRecord_t *) mmap((void *) 0, sizeof(insertQueueRecord_t) * queueRecordsMax,
                                         PROT_WRITE, MAP_SHARED, m_insertQueueShm, (off_t) 0);
            if (m_insertQueue == MAP_FAILED) {
                m_errCode = errno;
                return;
            }
            
            m_nearestInQueue =
            (nearestInQueueRecord_t *) mmap((void *) 0, sizeof(nearestInQueueRecord_t) * queueRecordsMax,
                                            PROT_WRITE, MAP_SHARED, m_nearestInQueueShm, (off_t) 0);
            if (m_nearestInQueue == MAP_FAILED) {
                m_errCode = errno;
                return;
            }
            
            m_nearestOutQueue =
            (nearestOutQueueRecord_t *) mmap((void *) 0, sizeof(nearestOutQueueRecord_t) * queueRecordsMax,
                                             PROT_WRITE, MAP_SHARED, m_nearestOutQueueShm, (off_t) 0);
            if (m_nearestOutQueue == MAP_FAILED) {
                m_errCode = errno;
                return;
            }
            
            m_OK = true;
        }
        
        ~inOutQueue_t() {
            if (m_insertQueue != nullptr) {
                munmap(m_insertQueue, sizeof(insertQueueRecord_t) * queueRecordsMax);
            }
            if (m_insertQueueShm > 0) {
                close(m_insertQueueShm);
            }
            if (m_insertQueueSem != SEM_FAILED) {
                sem_close(m_insertQueueSem);
            }
            
            if (m_nearestInQueue != nullptr) {
                munmap(m_nearestInQueue, sizeof(nearestInQueueRecord_t) * queueRecordsMax);
            }
            if (m_nearestInQueueShm > 0) {
                close(m_nearestInQueueShm);
            }
            if (m_nearestInQueueSem != SEM_FAILED) {
                sem_close(m_nearestInQueueSem);
            }
            
            if (m_nearestInQueue != nullptr) {
                munmap(m_nearestOutQueue, sizeof(nearestOutQueueRecord_t) * queueRecordsMax);
            }
            if (m_nearestOutQueueShm > 0) {
                close(m_nearestOutQueueShm);
            }
            if (m_nearestOutQueueSem != SEM_FAILED) {
                sem_close(m_nearestOutQueueSem);
            }
        }
        
        bool isOK() const {
            return m_OK;
        }
        
        int errCode() const {
            return m_errCode;
        }
        
        bool setInsertQueueRecord(int64_t _id, const std::string &_text) {
            bool ret = false;
            if (sem_trywait(m_insertQueueSem) == 0) {
                for (auto i = 0; i < queueRecordsMax; ++i) {
                    if (m_insertQueue[i].m_id == 0) {
                        m_insertQueue[i].m_id = _id;
                        
                        std::string text = _text;
                        if (text.length() > docLengthMax - 1) {
                            std::size_t pos = text.find_last_of(delimChars, docLengthMax - 2, 1);
                            if (pos == std::string::npos) {
                                pos = docLengthMax - 2;
                            }
                            text = text.substr(0, pos + 1);
                        }
                        strncpy(m_insertQueue[i].m_text, text.c_str(), docLengthMax - 1);
                        
                        ret = true;
                        break;
                    }
                }
                sem_post(m_insertQueueSem);
            }
            
            return ret;
        }
        
        int64_t getInsertQueueRecord(std::string &_text) {
            int64_t ret = 0;
            if (sem_trywait(m_insertQueueSem) == 0) {
                for (auto i = 0; i < queueRecordsMax; ++i) {
                    if (m_insertQueue[i].m_id != 0) {
                        ret = m_insertQueue[i].m_id;
                        m_insertQueue[i].m_id = 0;
                        _text = m_insertQueue[i].m_text;
                        bzero((void *) m_insertQueue[i].m_text, docLengthMax);
                        break;
                    }
                }
                sem_post(m_insertQueueSem);
            }
            
            return ret;
        }
        
        bool setNearestInQueueRecord(int64_t _id, float _distance, const char *_text = nullptr) {
            bool ret = false;
            if (sem_trywait(m_nearestInQueueSem) == 0) {
                for (auto i = 0; i < queueRecordsMax; ++i) {
                    if (m_nearestInQueue[i].m_id == 0) {
                        m_nearestInQueue[i].m_id = _id;
                        m_nearestInQueue[i].m_distance = _distance;
                        
                        if (_text != nullptr) {
                            std::string text = _text;
                            if (text.length() > docLengthMax - 1) {
                                std::size_t pos = text.find_last_of(delimChars, docLengthMax - 2, 1);
                                if (pos == std::string::npos) {
                                    pos = docLengthMax - 2;
                                }
                                text = text.substr(0, pos + 1);
                            }
                            strncpy(m_nearestInQueue[i].m_text, text.c_str(), docLengthMax - 1);
                        }
                        
                        ret = true;
                        break;
                    }
                }
                sem_post(m_nearestInQueueSem);
            }
            
            return ret;
        }
        
        int64_t getNearestInQueueRecord(int64_t &_id, float &_distance, std::string &_text) {
            int64_t ret = 0;
            if (sem_trywait(m_nearestInQueueSem) == 0) {
                for (auto i = 0; i < queueRecordsMax; ++i) {
                    if (m_nearestInQueue[i].m_id != 0) {
                        ret = _id = m_nearestInQueue[i].m_id;
                        _distance = m_nearestInQueue[i].m_distance;
                        _text = m_nearestInQueue[i].m_text;
                        m_nearestInQueue[i].m_id = 0;
                        bzero((void *) m_nearestInQueue[i].m_text, docLengthMax);
                        break;
                    }
                }
                sem_post(m_nearestInQueueSem);
            }
            
            return ret;
        }

        bool setNearestOutQueueRecord(int64_t _id, const std::vector<int64_t> &_nearestRecords) {
            bool ret = false;
            if (sem_trywait(m_nearestOutQueueSem) == 0) {
                for (auto i = 0; i < queueRecordsMax; ++i) {
                    if (m_nearestOutQueue[i].m_idWhat == 0) {
                        m_nearestOutQueue[i].m_idWhat = _id;
                        auto lim = (_nearestRecords.size() < nearestResultMax)?_nearestRecords.size():nearestResultMax;
                        for (auto j= 0; j < lim; ++j) {
                            m_nearestOutQueue[i].m_idWith[j] = _nearestRecords[j];
                        }
                        ret = true;
                        break;
                    }
                }
                sem_post(m_nearestOutQueueSem);
            }
            
            return ret;
        }
        
        bool getNearestOutQueueRecord(int64_t _id, std::vector<int64_t> &_nearestRecords) {
            bool ret = false;
            if (sem_trywait(m_nearestOutQueueSem) == 0) {
                for (auto i = 0; i < queueRecordsMax; ++i) {
                    if (m_nearestOutQueue[i].m_idWhat == _id) {
                        for (auto j= 0; j < nearestResultMax; ++j) {
                            if (m_nearestOutQueue[i].m_idWith[j] != 0) {
                                _nearestRecords.push_back(m_nearestOutQueue[i].m_idWith[j]);
                                m_nearestOutQueue[i].m_idWith[j] = 0;
                            }
                        }
                        m_nearestOutQueue[i].m_idWhat = 0;
                        ret = true;
                        break;
                    }
                }
                sem_post(m_nearestOutQueueSem);
            }
            
            return ret;
        }
        
    private:
        
    };
    
    const uint16_t inOutQueue_t::queueRecordsMax = 6;
    
    const char *inOutQueue_t::insertQueueSemName = "/pg_doc2vecInsertQueueSem";
    const char *inOutQueue_t::insertQueueShmName = "/pg_doc2vecInsertQueueShm";
    
    const char *inOutQueue_t::nearestInQueueSemName = "/pg_doc2vecNearestInQueueSem";
    const char *inOutQueue_t::nearestInQueueShmName = "/pg_doc2vecNearestInQueueShm";
    
    const char *inOutQueue_t::nearestOutQueueSemName = "/pg_doc2vecNearestOutQueueSem";
    const char *inOutQueue_t::nearestOutQueueShmName = "/pg_doc2vecNearestOutQueueShm";
    
    const char *inOutQueue_t::delimChars = " .,!@#$%^&*()_-+={[}];:'\"~`<>?/\n\t";

    
#ifndef SHARE_FOLDER
#error "SHARE_FOLDER must be defined"
#else
#define xstr(s) str(s)
#define str(s) #s
#define SHARE_FOLDER_STR xstr(SHARE_FOLDER)
#endif

    class d2vProcessor_t {
    private:
        mutable std::atomic_flag m_ExitLocker = ATOMIC_FLAG_INIT;
        bool m_terminated;
        const word2vec_t m_word2vec;
        doc2vec_t m_doc2vec;
//        mutable std::shared_mutex m_d2vMutex;
        
    public:
        d2vProcessor_t(): m_terminated(false),
                        m_word2vec(std::string(SHARE_FOLDER_STR) + "/model.w2v"),
                        m_doc2vec(m_word2vec, std::string(SHARE_FOLDER_STR) + "/model.d2v", true) {
//                        m_d2vMutex() {
        }
        
        ~d2vProcessor_t() {
        }
        
        void terminate() {
            while (m_ExitLocker.test_and_set(std::memory_order_acquire));
            m_terminated = true;
            m_ExitLocker.clear(std::memory_order_release);
        }
        
        void insertThread() {
            inOutQueue_t inOutQueue;
            if (!inOutQueue.isOK()) {
                return;
            }
            
            useconds_t currSleepTime = 1L;
            while (!terminated()) {
                std::string text;
                int64_t id = inOutQueue.getInsertQueueRecord(text);
                if (id > 0) {
//                    std::unique_lock<std::shared_mutex> lock(m_d2vMutex);
                    m_doc2vec.insert(id, text);
                    currSleepTime = 1L;
                } else {
                    currSleepTime *= 2L;
                    if (currSleepTime > queueWaitTimeoutMax) {
                        currSleepTime = queueWaitTimeoutMax;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(currSleepTime));
                }
            }
        }
        
        void nearestIDsThread() {
            inOutQueue_t inOutQueue;
            if (!inOutQueue.isOK()) {
                return;
            }
            
            useconds_t currSleepTime = 1L;
            while (!terminated()) {
                std::vector<int64_t> nearest;
                int64_t id = 0;
                float distance = 0.0;
                std::string text;
                if (inOutQueue.getNearestInQueueRecord(id, distance, text) > 0) {
                    {
//                        std::shared_lock<std::shared_mutex> lock(m_d2vMutex);
                        if (text.length() > 0) {
                            doc2vec_t::docVector_t docVector;
                            if (m_doc2vec.docVector(text, docVector)) {
                                m_doc2vec.nearest(docVector, nearest, distance, pg_d2v::nearestResultMax);
                            }
                        } else {
                            m_doc2vec.nearest(id, nearest, distance, pg_d2v::nearestResultMax);
                        }
                    }
                    while(!inOutQueue.setNearestOutQueueRecord(id, nearest)) {
                        usleep(1L);
                    }
                    currSleepTime = 1L;
                } else {
                    currSleepTime *= 2;
                    if (currSleepTime > queueWaitTimeoutMax) {
                        currSleepTime = queueWaitTimeoutMax;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(currSleepTime));
                }
            }
        }
        
    private:
        bool terminated() const {
            bool ret = false;
            while (m_ExitLocker.test_and_set(std::memory_order_acquire));
            ret = m_terminated;
            m_ExitLocker.clear(std::memory_order_release);
            
            return ret;
        }
    };
}

extern "C" {
    static volatile sig_atomic_t doc2vecTerminated = false;
    
    static void doc2vecSigterm(SIGNAL_ARGS) {
        int save_errno = errno;
        doc2vecTerminated = true;
        SetLatch(MyLatch);
        errno = save_errno;
    }
    
    void mainDoc2VecProc(Datum) {
        if (!pg_d2v::inOutQueue_t::init()) {
            elog(ERROR, "DOC2VEC: queue initialisation failed");
            proc_exit(1);
        }
        

        try {
            pg_d2v::d2vProcessor_t d2vProcessor;

            std::vector<std::thread> insertThreads;
            for (auto i = 0; i < 2; ++i) {
                insertThreads.push_back(std::thread(&pg_d2v::d2vProcessor_t::insertThread, &d2vProcessor));
            }

            std::vector<std::thread> nearestIDsThreads;
            for (auto i = 0; i < 6; ++i) {
                nearestIDsThreads.push_back(std::thread(&pg_d2v::d2vProcessor_t::nearestIDsThread, &d2vProcessor));
            }
            
            pqsignal(SIGTERM, doc2vecSigterm);
            BackgroundWorkerUnblockSignals();
            
            elog(LOG, "DOC2VEC: initialized");
            
            while (!doc2vecTerminated) {
                int rc = WaitLatch(MyLatch, WL_LATCH_SET | WL_TIMEOUT | WL_POSTMASTER_DEATH, 1000L);
                ResetLatch(MyLatch);
                if (rc & WL_POSTMASTER_DEATH) {
                    break;
                }
            }
            d2vProcessor.terminate();
            for (auto &i:insertThreads) {
                i.join();
            }
            for (auto &i:nearestIDsThreads) {
                i.join();
            }
        } catch (const errorWR_t &_err) {
            elog(ERROR, "DOC2VEC: %s", _err.err().c_str());
        } catch (const std::exception &_err) {
            elog(ERROR, "DOC2VEC: %s", _err.what());
        } catch (...) {
            elog(ERROR, "DOC2VEC: unknown error");
        }

        elog(LOG, "DOC2VEC: shutting down");
        
        pg_d2v::inOutQueue_t::release();
        proc_exit(0);
    }
    
    void _PG_init(void) {
        BackgroundWorker worker;
        
        sprintf(worker.bgw_name, "doc2vec process");
        worker.bgw_flags = BGWORKER_SHMEM_ACCESS;
        worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
        worker.bgw_restart_time = BGW_NEVER_RESTART;
        worker.bgw_main = mainDoc2VecProc;
        worker.bgw_notify_pid = 0;
        
        RegisterBackgroundWorker(&worker);
    }
    
    PG_FUNCTION_INFO_V1(d2v_insert);
    Datum d2v_insert(PG_FUNCTION_ARGS) {
        if(PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
            PG_RETURN_BOOL(false);
        }
        
        try {
            int64_t _id = PG_GETARG_INT64(0);
            
            text *_line = PG_GETARG_TEXT_P(1);
            std::string line(VARDATA(_line), VARSIZE(_line) - VARHDRSZ);
            
            if (line.length() > 0) {
                pg_d2v::inOutQueue_t inOutQueue;
                if (!inOutQueue.isOK()) {
                    PG_RETURN_BOOL(false);
                }

                while (!inOutQueue.setInsertQueueRecord(_id, line)) {
                    usleep(1L);
                }

                PG_RETURN_BOOL(true);
            }
        } catch (const std::exception &_e) {
            elog(LOG, "DOC2VEC: d2v_insert critical error: %s", _e.what());
        } catch (...) {
            elog(LOG, "DOC2VEC: d2v_insert unknown critical error");
        }
        
        PG_RETURN_BOOL(false);
    }

    PG_FUNCTION_INFO_V1(d2v_nearest_by_text);
    Datum d2v_nearest_by_text(PG_FUNCTION_ARGS) {
        if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
            PG_RETURN_NULL();
        }
        
        try {
            text *_line = PG_GETARG_TEXT_P(0);
            std::string line(VARDATA(_line), VARSIZE(_line) - VARHDRSZ);

            float _distance = PG_GETARG_FLOAT4(1);
            
            std::vector<int64_t> nearestRecords;
            if (line.length() > 0) {
                pg_d2v::inOutQueue_t inOutQueue;
                if (!inOutQueue.isOK()) {
                    PG_RETURN_INT16(0);
                }
                
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1, std::numeric_limits<int>::max());
                int64_t id = dis(gen);
                
                while (!inOutQueue.setNearestInQueueRecord(id, _distance, line.c_str())) {
                    usleep(1L);
                }
                
                while (!inOutQueue.getNearestOutQueueRecord(id, nearestRecords)) {
                    usleep(1L);
                }
            }
            
            if (nearestRecords.size() > 0) {
                Datum elements[nearestRecords.size()];
                int idx = 0;
                for (auto i:nearestRecords) {
                    elements[idx] = Int64GetDatum(i);
                    ++idx;
                }
                
                ArrayType *array = construct_array(elements, nearestRecords.size(), INT8OID, 8, true, 'd');
                PG_RETURN_ARRAYTYPE_P(array);
            }
        } catch (const std::exception &_e) {
            elog(LOG, "DOC2VEC: d2v_nearest critical error: %s", _e.what());
        } catch (...) {
            elog(LOG, "DOC2VEC: d2v_nearest unknown critical error");
        }
        
        PG_RETURN_NULL();
    }

    PG_FUNCTION_INFO_V1(d2v_nearest_by_id);
    Datum d2v_nearest_by_id(PG_FUNCTION_ARGS) {
        if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
            PG_RETURN_NULL();
        }
        
        try {
            int64_t _id = PG_GETARG_INT64(0);
            float _distance = PG_GETARG_FLOAT4(1);
            
            std::vector<int64_t> nearestRecords;
            if (_id > 0) {
                pg_d2v::inOutQueue_t inOutQueue;
                if (!inOutQueue.isOK()) {
                    PG_RETURN_INT16(0);
                }
                
                while (!inOutQueue.setNearestInQueueRecord(_id, _distance)) {
                    usleep(1L);
                }
                
                while (!inOutQueue.getNearestOutQueueRecord(_id, nearestRecords)) {
                    usleep(1L);
                }
            }
            
            if (nearestRecords.size() > 0) {
                Datum elements[nearestRecords.size()];
                int idx = 0;
                for (auto i:nearestRecords) {
                    elements[idx] = Int64GetDatum(i);
                    ++idx;
                }
                
                ArrayType *array = construct_array(elements, nearestRecords.size(), INT8OID, 8, true, 'd');
                PG_RETURN_ARRAYTYPE_P(array);
            }
        } catch (const std::exception &_e) {
            elog(LOG, "DOC2VEC: d2v_nearest critical error: %s", _e.what());
        } catch (...) {
            elog(LOG, "DOC2VEC: d2v_nearest unknown critical error");
        }
        
        PG_RETURN_NULL();
    }
}
