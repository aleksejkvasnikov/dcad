#pragma once
#include <QString>
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
};
struct CDData {
	double xMin, xMax, yMin, yMax, zMin, zMax;
	double freqValue;
	short unsigned int unitsType;
	short unsigned int freqType;
	short unsigned int allDirections;
	bool hasUnsavedChanges = false;
};