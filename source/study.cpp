#include "study.h"

Study::Study()
: mStatus( Study::EStatus::EActive )
{

}

Study::~Study()
{

}

int Study::studyID() const
{
	return mStudyID;
}

const QString& Study::patientID() const
{
	return mPatientID;
}

const QDateTime& Study::studyDateTime() const
{
	return mStudyDatetime;
}

int Study::patientWeight_kg() const
{
	return mPatientWeight_kb;
}

int Study::patientHeight_cm() const
{
	return mPatientHeight_cm;
}

Study::ESmokingStatus Study::smokingStatus() const
{
	return mSmokingStatus;
}

const QString& Study::patientHistory() const
{
	return mPatientHistory;
}

const QString& Study::diagnose() const
{
	return mDiagnose;
}

const QByteArray& Study::SPO2Record() const
{
	return mSPO2Record;
}

Study::EStatus Study::status() const
{
	return mStatus;
}

void Study::setPatientSmokingStatusAsStr( const QString& aSmokingStatusStr )
{
	if ( aSmokingStatusStr.toLower() == "no" )
		mSmokingStatus = ESmokingStatus::ENoSmoking;
	else if ( aSmokingStatusStr.toLower() == "yes" )
		mSmokingStatus = ESmokingStatus::ESmoking;
	else if ( aSmokingStatusStr.toLower() == "former" )
		mSmokingStatus = ESmokingStatus::EFormerSmoker;
}
