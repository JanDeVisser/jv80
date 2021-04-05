#include "../include/backplane.h"
#include "../include/memory.h"

#include "cputhread.h"

#include <QFile>

CPU::CPU(QObject *parent) : QObject(parent), m_running(false), m_status(), m_keys(), m_kbdMutex() {
  m_system = new BackPlane();
  m_system -> defaultSetup();
  m_system -> setOutputStream(m_status);
  m_keyboard = new IOChannel(0x00, "KEY", [this]() {
    byte ret = 0xFF;
    m_kbdMutex.lock();
    if (!m_keys.empty()) {
      ret = m_keys[0];
      m_keys.pop_front();
    }
    m_kbdMutex.unlock();
    return ret;
  });

  m_terminal = new IOChannel(0x01, "OUT", [this](byte out) {
    emit terminalWrite((int) out);
  });

  m_system -> insertIO(m_keyboard);
  m_system -> insertIO(m_terminal);

  m_thread = new Executor(m_system, this);
  connect(m_thread, &QThread::finished, this, &CPU::finished);

  QFile initial("./emu.bin");
  if (initial.exists()) {
    openImage(initial.fileName());
  }
}

void CPU::run(word addr) {
  reset();
  m_thread->startAddress(addr);
  continueExecution();
}

void CPU::continueExecution() {
  start(Controller::Continuous);
}

void CPU::step(word addr) {
  start(Controller::BreakAtInstruction);
}

void CPU::tick(word addr) {
  start(Controller::BreakAtClock);
}

void CPU::interrupt() {
  if (m_running) {
    setRunMode(Controller::BreakAtInstruction);
  }
}

void CPU::reset() {
  if (!m_running) {
    m_system->reset();
  }
}

void CPU::start(Controller::RunMode runMode) {
  if (!m_running && m_system -> bus().halt()) {
    m_kbdMutex.lock();
    m_keys.clear();
    m_kbdMutex.unlock();
    setRunMode(runMode);
    emit executionStart();
    m_thread->start();
    m_running = true;
  }
}

void CPU::finished() {
  m_running = false;
  if (!m_system->bus().halt()) {
    emit executionEnded(QString::fromStdString(m_status.str()));
  } else {
    emit executionInterrupted(QString::fromStdString(m_status.str()));
  }
}

bool CPU::isRunning() const {
  return m_running;
}

bool CPU::isHalted() const {
  return !m_system -> bus().halt();
}

bool CPU::isSuspended() const {
  return !m_system -> bus().sus();
}

void CPU::setRunMode(Controller::RunMode runMode) const {
  m_system -> setRunMode(runMode);
}

void CPU::openImage(QFile &img, word addr, bool writable) {
  img.open(QIODevice::ReadOnly);
  auto bytearr = img.readAll();
  m_system -> loadImage(bytearr.size(), (const byte *) bytearr.data(), addr, writable);
}

void CPU::openImage(QFile &&img, word addr, bool writable) {
  openImage(img, addr, writable);
}

void CPU::openImage(const QString &img, word addr, bool writable) {
  openImage(QFile(img), addr, writable);
}

void CPU::keyPressed(QKeyEvent *key) {
  if (m_running) {
    int k = -1;
    switch (key->key()) {
      case Qt::Key_Enter:
      case Qt::Key_Return:
        k = '\n';
        break;
      case Qt::Key_Backspace:
        k = '\b';
        break;
      case Qt::Key_Tab:
        k = '\t';
        break;
      case Qt::Key_Delete:
        k = 127;
        break;
      case Qt::Key_Up:
        k = 0x01;
        break;
      case Qt::Key_Down:
        k = 0x02;
        break;
      case Qt::Key_Left:
        k = 0x03;
        break;
      case Qt::Key_Right:
        k = 0x04;
        break;
      case Qt::Key_Home:
        k = 0x05;
        break;
      case Qt::Key_End:
        k = 0x06;
        break;
      case Qt::Key_PageUp:
        k = 0x07;
        break;
      case Qt::Key_PageDown:
        k = 0x09;
        break;
      default:
        break;
    }
    if ((key->key() >= Qt::Key_A) && (key->key() <= Qt::Key_Z)) {
      auto ch = (uchar) key->key() + 32;
      if (key->modifiers() & Qt::ShiftModifier) {
        ch -= 32;
      }
      k = ch;
    } else if ((key->key() >= Qt::Key_Space) && (key->key() <= Qt::Key_AsciiTilde)) {
      k = key->key();
    }
    if (k != -1) {
      m_kbdMutex.lock();
      m_keys.emplace_back(k);
      m_kbdMutex.unlock();
      m_system->bus().setNmi();
    }
  }
}

//#include "cputhread.moc"