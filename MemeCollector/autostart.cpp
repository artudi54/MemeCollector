#include "stdafx.h"
#include "autostart.hpp"

#ifdef Q_OS_X11

QDir autostart::get_autostart_directory() {
	static const QString AUTOSTART = QStringLiteral("autostart");

	const char *configDir = std::getenv("XDG_CONFIG_HOME");
	if (configDir == nullptr)
		configDir = "~/.config";

	QDir autostartDir = QDir::fromNativeSeparators(QString(configDir));
	if (autostartDir.cd(AUTOSTART))
		return autostartDir;
	else {
		autostartDir.mkdir(AUTOSTART);
		if (autostartDir.cd(AUTOSTART))
			return autostartDir;
	}
	return QDir();
}

void autostart::write_desktop_file(QFile & desktopFile, const QString &name, const QString &filePath) {
	if (!desktopFile.open(QIODevice::WriteOnly))
		return;
	QTextStream stream(&desktopFile);
	stream << QStringLiteral(
		"[Desktop]\n"
		"Type=Application\n"
		"Version=1.0\n"
		"Encoding=") << name << '\n' << QStringLiteral(
		"Exec=") << '\"' << filePath << "\" --autostart\n" << QStringLiteral(
		"X-GNOME-Autostart-enabled=true\n"
	);
}

void autostart::add_to_autostart(const QString &name, const QString &filePath) {
	QDir autostartDir = get_autostart_directory();
	if (autostartDir.exists()) {
		QFileInfo fileInfo(filePath);
		QString desktopFileName = fileInfo.completeBaseName() + QStringLiteral(".desktop");
		QString scriptFilePath = fileInfo.dir().filePath(fileInfo.completeBaseName() + QStringLiteral(".sh"));
		QFile file(autostartDir.filePath(desktopFileName));
		write_desktop_file(file, name, scriptFilePath);
	}
}



void autostart::remove_from_autostart(const QString &name) {
	QDir autostartDir = get_autostart_directory();
	if (autostartDir.exists())
		autostartDir.remove(name + QDir::separator() + QStringLiteral(".desktop"));
}









#elif defined(Q_OS_WIN)

static const QString AUTOSTART_KEY = 
	QStringLiteral(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)");

void autostart::add_to_autostart(const QString &name, const QString &filePath) {
	QSettings registry(AUTOSTART_KEY, QSettings::NativeFormat);
	QString value = "\"" + filePath + "\" --autostart";
	value.replace('/', '\\');
	registry.setValue(name, value);
}



void autostart::remove_from_autostart(const QString &name) {
	QSettings registry(AUTOSTART_KEY, QSettings::NativeFormat);
	registry.remove(name);
}











#else
void autostart::add_to_autostart(const QString &name, const QString &filePath) {}
void autostart::remove_from_autostart(const QString &name) {}
#endif
