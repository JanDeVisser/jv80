#ifndef EMU_CPUTHREAD_H
#define EMU_CPUTHREAD_H

#include <deque>
#include <mutex>

#include <QKeyEvent>
#include <QThread>

#include "../include/backplane.h"
#include "../include/iochannel.h"

class Executor : public QThread {
Q_OBJECT
  BackPlane *m_system;
  word       m_address;

public:
  explicit Executor(BackPlane *system, QObject *parent = nullptr)
    : QThread(parent), m_system(system), m_address(0xFFFF) { }
  void startAddress(word address) { m_address = address; }

protected:
  void run() override {
    m_system -> run(m_address);
  }
};

class CPU : public QObject {
  Q_OBJECT

public:
  explicit CPU(QObject * = nullptr);
  ~CPU() override = default;
  BackPlane *    getSystem() { return m_system; }
  void           setRunMode(SystemBus::RunMode) const;
  void           openImage(const QString &, word addr = 0, bool writable = true);
  void           openImage(QFile &, word addr = 0, bool writable = true);
  void           openImage(QFile &&, word addr = 0, bool writable = true);

  void           run(word = 0xFFFF);
  void           continueExecution();
  void           step(word = 0xFFFF);
  void           tick(word = 0xFFFF);
  void           reset();
  void           interrupt();
  bool           isRunning() const;
  bool           isHalted() const;
  bool           isSuspended() const;

public slots:
  void           keyPressed(QKeyEvent *);

signals:
  void executionStart();
  void executionEnded(const QString &);
  void executionInterrupted(const QString &);
  void terminalWrite(int);


private:
  Executor          *m_thread;
  BackPlane         *m_system;
  IOChannel         *m_keyboard;
  IOChannel         *m_terminal;
  bool               m_running;
  std::stringstream  m_status;
  std::deque<int>    m_keys;
  std::mutex         m_kbdMutex;

  void start(SystemBus::RunMode);

private slots:
  void finished();
};


#endif //EMU_CPUTHREAD_H
