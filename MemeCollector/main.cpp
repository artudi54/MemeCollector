#include "stdafx.h"
#include "MemeCollector.hpp"
#include "MemeCollectorApplication.hpp"

int main(int argc, char *argv[]) {
	MemeCollectorApplication application(argc, argv);
	if (!application.is_only_instance())
		return EXIT_SUCCESS;

	MemeCollectorApplication::setWindowIcon(QIcon(QStringLiteral(":/MemeCollector/ProgramIcon.ico")));
	MemeCollectorApplication::setQuitOnLastWindowClosed(false);
	application.set_working_directory();

	MemeCollector *collector = new MemeCollector;
	collector->setAttribute(Qt::WA_DeleteOnClose);
	return application.exec();
}
