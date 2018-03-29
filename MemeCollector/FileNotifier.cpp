#include "stdafx.h"
#include "FileNotifier.hpp"

namespace internal {

WorkingThread::WorkingThread(QObject *parent, const QList<QPair<QFileInfo, QFileInfo>> & _fileList)
    : QThread(parent)
    , fileList(&_fileList)
    , stopFlag(false)
    , progressSize(0)
    , totalSize(0)
    , errNum(FileNotifierError::NoError) {
}

void WorkingThread::stop() {
    stopFlag = true;
}

bool WorkingThread::has_error() {
    return static_cast<bool>(errNum);
}

FileNotifierError WorkingThread::error() {
    return errNum;
}

void WorkingThread::run() {
    if (!this->calculate_size_and_validate())
        return;

    for (auto &filePair : *fileList) {
        QFile input(filePair.first.filePath());
        QFile output(filePair.second.filePath());
        if (!input.open(QFile::ReadOnly)) {
            errNum = FileNotifierError::InputOpenFailed;
            return;
        }
        if (!output.open(QFile::WriteOnly)) {
            errNum = FileNotifierError::OutputOpenFailed;
            return;
        }

        QByteArray buffer;
        uint64_t currentSize = filePair.first.size();
        uint64_t currentProgress = 0;
        while (currentProgress < currentSize) {
            emit progress(progressSize, totalSize);
            if (stopFlag) {
                errNum = FileNotifierError::StoppedByUser;
                return;
            }

            uint64_t bytesToProcess = std::min(CHUNK_SIZE, currentSize - currentProgress);
            currentProgress += bytesToProcess;
            progressSize += bytesToProcess;

            buffer = input.read(bytesToProcess);
            if (buffer.size() != bytesToProcess) {
                errNum = FileNotifierError::InputReadFailed;
                return;
            }

            if (output.write(buffer) != bytesToProcess) {
                errNum = FileNotifierError::OutputWriteFailed;
                return;
            }
        }
        emit progress(progressSize, totalSize);
    }
}

bool WorkingThread::calculate_size_and_validate() {
    for (auto &filePair : *fileList) {
        if (!filePair.first.exists()) {
            errNum = FileNotifierError::InputNonexistent;
            return false;
        }
        totalSize += filePair.first.size();
    }
    return true;
}

}































FileNotifier::FileNotifier(QObject *parent)
    : QObject(parent)
    , worker(nullptr)
    , fileList() {}

FileNotifier::~FileNotifier() {
    if (worker != nullptr) {
        worker->stop();
        worker->wait();
    }
}

bool FileNotifier::copy(const QFileInfo & file, const QFileInfo & newFile) {
    if (worker != nullptr)
        return false;
    fileList.push_back(qMakePair(file, newFile));
    worker = new internal::WorkingThread(this, fileList);
    QObject::connect(worker, &internal::WorkingThread::progress, this, &FileNotifier::progress, Qt::QueuedConnection);
    QObject::connect(worker, &QThread::finished, this, &FileNotifier::on_finish);
    worker->start();
    return true;
}

bool FileNotifier::copy(const QList<QPair<QFileInfo, QFileInfo>>& _fileList) {
    if (worker != nullptr)
        return false;
    fileList = _fileList;
    worker = new internal::WorkingThread(this, fileList);
    QObject::connect(worker, &internal::WorkingThread::progress, this, &FileNotifier::progress, Qt::QueuedConnection);
    QObject::connect(worker, &QThread::finished, this, &FileNotifier::on_finish);
    worker->start();
    return true;
}

void FileNotifier::cancel() {
    if (worker != nullptr)
        worker->stop();
}

bool FileNotifier::has_error() {
    if (worker != nullptr)
        return worker->has_error();
    return false;
}

FileNotifierError FileNotifier::error() {
    if (worker != nullptr)
        return worker->error();
    return FileNotifierError::NoError;
}

QString FileNotifier::error_string() {
    switch (this->error()) {
    case FileNotifierError::NoError:
        return "Operation successful";
    case FileNotifierError::InputNonexistent:
        return "File to be copied does not exist";
    case FileNotifierError::InputOpenFailed:
        return "Could not open file for reading";
    case FileNotifierError::InputReadFailed:
        return "Reading data from file failed";
    case FileNotifierError::OutputOpenFailed:
        return "Could not create or open file for writing";
    case FileNotifierError::OutputWriteFailed:
        return "Writing data to file failed";
    case FileNotifierError::StoppedByUser:
        return "Operation was stopped by user";
    default:
        return "";
    }
}


void FileNotifier::on_finish() {
    emit finished(!worker->has_error());
    delete worker;
    worker = nullptr;
}
