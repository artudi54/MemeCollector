#pragma once

namespace autostart {
#ifdef Q_OS_LINUX
QDir get_autostart_directory();
void write_desktop_file(QFile &desktopFile, const QString &name, const QString &filePath);
#endif
void add_to_autostart(const QString &name, const QString &filePath);
void remove_from_autostart(const QString &name);
}
