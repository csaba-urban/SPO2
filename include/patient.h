#pragma once

#include <QString>
#include <QDate>

// Memory representation of the patient's info
class Patient {
public:
	Patient();
	~Patient();

	enum EGender { EMale, EFemale, EOther };
	enum EStatus { EInactive, EActive };

	const QString& firstName() const;
	const QString& middleName() const;
	const QString& lastName() const;
	QString name() const;
	const QString& patientID();
	const QDate& birthDate() const;
	EGender gender() const;
	EStatus status() const;
	QString genderString() const;

	void setFirstName( const QString& aFirstName ) { mFirstName = aFirstName; }
	void setMiddleName( const QString& aMiddleName ) { mMiddleName = aMiddleName; }
	void setLastName( const QString& aLastName ) { mLastName = aLastName; }
	void setPatientID( const QString& aPatientID ) { mPatientID = aPatientID; }
	void setBirthDate( const QDate& aBirthDate ) { mBirthDate = aBirthDate; }
	void setGenderName( EGender aGender ) { mGender = aGender; }
	void setGenderName( const QString& aGender ) { mGender = genderFromString( aGender ); }
	void setStatus( EStatus aStatus ) { mStatus = aStatus; }

private:
	QString mFirstName;
	QString mMiddleName;
	QString mLastName;
	QString mPatientID;
	QDate mBirthDate;
	EGender mGender;
	EStatus mStatus;

	static EGender genderFromString( const QString& aGender );
};
