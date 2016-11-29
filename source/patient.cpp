#include "patient.h"

Patient::Patient()
: mStatus( Patient::EStatus::EActive )
{

}

Patient::~Patient()
{

}

const QString& Patient::firstName() const
{
	return mFirstName;
}

const QString& Patient::middleName() const
{
	return mMiddleName;
}

const QString& Patient::lastName() const
{
	return mLastName;
}

QString Patient::name() const
{
	return QString( "%1 %2 %3" ).arg( mFirstName, mMiddleName, mLastName );
}

const QString& Patient::patientID() const
{
	return mPatientID;
}

const QDate& Patient::birthDate() const
{
	return mBirthDate;
}

Patient::EGender Patient::gender() const
{
	return mGender;
}

Patient::EStatus Patient::status() const
{
	return mStatus;
}
