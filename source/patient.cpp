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

const QString& Patient::patientID()
{
	if ( mPatientID.isEmpty() )
	{
		//generate ID from time
		mPatientID = QDateTime::currentDateTime().toString( "yyyyMMddMMmmsszzz" );
	}
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

QString Patient::genderString() const
{
	switch ( mGender )
	{
	case EGender::EFemale: return "Female"; break;
	case EGender::EMale: return "Male"; break;
	case EGender::EOther: return "Other"; break;
	default: return "N/A"; break;
	}
}

Patient::EGender Patient::genderFromString( const QString& aGender )
{
	if ( aGender == "male" )
	{
		return EGender::EMale;
	}
	if ( aGender == "female" )
	{
		return EGender::EFemale;
	}
	if ( aGender == "other" )
	{
		return EGender::EOther;
	}
}

