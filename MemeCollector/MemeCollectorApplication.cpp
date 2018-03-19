#include "stdafx.h"
#include "MemeCollectorApplication.hpp"

MemeCollectorApplication::MemeCollectorApplication(int &argc, char **argv)
	: QApplication(argc, argv)
	, oneInstanceMemory("MemeCollector_OneInstanceMemory", this) {
	oneInstanceMemory.create(1);
	qRegisterMetaType<QFileInfo>();

}

MemeCollectorApplication::~MemeCollectorApplication() {}

bool MemeCollectorApplication::is_only_instance() const {
	if (oneInstanceMemory.isAttached())
		return true;

	return oneInstanceMemory.create(1);
}

void MemeCollectorApplication::set_working_directory() const {
	QString appPath = MemeCollectorApplication::applicationDirPath();
	QString currentPath = QDir::currentPath();
	if (appPath != currentPath)
		QDir::setCurrent(appPath);
}
