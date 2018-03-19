#pragma once

class MemeCollectorApplication : public QApplication {
	Q_OBJECT
public:
	MemeCollectorApplication(int &argc, char **argv);
	virtual ~MemeCollectorApplication();
	bool is_only_instance() const;
	void set_working_directory() const;
private:
	mutable QSharedMemory oneInstanceMemory;
};