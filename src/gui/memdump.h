#ifndef EMU_MEMDUMP_H
#define EMU_MEMDUMP_H

#include <QAbstractListModel>
#include <QListView>
#include <QComboBox>
#include <QWindow>

#include "backplane.h"
#include "memory.h"

class MemModel : public QAbstractListModel {
    Q_OBJECT

public:
  explicit     MemModel(const Memory *, word, QObject *parent = nullptr);

  int          rowCount(const QModelIndex &parent) const override;
  QVariant     data(const QModelIndex &index, int role) const override;
  QModelIndex  indexOf(word addr=0xFFFF) const;
  word         currentAddress() const { return m_address; }
  MemoryBank & getBank() { return m_bank; }

signals:
  void         numberPopulated(int number);

public slots:

private:
  word           m_address;
  MemoryBank     m_bank;

  QVariant       getRow(int) const;
};


class MemDump : public QWidget {
  Q_OBJECT

public:
  explicit       MemDump(BackPlane *, QWidget * = nullptr);
                 ~MemDump() override = default;
  BackPlane *    getSystem() const  { return m_system; }
  void           reload();
  void           focusOnAddress(word addr=0xFFFF);
  MemoryBank &   currentBank() { return m_model->getBank(); }

public slots:
  void           focus();
  void           selectBank(int ix);

signals:

protected:

private:
  BackPlane *m_system;
  Memory    *m_memory;
  MemModel  *m_model = nullptr;
  QListView *m_view;
  QComboBox *m_banks;

  void       createModel(word);
};


#endif //EMU_CPUTHREAD_H

