#pragma once
#include <QString>
#include <vector>
using namespace std;
struct MonitorData {
	QString name;
	short unsigned int id;
	short unsigned int type, nameBox, placeBox;
	double freqValue;
	double farFieldRadius;
	double xMin, xMax, yMin, yMax, zMin, zMax;
	bool hasUnsavedChanges = false;
};
struct ProjectData {
	QString name;
	QString directory;
	QString creationDate;
	QString lastModifiedDate;
	QString author;
	QString geometryUnits;
	QString frequencyUnits;
	QString timeUnits;
	QString solverType;
	QString freqMin;
	QString freqMax;
	QString freqStep;
	QString pointsNumber;
	bool useFreqStep;
	bool usePointsNumber;
	bool hasUnsavedChanges = false;
	bool hasModel = false;
	std::vector<MonitorData> monitors;
};
struct CDData {	
	double xMin, xMax, yMin, yMax, zMin, zMax;
	double freqValue;
	short unsigned int unitsType;
	short unsigned int freqType;
	short unsigned int allDirections;
	bool hasUnsavedChanges = false;
};
