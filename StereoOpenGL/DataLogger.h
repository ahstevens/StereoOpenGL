#pragma once

#include <map>
#include <sstream>
#include <fstream>
#include <filesystem>

class DataLogger
{
public:

	static DataLogger& getInstance()
	{
		static DataLogger s_instance;
		return s_instance;
	}

	static std::string getTimeString();

	void setLogDirectory(std::string dir);
	bool openLog(std::string logName, bool appendTimestampToLogname = true);
	void closeLog();

	void start();
	void stop();
	bool logging();

	void setID(std::string id);
	// do not include 'id' field, it is added automatically
	void setHeader(std::string header);

	void logMessage(std::string message);

	std::string getTimeSinceLogStartString();

private:
	DataLogger();
	~DataLogger();

	bool m_bLogging;

	std::ofstream m_fsLog;

	std::string m_strID;
	std::string m_strHeader;

	std::experimental::filesystem::v1::path m_LogDirectory;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLogStart;

public:
	// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
	DataLogger(DataLogger const&) = delete;
	void operator=(DataLogger const&) = delete;
};

