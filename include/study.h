#pragma once

#include <QString>
#include <QDateTime>

// Memory representation of a Study
class Study
{
public:
	Study();
	~Study();

	enum ESmokingStatus { ESmoking, ENoSmoking, EFormerSmoker };
	enum EStatus { EInactive, EActive };

	int studyID() const;
	const QString& patientID() const;
	const QDateTime& studyDateTime() const;
	int patientWeight_kg() const;
	int patientHeight_cm() const;
	ESmokingStatus smokingStatus() const;
	const QString& patientHistory() const;
	const QString& diagnose() const;
	const QByteArray& SPO2Record() const;
	EStatus status() const;

	void setStudyID( int aStudyID ) { mStudyID = aStudyID; }
	void setPatientID( const QString& aPatientID ) { mPatientID = aPatientID; }
	void setStudyDateTime( const QDateTime& aStudyDateTime ) { mStudyDatetime = aStudyDateTime; }
	void setPatientWeight( int aPatientWeight_kg ) { mPatientWeight_kb = aPatientWeight_kg; }
	void setPatientHeight( int aPatientHeight_cm ) { mPatientHeight_cm = aPatientHeight_cm; }
	void setPatientSmokingStatus( ESmokingStatus aSmokingStatus ) { mSmokingStatus = aSmokingStatus; }
	void setPatientSmokingStatusAsStr( const QString& aSmokingStatusStr );
	void setPatientHistory( const QString& aPatientHistory ) { mPatientHistory = aPatientHistory; }
	void setDiagnose( const QString& aDiagnose ) { mDiagnose = aDiagnose; }
	void setSPO2Record( const QByteArray& aSPO2Record ) { mSPO2Record = aSPO2Record; }
	void setStatus( EStatus aStatus ) { mStatus = aStatus; }

private:
	int mStudyID;
	QString mPatientID;
	QDateTime mStudyDatetime;
	int mPatientWeight_kb;
	int mPatientHeight_cm;
	ESmokingStatus mSmokingStatus;
	QString mPatientHistory;
	QString mDiagnose;
	QByteArray mSPO2Record;
	EStatus mStatus;
};

