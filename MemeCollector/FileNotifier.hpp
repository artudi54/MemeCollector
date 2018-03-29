#pragma once

#include <atomic>

enum class FileNotifierError {
    NoError,
    InputNonexistent,
    InputOpenFailed,
    InputReadFailed,
    OutputOpenFailed,
    OutputWriteFailed,
    StoppedByUser
};


namespace internal {

class WorkingThread : public QThread {
    Q_OBJECT
public:
    explicit WorkingThread(QObject *parent, const QList<QPair<QFileInfo, QFileInfo>> &fileList);
    void stop();
    bool has_error();
    FileNotifierError error();
signals:
    void progress(uint64_t, uint64_t);
protected:
    virtual void run() override;
private:
    bool calculate_size_and_validate();

    const QList<QPair<QFileInfo, QFileInfo>> *fileList;
    std::atomic_bool stopFlag;
    std::uint64_t progressSize;
    std::uint64_t totalSize;
    FileNotifierError errNum;
    static constexpr std::uint64_t CHUNK_SIZE = 1048576;
};

}


class FileNotifier : public QObject {
    Q_OBJECT
public:

    explicit FileNotifier(QObject *parent);
    virtual ~FileNotifier();
    bool copy(const QFileInfo &file, const QFileInfo &newFile);
    bool copy(const QList<QPair<QFileInfo, QFileInfo>> &fileList);
    void cancel();
    bool has_error();
    FileNotifierError error();
    QString error_string();
signals:
    void progress(uint64_t current, uint64_t total);
    void finished(bool success);
private slots:
    void on_finish();




private:
    internal::WorkingThread *worker;
    QList<QPair<QFileInfo, QFileInfo>> fileList;
};
